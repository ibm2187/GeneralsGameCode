// CoH Upkeep: tests for population costs and the cap.

#include <gtest/gtest.h>

#include "Features/Upkeep/Domain/Population.h"

using namespace CoH::Upkeep;

TEST(UpkeepPopulation, CostFollowsTheBuildCost)
{
	const CostRules rules;	// 250 per population, at least 1
	EXPECT_EQ(populationCost(225, -1, rules), 1);	// Ranger
	EXPECT_EQ(populationCost(900, -1, rules), 4);	// Crusader: 3.6 rounds to 4
	EXPECT_EQ(populationCost(2000, -1, rules), 8);	// Overlord
	EXPECT_EQ(populationCost(100, -1, rules), 1);	// cheap units take the minimum
}

TEST(UpkeepPopulation, FreeUnitsTakeNothing)
{
	const CostRules rules;
	EXPECT_EQ(populationCost(0, -1, rules), 0);
	CostRules noScale;
	noScale.buildCostPerPopulation = 0;
	EXPECT_EQ(populationCost(900, -1, noScale), 0);
}

TEST(UpkeepPopulation, ThePackOverridesTheCost)
{
	const CostRules rules;
	EXPECT_EQ(populationCost(2000, 3, rules), 3);
	EXPECT_EQ(populationCost(2000, 0, rules), 0);
	EXPECT_EQ(populationCost(0, 2, rules), 2);
}

TEST(UpkeepCap, TheCapRisesWithBuildingsAndSectors)
{
	const CapRules rules;	// 10 base, 3 per sector, 150 most
	EXPECT_EQ(populationCap(0, 0, rules), 10);
	EXPECT_EQ(populationCap(10, 0, rules), 20);
	EXPECT_EQ(populationCap(10, 4, rules), 32);
}

TEST(UpkeepCap, TheCapStopsAtItsMost)
{
	const CapRules rules;
	EXPECT_EQ(populationCap(100, 30, rules), 150);
}

TEST(UpkeepCap, NegativeBonusesCountAsNone)
{
	const CapRules rules;
	EXPECT_EQ(populationCap(-5, -2, rules), 10);
}

TEST(UpkeepCap, AUnitMustFitUnderTheCap)
{
	EXPECT_TRUE(fitsUnderCap(16, 4, 20));
	EXPECT_FALSE(fitsUnderCap(17, 4, 20));
	EXPECT_TRUE(fitsUnderCap(25, 0, 20));	// free units always fit, even over the cap
	EXPECT_FALSE(fitsUnderCap(25, 1, 20));
}
