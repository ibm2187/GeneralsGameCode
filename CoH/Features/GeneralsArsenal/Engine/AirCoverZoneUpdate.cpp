// CoH GeneralsArsenal: see AirCoverZoneUpdate.h.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/INI.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"
#include "GameLogic/AI.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Locomotor.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Module/BodyModule.h"
#include "GameLogic/Module/PhysicsUpdate.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/TerrainLogic.h"
#include "GameLogic/Weapon.h"

#include "Kernel/Log.h"

#include "Features/GeneralsArsenal/Engine/AirCoverZoneUpdate.h"
#include "Features/GeneralsArsenal/Engine/Internal.h"

namespace
{

const UnsignedInt THINK_FRAMES = LOGICFRAMES_PER_SECOND / 2;
const Real FORMATION_SPACING = 60.0f;
const Real EXIT_DISTANCE = 80.0f;		///< a leaving jet this close to the map edge is gone
const UnsignedInt LEAVE_FRAMES = 20 * LOGICFRAMES_PER_SECOND;	///< a jet still flying out after this is gone too
const Real KEEP_TARGET_SLACK = 1.5f;	///< a target is chased this far past the zone's radius
const Int ORBIT_POINTS = 8;

Real distance2D( const Coord3D &a, const Coord3D &b )
{
	const Real dx = a.x - b.x;
	const Real dy = a.y - b.y;
	return sqrtf( dx * dx + dy * dy );
}

} // namespace

AirCoverZoneUpdateModuleData::AirCoverZoneUpdateModuleData() :
	m_count( 2 ),
	m_durationFrames( 45 * LOGICFRAMES_PER_SECOND ),
	m_radius( 350.0f )
{
	m_targetKinds.set( KINDOF_AIRCRAFT );
}

void AirCoverZoneUpdateModuleData::buildFieldParse( MultiIniFieldParse &p )
{
	UpdateModuleData::buildFieldParse( p );

	static const FieldParse dataFieldParse[] =
	{
		{ "Aircraft",		INI::parseAsciiString,			nullptr, offsetof( AirCoverZoneUpdateModuleData, m_aircraftName ) },
		{ "Count",			INI::parseInt,					nullptr, offsetof( AirCoverZoneUpdateModuleData, m_count ) },
		{ "Duration",		INI::parseDurationUnsignedInt,	nullptr, offsetof( AirCoverZoneUpdateModuleData, m_durationFrames ) },
		{ "Radius",			INI::parseReal,					nullptr, offsetof( AirCoverZoneUpdateModuleData, m_radius ) },
		{ "TargetKinds",	KindOfMaskType::parseFromINI,	nullptr, offsetof( AirCoverZoneUpdateModuleData, m_targetKinds ) },
		{ nullptr, nullptr, nullptr, 0 }
	};
	p.add( dataFieldParse );
}

AirCoverZoneUpdate::AirCoverZoneUpdate( Thing *thing, const ModuleData *moduleData ) :
	UpdateModule( thing, moduleData ),
	m_phase( PHASE_CALLING ),
	m_startFrame( 0 ),
	m_leaveFrame( 0 ),
	m_orbitStep( 0 )
{
	CoH::GeneralsArsenal::registerZone( this );
}

AirCoverZoneUpdate::~AirCoverZoneUpdate()
{
	CoH::GeneralsArsenal::unregisterZone( this );
}

AirCoverZoneUpdate *AirCoverZoneUpdate::find( Object *object )
{
	static const NameKeyType key = NAMEKEY( "AirCoverZoneUpdate" );
	return object ? (AirCoverZoneUpdate *)object->findUpdateModule( key ) : nullptr;
}

// The jets come in from the nearest map edge side by side, already flying at full speed.
void AirCoverZoneUpdate::callAircraft()
{
	const AirCoverZoneUpdateModuleData *data = getAirCoverZoneUpdateModuleData();
	const ThingTemplate *jetTemplate = TheThingFactory->findTemplate( data->m_aircraftName );
	Object *zone = getObject();
	m_phase = PHASE_PATROLLING;
	m_startFrame = TheGameLogic->getFrame();
	if( jetTemplate == nullptr )
	{
		CoH::log( "GeneralsArsenal: air cover %s has no aircraft '%s'", zone->getTemplate()->getName().str(), data->m_aircraftName.str() );
		return;
	}

	const Coord3D center = *zone->getPosition();
	const Coord3D edge = TheTerrainLogic->findClosestEdgePoint( &center );
	const Real heading = atan2( center.y - edge.y, center.x - edge.x );
	const Coord3D side = { -Sin( heading ), Cos( heading ), 0.0f };

	for( Int i = 0; i < data->m_count; ++i )
	{
		Object *jet = TheThingFactory->newObject( jetTemplate, zone->getTeam() );
		if( jet == nullptr )
			continue;
		const Real offset = ( i - ( data->m_count - 1 ) * 0.5f ) * FORMATION_SPACING;
		Coord3D start = { edge.x + side.x * offset, edge.y + side.y * offset, 0.0f };
		AIUpdateInterface *ai = jet->getAIUpdateInterface();
		const Real height = ( ai && ai->getCurLocomotor() ) ? ai->getCurLocomotor()->getPreferredHeight() : 100.0f;
		start.z = TheTerrainLogic->getGroundHeight( start.x, start.y ) + height;
		jet->setPosition( &start );
		jet->setOrientation( heading );

		PhysicsBehavior *physics = jet->getPhysics();
		if( physics && ai && ai->getCurLocomotor() )
		{
			Coord3D force = *jet->getUnitDirectionVector2D();
			const Real speed = ai->getCurLocomotor()->getMaxSpeedForCondition( jet->getBodyModule()->getDamageState() );
			force.scale( speed * physics->getMass() );
			physics->applyMotiveForce( &force );
		}
		m_aircraft.push_back( jet->getID() );
	}
	CoH::log( "GeneralsArsenal: frame %u air cover over (%.0f, %.0f) with %d %s", m_startFrame, center.x, center.y,
		(Int)m_aircraft.size(), data->m_aircraftName.str() );
}

void AirCoverZoneUpdate::forgetLostAircraft()
{
	std::vector<ObjectID> flying;
	for( ObjectID id : m_aircraft )
	{
		Object *jet = TheGameLogic->findObjectByID( id );
		if( jet && !jet->isEffectivelyDead() )
			flying.push_back( id );
	}
	m_aircraft.swap( flying );
}

Bool AirCoverZoneUpdate::isWorthKeeping( const Object *target ) const
{
	if( target == nullptr || target->isEffectivelyDead() )
		return FALSE;
	const Real reach = getAirCoverZoneUpdateModuleData()->m_radius * KEEP_TARGET_SLACK;
	return distance2D( *target->getPosition(), *getObject()->getPosition() ) <= reach;
}

// The closest enemy of the target kinds inside the zone that the jet can hit and see.
Object *AirCoverZoneUpdate::findTarget( const Object *jet ) const
{
	const AirCoverZoneUpdateModuleData *data = getAirCoverZoneUpdateModuleData();
	PartitionFilterRelationship enemies( jet, PartitionFilterRelationship::ALLOW_ENEMIES );
	PartitionFilterAlive alive;
	PartitionFilterSameMapStatus sameMap( jet );
	PartitionFilterStealthedAndUndetected visible( jet, FALSE );
	PartitionFilterPossibleToAttack attackable( ATTACK_NEW_TARGET, jet, CMD_FROM_AI );
	PartitionFilter *filters[] = { &enemies, &alive, &sameMap, &visible, &attackable, nullptr };

	Object *best = nullptr;
	Real bestDistance = 0.0f;
	ObjectIterator *iter = ThePartitionManager->iterateObjectsInRange( getObject()->getPosition(), data->m_radius, FROM_CENTER_2D, filters );
	MemoryPoolObjectHolder hold( iter );
	for( Object *other = iter->first(); other; other = iter->next() )
	{
		if( !other->isAnyKindOf( data->m_targetKinds ) )
			continue;
		const Real distance = distance2D( *other->getPosition(), *jet->getPosition() );
		if( best == nullptr || distance < bestDistance )
		{
			best = other;
			bestDistance = distance;
		}
	}
	return best;
}

// Idle jets fly from point to point round the zone; each jet is a little further round than the last.
Coord3D AirCoverZoneUpdate::orbitPoint( Int index ) const
{
	const Coord3D center = *getObject()->getPosition();
	const Real radius = getAirCoverZoneUpdateModuleData()->m_radius * 0.6f;
	const Real angle = ( m_orbitStep + index * 2 ) * ( 2.0f * PI / ORBIT_POINTS );
	Coord3D point = { center.x + Cos( angle ) * radius, center.y + Sin( angle ) * radius, 0.0f };
	point.z = TheTerrainLogic->getGroundHeight( point.x, point.y );
	return point;
}

void AirCoverZoneUpdate::patrol()
{
	m_orbitStep = ( m_orbitStep + 1 ) % ORBIT_POINTS;
	for( size_t i = 0; i < m_aircraft.size(); ++i )
	{
		Object *jet = TheGameLogic->findObjectByID( m_aircraft[ i ] );
		AIUpdateInterface *ai = jet ? jet->getAIUpdateInterface() : nullptr;
		if( ai == nullptr )
			continue;
		if( ai->isAttacking() && isWorthKeeping( ai->getGoalObject() ) )
			continue;

		Object *target = findTarget( jet );
		if( target )
		{
			ai->aiAttackObject( target, NO_MAX_SHOTS_LIMIT, CMD_FROM_AI );
		}
		else
		{
			const Coord3D point = orbitPoint( (Int)i );
			ai->aiMoveToPosition( &point, CMD_FROM_AI );
		}
	}
}

// Each jet flies to the map edge nearest it and is removed there. One that can't get there (the
// map's edge may be out of its reach) is removed when the time to leave is up.
void AirCoverZoneUpdate::leave()
{
	const Bool timeIsUp = TheGameLogic->getFrame() >= m_leaveFrame + LEAVE_FRAMES;
	std::vector<ObjectID> flying;
	for( ObjectID id : m_aircraft )
	{
		Object *jet = TheGameLogic->findObjectByID( id );
		AIUpdateInterface *ai = jet ? jet->getAIUpdateInterface() : nullptr;
		if( ai == nullptr )
			continue;
		const Coord3D edge = TheTerrainLogic->findClosestEdgePoint( jet->getPosition() );
		if( timeIsUp || distance2D( edge, *jet->getPosition() ) <= EXIT_DISTANCE )
		{
			TheGameLogic->destroyObject( jet );	// gone, not lost: no loss penalty
			continue;
		}
		ai->aiMoveToPosition( &edge, CMD_FROM_AI );
		flying.push_back( id );
	}
	m_aircraft.swap( flying );
}

UpdateSleepTime AirCoverZoneUpdate::update()
{
	if( m_phase == PHASE_CALLING )
		callAircraft();

	forgetLostAircraft();
	if( m_phase == PHASE_PATROLLING )
	{
		const Bool over = TheGameLogic->getFrame() >= m_startFrame + getAirCoverZoneUpdateModuleData()->m_durationFrames;
		if( over || m_aircraft.empty() )
		{
			m_phase = PHASE_LEAVING;
			m_leaveFrame = TheGameLogic->getFrame();
		}
		else
			patrol();
	}
	if( m_phase == PHASE_LEAVING )
	{
		leave();
		if( m_aircraft.empty() )
		{
			TheGameLogic->destroyObject( getObject() );
			return UPDATE_SLEEP_FOREVER;
		}
	}
	return UPDATE_SLEEP( THINK_FRAMES );
}

// The engine's object CRC doesn't include modules; this is here for a CRC that does.
void AirCoverZoneUpdate::crc( Xfer *xfer )
{
	AirCoverZoneUpdate::xfer( xfer );
}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void AirCoverZoneUpdate::xfer( Xfer *xfer )
{
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	UpdateModule::xfer( xfer );

	xfer->xferInt( &m_phase );
	xfer->xferUnsignedInt( &m_startFrame );
	xfer->xferUnsignedInt( &m_leaveFrame );
	xfer->xferInt( &m_orbitStep );
	UnsignedShort count = (UnsignedShort)m_aircraft.size();
	xfer->xferUnsignedShort( &count );
	m_aircraft.resize( count );
	for( ObjectID &id : m_aircraft )
		xfer->xferObjectID( &id );
}

void AirCoverZoneUpdate::loadPostProcess()
{
	UpdateModule::loadPostProcess();
}
