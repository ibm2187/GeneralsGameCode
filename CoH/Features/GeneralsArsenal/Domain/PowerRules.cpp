// CoH GeneralsArsenal: see PowerRules.h.

#include "PowerRules.h"

namespace CoH::GeneralsArsenal
{

PowerState stateOf(const PowerRule &rule, const PlayerStanding &player)
{
	if (player.rank < rule.minRank)
		return PowerState::LockedByRank;
	if (rule.needsAirfield && !player.hasAirfield)
		return PowerState::NeedsAirfield;
	if (player.coolingDown)
		return PowerState::CoolingDown;
	if (player.points < rule.cost)
		return PowerState::TooExpensive;
	return PowerState::Ready;
}

bool isAllowed(const PowerRule &rule, const PlayerStanding &player)
{
	PlayerStanding ignoringCooldown = player;
	ignoringCooldown.coolingDown = false;
	return stateOf(rule, ignoringCooldown) == PowerState::Ready;
}

std::vector<std::string> sciencesDue(const std::vector<PowerLevel> &levels, int rank)
{
	std::vector<std::string> due;
	for (size_t i = 0; i < levels.size(); ++i)
	{
		if (i > 0 && levels[i].rank > rank)
			break;
		due.push_back(levels[i].science);
	}
	return due;
}

} // namespace CoH::GeneralsArsenal
