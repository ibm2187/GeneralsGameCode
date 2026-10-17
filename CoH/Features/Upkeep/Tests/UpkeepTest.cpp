// CoH Upkeep: tests for the upkeep steps and the income split.

#include <gtest/gtest.h>

#include "Features/Upkeep/Domain/Upkeep.h"

using namespace CoH::Upkeep;

namespace
{

UpkeepSteps exampleSteps()
{
	UpkeepSteps steps;
	steps.add({ 60, 25 });	// out of order on purpose
	steps.add({ 0, 0 });
	steps.add({ 100, 40 });
	steps.add({ 30, 10 });
	return steps;
}

} // namespace

TEST(UpkeepSteps, TheShareGrowsInSteps)
{
	const UpkeepSteps steps = exampleSteps();
	EXPECT_EQ(steps.percentAt(0), 0);
	EXPECT_EQ(steps.percentAt(29), 0);
	EXPECT_EQ(steps.percentAt(30), 10);
	EXPECT_EQ(steps.percentAt(59), 10);
	EXPECT_EQ(steps.percentAt(60), 25);
	EXPECT_EQ(steps.percentAt(100), 40);
	EXPECT_EQ(steps.percentAt(500), 40);
}

TEST(UpkeepSteps, NoStepsMeansNoUpkeep)
{
	const UpkeepSteps steps;
	EXPECT_EQ(steps.percentAt(200), 0);
	EXPECT_EQ(steps.nextStepPopulation(0), -1);
}

TEST(UpkeepSteps, BelowTheFirstStepNothingIsTaken)
{
	UpkeepSteps steps;
	steps.add({ 20, 15 });
	EXPECT_EQ(steps.percentAt(19), 0);
	EXPECT_EQ(steps.percentAt(20), 15);
}

TEST(UpkeepSteps, ASecondStepAtTheSamePopulationReplacesTheFirst)
{
	UpkeepSteps steps = exampleSteps();
	steps.add({ 30, 5 });
	EXPECT_EQ(steps.steps().size(), 4u);
	EXPECT_EQ(steps.percentAt(30), 5);
}

TEST(UpkeepSteps, TheNextStepIsTheOneAbove)
{
	const UpkeepSteps steps = exampleSteps();
	EXPECT_EQ(steps.nextStepPopulation(0), 30);
	EXPECT_EQ(steps.nextStepPopulation(30), 60);
	EXPECT_EQ(steps.nextStepPopulation(99), 100);
	EXPECT_EQ(steps.nextStepPopulation(100), -1);
}

TEST(UpkeepIncome, UpkeepIsRoundedDown)
{
	const IncomeSplit split = splitIncome(75, 25);
	EXPECT_EQ(split.gross, 75);
	EXPECT_EQ(split.upkeep, 18);
	EXPECT_EQ(split.net, 57);
}

TEST(UpkeepIncome, TheShareIsHeldBetweenNothingAndAll)
{
	EXPECT_EQ(splitIncome(100, -10).upkeep, 0);
	EXPECT_EQ(splitIncome(100, 150).net, 0);
	EXPECT_EQ(splitIncome(-5, 40).gross, 0);
}

TEST(UpkeepAI, TheAIStopsBeforeAStepAboveItsLimit)
{
	const UpkeepSteps steps = exampleSteps();
	EXPECT_TRUE(aiMayGrowTo(99, steps, 25));
	EXPECT_FALSE(aiMayGrowTo(100, steps, 25));
	EXPECT_FALSE(aiMayGrowTo(30, steps, 0));
	EXPECT_TRUE(aiMayGrowTo(29, steps, 0));
}
