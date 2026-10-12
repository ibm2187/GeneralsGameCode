// IBM Engine: IbmSquadUpdate, the module that makes a mob leader a squad leader. See SquadUpdate.h.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/NameKeyGenerator.h"
#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"
#include "GameClient/Drawable.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Module/BodyModule.h"
#include "GameLogic/Module/SpecialPowerUpdateModule.h"

#include "Features/Squads/Squads.h"
#include "Features/Squads/SquadUpdate.h"

//-------------------------------------------------------------------------------------------------
IbmSquadUpdate *IbmEngine::getSquad(const Object *obj)
{
	if (obj == nullptr || !obj->isKindOf(KINDOF_MOB_NEXUS))
		return nullptr;

	static const NameKeyType key = NAMEKEY("IbmSquadUpdate");
	return static_cast<IbmSquadUpdate *>(obj->findUpdateModule(key));
}

//-------------------------------------------------------------------------------------------------
const SpawnBehaviorModuleData *IbmEngine::findSquadSpawnData(const ThingTemplate *leaderTemplate)
{
	const ModuleInfo &modules = leaderTemplate->getBehaviorModuleInfo();
	for (Int i = 0; i < modules.getCount(); ++i)
	{
		if (modules.getNthName(i) == "SpawnBehavior")
			return static_cast<const SpawnBehaviorModuleData *>(modules.getNthData(i));
	}
	return nullptr;
}

//-------------------------------------------------------------------------------------------------
Object *IbmEngine::getSquadLeaderOf(const Object *obj)
{
	if (obj == nullptr || !obj->isKindOf(KINDOF_IGNORED_IN_GUI))
		return nullptr;

	for (BehaviorModule **m = obj->getBehaviorModules(); *m; ++m)
	{
		SlavedUpdateInterface *slaved = (*m)->getSlavedUpdateInterface();
		if (slaved != nullptr)
		{
			Object *leader = TheGameLogic->findObjectByID(slaved->getSlaverID());
			return isSquadLeader(leader) ? leader : nullptr;
		}
	}
	return nullptr;
}

//-------------------------------------------------------------------------------------------------
Bool IbmEngine::isSquadMemberOnOwnOrder(Object *member, const Object *leader)
{
	IbmSquadUpdate *squad = getSquad(leader);
	return squad != nullptr && squad->isMemberOnOwnOrder(member);
}

//-------------------------------------------------------------------------------------------------
// TRUE while a special ability (Burton's C4, a hack) is still running, even when the AI stands idle.
static Bool isUsingAbility(const Object *obj)
{
	if (obj->testStatus(OBJECT_STATUS_IS_USING_ABILITY))
		return TRUE;

	for (BehaviorModule **m = obj->getBehaviorModules(); *m; ++m)
	{
		const SpecialPowerUpdateInterface *power = (*m)->getSpecialPowerUpdateInterface();
		if (power != nullptr && power->isActive())
			return TRUE;
	}
	return FALSE;
}

//-------------------------------------------------------------------------------------------------
IbmSquadUpdateModuleData::IbmSquadUpdateModuleData()
{
	m_stealthWithMembers = FALSE;
	m_reinforceRadius = 200.0f;
	m_reinforceDelay = LOGICFRAMES_PER_SECOND * 2;
}

//-------------------------------------------------------------------------------------------------
/*static*/ void IbmSquadUpdateModuleData::buildFieldParse(MultiIniFieldParse& p)
{
	UpdateModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "AIReplaces",			INI::parseAsciiString,	nullptr, offsetof( IbmSquadUpdateModuleData, m_aiReplaces ) },
		{ "StealthWithMembers",	INI::parseBool,			nullptr, offsetof( IbmSquadUpdateModuleData, m_stealthWithMembers ) },
		{ "ReinforceAt",		INI::parseAsciiStringVectorAppend,	nullptr, offsetof( IbmSquadUpdateModuleData, m_reinforceAt ) },
		{ "ReinforceRadius",	INI::parseReal,			nullptr, offsetof( IbmSquadUpdateModuleData, m_reinforceRadius ) },
		{ "ReinforceDelay",		INI::parseDurationUnsignedInt,	nullptr, offsetof( IbmSquadUpdateModuleData, m_reinforceDelay ) },
		{ nullptr, nullptr, nullptr, 0 }
	};
	p.add(dataFieldParse);
}

//-------------------------------------------------------------------------------------------------
IbmSquadUpdate::IbmSquadUpdate( Thing *thing, const ModuleData* moduleData ) : UpdateModule( thing, moduleData )
{
	m_parked = FALSE;
	m_nextReinforceFrame = 0;
	m_lastReinforceFrame = 0;
}

//-------------------------------------------------------------------------------------------------
IbmSquadUpdate::~IbmSquadUpdate()
{
}

//-------------------------------------------------------------------------------------------------
void IbmSquadUpdate::setMemberOnOwnOrder( ObjectID member )
{
	if (std::find(m_ownOrderMembers.begin(), m_ownOrderMembers.end(), member) == m_ownOrderMembers.end())
		m_ownOrderMembers.push_back(member);
}

//-------------------------------------------------------------------------------------------------
Bool IbmSquadUpdate::isMemberOnOwnOrder( Object *member )
{
	if (member->isContained())
		return TRUE;

	std::list<ObjectID>::iterator it = std::find(m_ownOrderMembers.begin(), m_ownOrderMembers.end(), member->getID());
	if (it == m_ownOrderMembers.end())
		return FALSE;

	const AIUpdateInterface *ai = member->getAI();
	if (ai != nullptr && (!ai->isIdle() || isUsingAbility(member)))
		return TRUE;

	// Done: the member rejoins the squad.
	m_ownOrderMembers.erase(it);
	return FALSE;
}

//-------------------------------------------------------------------------------------------------
UpdateSleepTime IbmSquadUpdate::update()
{
	Object *leader = getObject();

	Int alive = 0;
	Int inside = 0;
	Object *container = nullptr;
	Object *outside = nullptr;
	Bool allHidden = TRUE;

	IbmEngine::forEachSquadMember(leader, [&](Object *member)
	{
		++alive;
		if (member->isContained())
		{
			++inside;
			if (container == nullptr)
				container = member->getContainedBy();
		}
		else if (outside == nullptr)
		{
			outside = member;
		}

		if (!member->testStatus(OBJECT_STATUS_STEALTHED) || member->testStatus(OBJECT_STATUS_DETECTED))
			allHidden = FALSE;
	});

	// The leader rides in the container its members ride in, so the squad leaves it as one unit.
	if (m_parked)
	{
		if (alive == 0 || inside < alive)
		{
			unpark(outside != nullptr ? outside->getPosition() : leader->getPosition());
		}
		else if (container != nullptr)
		{
			leader->setPosition(container->getPosition());

			Drawable *draw = leader->getDrawable();
			if (draw != nullptr && !draw->isDrawableEffectivelyHidden())
				draw->setDrawableHidden(TRUE);
		}
	}
	else if (alive > 0 && inside == alive && container != nullptr)
	{
		park(container);
	}

	// Lost members come back near the Barracks, one every ReinforceDelay.
	const UnsignedInt now = TheGameLogic->getFrame();
	if (!m_parked && alive > 0 && now >= m_nextReinforceFrame)
	{
		m_nextReinforceFrame = now + getData()->m_reinforceDelay;
		if (IbmEngine::reinforceSquad(leader, getData()))
			m_lastReinforceFrame = now;
	}

	// An invisible leader must not give its hidden members away (health bar, icons, AI scans).
	if (getData()->m_stealthWithMembers)
		leader->setStatus(MAKE_OBJECT_STATUS_MASK(OBJECT_STATUS_CAN_STEALTH), alive > 0 && allHidden && !m_parked);

	return UPDATE_SLEEP_NONE;
}

//-------------------------------------------------------------------------------------------------
// As long as the engine shows its heal icon over a healed unit. The Barracks heals every second.
static const UnsignedInt SQUAD_HEALING_ICON_FRAMES = LOGICFRAMES_PER_SECOND * 3;

Bool IbmSquadUpdate::isHealing() const
{
	const UnsignedInt now = TheGameLogic->getFrame();
	if (m_lastReinforceFrame != 0 && now - m_lastReinforceFrame <= SQUAD_HEALING_ICON_FRAMES)
		return TRUE;

	Bool healing = FALSE;
	IbmEngine::forEachSquadMember(getObject(), [&](Object *member)
	{
		const BodyModuleInterface *body = member->getBodyModule();
		const UnsignedInt *lastHealed = body != nullptr ? body->getLastHealingTimestamp() : nullptr;
		if (lastHealed != nullptr && *lastHealed != 0 && now - *lastHealed <= SQUAD_HEALING_ICON_FRAMES)
			healing = TRUE;
	});
	return healing;
}

//-------------------------------------------------------------------------------------------------
Bool IbmEngine::isSquadHealing(const Object *obj)
{
	const IbmSquadUpdate *squad = getSquad(obj);
	return squad != nullptr && !squad->isParked() && squad->isHealing();
}

//-------------------------------------------------------------------------------------------------
void IbmSquadUpdate::park( Object *container )
{
	Object *leader = getObject();
	m_parked = TRUE;

	AIUpdateInterface *ai = leader->getAI();
	if (ai != nullptr)
		ai->aiIdle(CMD_FROM_AI);

	leader->setDisabled(DISABLED_HELD);
	leader->setStatus(MAKE_OBJECT_STATUS_MASK(OBJECT_STATUS_UNSELECTABLE));
	TheGameLogic->deselectObject(leader, PLAYERMASK_ALL, TRUE);
	leader->setPosition(container->getPosition());

	Drawable *draw = leader->getDrawable();
	if (draw != nullptr)
		draw->setDrawableHidden(TRUE);
}

//-------------------------------------------------------------------------------------------------
void IbmSquadUpdate::unpark( const Coord3D *pos )
{
	Object *leader = getObject();
	m_parked = FALSE;

	leader->clearDisabled(DISABLED_HELD);
	leader->clearStatus(MAKE_OBJECT_STATUS_MASK(OBJECT_STATUS_UNSELECTABLE));
	leader->setPosition(pos);

	Drawable *draw = leader->getDrawable();
	if (draw != nullptr)
		draw->setDrawableHidden(FALSE);
}

//-------------------------------------------------------------------------------------------------
void IbmSquadUpdate::crc( Xfer *xfer )
{
	UpdateModule::crc( xfer );
}

//-------------------------------------------------------------------------------------------------
// Version Info:
// 1: Initial version
void IbmSquadUpdate::xfer( Xfer *xfer )
{
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	UpdateModule::xfer( xfer );

	xfer->xferSTLObjectIDList( &m_ownOrderMembers );
	xfer->xferBool( &m_parked );
	xfer->xferUnsignedInt( &m_nextReinforceFrame );
}

//-------------------------------------------------------------------------------------------------
void IbmSquadUpdate::loadPostProcess()
{
	UpdateModule::loadPostProcess();
}
