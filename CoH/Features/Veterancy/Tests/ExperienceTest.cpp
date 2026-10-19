// CoH Veterancy: tests for experience from damage, the squad pool, and swapping the engine's rank
// bonuses.

#include <gtest/gtest.h>

#include "Features/Veterancy/Domain/Experience.h"

#include <vector>

using namespace CoH::Veterancy;

TEST(VeterancyExperience, DamageEarnsItsShareOfTheKillValue)
{
	// A Crusader (kill value 100, 480 health) half destroyed at a 50% share: 25 points.
	EXPECT_FLOAT_EQ(experienceFromDamage(0.5f, 240.0f, 480.0f, 100), 25.0f);
	// A full health's worth earns the share, and no more.
	EXPECT_FLOAT_EQ(experienceFromDamage(0.5f, 480.0f, 480.0f, 100), 50.0f);
	EXPECT_FLOAT_EQ(experienceFromDamage(0.5f, 5000.0f, 480.0f, 100), 50.0f);
}

TEST(VeterancyExperience, NothingIsEarnedFromNothing)
{
	EXPECT_FLOAT_EQ(experienceFromDamage(0.0f, 240.0f, 480.0f, 100), 0.0f);
	EXPECT_FLOAT_EQ(experienceFromDamage(0.5f, 0.0f, 480.0f, 100), 0.0f);
	EXPECT_FLOAT_EQ(experienceFromDamage(0.5f, 240.0f, 0.0f, 100), 0.0f);
	EXPECT_FLOAT_EQ(experienceFromDamage(0.5f, 240.0f, 480.0f, 0), 0.0f);
}

TEST(VeterancyExperience, SmallHitsAddUpToWholePoints)
{
	// A rifle's 10 damage on a 200 health tank worth 20: 0.5 points a hit.
	float carry = 0.0f;
	const float hit = experienceFromDamage(0.5f, 10.0f, 200.0f, 20);
	int total = 0;
	for (int i = 0; i < 9; ++i)
		total += wholePoints(carry, hit);
	EXPECT_EQ(total, 4);
	EXPECT_FLOAT_EQ(carry, 0.5f);
	EXPECT_EQ(wholePoints(carry, hit), 1);
	EXPECT_FLOAT_EQ(carry, 0.0f);
	EXPECT_EQ(wholePoints(carry, 0.0f), 0);
}

TEST(VeterancyExperience, ASquadPoolsWhatEachMemberEarned)
{
	// Three members at 40 after the last share; one killed a soldier (+20), one dealt damage (+5).
	const std::vector<MemberExperience> members = {
		{ .known = true, .seen = 40, .now = 60 },
		{ .known = true, .seen = 40, .now = 45 },
		{ .known = true, .seen = 40, .now = 40 },
	};
	EXPECT_EQ(squadPool(members), 65);
}

TEST(VeterancyExperience, ANewMemberJoinsAtThePool)
{
	const std::vector<MemberExperience> members = {
		{ .known = true, .seen = 120, .now = 120 },
		{ .known = false, .seen = 0, .now = 0 },	// a reinforcement
	};
	EXPECT_EQ(squadPool(members), 120);
}

TEST(VeterancyExperience, ANewOrLoadedSquadTakesItsBestMember)
{
	const std::vector<MemberExperience> members = {
		{ .known = false, .now = 0 },
		{ .known = false, .now = 60 },	// made veteran by a science
		{ .known = false, .now = 0 },
	};
	EXPECT_EQ(squadPool(members), 60);
	EXPECT_EQ(squadPool(std::vector<MemberExperience> {}), 0);
}

TEST(VeterancyExperience, SwappingTheRankBonusDoesntCareAboutOrder)
{
	// Veteran rate of fire: the engine's x1.2 in a total with another +25% bonus, ours x1.15.
	const float engineRank = 1.2f;
	const float ours = 1.15f;
	const float total = 1.0f + 0.2f + 0.25f;
	const float fireRate = 3.0f;	// FireRate slows the weapon three times

	const float swapFirst = swapRankBonus(total, engineRank, ours) / fireRate;
	const float swapAfter = swapRankBonus(total / fireRate, engineRank, ours);
	EXPECT_FLOAT_EQ(swapFirst, swapAfter);

	// Alone, the rank's reload is exactly ours.
	EXPECT_FLOAT_EQ(swapRankBonus(engineRank, engineRank, ours), ours);
	// A regular has no engine rank bonus: x1 is swapped for its own.
	EXPECT_FLOAT_EQ(swapRankBonus(1.0f, 1.0f, 1.0f), 1.0f);
}

TEST(VeterancyExperience, ARankChangeScalesByTheRatio)
{
	EXPECT_FLOAT_EQ(rankChange(1.0f, 1.1f), 1.1f);
	EXPECT_FLOAT_EQ(rankChange(1.1f, 1.2f), 1.2f / 1.1f);
	EXPECT_FLOAT_EQ(rankChange(1.2f, 1.0f), 1.0f / 1.2f);	// a rank lost
	EXPECT_FLOAT_EQ(rankChange(0.0f, 1.2f), 1.0f);
	// Up and back down leaves a sight that a battle plan also scaled where it was.
	EXPECT_FLOAT_EQ(300.0f * 1.2f * rankChange(1.0f, 1.15f) * rankChange(1.15f, 1.0f), 360.0f);
}
