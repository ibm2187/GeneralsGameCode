// IBM Engine: IbmWatchZoneUpdate, a watch zone that calls in a jet for each enemy that enters.
// Also keeps track of the jets in the air and the enemies they follow. See WatchZoneUpdate.h.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include <map>

#include "Common/Xfer.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/ObjectCreationList.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/TerrainLogic.h"

#include "Features/WatchZones/WatchZoneUpdate.h"
#include "Features/WatchZones/WatchZones.h"

namespace
{

// A jet a watch zone called in, and the enemy it follows.
struct WatchTrack
{
	ObjectID target;
	Coord3D zoneCenter;
	Real followRange;
};

// By jet. Not saved: after loading a game, jets in the air fly to where their enemy was.
std::map<ObjectID, WatchTrack> s_tracks;

// The distance an enemy must move before its jet is sent after it again.
const Real FOLLOW_STEP = 10.0f;

// How often a zone looks for enemies while a jet is ready.
const UnsignedInt SCAN_FRAMES = LOGICFRAMES_PER_SECOND / 4;

Bool isHidden(const Object *obj)
{
	return obj->testStatus(OBJECT_STATUS_STEALTHED) && !obj->testStatus(OBJECT_STATUS_DETECTED);
}

} // namespace

//-------------------------------------------------------------------------------------------------
Bool IbmEngine::followWatchTarget(const Object *jet, Coord3D &targetPos, Coord3D &moveToPos)
{
	if (s_tracks.empty())
		return FALSE;

	std::map<ObjectID, WatchTrack>::iterator it = s_tracks.find(jet->getID());
	if (it == s_tracks.end())
		return FALSE;

	const WatchTrack &track = it->second;
	Object *target = TheGameLogic->findObjectByID(track.target);
	if (target == nullptr || target->isEffectivelyDead() || isHidden(target))
	{
		s_tracks.erase(it);
		return FALSE;
	}

	const Coord3D *pos = target->getPosition();
	Real dxZone = pos->x - track.zoneCenter.x;
	Real dyZone = pos->y - track.zoneCenter.y;
	if (dxZone * dxZone + dyZone * dyZone > track.followRange * track.followRange)
	{
		s_tracks.erase(it);	// it got away; the jet strikes where it last saw it
		return FALSE;
	}

	Real dx = pos->x - targetPos.x;
	Real dy = pos->y - targetPos.y;
	if (dx * dx + dy * dy < FOLLOW_STEP * FOLLOW_STEP)
		return FALSE;

	moveToPos.x += dx;
	moveToPos.y += dy;
	targetPos = *pos;
	return TRUE;
}

//-------------------------------------------------------------------------------------------------
Object *IbmEngine::getWatchTarget(const Object *jet)
{
	if (s_tracks.empty())
		return nullptr;

	std::map<ObjectID, WatchTrack>::const_iterator it = s_tracks.find(jet->getID());
	if (it == s_tracks.end())
		return nullptr;

	Object *target = TheGameLogic->findObjectByID(it->second.target);
	if (target == nullptr || target->isEffectivelyDead())
		return nullptr;
	return target;
}

//-------------------------------------------------------------------------------------------------
void IbmEngine::resetWatchZones()
{
	s_tracks.clear();
}

//-------------------------------------------------------------------------------------------------
IbmWatchZoneUpdateModuleData::IbmWatchZoneUpdateModuleData()
{
	m_radius = 150.0f;
	m_duration = LOGICFRAMES_PER_SECOND * 30;
	m_targets = TARGETS_ANY;
	m_strikes = 1;
	m_strikeDelay = LOGICFRAMES_PER_SECOND * 5;
	m_strikeOCL = nullptr;
	m_approachDistance = 0.0f;
	m_followRange = 300.0f;
}

//-------------------------------------------------------------------------------------------------
/*static*/ void IbmWatchZoneUpdateModuleData::buildFieldParse(MultiIniFieldParse& p)
{
	UpdateModuleData::buildFieldParse(p);

	static const char *const targetNames[] = { "ANY", "AIRBORNE", "GROUND", nullptr };

	static const FieldParse dataFieldParse[] =
	{
		{ "Radius",				INI::parseReal,						nullptr, offsetof( IbmWatchZoneUpdateModuleData, m_radius ) },
		{ "Duration",			INI::parseDurationUnsignedInt,		nullptr, offsetof( IbmWatchZoneUpdateModuleData, m_duration ) },
		{ "TargetKindOf",		KindOfMaskType::parseFromINI,		nullptr, offsetof( IbmWatchZoneUpdateModuleData, m_targetKindOf ) },
		{ "ForbiddenKindOf",	KindOfMaskType::parseFromINI,		nullptr, offsetof( IbmWatchZoneUpdateModuleData, m_forbiddenKindOf ) },
		{ "Targets",			INI::parseIndexList,				targetNames, offsetof( IbmWatchZoneUpdateModuleData, m_targets ) },
		{ "Strikes",			INI::parseInt,						nullptr, offsetof( IbmWatchZoneUpdateModuleData, m_strikes ) },
		{ "StrikeDelay",		INI::parseDurationUnsignedInt,		nullptr, offsetof( IbmWatchZoneUpdateModuleData, m_strikeDelay ) },
		{ "StrikeOCL",			INI::parseObjectCreationList,		nullptr, offsetof( IbmWatchZoneUpdateModuleData, m_strikeOCL ) },
		{ "ApproachDistance",	INI::parseReal,						nullptr, offsetof( IbmWatchZoneUpdateModuleData, m_approachDistance ) },
		{ "FollowRange",		INI::parseReal,						nullptr, offsetof( IbmWatchZoneUpdateModuleData, m_followRange ) },
		{ "ZoneDecal",			RadiusDecalTemplate::parseRadiusDecalTemplate,	nullptr, offsetof( IbmWatchZoneUpdateModuleData, m_zoneDecal ) },
		{ nullptr, nullptr, nullptr, 0 }
	};
	p.add(dataFieldParse);
}

//-------------------------------------------------------------------------------------------------
IbmWatchZoneUpdate::IbmWatchZoneUpdate( Thing *thing, const ModuleData* moduleData ) : UpdateModule( thing, moduleData )
{
	m_started = FALSE;
	m_endFrame = 0;
	m_nextStrikeFrame = 0;
	m_strikesLeft = 0;
	m_home.zero();
}

//-------------------------------------------------------------------------------------------------
IbmWatchZoneUpdate::~IbmWatchZoneUpdate()
{
	m_decal.clear();
}

//-------------------------------------------------------------------------------------------------
UpdateSleepTime IbmWatchZoneUpdate::update()
{
	const IbmWatchZoneUpdateModuleData *data = getData();
	Object *zone = getObject();
	UnsignedInt now = TheGameLogic->getFrame();

	if (!m_started)
	{
		m_started = TRUE;
		m_endFrame = now + data->m_duration;
		m_nextStrikeFrame = now;
		m_strikesLeft = data->m_strikes;

		// Jets come from the map edge nearest the building that made the zone.
		const Object *producer = TheGameLogic->findObjectByID(zone->getProducerID());
		m_home = TheTerrainLogic->findClosestEdgePoint(producer != nullptr ? producer->getPosition() : zone->getPosition());
	}

	if (m_decal.isEmpty() && data->m_zoneDecal.valid())
		data->m_zoneDecal.createRadiusDecal(*zone->getPosition(), data->m_radius, zone->getControllingPlayer(), m_decal);
	m_decal.update();

	// Forget jets that are gone.
	for (std::list<ObjectID>::iterator it = m_jets.begin(); it != m_jets.end(); )
	{
		if (TheGameLogic->findObjectByID(*it) == nullptr)
		{
			s_tracks.erase(*it);
			it = m_jets.erase(it);
		}
		else
			++it;
	}

	if (now >= m_endFrame || m_strikesLeft <= 0)
	{
		m_decal.clear();
		TheGameLogic->destroyObject(zone);
		return UPDATE_SLEEP_FOREVER;
	}

	if (now >= m_nextStrikeFrame)
	{
		Object *target = findTarget();
		if (target != nullptr)
		{
			callStrike(target);
			m_nextStrikeFrame = now + data->m_strikeDelay;
		}
		else
			m_nextStrikeFrame = now + SCAN_FRAMES;
	}

	return UPDATE_SLEEP_NONE;	// every frame, so the circle throbs smoothly
}

//-------------------------------------------------------------------------------------------------
Bool IbmWatchZoneUpdate::isTarget( const Object *obj ) const
{
	const IbmWatchZoneUpdateModuleData *data = getData();

	if (data->m_targetKindOf.any() && !obj->isAnyKindOf(data->m_targetKindOf))
		return FALSE;
	if (obj->isAnyKindOf(data->m_forbiddenKindOf) || obj->isKindOf(KINDOF_UNATTACKABLE))
		return FALSE;
	if (data->m_targets == IbmWatchZoneUpdateModuleData::TARGETS_AIRBORNE && !obj->isAirborneTarget())
		return FALSE;
	if (data->m_targets == IbmWatchZoneUpdateModuleData::TARGETS_GROUND && obj->isAirborneTarget())
		return FALSE;
	if (isHidden(obj))
		return FALSE;
	return TRUE;
}

//-------------------------------------------------------------------------------------------------
Bool IbmWatchZoneUpdate::isJetAfter( ObjectID target ) const
{
	for (std::list<ObjectID>::const_iterator it = m_jets.begin(); it != m_jets.end(); ++it)
	{
		std::map<ObjectID, WatchTrack>::const_iterator track = s_tracks.find(*it);
		if (track != s_tracks.end() && track->second.target == target)
			return TRUE;
	}
	return FALSE;
}

//-------------------------------------------------------------------------------------------------
// The nearest enemy no jet of this zone is after; failing that, the nearest enemy.
Object *IbmWatchZoneUpdate::findTarget() const
{
	const Object *zone = getObject();

	PartitionFilterRelationship enemies(zone, PartitionFilterRelationship::ALLOW_ENEMIES);
	PartitionFilterAlive alive;
	PartitionFilter *filters[] = { &enemies, &alive, nullptr };

	SimpleObjectIterator *iter = ThePartitionManager->iterateObjectsInRange(zone, getData()->m_radius, FROM_CENTER_2D, filters, ITER_SORTED_NEAR_TO_FAR);
	MemoryPoolObjectHolder hold(iter);

	Object *taken = nullptr;
	for (Object *obj = iter->first(); obj != nullptr; obj = iter->next())
	{
		if (!isTarget(obj))
			continue;
		if (!isJetAfter(obj->getID()))
			return obj;
		if (taken == nullptr)
			taken = obj;
	}
	return taken;
}

//-------------------------------------------------------------------------------------------------
void IbmWatchZoneUpdate::callStrike( Object *target )
{
	const IbmWatchZoneUpdateModuleData *data = getData();
	--m_strikesLeft;

	if (data->m_strikeOCL == nullptr)
		return;

	Coord3D targetPos = *target->getPosition();
	Coord3D start = m_home;
	if (data->m_approachDistance > 0.0f)
	{
		Real dx = m_home.x - targetPos.x;
		Real dy = m_home.y - targetPos.y;
		Real length = sqrt(dx * dx + dy * dy);
		if (length > data->m_approachDistance)
		{
			start.x = targetPos.x + dx * data->m_approachDistance / length;
			start.y = targetPos.y + dy * data->m_approachDistance / length;
		}
	}

	Object *jet = ObjectCreationList::create(data->m_strikeOCL, getObject(), &start, &targetPos, 0.0f);
	if (jet == nullptr)
		return;

	WatchTrack track;
	track.target = target->getID();
	track.zoneCenter = *getObject()->getPosition();
	track.followRange = data->m_followRange;
	s_tracks[jet->getID()] = track;
	m_jets.push_back(jet->getID());
}

//-------------------------------------------------------------------------------------------------
void IbmWatchZoneUpdate::crc( Xfer *xfer )
{
	UpdateModule::crc( xfer );
}

//-------------------------------------------------------------------------------------------------
// Version Info:
// 1: Initial version
void IbmWatchZoneUpdate::xfer( Xfer *xfer )
{
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	UpdateModule::xfer( xfer );

	xfer->xferBool( &m_started );
	xfer->xferUnsignedInt( &m_endFrame );
	xfer->xferUnsignedInt( &m_nextStrikeFrame );
	xfer->xferInt( &m_strikesLeft );
	xfer->xferCoord3D( &m_home );
	xfer->xferSTLObjectIDList( &m_jets );
}

//-------------------------------------------------------------------------------------------------
void IbmWatchZoneUpdate::loadPostProcess()
{
	UpdateModule::loadPostProcess();
}
