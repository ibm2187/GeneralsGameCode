// CoH GeneralsArsenal: tests for the rank gates, the airfield rule, cost, cooldown and the AI's choice.

#include <gtest/gtest.h>

#include "Features/GeneralsArsenal/Domain/AiChoice.h"
#include "Features/GeneralsArsenal/Domain/Cooldown.h"
#include "Features/GeneralsArsenal/Domain/PowerRules.h"

using namespace CoH::GeneralsArsenal;

namespace
{

PowerRule airStrike()
{
	PowerRule rule;
	rule.cost = 40;
	rule.minRank = 3;
	rule.needsAirfield = true;
	return rule;
}

PlayerStanding standing(int rank, int points, bool airfield = true, bool cooling = false)
{
	PlayerStanding player;
	player.rank = rank;
	player.points = points;
	player.hasAirfield = airfield;
	player.coolingDown = cooling;
	return player;
}

} // namespace

TEST(GeneralsArsenalRules, RankOpensTheTier)
{
	EXPECT_EQ(stateOf(airStrike(), standing(1, 500)), PowerState::LockedByRank);
	EXPECT_EQ(stateOf(airStrike(), standing(2, 500)), PowerState::LockedByRank);
	EXPECT_EQ(stateOf(airStrike(), standing(3, 500)), PowerState::Ready);
	EXPECT_EQ(stateOf(airStrike(), standing(5, 500)), PowerState::Ready);
}

TEST(GeneralsArsenalRules, AirPowersNeedAnAirfield)
{
	EXPECT_EQ(stateOf(airStrike(), standing(3, 500, false)), PowerState::NeedsAirfield);

	PowerRule ground = airStrike();
	ground.needsAirfield = false;
	EXPECT_EQ(stateOf(ground, standing(3, 500, false)), PowerState::Ready);
}

TEST(GeneralsArsenalRules, EachUseIsPaid)
{
	EXPECT_EQ(stateOf(airStrike(), standing(3, 39)), PowerState::TooExpensive);
	EXPECT_EQ(stateOf(airStrike(), standing(3, 40)), PowerState::Ready);
}

TEST(GeneralsArsenalRules, RankIsCheckedBeforeCostAndCooldown)
{
	EXPECT_EQ(stateOf(airStrike(), standing(1, 0, false, true)), PowerState::LockedByRank);
	EXPECT_EQ(stateOf(airStrike(), standing(3, 0, true, true)), PowerState::CoolingDown);
}

TEST(GeneralsArsenalRules, TheCooldownIsTheEnginesClockNotARefusal)
{
	EXPECT_TRUE(isAllowed(airStrike(), standing(3, 40, true, true)));
	EXPECT_FALSE(isAllowed(airStrike(), standing(3, 39, true, true)));
}

TEST(GeneralsArsenalRules, LevelsComeWithRank)
{
	const std::vector<PowerLevel> levels = { { "A10Strike1", 3 }, { "A10Strike2", 4 }, { "A10Strike3", 5 } };
	EXPECT_EQ(sciencesDue(levels, 1), (std::vector<std::string>{ "A10Strike1" }));	// shown, locked by rank
	EXPECT_EQ(sciencesDue(levels, 4), (std::vector<std::string>{ "A10Strike1", "A10Strike2" }));
	EXPECT_EQ(sciencesDue(levels, 5).size(), 3u);
	EXPECT_TRUE(sciencesDue({}, 5).empty());
}

TEST(GeneralsArsenalRules, ALevelWaitsForTheOneBeforeIt)
{
	const std::vector<PowerLevel> levels = { { "L1", 1 }, { "L2", 5 }, { "L3", 3 } };
	EXPECT_EQ(sciencesDue(levels, 4), (std::vector<std::string>{ "L1" }));
}

TEST(GeneralsArsenalCooldown, ALossAddsToWhatIsLeft)
{
	// 30 s left (900 frames at 30 fps), 20 s penalty: ready 50 s from now.
	EXPECT_EQ(readyFrameAfterLoss(1900, 1000, 600, 3600, 9000), 2500u);
}

TEST(GeneralsArsenalCooldown, ALossDelaysAReadyPower)
{
	EXPECT_EQ(readyFrameAfterLoss(400, 1000, 600, 3600, 9000), 1600u);
}

TEST(GeneralsArsenalCooldown, ALossRightAfterALongCooldownStillCounts)
{
	// A 6 minute power just fired: a lost plane still adds its 15 s.
	EXPECT_EQ(readyFrameAfterLoss(1000 + 10800, 1000, 450, 10800, 9000), 1000u + 10800u + 450u);
}

TEST(GeneralsArsenalCooldown, LossesAddUpToTheCap)
{
	// A 2 minute power and a 100 s cap: at most 220 s from now.
	unsigned int ready = 1000;
	for (int plane = 0; plane < 20; ++plane)
		ready = readyFrameAfterLoss(ready, 1000, 600, 3600, 3000);
	EXPECT_EQ(ready, 1000u + 3600u + 3000u);
}

TEST(GeneralsArsenalCooldown, TheCapNeverShortensACooldown)
{
	EXPECT_EQ(readyFrameAfterLoss(1000 + 20000, 1000, 450, 3600, 3000), 21000u);
}

TEST(GeneralsArsenalCooldown, SecondsBecomeFrames)
{
	EXPECT_EQ(secondsToFrames(20.0f, 30), 600u);
	EXPECT_EQ(secondsToFrames(0.5f, 30), 15u);
	EXPECT_EQ(secondsToFrames(0.0f, 30), 0u);
	EXPECT_EQ(secondsToFrames(-1.0f, 30), 0u);
}

namespace
{

AiCandidate candidate(AiUse use, int cost, bool allowed = true, bool ready = true, bool target = true)
{
	AiCandidate c;
	c.use = use;
	c.cost = cost;
	c.allowed = allowed;
	c.ready = ready;
	c.unlocked = true;
	c.hasTarget = target;
	return c;
}

AiSituation points(int value)
{
	AiSituation s;
	s.points = value;
	return s;
}

} // namespace

TEST(GeneralsArsenalAi, TheSuperWeaponGoesFirst)
{
	const std::vector<AiCandidate> powers = { candidate(AiUse::Strike, 40), candidate(AiUse::Superweapon, 250) };
	EXPECT_EQ(chooseAiPower(powers, points(300), AiPolicy()), 1);
}

TEST(GeneralsArsenalAi, StrikesComeBeforeCover)
{
	const std::vector<AiCandidate> powers = { candidate(AiUse::Cover, 30), candidate(AiUse::Strike, 40) };
	EXPECT_EQ(chooseAiPower(powers, points(100), AiPolicy()), 1);
}

TEST(GeneralsArsenalAi, APowerNeedsATarget)
{
	const std::vector<AiCandidate> powers = { candidate(AiUse::Strike, 40, true, true, false), candidate(AiUse::Cover, 30) };
	EXPECT_EQ(chooseAiPower(powers, points(100), AiPolicy()), 1);
	const std::vector<AiCandidate> none = { candidate(AiUse::Cover, 30, true, true, false) };
	EXPECT_EQ(chooseAiPower(none, points(100), AiPolicy()), -1);
}

TEST(GeneralsArsenalAi, ItUsesOnlyWhatIsAllowedAndReady)
{
	const std::vector<AiCandidate> powers = { candidate(AiUse::Strike, 40, false), candidate(AiUse::Strike, 50, true, false),
		candidate(AiUse::Strike, 60) };
	EXPECT_EQ(chooseAiPower(powers, points(100), AiPolicy()), 2);
}

TEST(GeneralsArsenalAi, PowersItsScriptsUseAreLeftAlone)
{
	const std::vector<AiCandidate> powers = { candidate(AiUse::None, 10) };
	EXPECT_EQ(chooseAiPower(powers, points(100), AiPolicy()), -1);
}

TEST(GeneralsArsenalAi, ItSavesTowardAnUnlockedSuperWeapon)
{
	const AiCandidate superweapon = candidate(AiUse::Superweapon, 300, false);	// can't pay for it yet
	const std::vector<AiCandidate> powers = { candidate(AiUse::Strike, 40), superweapon };
	AiPolicy policy;
	policy.superweaponSavingShare = 0.5f;	// keeps 150
	EXPECT_EQ(chooseAiPower(powers, points(180), policy), -1);
	EXPECT_EQ(chooseAiPower(powers, points(190), policy), 0);
}
