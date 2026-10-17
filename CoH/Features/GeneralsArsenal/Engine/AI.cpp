// CoH GeneralsArsenal: the skirmish AI's General's powers. Its scripts still use the powers they know
// (A-10 strike, paradrop, artillery and so on), and the specialPowerUse veto makes them pay for them.
// This uses the rest, the powers the pack adds: every few seconds each AI player picks at most one
// (AiChoice) and aims it, a super weapon at the richest spot of an enemy base, a strike at the
// richest enemy group (or base), air cover over its units that are fighting what the cover is for.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/SpecialPower.h"
#include "Common/ThingTemplate.h"
#include "GameClient/ControlBar.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Module/SpecialPowerModule.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"

#include "Kernel/Log.h"

#include "Features/GeneralsArsenal/Engine/Config.h"
#include "Features/GeneralsArsenal/Engine/Internal.h"

namespace CoH::GeneralsArsenal
{

namespace
{

struct AiPower
{
	const SpecialPowerTemplate *power;
	const PowerConfig *settings;
	const Object *source;
	Coord3D target;
};

struct PowerSearch
{
	std::vector<const SpecialPowerTemplate *> powers;
};

void collectPowers(Object *object, void *userData)
{
	PowerSearch *search = (PowerSearch *)userData;
	for (BehaviorModule **module = object->getBehaviorModules(); *module; ++module)
	{
		const SpecialPowerModuleInterface *power = (*module)->getSpecialPower();
		if (power == nullptr)
			continue;
		const SpecialPowerTemplate *powerTemplate = power->getSpecialPowerTemplate();
		const PowerConfig *settings = powerConfig(powerTemplate);
		if (settings == nullptr || aiUseOf(*settings) == AiUse::None)
			continue;
		if (std::find(search->powers.begin(), search->powers.end(), powerTemplate) == search->powers.end())
			search->powers.push_back(powerTemplate);
	}
}

bool isEnemyPlayer(const Player *player, const Player *other)
{
	return other != player && other->isPlayableSide() && other->isPlayerActive() &&
		player->getRelationship(other->getDefaultTeam()) == ENEMIES;
}

// ---- Targets ------------------------------------------------------------------------------------

struct Unit
{
	Coord3D position;
	Int value;
};

struct UnitSearch
{
	const Player *owner;
	std::vector<Unit> units;
};

bool isStrikeable(const Object *object)
{
	if (object->isEffectivelyDead() || object->isKindOf(KINDOF_STRUCTURE) || object->isKindOf(KINDOF_IGNORED_IN_GUI))
		return false;
	if (object->isAirborneTarget())
		return false;	// gone before the strike lands
	return object->isKindOf(KINDOF_VEHICLE) || object->isKindOf(KINDOF_INFANTRY);
}

void collectStrikeable(Object *object, void *userData)
{
	UnitSearch *search = (UnitSearch *)userData;
	if (isStrikeable(object))
		search->units.push_back({ *object->getPosition(), object->getTemplate()->calcCostToBuild(search->owner) });
}

// The spot where enemy units are worth the most within the strike's radius.
bool findEnemyGroup(const Player *player, Real radius, Coord3D &target)
{
	UnitSearch search;
	for (Int i = 0; i < ThePlayerList->getPlayerCount(); ++i)
	{
		const Player *other = ThePlayerList->getNthPlayer(i);
		if (other && isEnemyPlayer(player, other))
		{
			search.owner = other;
			other->iterateObjects(collectStrikeable, &search);
		}
	}

	const Real radiusSq = radius * radius;
	Int bestValue = 0;
	for (const Unit &center : search.units)
	{
		Int value = 0;
		for (const Unit &other : search.units)
		{
			const Real dx = other.position.x - center.position.x;
			const Real dy = other.position.y - center.position.y;
			if (dx * dx + dy * dy <= radiusSq)
				value += other.value;
		}
		if (value > bestValue)
		{
			bestValue = value;
			target = center.position;
		}
	}
	return bestValue >= config().aiStrikeMinValue;
}

bool findEnemyBase(Player *player, const SpecialPowerTemplate *power, Real radius, Coord3D &target)
{
	for (Int i = 0; i < ThePlayerList->getPlayerCount(); ++i)
	{
		const Player *other = ThePlayerList->getNthPlayer(i);
		if (other == nullptr || !isEnemyPlayer(player, other))
			continue;
		if (player->computeSuperweaponTarget(power, &target, other->getPlayerIndex(), radius) && target.lengthSqr() > 0.0f)
			return true;
	}
	return false;
}

struct CoverSearch
{
	const KindOfMaskType *threatKinds;
	Int bestThreats;
	Coord3D target;
};

// One of the AI's own fighting units, with enemies the cover is for near it.
void considerCover(Object *object, void *userData)
{
	CoverSearch *search = (CoverSearch *)userData;
	if (object->isEffectivelyDead() || object->isKindOf(KINDOF_STRUCTURE) || !object->isKindOf(KINDOF_CAN_ATTACK))
		return;

	PartitionFilterRelationship enemies(object, PartitionFilterRelationship::ALLOW_ENEMIES);
	PartitionFilterAlive alive;
	PartitionFilter *filters[] = { &enemies, &alive, nullptr };
	ObjectIterator *iter = ThePartitionManager->iterateObjectsInRange(object, config().aiCoverRadius, FROM_CENTER_2D, filters);
	MemoryPoolObjectHolder hold(iter);
	Int threats = 0;
	for (Object *other = iter->first(); other; other = iter->next())
	{
		if (other->isAnyKindOf(*search->threatKinds) && !other->isKindOf(KINDOF_STRUCTURE))
			++threats;
	}
	if (threats > search->bestThreats)
	{
		search->bestThreats = threats;
		search->target = *object->getPosition();
	}
}

bool findArmyFight(const Player *player, const KindOfMaskType &threatKinds, Coord3D &target)
{
	CoverSearch search = { &threatKinds, 0, Coord3D() };
	player->iterateObjects(considerCover, &search);
	target = search.target;
	return search.bestThreats > 0;
}

bool findTarget(Player *player, AiPower &power)
{
	const Real radius = std::max(50.0f, power.power->getRadiusCursorRadius());
	switch (aiUseOf(*power.settings))
	{
		case AiUse::Superweapon:
			return findEnemyBase(player, power.power, radius, power.target);
		case AiUse::Strike:
			return findEnemyGroup(player, std::max(radius, config().aiStrikeRadius), power.target) ||
				findEnemyBase(player, power.power, radius, power.target);
		case AiUse::Cover:
			return findArmyFight(player, power.settings->aiTargetKinds, power.target);
		default:
			return false;
	}
}

// The powers the AI may pick from, with whether it can use each now. A target is looked for only
// for the power it picks (thinkFor), so each usable power counts as having one until then.
void gatherCandidates(Player *player, std::vector<AiPower> &powers, std::vector<AiCandidate> &candidates)
{
	PowerSearch search;
	player->iterateObjects(collectPowers, &search);

	for (const SpecialPowerTemplate *powerTemplate : search.powers)
	{
		AiPower power = { powerTemplate, powerConfig(powerTemplate), findPowerSource(player, powerTemplate), Coord3D() };
		if (power.source == nullptr)
			continue;
		const PlayerStanding standing = standingFor(player, *power.settings, power.source, powerTemplate);

		AiCandidate candidate;
		candidate.use = aiUseOf(*power.settings);
		candidate.cost = power.settings->rule.cost;
		candidate.allowed = isAllowed(power.settings->rule, standing);
		candidate.ready = !standing.coolingDown;
		candidate.unlocked = standing.rank >= power.settings->rule.minRank;
		candidate.hasTarget = candidate.allowed && candidate.ready;
		powers.push_back(power);
		candidates.push_back(candidate);
	}
}

void thinkFor(Player *player)
{
	removeAIBuildListEntries(player);
	if (walletOf(player) == nullptr)
		return;

	std::vector<AiPower> powers;
	std::vector<AiCandidate> candidates;
	gatherCandidates(player, powers, candidates);

	AiSituation situation;
	situation.points = pointsOf(player);

	// The best usable power that has a target: one without is dropped and the next best tried.
	int chosen = -1;
	for (;;)
	{
		chosen = chooseAiPower(candidates, situation, config().aiPolicy);
		if (chosen < 0)
			return;
		if (findTarget(player, powers[chosen]))
			break;
		CoH::log("GeneralsArsenal: frame %u AI %ls has no target for %s", TheGameLogic->getFrame(),
			player->getPlayerDisplayName().str(), powers[chosen].power->getName().str());
		candidates[chosen].hasTarget = false;
	}

	AiPower &power = powers[chosen];
	CoH::log("GeneralsArsenal: frame %u AI %ls uses %s at (%.0f, %.0f) with %d points", TheGameLogic->getFrame(),
		player->getPlayerDisplayName().str(), power.power->getName().str(), power.target.x, power.target.y, situation.points);
	Object *source = const_cast<Object *>(power.source);
	source->doSpecialPowerAtLocation(power.power, &power.target, INVALID_ANGLE, COMMAND_FIRED_BY_SCRIPT, FALSE);
}

} // namespace

// Only what can be read without changing the game: looking for targets draws synced random numbers.
void logAIPowers()
{
	for (Int i = 0; i < ThePlayerList->getPlayerCount(); ++i)
	{
		Player *player = ThePlayerList->getNthPlayer(i);
		if (player == nullptr || player->getPlayerType() != PLAYER_COMPUTER || !player->isPlayerActive() ||
			walletOf(player) == nullptr)
			continue;
		std::vector<AiPower> powers;
		std::vector<AiCandidate> candidates;
		gatherCandidates(player, powers, candidates);
		AsciiString line;
		for (size_t k = 0; k < powers.size(); ++k)
		{
			AsciiString entry;
			entry.format(" %s[%s%s]", powers[k].power->getName().str(), candidates[k].allowed ? "allowed" : "refused",
				candidates[k].ready ? ",ready" : ",cooling");
			line.concat(entry);
		}
		CoH::log("GeneralsArsenal: frame %u AI %ls (%s) powers:%s", TheGameLogic->getFrame(),
			player->getPlayerDisplayName().str(), player->getSide().str(), line.isEmpty() ? " none" : line.str());
	}
}

// Each AI thinks every AIThinkInterval, on a frame of its own, so they don't all search at once.
void thinkForAIs()
{
	const UnsignedInt now = TheGameLogic->getFrame();
	const UnsignedInt interval = secondsToLogicFrames(config().aiThinkSeconds);
	const UnsignedInt firstUse = secondsToLogicFrames(config().aiFirstUseSeconds);
	for (Int i = 0; i < ThePlayerList->getPlayerCount(); ++i)
	{
		Player *player = ThePlayerList->getNthPlayer(i);
		if (player == nullptr || player->getPlayerType() != PLAYER_COMPUTER || !player->isPlayerActive() || !player->isPlayableSide())
			continue;
		const UnsignedInt offset = (UnsignedInt)player->getPlayerIndex() * LOGICFRAMES_PER_SECOND;
		if (now == 0 || (now + offset) % interval != 0)
			continue;
		if (now < firstUse)
			removeAIBuildListEntries(player);
		else
			thinkFor(player);
	}
}

} // namespace CoH::GeneralsArsenal
