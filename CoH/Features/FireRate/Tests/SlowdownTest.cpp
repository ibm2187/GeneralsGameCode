// CoH FireRate: tests for the fire rate of a weapon and how much it is slowed.

#include <gtest/gtest.h>

#include "Features/FireRate/Domain/Slowdown.h"

using namespace CoH::FireRate;

namespace
{

const double Fps = 30;

// A retail Ranger rifle: bursts of 3, 100 ms apart, then a 700 ms reload.
FireCycle rangerRifle()
{
	FireCycle cycle;
	cycle.minDelay = cycle.maxDelay = 3;
	cycle.clipSize = 3;
	cycle.clipReload = 21;
	return cycle;
}

FireCycle everyFrames(int frames)
{
	FireCycle cycle;
	cycle.minDelay = cycle.maxDelay = frames;
	return cycle;
}

WeaponFacts rifle(double classSeconds)
{
	WeaponFacts facts;
	facts.cycle = rangerRifle();
	facts.classSeconds = classSeconds;
	return facts;
}

} // namespace

TEST(FireRateCycle, AnEndlessClipFiresOncePerDelay)
{
	EXPECT_DOUBLE_EQ(shotsPerSecond(everyFrames(6), Fps), 5.0);	// a Humvee gun, 200 ms
	EXPECT_DOUBLE_EQ(shotsPerSecond(everyFrames(60), Fps), 0.5);	// a Crusader, 2 s
}

TEST(FireRateCycle, TheReloadReplacesTheDelayAfterTheLastShot)
{
	// 3 shots in 2 delays and a reload: 27 frames.
	EXPECT_DOUBLE_EQ(shotsPerSecond(rangerRifle(), Fps), 3 * 30.0 / 27);

	// An Overlord: 2 shells 300 ms apart, then 2 s.
	FireCycle overlord = everyFrames(9);
	overlord.clipSize = 2;
	overlord.clipReload = 60;
	EXPECT_DOUBLE_EQ(shotsPerSecond(overlord, Fps), 2 * 30.0 / 69);
}

TEST(FireRateCycle, ARandomDelayCountsAtItsMiddle)
{
	FireCycle cycle;
	cycle.minDelay = 10;
	cycle.maxDelay = 20;
	EXPECT_DOUBLE_EQ(shotsPerSecond(cycle, Fps), 2.0);
}

TEST(FireRateCycle, ACycleWithNoTimeHasNoRate)
{
	EXPECT_DOUBLE_EQ(shotsPerSecond(everyFrames(0), Fps), 0.0);

	FireCycle oneShot;		// a clip of one, never reloaded in the field
	oneShot.clipSize = 1;
	EXPECT_DOUBLE_EQ(shotsPerSecond(oneShot, Fps), 0.0);
}

TEST(FireRateLimit, TheClassLineHoldsAWeapon)
{
	EXPECT_DOUBLE_EQ(secondsPerShotLimit(rifle(2), Rules()), 2.0);
	EXPECT_DOUBLE_EQ(secondsPerShotLimit(rifle(0), Rules()), 0.0);	// no Class line
}

TEST(FireRateLimit, AreaWeaponsScatterAndSelfBlastsAreLeftAlone)
{
	WeaponFacts area = rifle(3);
	area.damageRadius = 25;
	EXPECT_DOUBLE_EQ(secondsPerShotLimit(area, Rules()), 0.0);

	WeaponFacts smallBlast = rifle(3);
	smallBlast.damageRadius = 8;	// up to the limit still counts as a direct hit
	EXPECT_DOUBLE_EQ(secondsPerShotLimit(smallBlast, Rules()), 3.0);

	WeaponFacts artillery = rifle(3);
	artillery.scatterRadius = 20;
	EXPECT_DOUBLE_EQ(secondsPerShotLimit(artillery, Rules()), 0.0);

	WeaponFacts suicide = rifle(3);
	suicide.damageAtSelf = true;
	EXPECT_DOUBLE_EQ(secondsPerShotLimit(suicide, Rules()), 0.0);
}

TEST(FireRateLimit, AWeaponsOwnLineWinsEvenForAnAreaWeapon)
{
	WeaponFacts heavyGun = rifle(2);
	heavyGun.hasOwnLine = true;
	heavyGun.ownSeconds = 1;
	heavyGun.damageRadius = 25;
	EXPECT_DOUBLE_EQ(secondsPerShotLimit(heavyGun, Rules()), 1.0);

	WeaponFacts sniper = rifle(2);
	sniper.hasOwnLine = true;
	sniper.ownSeconds = 0;		// 0 never slows it
	EXPECT_DOUBLE_EQ(secondsPerShotLimit(sniper, Rules()), 0.0);
}

TEST(FireRateScaling, AFastWeaponFiresAtTheLimitAndHitsHarder)
{
	const Scaling scaling = scalingFor(rifle(2), Rules(), Fps);
	EXPECT_NEAR(scaling.slowDown, 3 * 30.0 / 27 * 2, 1e-9);	// 3.33 shots a second down to 0.5
	EXPECT_DOUBLE_EQ(scaling.damage, scaling.slowDown);
	EXPECT_NEAR(shotsPerSecond(rangerRifle(), Fps) / scaling.slowDown, 0.5, 1e-9);
}

TEST(FireRateScaling, DamagePerSecondStaysTheSame)
{
	const double damage = 5;
	const double rate = shotsPerSecond(rangerRifle(), Fps);
	const Scaling scaling = scalingFor(rifle(2), Rules(), Fps);
	EXPECT_NEAR((rate / scaling.slowDown) * (damage * scaling.damage), rate * damage, 1e-9);
}

TEST(FireRateScaling, AWeaponAlreadySlowEnoughIsUnchanged)
{
	WeaponFacts crusader;
	crusader.cycle = everyFrames(60);
	crusader.classSeconds = 2;		// 0.5 a second is exactly the limit
	EXPECT_FALSE(scalingFor(crusader, Rules(), Fps).changes());

	crusader.classSeconds = 1;
	EXPECT_FALSE(scalingFor(crusader, Rules(), Fps).changes());
}

TEST(FireRateScaling, TheSlowdownStopsAtTheMax)
{
	WeaponFacts quadCannon;
	quadCannon.cycle = everyFrames(1);	// 30 a second
	quadCannon.classSeconds = 1;
	Rules rules;
	rules.maxSlowDown = 8;
	const Scaling scaling = scalingFor(quadCannon, rules, Fps);
	EXPECT_DOUBLE_EQ(scaling.slowDown, 8.0);
	EXPECT_DOUBLE_EQ(scaling.damage, 8.0);
}

TEST(FireRateScaling, ALowerDamageShareNeverMakesAShotWeaker)
{
	Rules rules;
	rules.damagePerSecond = 0.5;

	const Scaling rifleScaling = scalingFor(rifle(2), rules, Fps);
	EXPECT_NEAR(rifleScaling.damage, rifleScaling.slowDown * 0.5, 1e-9);

	WeaponFacts barelyFast;
	barelyFast.cycle = everyFrames(25);	// 1.2 a second against a limit of 1
	barelyFast.classSeconds = 1;
	const Scaling scaling = scalingFor(barelyFast, rules, Fps);
	EXPECT_NEAR(scaling.slowDown, 1.2, 1e-9);
	EXPECT_DOUBLE_EQ(scaling.damage, 1.0);
}

TEST(FireRateScaling, NoLimitOrNoRateMeansNoChange)
{
	EXPECT_FALSE(scalingFor(rifle(0), Rules(), Fps).changes());

	WeaponFacts noRate = rifle(2);
	noRate.cycle = everyFrames(0);
	EXPECT_FALSE(scalingFor(noRate, Rules(), Fps).changes());
}

TEST(FireRateScaling, AGattlingIsHeldToTheLimitOnceFullySpunUp)
{
	WeaponFacts gattlingTank;
	gattlingTank.cycle = everyFrames(12);	// 2.5 a second at first...
	gattlingTank.spinUp = 3;				// ...7.5 once spun up
	gattlingTank.classSeconds = 1;
	EXPECT_DOUBLE_EQ(topShotsPerSecond(gattlingTank, Fps), 7.5);
	const Scaling scaling = scalingFor(gattlingTank, Rules(), Fps);
	EXPECT_DOUBLE_EQ(scaling.slowDown, 7.5);
	EXPECT_DOUBLE_EQ(scaling.damage, 7.5);

	gattlingTank.spinUp = 0.5;	// a slowing bonus doesn't count
	EXPECT_DOUBLE_EQ(topShotsPerSecond(gattlingTank, Fps), 2.5);
}
