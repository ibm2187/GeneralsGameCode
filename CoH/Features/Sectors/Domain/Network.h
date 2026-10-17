// CoH Sectors: supply lines and income. A sector pays its owner only while a chain of the owner's own
// sectors joins it to one of the owner's roots (the home sector, or a sector with a command center).
// A held sector that isn't joined is cut off.

#pragma once

#include "Types.h"

#include <vector>

namespace CoH::Sectors
{

// For each sector, whether 'player' owns it and reaches it from one of 'roots' through the player's
// own sectors. Roots the player doesn't own reach nothing.
std::vector<bool> findConnected(const std::vector<std::vector<int>> &neighbors, const std::vector<PlayerId> &owners,
	PlayerId player, const std::vector<int> &roots);

struct IncomeValues
{
	int supply = 60;
	int highValue = 120;
	int home = 60;
};

int sectorValue(SectorKind kind, const IncomeValues &values);

// What the connected sectors pay in one income period.
int incomeFor(const std::vector<SectorKind> &kinds, const std::vector<bool> &connected, const IncomeValues &values);

// One period's income, given as money per minute.
int incomePerMinute(int incomePerPeriod, float periodSeconds);

} // namespace CoH::Sectors
