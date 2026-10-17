// CoH Accuracy: tests for the hit chance: which weapons roll, accuracy by range, the size curve, the
// built-in modifiers, the clamps, the roll at 0% and 100%, and where a miss lands.

#include <gtest/gtest.h>

#include "Features/Accuracy/Domain/HitChance.h"
#include "Kernel/Domain/Feature.h"

#include <cmath>

using namespace CoH::Accuracy;

namespace
{

const Rules &testRules()
{
	static Rules rules;
	return rules;
}

HitModifier shooterMoving(const Shot &shot) { return shooterMovingModifier(testRules(), shot); }
HitModifier targetMoving(const Shot &shot) { return targetMovingModifier(testRules(), shot); }
HitModifier targetSize(const Shot &shot) { return targetSizeModifier(testRules(), shot); }
HitModifier veterancy(const Shot &shot) { return veterancyModifier(testRules(), shot); }
HitModifier tenPointsOff(const Shot &) { return addToChance(-0.1f); }

RangeProfile profile(float nearAccuracy, float midAccuracy, float farAccuracy)
{
	RangeProfile result;
	result.nearAccuracy = nearAccuracy;
	result.midAccuracy = midAccuracy;
	result.farAccuracy = farAccuracy;
	return result;
}

const CoH::FeatureInfo AccuracyInfo = { "Accuracy", nullptr, nullptr };
const CoH::FeatureInfo CoverInfo = { "Cover", nullptr, nullptr };

// The stack as the Accuracy slice builds it, with the default rules.
class HitChanceTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		accuracy = registry.add(AccuracyInfo);
		cover = registry.add(CoverInfo);
		registry.activate(CoH::FeatureSet::fromList("Accuracy"));

		stack.addBuiltIn(*accuracy, "ShooterMoving", ModifierOrder::Movement, shooterMoving);
		stack.addBuiltIn(*accuracy, "TargetMoving", ModifierOrder::Movement, targetMoving);
		stack.addBuiltIn(*accuracy, "TargetSize", ModifierOrder::TargetSize, targetSize);
		stack.addBuiltIn(*accuracy, "Veterancy", ModifierOrder::Veterancy, veterancy);

		// A shot at a tank-sized vehicle (size x1) by a regular, nobody moving: every modifier is x1.
		shot.targetSize = TargetSize::Vehicle;
		shot.targetRadius = 20.0f;
	}

	float chance(float base) const { return hitChance(rules, base, stack, shot); }

	Rules rules;
	CoH::FeatureRegistry registry;
	CoH::Feature *accuracy = nullptr;
	CoH::Feature *cover = nullptr;
	ModifierStack stack;
	Shot shot;
};

} // namespace

// ---- Which weapons roll -------------------------------------------------------------------------

TEST(RollingProfile, ComesFromTheWeaponsClass)
{
	WeaponFacts rifle;
	rifle.classProfile = profile(0.85f, 0.7f, 0.5f);
	EXPECT_FLOAT_EQ(rollingProfile(Rules(), rifle).midAccuracy, 0.7f);
}

TEST(RollingProfile, TheWeaponsOwnWinsOverItsClass)
{
	WeaponFacts sniper;
	sniper.classProfile = profile(0.85f, 0.7f, 0.5f);
	sniper.ownProfile = profile(1.5f, 1.5f, 1.4f);
	EXPECT_FLOAT_EQ(rollingProfile(Rules(), sniper).farAccuracy, 1.4f);
}

TEST(RollingProfile, SomeWeaponsNeverRoll)
{
	const Rules rules;
	WeaponFacts unlisted;
	EXPECT_FALSE(rollingProfile(rules, unlisted).isSet());	// a damage type with no class: flame, poison

	WeaponFacts listed;
	listed.classProfile = profile(0.7f, 0.7f, 0.7f);
	listed.neverMisses = true;
	EXPECT_FALSE(rollingProfile(rules, listed).isSet());

	WeaponFacts shell;
	shell.classProfile = profile(0.9f, 0.75f, 0.55f);
	shell.damageRadius = rules.maxDamageRadius + 1.0f;
	EXPECT_FALSE(rollingProfile(rules, shell).isSet());
	shell.damageRadius = rules.maxDamageRadius;
	EXPECT_TRUE(rollingProfile(rules, shell).isSet());

	WeaponFacts artillery;
	artillery.classProfile = profile(0.9f, 0.75f, 0.55f);
	artillery.scatterRadius = 20.0f;
	EXPECT_FALSE(rollingProfile(rules, artillery).isSet());

	WeaponFacts other;
	other.classProfile = profile(0.7f, 0.7f, 0.7f);
	other.contact = true;
	EXPECT_FALSE(rollingProfile(rules, other).isSet());
	other.contact = false;
	other.laser = true;
	EXPECT_FALSE(rollingProfile(rules, other).isSet());
	other.laser = false;
	other.damageAtSelf = true;
	EXPECT_FALSE(rollingProfile(rules, other).isSet());
}

TEST(RollingProfile, ZeroPercentStillRolls)
{
	WeaponFacts hopeless;
	hopeless.ownProfile = profile(0.0f, 0.0f, 0.0f);
	EXPECT_TRUE(rollingProfile(Rules(), hopeless).isSet());
}

// ---- Accuracy by range ---------------------------------------------------------------------------

TEST(AccuracyAt, NearMidAndFar)
{
	const RangeProfile rifle = profile(0.85f, 0.7f, 0.5f);
	EXPECT_FLOAT_EQ(accuracyAt(rifle, 0.0f, 200.0f), 0.85f);
	EXPECT_FLOAT_EQ(accuracyAt(rifle, 100.0f, 200.0f), 0.7f);
	EXPECT_FLOAT_EQ(accuracyAt(rifle, 200.0f, 200.0f), 0.5f);
}

TEST(AccuracyAt, StraightLinesInBetween)
{
	const RangeProfile rifle = profile(0.85f, 0.7f, 0.5f);
	EXPECT_FLOAT_EQ(accuracyAt(rifle, 50.0f, 200.0f), 0.775f);
	EXPECT_FLOAT_EQ(accuracyAt(rifle, 150.0f, 200.0f), 0.6f);
}

TEST(AccuracyAt, OutsideTheRangeOrWithoutOne)
{
	const RangeProfile rifle = profile(0.85f, 0.7f, 0.5f);
	EXPECT_FLOAT_EQ(accuracyAt(rifle, 300.0f, 200.0f), 0.5f);	// a shot past its range (leech range, ignoreRanges)
	EXPECT_FLOAT_EQ(accuracyAt(rifle, 50.0f, 0.0f), 0.7f);
}

// ---- Target size ---------------------------------------------------------------------------------

TEST(SizeMultiplier, TheBiggerTheTargetTheEasier)
{
	const Rules rules;
	const float soldier = sizeMultiplier(rules, TargetSize::Infantry, 7.0f);
	const float humvee = sizeMultiplier(rules, TargetSize::Vehicle, 15.7f);
	const float crusader = sizeMultiplier(rules, TargetSize::Vehicle, 18.0f);
	const float overlord = sizeMultiplier(rules, TargetSize::Vehicle, 29.0f);
	const float barracks = sizeMultiplier(rules, TargetSize::Building, 71.0f);
	EXPECT_FLOAT_EQ(soldier, 0.6f);
	EXPECT_LT(soldier, humvee);
	EXPECT_LT(humvee, crusader);
	EXPECT_LT(crusader, overlord);
	EXPECT_LT(overlord, barracks);
}

TEST(SizeMultiplier, StraightLinesBetweenPointsAndFlatOutside)
{
	const Rules rules;
	EXPECT_FLOAT_EQ(sizeMultiplier(rules, TargetSize::Vehicle, 18.0f), 0.95f);	// halfway from 16 (0.9) to 20 (1.0)
	EXPECT_FLOAT_EQ(sizeMultiplier(rules, TargetSize::Infantry, 2.0f), 0.6f);
	EXPECT_FLOAT_EQ(sizeMultiplier(rules, TargetSize::Vehicle, 200.0f), 1.4f);
}

TEST(SizeMultiplier, BuildingsAreNeverSmallTargets)
{
	const Rules rules;
	EXPECT_FLOAT_EQ(sizeMultiplier(rules, TargetSize::Building, 11.0f), rules.buildingSizeMin);
	EXPECT_FLOAT_EQ(sizeMultiplier(rules, TargetSize::Vehicle, 10.0f), 0.7f);
}

TEST(SizeMultiplier, NoCurveIsTimesOne)
{
	Rules rules;
	rules.sizeCurve.clear();
	EXPECT_FLOAT_EQ(sizeMultiplier(rules, TargetSize::Infantry, 7.0f), 1.0f);
}

// ---- The built-in modifiers ----------------------------------------------------------------------

TEST_F(HitChanceTest, AStillTankSizedTargetGetsTheBase)
{
	EXPECT_FLOAT_EQ(chance(0.7f), 0.7f);
}

TEST_F(HitChanceTest, InfantryAreHarderToHitThanVehicles)
{
	shot.targetSize = TargetSize::Infantry;
	shot.targetRadius = 7.0f;
	EXPECT_FLOAT_EQ(chance(0.7f), 0.7f * 0.6f);
}

TEST_F(HitChanceTest, MovingMakesItHarder)
{
	shot.shooterMoving = true;
	EXPECT_FLOAT_EQ(chance(0.8f), 0.8f * 0.75f);
	shot.targetMoving = true;
	EXPECT_FLOAT_EQ(chance(0.8f), 0.8f * 0.75f * 0.85f);
}

TEST_F(HitChanceTest, EachRankAddsFifteenPercent)
{
	shot.veterancy = 1;
	EXPECT_FLOAT_EQ(chance(0.6f), 0.6f * 1.15f);
	shot.veterancy = 2;
	EXPECT_FLOAT_EQ(chance(0.6f), 0.6f * 1.3f);
	shot.veterancy = 3;
	EXPECT_FLOAT_EQ(chance(0.6f), 0.6f * 1.45f);
}

TEST_F(HitChanceTest, ModifiersMultiply)
{
	// A moving veteran at running infantry.
	shot.shooterMoving = true;
	shot.targetMoving = true;
	shot.targetSize = TargetSize::Infantry;
	shot.targetRadius = 10.0f;
	shot.veterancy = 1;
	EXPECT_NEAR(chance(0.7f), 0.7f * 0.75f * 0.85f * 0.7f * 1.15f, 1e-6f);
}

TEST_F(HitChanceTest, AnotherSlicesModifierStacksWhileItIsOn)
{
	stack.add(*cover, "Cover", ModifierOrder::Cover, tenPointsOff);
	shot.veterancy = 2;
	EXPECT_FLOAT_EQ(chance(0.5f), 0.5f * 1.3f);

	registry.activate(CoH::FeatureSet::fromList("Accuracy,Cover"));
	EXPECT_FLOAT_EQ(chance(0.5f), (0.5f - 0.1f) * 1.3f);	// cover applies before veterancy
}

// ---- The clamps and the roll ---------------------------------------------------------------------

TEST_F(HitChanceTest, UnitsAreKeptBetweenMinAndMax)
{
	shot.veterancy = 3;
	EXPECT_FLOAT_EQ(chance(0.9f), 0.9f);

	shot = Shot();
	shot.targetRadius = 7.0f;
	shot.shooterMoving = true;
	shot.targetMoving = true;
	EXPECT_FLOAT_EQ(chance(0.05f), 0.05f);
	EXPECT_FLOAT_EQ(chance(0.0f), 0.05f);
}

TEST_F(HitChanceTest, BuildingsAreTheEasiestButNeverCertain)
{
	shot.targetSize = TargetSize::Building;
	shot.targetRadius = 50.0f;
	EXPECT_FLOAT_EQ(chance(0.7f), 0.95f);
	EXPECT_FLOAT_EQ(chance(1.5f), 0.95f);

	shot.targetSize = TargetSize::Vehicle;
	EXPECT_FLOAT_EQ(chance(1.5f), 0.9f);
}

TEST_F(HitChanceTest, WithoutLimitsTheChanceCanReachZeroAndCertainty)
{
	rules.minChance = 0.0f;
	rules.maxChance = 1.0f;
	EXPECT_FLOAT_EQ(chance(0.0f), 0.0f);
	EXPECT_FLOAT_EQ(chance(1.2f), 1.0f);

	stack.add(*cover, "Cover", ModifierOrder::Cover, tenPointsOff);
	registry.activate(CoH::FeatureSet::fromList("Accuracy,Cover"));
	EXPECT_FLOAT_EQ(chance(0.05f), 0.0f);	// below zero is kept at zero
}

TEST(HitRoll, CertainChancesNeedNoRoll)
{
	EXPECT_FALSE(needsRoll(0.0f));
	EXPECT_FALSE(needsRoll(-0.5f));
	EXPECT_FALSE(needsRoll(1.0f));
	EXPECT_FALSE(needsRoll(1.5f));
	EXPECT_TRUE(needsRoll(0.0001f));
	EXPECT_TRUE(needsRoll(0.9999f));
	EXPECT_TRUE(needsRoll(0.5f));
}

TEST(HitRoll, ZeroNeverHitsAndOneAlwaysHits)
{
	for (int roll : { 0, 1, RollSides / 2, RollSides - 1 })
	{
		EXPECT_FALSE(isHit(0.0f, roll));
		EXPECT_FALSE(isHit(-1.0f, roll));
		EXPECT_TRUE(isHit(1.0f, roll));
		EXPECT_TRUE(isHit(2.0f, roll));
	}
}

TEST(HitRoll, TheChanceIsTheShareOfRollsThatHit)
{
	for (float chance : { 0.05f, 0.25f, 0.7f, 0.95f })
	{
		int hits = 0;
		for (int roll = 0; roll < RollSides; ++roll)
		{
			if (isHit(chance, roll))
				++hits;
		}
		EXPECT_EQ(hits, (int)std::lround(chance * RollSides));
	}
}

// ---- Where a miss lands --------------------------------------------------------------------------

TEST(MissOffset, LandsBeyondTheTargetsEdge)
{
	const Rules rules;
	const float radius = 10.0f;
	for (float spread : { 0.0f, 0.5f, 1.0f })
	{
		for (float angle : { 0.0f, 1.0f, 3.0f, 5.5f })
		{
			const Offset offset = missOffset(rules, radius, angle, spread);
			const float distance = std::sqrt(offset.x * offset.x + offset.y * offset.y);
			EXPECT_GE(distance, radius + rules.missDistanceMin - 1e-4f);
			EXPECT_LE(distance, radius + rules.missDistanceMax + 1e-4f);
		}
	}
	EXPECT_NEAR(missOffset(rules, radius, 0.0f, 0.0f).x, radius + rules.missDistanceMin, 1e-4f);
	EXPECT_NEAR(missOffset(rules, radius, 0.0f, 1.0f).x, radius + rules.missDistanceMax, 1e-4f);
	EXPECT_NEAR(missOffset(rules, 0.0f, 3.14159265f / 2.0f, 0.0f).y, rules.missDistanceMin, 1e-4f);
}
