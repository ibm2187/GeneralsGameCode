// CoH VehicleMovement: each vehicle's weight, from its Unit line and its upgrades. See Internal.h.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Player.h"
#include "Common/ThingTemplate.h"
#include "Common/Upgrade.h"
#include "GameLogic/Module/BehaviorModule.h"
#include "GameLogic/Module/UpgradeModule.h"
#include "GameLogic/Object.h"

#include "Kernel/Log.h"

#include "Features/VehicleMovement/Domain/Names.h"
#include "Features/VehicleMovement/Engine/Config.h"
#include "Features/VehicleMovement/Engine/Internal.h"

#include <map>
#include <optional>
#include <unordered_map>

namespace CoH::VehicleMovement
{

namespace
{

// What a vehicle's movement was worked out from: when one of these changes, it is worked out again.
struct Known
{
	const ThingTemplate *thing = nullptr;
	UpgradeMaskType ownUpgrades;
	UpgradeMaskType playerUpgrades;
	Movement movement;
};

// Looked up only, never walked, so neither order matters to the game.
std::map<ObjectID, Known> &knownVehicles()
{
	static std::map<ObjectID, Known> theVehicles;
	return theVehicles;
}

std::unordered_map<const ThingTemplate *, std::optional<float>> &knownBaseWeights()
{
	static std::unordered_map<const ThingTemplate *, std::optional<float>> theWeights;
	return theWeights;
}

// The Upgrade lines with their templates, found once a game.
struct WeighedUpgrade
{
	const UpgradeTemplate *upgrade = nullptr;
	float tons = 0.0f;
};

std::optional<std::vector<WeighedUpgrade>> &knownUpgrades()
{
	static std::optional<std::vector<WeighedUpgrade>> theUpgrades;
	return theUpgrades;
}

const std::vector<WeighedUpgrade> &weighedUpgrades()
{
	std::optional<std::vector<WeighedUpgrade>> &upgrades = knownUpgrades();
	if (!upgrades)
	{
		upgrades.emplace();
		for (const UpgradeLine &line : config().upgrades)
		{
			const UpgradeTemplate *upgrade = TheUpgradeCenter->findUpgrade(line.name);
			DEBUG_ASSERTCRASH(upgrade != nullptr, ("CoHVehicleMovement: unknown upgrade %s", line.name.str()));
			if (upgrade != nullptr)
				upgrades->push_back({ upgrade, line.tons });
		}
	}
	return *upgrades;
}

std::optional<float> lookUpBaseWeight(const ThingTemplate &thing)
{
	const Config &c = config();
	if (!thing.isAnyKindOf(c.vehicles) || thing.isAnyKindOf(c.exempt))
		return std::nullopt;
	const UnitLine *best = nullptr;
	int bestMatch = 0;
	for (const UnitLine &line : c.units)
	{
		const int match = nameMatch(line.name.str(), thing.getName().str());
		if (match > bestMatch)
		{
			best = &line;
			bestMatch = match;
		}
	}
	if (best == nullptr)
		return classTons(c.defaultClass);
	return best->tons >= 0.0f ? best->tons : classTons(best->weightClass);
}

std::optional<float> baseWeightOf(const ThingTemplate &thing)
{
	auto [place, isNew] = knownBaseWeights().try_emplace(&thing);
	if (isNew)
		place->second = lookUpBaseWeight(thing);
	return place->second;
}

// Does one of the object's upgrade modules answer to this upgrade (Composite Armor's ArmorUpgrade)?
// A player upgrade only weighs on the vehicles it changes. testUpgradeConditions also says yes for a
// module with no triggers at all (it only has conflicts), so a module that answers to no upgrade
// doesn't count.
bool answersTo(const Object &object, const UpgradeMaskType &upgrade)
{
	const UpgradeMaskType none;
	for (BehaviorModule **module = object.getBehaviorModules(); *module != nullptr; ++module)
	{
		const UpgradeModuleInterface *upgradeModule = (*module)->getUpgrade();
		if (upgradeModule != nullptr && upgradeModule->testUpgradeConditions(upgrade)
			&& !upgradeModule->testUpgradeConditions(none))
			return true;
	}
	return false;
}

float upgradeWeightOf(const Object &object, const UpgradeMaskType &own, const UpgradeMaskType &player)
{
	float tons = 0.0f;
	for (const WeighedUpgrade &line : weighedUpgrades())
	{
		const UpgradeMaskType &mask = line.upgrade->getUpgradeMask();
		const bool owned = own.testForAny(mask);
		const bool fromPlayer = line.upgrade->getUpgradeType() == UPGRADE_TYPE_PLAYER && player.testForAny(mask)
			&& answersTo(object, mask);
		if (owned || fromPlayer)
			tons += line.tons;
	}
	return tons;
}

UpgradeMaskType playerUpgradesOf(const Object &object)
{
	const Player *owner = object.getControllingPlayer();
	return owner != nullptr ? owner->getCompletedUpgradeMask() : UpgradeMaskType();
}

void logMovement(const Object &object, const Movement &movement)
{
	if (!isLogging())
		return;
	const Handling &h = movement.handling;
	log("VehicleMovement: %s %u weighs %.1f t: speed %.0f%%, acceleration %.0f%%, braking %.0f%%, turn %.0f%%, "
		"uphill %.0f%% slower per 100%% grade", object.getTemplate()->getName().str(), (unsigned)object.getID(), movement.weight,
		h.speed * 100.0f, h.acceleration * 100.0f, h.braking * 100.0f, h.turnRate * 100.0f, h.slopePenalty * 100.0f);
}

} // namespace

const Movement *movementOf(const Object &object)
{
	const ThingTemplate *thing = object.getTemplate();
	const std::optional<float> base = baseWeightOf(*thing);
	if (!base)
		return nullptr;

	const UpgradeMaskType &own = object.getObjectCompletedUpgradeMask();
	const UpgradeMaskType player = playerUpgradesOf(object);
	auto [place, isNew] = knownVehicles().try_emplace(object.getID());
	Known &known = place->second;
	if (isNew || known.thing != thing || known.ownUpgrades != own || known.playerUpgrades != player)
	{
		known.thing = thing;
		known.ownUpgrades = own;
		known.playerUpgrades = player;
		known.movement.weight = totalWeight(*base, { upgradeWeightOf(object, own, player) });
		known.movement.handling = handlingFor(known.movement.weight, config().curves);
		logMovement(object, known.movement);
	}
	return &known.movement;
}

void forgetObject(ObjectID id)
{
	knownVehicles().erase(id);
}

void forgetAll()
{
	knownVehicles().clear();
	knownBaseWeights().clear();
	knownUpgrades().reset();
}

} // namespace CoH::VehicleMovement
