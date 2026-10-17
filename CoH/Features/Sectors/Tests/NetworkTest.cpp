// CoH Sectors: tests for the cut-off graph walk, income and the build rule.

#include <gtest/gtest.h>

#include "Features/Sectors/Domain/BuildRule.h"
#include "Features/Sectors/Domain/Network.h"

using namespace CoH::Sectors;

namespace
{

// A row of five sectors: 0 - 1 - 2 - 3 - 4.
std::vector<std::vector<int>> row()
{
	return { { 1 }, { 0, 2 }, { 1, 3 }, { 2, 4 }, { 3 } };
}

} // namespace

TEST(SectorsNetwork, AChainOfOwnSectorsReachesTheRoot)
{
	const std::vector<PlayerId> owners = { 0, 0, 0, NoPlayer, 0 };
	const std::vector<bool> connected = findConnected(row(), owners, 0, { 0 });
	EXPECT_EQ(connected, (std::vector<bool>{ true, true, true, false, false }));	// 4 is cut off
}

TEST(SectorsNetwork, AnEnemySectorCutsTheLine)
{
	const std::vector<PlayerId> owners = { 0, 1, 0, 0, 0 };
	EXPECT_EQ(findConnected(row(), owners, 0, { 0 }), (std::vector<bool>{ true, false, false, false, false }));
}

TEST(SectorsNetwork, EveryRootFeedsTheNetwork)
{
	const std::vector<PlayerId> owners = { 0, NoPlayer, 0, 0, NoPlayer };
	EXPECT_EQ(findConnected(row(), owners, 0, { 0, 3 }), (std::vector<bool>{ true, false, true, true, false }));
}

TEST(SectorsNetwork, ALostRootReachesNothing)
{
	const std::vector<PlayerId> owners = { 1, 0, 0, 0, 0 };
	EXPECT_EQ(findConnected(row(), owners, 0, { 0 }), std::vector<bool>(5, false));
	EXPECT_EQ(findConnected(row(), owners, 0, {}), std::vector<bool>(5, false));
}

TEST(SectorsNetwork, OnlyConnectedSectorsPay)
{
	const IncomeValues values = { 10, 30, 5 };
	const std::vector<SectorKind> kinds = { SectorKind::Home, SectorKind::Supply, SectorKind::HighValue, SectorKind::Supply };
	EXPECT_EQ(incomeFor(kinds, { true, true, true, false }, values), 45);
	EXPECT_EQ(incomeFor(kinds, { false, false, false, false }, values), 0);
}

TEST(SectorsNetwork, IncomeIsShownPerMinute)
{
	EXPECT_EQ(incomePerMinute(100, 10.0f), 600);
	EXPECT_EQ(incomePerMinute(100, 0.0f), 0);
}

TEST(SectorsBuildRule, OwnSectorsOnly)
{
	const std::vector<PlayerId> owners = { 0, NoPlayer, 1, 0, NoPlayer };
	BuildRules rules;
	rules.defensesOnNeutralNeighbors = false;

	EXPECT_TRUE(mayBuildIn(0, 0, false, owners, row(), rules));
	EXPECT_FALSE(mayBuildIn(1, 0, false, owners, row(), rules));
	EXPECT_FALSE(mayBuildIn(2, 0, true, owners, row(), rules));
	EXPECT_FALSE(mayBuildIn(-1, 0, false, owners, row(), rules));
}

TEST(SectorsBuildRule, DefensesMayGoOnNeutralGroundNextToOwnSectors)
{
	const std::vector<PlayerId> owners = { 0, NoPlayer, 1, NoPlayer, NoPlayer };
	const BuildRules rules;

	EXPECT_TRUE(mayBuildIn(1, 0, true, owners, row(), rules));
	EXPECT_FALSE(mayBuildIn(1, 0, false, owners, row(), rules));	// not a defense
	EXPECT_FALSE(mayBuildIn(4, 0, true, owners, row(), rules));		// not next to an owned sector
	EXPECT_FALSE(mayBuildIn(2, 0, true, owners, row(), rules));		// an enemy's
}

TEST(SectorsBuildRule, CanBeSwitchedOff)
{
	BuildRules rules;
	rules.ownedSectorsOnly = false;
	EXPECT_TRUE(mayBuildIn(2, 0, false, { 0, NoPlayer, 1 }, row(), rules));
}
