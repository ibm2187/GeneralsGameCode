// CoH Sectors: which sectors the skirmish AI goes for, best first. It defends its contested sectors,
// takes the neighbors of its territory (first those that join cut-off sectors back up), prefers
// high-value and near ones, and retakes what it lost.

#pragma once

#include "Types.h"

#include <vector>

namespace CoH::Sectors
{

// A sector as one AI player sees it.
struct AiSector
{
	Vec2 position;
	SectorKind kind = SectorKind::Supply;
	PlayerId owner = NoPlayer;
	bool ownerIsFriend = false;	///< an ally owns it: not a target
	bool locked = false;		///< an enemy home whose command center stands: can't be taken
	bool threatened = false;	///< enemies are on it: contested, or taking it
};

struct AiWeights
{
	float defend = 1000.0f;			///< one of ours, with enemies on it
	float reconnect = 400.0f;		///< joins one of our cut-off sectors back up
	float frontier = 300.0f;		///< borders our connected territory
	float highValue = 150.0f;
	float enemyOwned = -50.0f;		///< held by an enemy, so probably defended
	float distancePer1000 = -200.0f;	///< per 1000 world units from the AI's base
};

// The sectors worth sending units to, best first. 'connected' is the player's connected sectors.
std::vector<int> rankTargets(PlayerId player, const std::vector<AiSector> &sectors,
	const std::vector<std::vector<int>> &neighbors, const std::vector<bool> &connected, Vec2 base,
	const AiWeights &weights);

} // namespace CoH::Sectors
