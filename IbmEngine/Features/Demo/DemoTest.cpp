// IBM Engine: "-demoStrike <SpecialPower>" (debug builds only), a test for strikes. With -demo, three
// seconds into the game the local player's object that has the power fires it at the first enemy's
// Command Center, then the log (DebugLogFileD.txt) follows every projectile and launcher once a
// third of a second for 40 seconds, and the game quits. See README.md.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/GameEngine.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/SpecialPower.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "GameClient/ControlBar.h"
#include "GameLogic/ObjectCreationList.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/Weapon.h"
#include "GameLogic/Module/SpecialPowerModule.h"
#include "GameLogic/Module/ContainModule.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Module/DeliverPayloadAIUpdate.h"

#include "Features/Demo/Demo.h"

#if defined(RTS_DEBUG)

namespace
{

const UnsignedInt FIRE_FRAME = 90;
const UnsignedInt WATCH_FRAMES = 30 * 40;

const char *findArgument(const char *name)
{
	for (Int arg = 1; arg + 1 < __argc; ++arg)
		if (stricmp(__argv[arg], name) == 0)
			return __argv[arg + 1];
	return nullptr;
}

// "-demoStrikeAt <Object>": aim at the first enemy object of this kind instead of the Command Center.
const char *s_aimAt = nullptr;

struct PowerSearch
{
	const SpecialPowerTemplate *power;
	Object *found;
};

void findPowerOwner(Object *obj, void *userData)
{
	PowerSearch *search = static_cast<PowerSearch *>(userData);
	if (search->found == nullptr && obj->getSpecialPowerModule(search->power) != nullptr)
		search->found = obj;
}

struct CommandCenterSearch
{
	Object *found;
};

void findCommandCenter(Object *obj, void *userData)
{
	CommandCenterSearch *search = static_cast<CommandCenterSearch *>(userData);
	if (search->found != nullptr || obj->isEffectivelyDead())
		return;
	if (s_aimAt != nullptr ? obj->getTemplate()->getName().compareNoCase(s_aimAt) == 0 : obj->isKindOf(KINDOF_COMMANDCENTER))
		search->found = obj;
}

void fire(const char *powerName)
{
	const SpecialPowerTemplate *power = TheSpecialPowerStore->findSpecialPowerTemplate(powerName);
	Player *player = ThePlayerList->getLocalPlayer();
	if (power == nullptr || player == nullptr)
	{
		DEBUG_LOG(("IbmEngine demoStrike: no power '%s' or no local player", powerName));
		return;
	}

	PowerSearch search = { power, nullptr };
	player->iterateObjects(findPowerOwner, &search);
	if (search.found == nullptr)
	{
		DEBUG_LOG(("IbmEngine demoStrike: the local player has nothing with '%s'", powerName));
		return;
	}

	Coord3D target = *search.found->getPosition();
	for (Int i = 0; i < ThePlayerList->getPlayerCount(); ++i)
	{
		Player *other = ThePlayerList->getNthPlayer(i);
		if (other == player || !other->isPlayableSide() || other->getDefaultTeam() == nullptr
			|| player->getRelationship(other->getDefaultTeam()) != ENEMIES)
			continue;
		CommandCenterSearch cc = { nullptr };
		other->iterateObjects(findCommandCenter, &cc);
		if (cc.found != nullptr)
		{
			target = *cc.found->getPosition();
			break;
		}
	}

	DEBUG_LOG(("IbmEngine demoStrike: %s fires %s at (%.0f, %.0f) from (%.0f, %.0f), science owned %d",
		search.found->getTemplate()->getName().str(), powerName, target.x, target.y,
		search.found->getPosition()->x, search.found->getPosition()->y,
		power->getRequiredScience() == SCIENCE_INVALID || player->hasScience(power->getRequiredScience())));
	SpecialPowerModuleInterface *module = search.found->getSpecialPowerModule(power);
	DEBUG_LOG(("IbmEngine demoStrike: owner disabled %d, power ready frame %u (now %u), module %s", search.found->isDisabled(),
		module->getReadyFrame(), TheGameLogic->getFrame(), module->getSpecialPowerTemplate()->getName().str()));

	// The buttons the player sees on it.
	const CommandSet *commandSet = TheControlBar->findCommandSet(search.found->getCommandSetString());
	for (Int i = 0; commandSet != nullptr && i < MAX_COMMANDS_PER_SET; ++i)
	{
		const CommandButton *button = commandSet->getCommandButton(i);
		if (button != nullptr)
			DEBUG_LOG(("IbmEngine demoStrike: %s slot %d = %s", search.found->getCommandSetString().str(), i + 1, button->getName().str()));
	}

	ObjectID lastID = INVALID_ID;
	for (Object *obj = TheGameLogic->getFirstObject(); obj != nullptr; obj = obj->getNextObject())
		lastID = max(lastID, obj->getID());

	search.found->getSpecialPowerModule(power)->doSpecialPowerAtLocation(&target, INVALID_ANGLE, 0);

	for (Object *obj = TheGameLogic->getFirstObject(); obj != nullptr; obj = obj->getNextObject())
	{
		if (obj->getID() <= lastID)
			continue;
		const Coord3D *pos = obj->getPosition();
		DEBUG_LOG(("IbmEngine demoStrike:   created %s #%u at (%.0f, %.0f, %.0f)", obj->getTemplate()->getName().str(),
			obj->getID(), pos->x, pos->y, pos->z));
	}
}

// Logs whether the pack's strike definitions loaded.
void checkDefinitions()
{
	static const char *const objects[] =
	{
		"IbmScudStrikeLauncher", "IbmMineRocketLauncher", "IbmNuclearStrikeLauncher", "IbmCruiseStrikeLauncher",
		"IbmScudMissile", "IbmGLAMineRocket", "IbmNuclearMissile", "IbmCruiseMissile", "IbmPrecisionBomb",
	};
	for (Int i = 0; i < ARRAY_SIZE(objects); ++i)
		DEBUG_LOG(("IbmEngine demoStrike: object %s %s", objects[i], TheThingFactory->findTemplate(objects[i], FALSE) ? "loaded" : "MISSING"));

	static const char *const ocls[] =
	{
		"IbmSUPERWEAPON_ScudStormStrike", "IbmSUPERWEAPON_MineRockets", "IbmSUPERWEAPON_NuclearStrike", "IbmSUPERWEAPON_CruiseStrike",
	};
	for (Int i = 0; i < ARRAY_SIZE(ocls); ++i)
		DEBUG_LOG(("IbmEngine demoStrike: OCL %s %s", ocls[i], TheObjectCreationListStore->findObjectCreationList(ocls[i]) ? "loaded" : "MISSING"));
}

void logFlyingThings()
{
	for (Object *obj = TheGameLogic->getFirstObject(); obj != nullptr; obj = obj->getNextObject())
	{
		const AsciiString &name = obj->getTemplate()->getName();
		const Bool watched = strstr(name.str(), "Jet") != nullptr || name.startsWith("IbmGLA") || name.startsWith("IbmWatchZone")
			|| (s_aimAt != nullptr && name.compareNoCase(s_aimAt) == 0);
		if (!watched && !obj->isKindOf(KINDOF_PROJECTILE) && strstr(name.str(), "Launcher") == nullptr && strstr(name.str(), "Napalm") == nullptr
			&& strstr(name.str(), "Missile") == nullptr)
			continue;
		if (name.endsWith("Shell") || name.endsWith("Bullet"))
			continue; // tank shells and bullets of the demo's fight
		if (obj->getContainedBy() != nullptr)
			continue; // payload riding in a launcher
		const Coord3D *pos = obj->getPosition();
		AsciiString extra;
		static NameKeyType key_DeliverPayloadAIUpdate = NAMEKEY("DeliverPayloadAIUpdate");
		DeliverPayloadAIUpdate *deliver = (DeliverPayloadAIUpdate *)obj->findUpdateModule(key_DeliverPayloadAIUpdate);
		if (deliver != nullptr)
		{
			const Weapon *weapon = obj->getCurrentWeapon();
			extra.format(" ai state %d, deliver distance %.0f, carrying %d, weapon %s status %d range %.0f", (Int)deliver->getAIStateType(),
				deliver->getAllowedDistanceToTarget(), obj->getContain() ? (Int)obj->getContain()->getContainCount() : -1,
				weapon ? weapon->getName().str() : "NONE", weapon ? (Int)weapon->getStatus() : -1,
				weapon ? weapon->getAttackRange(obj) : 0.0f);
		}
		else if (watched)
		{
			const Weapon *weapon = obj->getCurrentWeapon();
			const AIUpdateInterface *ai = obj->getAI();
			const Object *victim = ai ? ai->getCurrentVictim() : nullptr;
			extra.format(" airborne %d stealthed %d detected %d, ai state %d victim %s, weapon %s status %d",
				obj->isAirborneTarget(), obj->testStatus(OBJECT_STATUS_STEALTHED), obj->testStatus(OBJECT_STATUS_DETECTED),
				ai ? (Int)ai->getAIStateType() : -1, victim ? victim->getTemplate()->getName().str() : "none",
				weapon ? weapon->getName().str() : "NONE", weapon ? (Int)weapon->getStatus() : -1);
		}
		DEBUG_LOG(("IbmEngine demoStrike:   frame %u %s #%u at (%.0f, %.0f, %.0f)%s%s", TheGameLogic->getFrame(),
			name.str(), obj->getID(), pos->x, pos->y, pos->z, obj->isEffectivelyDead() ? " dead" : "", extra.str()));
	}
}

} // namespace

//-------------------------------------------------------------------------------------------------
void IbmEngine::updateDemoTest()
{
	static const char *powerName = findArgument("-demoStrike");
	if (powerName == nullptr)
		return;
	s_aimAt = findArgument("-demoStrikeAt");

	const UnsignedInt frame = TheGameLogic->getFrame();
	if (frame == FIRE_FRAME)
	{
		checkDefinitions();
		fire(powerName);
	}
	else if (frame > FIRE_FRAME && frame < FIRE_FRAME + WATCH_FRAMES)
	{
		// Everything created since the last frame, then the flying things (every frame for the first
		// 5 seconds, then every third of a second).
		static ObjectID lastSeen = INVALID_ID;
		for (Object *obj = TheGameLogic->getFirstObject(); obj != nullptr; obj = obj->getNextObject())
		{
			if (lastSeen != INVALID_ID && obj->getID() > lastSeen)
				DEBUG_LOG(("IbmEngine demoStrike:   frame %u new %s #%u", frame, obj->getTemplate()->getName().str(), obj->getID()));
		}
		for (Object *obj = TheGameLogic->getFirstObject(); obj != nullptr; obj = obj->getNextObject())
			lastSeen = max(lastSeen, obj->getID());
		if (frame < FIRE_FRAME + 150 || frame % 10 == 0)
			logFlyingThings();
	}
	else if (frame == FIRE_FRAME + WATCH_FRAMES)
		TheGameEngine->setQuitting(TRUE);
}

#else

void IbmEngine::updateDemoTest()
{
}

#endif
