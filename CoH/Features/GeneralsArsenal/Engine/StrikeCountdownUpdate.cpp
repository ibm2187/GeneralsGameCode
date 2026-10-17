// CoH GeneralsArsenal: see StrikeCountdownUpdate.h.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/INI.h"
#include "Common/Player.h"
#include "Common/SpecialPower.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"
#include "GameClient/ControlBar.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/TerrainLogic.h"

#include "Kernel/Log.h"

#include "Features/GeneralsArsenal/Engine/Internal.h"
#include "Features/GeneralsArsenal/Engine/StrikeCountdownUpdate.h"

using namespace CoH::GeneralsArsenal;

StrikeCountdownUpdateModuleData::StrikeCountdownUpdateModuleData() :
	m_delayFrames( 0 ),
	m_hostPower( nullptr ),
	m_hostLifetimeFrames( 0 ),
	m_radius( 150.0f ),
	m_warnEveryone( TRUE )
{
}

void StrikeCountdownUpdateModuleData::buildFieldParse( MultiIniFieldParse &p )
{
	UpdateModuleData::buildFieldParse( p );

	static const FieldParse dataFieldParse[] =
	{
		{ "Delay",			INI::parseDurationUnsignedInt,	nullptr, offsetof( StrikeCountdownUpdateModuleData, m_delayFrames ) },
		{ "Host",			INI::parseAsciiString,			nullptr, offsetof( StrikeCountdownUpdateModuleData, m_hostName ) },
		{ "HostPower",		INI::parseSpecialPowerTemplate,	nullptr, offsetof( StrikeCountdownUpdateModuleData, m_hostPower ) },
		{ "HostLifetime",	INI::parseDurationUnsignedInt,	nullptr, offsetof( StrikeCountdownUpdateModuleData, m_hostLifetimeFrames ) },
		{ "Label",			INI::parseAsciiString,			nullptr, offsetof( StrikeCountdownUpdateModuleData, m_label ) },
		{ "Radius",			INI::parseReal,					nullptr, offsetof( StrikeCountdownUpdateModuleData, m_radius ) },
		{ "WarnEveryone",	INI::parseBool,					nullptr, offsetof( StrikeCountdownUpdateModuleData, m_warnEveryone ) },
		{ nullptr, nullptr, nullptr, 0 }
	};
	p.add( dataFieldParse );
}

StrikeCountdownUpdate::StrikeCountdownUpdate( Thing *thing, const ModuleData *moduleData ) :
	UpdateModule( thing, moduleData ),
	m_startFrame( TheGameLogic->getFrame() ),
	m_hostID( INVALID_ID ),
	m_fired( FALSE ),
	m_doneFrame( 0 )
{
	registerMarker( this );
}

StrikeCountdownUpdate::~StrikeCountdownUpdate()
{
	unregisterMarker( this );
}

WarningTimer StrikeCountdownUpdate::timer() const
{
	return WarningTimer( m_startFrame, getStrikeCountdownUpdateModuleData()->m_delayFrames );
}

StrikeCountdownUpdate *StrikeCountdownUpdate::find( Object *object )
{
	static const NameKeyType key = NAMEKEY( "StrikeCountdownUpdate" );
	return object ? (StrikeCountdownUpdate *)object->findUpdateModule( key ) : nullptr;
}

// The host goes to the map's edge nearest the target, outside the playable area, where it is out of
// sight and out of reach. It is made when the countdown starts, so a building that charges up (the
// particle cannon's antenna, the silo's doors) is ready when the countdown ends.
void StrikeCountdownUpdate::createHost()
{
	const StrikeCountdownUpdateModuleData *data = getStrikeCountdownUpdateModuleData();
	const ThingTemplate *hostTemplate = TheThingFactory->findTemplate( data->m_hostName );
	Object *marker = getObject();
	if( hostTemplate == nullptr || marker->getControllingPlayer() == nullptr )
	{
		CoH::log( "GeneralsArsenal: marker %s has no host '%s'", marker->getTemplate()->getName().str(), data->m_hostName.str() );
		return;
	}

	Object *host = TheThingFactory->newObject( hostTemplate, marker->getTeam() );
	if( host == nullptr )
		return;
	Coord3D edge = TheTerrainLogic->findClosestEdgePoint( marker->getPosition() );
	host->setPosition( &edge );
	m_hostID = host->getID();
}

void StrikeCountdownUpdate::fire()
{
	const StrikeCountdownUpdateModuleData *data = getStrikeCountdownUpdateModuleData();
	Object *marker = getObject();
	Object *host = TheGameLogic->findObjectByID( m_hostID );
	m_fired = TRUE;
	m_doneFrame = TheGameLogic->getFrame() + data->m_hostLifetimeFrames;

	if( host == nullptr || data->m_hostPower == nullptr )
	{
		CoH::log( "GeneralsArsenal: marker %s fires nothing (no host)", marker->getTemplate()->getName().str() );
		return;
	}
	host->doSpecialPowerAtLocation( data->m_hostPower, marker->getPosition(), INVALID_ANGLE, COMMAND_FIRED_BY_SCRIPT, TRUE );
	CoH::log( "GeneralsArsenal: frame %u %s fires at (%.0f, %.0f)", TheGameLogic->getFrame(), host->getTemplate()->getName().str(),
		marker->getPosition()->x, marker->getPosition()->y );
}

void StrikeCountdownUpdate::finish()
{
	Object *host = TheGameLogic->findObjectByID( m_hostID );
	if( host )
		TheGameLogic->destroyObject( host );
	TheGameLogic->destroyObject( getObject() );
}

UpdateSleepTime StrikeCountdownUpdate::update()
{
	const UnsignedInt now = TheGameLogic->getFrame();
	if( !m_fired && m_hostID == INVALID_ID )
		createHost();

	if( !m_fired && timer().isDue( now ) )
		fire();
	else if( m_fired && now >= m_doneFrame )
		finish();

	return UPDATE_SLEEP_NONE;
}

// The engine's object CRC doesn't include modules; this is here for a CRC that does.
void StrikeCountdownUpdate::crc( Xfer *xfer )
{
	StrikeCountdownUpdate::xfer( xfer );
}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void StrikeCountdownUpdate::xfer( Xfer *xfer )
{
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	UpdateModule::xfer( xfer );

	xfer->xferUnsignedInt( &m_startFrame );
	xfer->xferObjectID( &m_hostID );
	xfer->xferBool( &m_fired );
	xfer->xferUnsignedInt( &m_doneFrame );
}

void StrikeCountdownUpdate::loadPostProcess()
{
	UpdateModule::loadPostProcess();
}
