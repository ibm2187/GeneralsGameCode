// CoH GeneralsArsenal: General's powers paid with command points. The specialPowerUse veto refuses a
// power the player's rank hasn't opened, an air power without an airfield, or one the player can't
// pay for; a power that fires is paid for; a plane shot down lengthens its power's cooldown. The
// powers' sciences are no longer bought: a player has the first level of each power its buildings
// carry, and the higher levels come with rank.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/Science.h"
#include "Common/SpecialPower.h"
#include "Common/ThingTemplate.h"
#include "GameClient/GameText.h"
#include "GameClient/InGameUI.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Module/SpecialPowerModule.h"
#include "GameLogic/Object.h"

#include "Kernel/Log.h"

#include "Features/GeneralsArsenal/Domain/Cooldown.h"
#include "Features/GeneralsArsenal/Engine/Config.h"
#include "Features/GeneralsArsenal/Engine/Internal.h"
#include "Features/GeneralsArsenal/Engine/SortieLossDie.h"

namespace CoH::GeneralsArsenal
{

namespace
{

bool &airfieldFlag(Int playerIndex)
{
	static bool flags[MAX_PLAYER_COUNT] = {};
	static bool nobody = false;
	if (playerIndex < 0 || playerIndex >= MAX_PLAYER_COUNT)
		return nobody = false;
	return flags[playerIndex];
}

bool isStandingAirfield(const Object *object)
{
	return object->isAnyKindOf(config().airfieldKinds) && !object->testStatus(OBJECT_STATUS_UNDER_CONSTRUCTION) &&
		!object->testStatus(OBJECT_STATUS_SOLD) && !object->isEffectivelyDead();
}

void markAirfield(Object *object, void *found)
{
	if (isStandingAirfield(object))
		*(bool *)found = true;
}

bool isCoolingDown(const Object *source, const SpecialPowerTemplate *power)
{
	const SpecialPowerModuleInterface *module = source ? source->getSpecialPowerModule(power) : nullptr;
	return module != nullptr && !module->isReady();
}

void grantScience(Player *player, const std::string &name)
{
	const ScienceType science = TheScienceStore->getScienceFromInternalName(name.c_str());
	if (science == SCIENCE_INVALID || player->hasScience(science) || !TheScienceStore->isScienceGrantable(science))
		return;
	if (player->grantScience(science))
		CoH::log("GeneralsArsenal: frame %u %ls has %s", TheGameLogic->getFrame(), player->getPlayerDisplayName().str(), name.c_str());
}

struct GrantContext
{
	Player *player;
};

// The sciences due for each General's power module on one of the player's objects.
void grantForObject(Object *object, void *userData)
{
	Player *player = ((GrantContext *)userData)->player;
	for (BehaviorModule **module = object->getBehaviorModules(); *module; ++module)
	{
		const SpecialPowerModuleInterface *power = (*module)->getSpecialPower();
		const PowerConfig *settings = power ? powerConfig(power->getSpecialPowerTemplate()) : nullptr;
		if (settings == nullptr)
			continue;
		for (const std::string &science : sciencesDue(settings->rule.levels, player->getRankLevel()))
			grantScience(player, science);
	}
}

// The power fired last, so the planes its OCL makes in the same moment can be told it sent them.
struct LastTrigger
{
	Int playerIndex = -1;
	const SpecialPowerTemplate *power = nullptr;
	UnsignedInt frame = 0;
};

LastTrigger &lastTrigger()
{
	static LastTrigger trigger;
	return trigger;
}

void tellOwner(const Player *player, const UnicodeString &text)
{
	if (player != nullptr && player == ThePlayerList->getLocalPlayer() && TheInGameUI != nullptr)
		TheInGameUI->messageNoFormat(text);
}

} // namespace

void refreshAirfields()
{
	for (Int i = 0; i < ThePlayerList->getPlayerCount(); ++i)
	{
		const Player *player = ThePlayerList->getNthPlayer(i);
		bool found = false;
		if (player)
			player->iterateObjects(markAirfield, &found);
		airfieldFlag(player ? player->getPlayerIndex() : -1) = found;
	}
}

bool hasAirfield(const Player *player)
{
	return player != nullptr && airfieldFlag(player->getPlayerIndex());
}

PlayerStanding standingFor(const Player *player, const PowerConfig &power, const Object *source,
	const SpecialPowerTemplate *powerTemplate)
{
	PlayerStanding standing;
	standing.rank = player->getRankLevel();
	standing.points = pointsOf(player);
	standing.hasAirfield = !power.rule.needsAirfield || hasAirfield(player);
	standing.coolingDown = isCoolingDown(source, powerTemplate);
	return standing;
}

bool allowsPower(const Object *source, const SpecialPowerTemplate *power)
{
	const PowerConfig *settings = powerConfig(power);
	const Player *player = source ? source->getControllingPlayer() : nullptr;
	if (settings == nullptr || player == nullptr || walletOf(player) == nullptr)
		return true;
	return isAllowed(settings->rule, standingFor(player, *settings, source, power));
}

// A power the veto let through is paid in full. One fired without asking (a script forcing it) is
// charged what the player has.
void payForPower(Object *source, const SpecialPowerTemplate *power, const Coord3D * /*target*/)
{
	const PowerConfig *settings = powerConfig(power);
	Player *player = source ? source->getControllingPlayer() : nullptr;
	Wallet *wallet = walletOf(player);
	if (settings == nullptr || wallet == nullptr)
		return;

	// A power some objects carry in two modules (the Spectre gunship) fires both at once: paid once.
	const LastTrigger &last = lastTrigger();
	if (last.power == power && last.playerIndex == player->getPlayerIndex() && last.frame == TheGameLogic->getFrame())
		return;
	lastTrigger() = { player->getPlayerIndex(), power, TheGameLogic->getFrame() };

	if (!wallet->spend(settings->rule.cost))
		wallet->charge(settings->rule.cost);
	CoH::log("GeneralsArsenal: frame %u %ls used %s for %d, %d left", TheGameLogic->getFrame(),
		player->getPlayerDisplayName().str(), power->getName().str(), settings->rule.cost, wallet->whole());
}

void tagPlane(Object *object)
{
	const LastTrigger &trigger = lastTrigger();
	if (trigger.power == nullptr || trigger.frame != TheGameLogic->getFrame())
		return;
	const Player *owner = object->getControllingPlayer();
	if (owner == nullptr || owner->getPlayerIndex() != trigger.playerIndex)
		return;
	if (SortieLossDie *loss = SortieLossDie::find(object))
		loss->setSpecialPower(trigger.power);
}

void forgetLastTrigger()
{
	lastTrigger() = LastTrigger();
}

void grantPowerSciences()
{
	for (Int i = 0; i < ThePlayerList->getPlayerCount(); ++i)
	{
		Player *player = ThePlayerList->getNthPlayer(i);
		if (player == nullptr || !player->isPlayableSide())
			continue;
		GrantContext context = { player };
		player->iterateObjects(grantForObject, &context);
	}
}

void penalizeLoss(Player *player, const SpecialPowerTemplate *power, const Object *lost)
{
	const PowerConfig *settings = powerConfig(power);
	if (settings == nullptr || player == nullptr || settings->rule.lossPenaltySeconds <= 0.0f)
		return;
	if (!power->isSharedNSync())
		return;	// every General's power in the pack has a shared timer; a building's own isn't ours to change

	const UnsignedInt now = TheGameLogic->getFrame();
	const UnsignedInt ready = player->getOrStartSpecialPowerReadyFrame(power);
	const UnsignedInt penalty = secondsToLogicFrames(settings->rule.lossPenaltySeconds);
	const UnsignedInt newReady = readyFrameAfterLoss(ready, now, penalty, power->getReloadTime(),
		secondsToLogicFrames(config().maxLossDelaySeconds));
	player->expressSpecialPowerReadyFrame(power, newReady);

	CoH::log("GeneralsArsenal: frame %u %ls lost %s; %s ready at %u (was %u)", now, player->getPlayerDisplayName().str(),
		lost ? lost->getTemplate()->getName().str() : "a plane", power->getName().str(), newReady, ready);

	UnicodeString text;
	text.format(TheGameText->FETCH_OR_SUBSTITUTE("CoH:PlaneLost", L"Aircraft shot down: that power is ready %d s later").str(),
		(Int)settings->rule.lossPenaltySeconds);
	tellOwner(player, text);
}

namespace
{

struct SourceSearch
{
	const SpecialPowerTemplate *power;
	const Object *best;
	Real bestReady;
};

void considerSource(Object *object, void *userData)
{
	SourceSearch *search = (SourceSearch *)userData;
	if (object->testStatus(OBJECT_STATUS_UNDER_CONSTRUCTION) || object->isDisabled())
		return;
	const SpecialPowerModuleInterface *module = object->getSpecialPowerModule(search->power);
	if (module == nullptr)
		return;
	const Real ready = module->getPercentReady();
	if (search->best == nullptr || ready > search->bestReady)
	{
		search->best = object;
		search->bestReady = ready;
	}
}

} // namespace

const Object *findPowerSource(const Player *player, const SpecialPowerTemplate *power)
{
	if (player == nullptr || power == nullptr)
		return nullptr;
	SourceSearch search = { power, nullptr, 0.0f };
	player->iterateObjects(considerSource, &search);
	return search.best;
}

} // namespace CoH::GeneralsArsenal
