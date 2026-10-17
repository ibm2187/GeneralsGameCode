// CoH Accuracy: tests for the hit chance: which weapons roll, the built-in modifiers, the clamp, the
// roll at 0% and 100%, and where a miss lands.

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

HitModifier range(const Shot &shot) { return rangeModifier(testRules(), shot); }
HitModifier shooterMoving(const Shot &shot) { return shooterMovingModifier(testRules(), shot); }
HitModifier targetMoving(const Shot &shot) { return targetMovingModifier(testRules(), shot); }
HitModifier targetSize(const Shot &shot) { return targetSizeModifier(testRules(), shot); }
HitModifier veterancy(const Shot &shot) { return veterancyModifier(testRules(), shot); }
HitModifier tenPointsOff(const Shot &) { return addToChance(-0.1f); }

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

		stack.addBuiltIn(*accuracy, "Range", ModifierOrder::Range, range);
		stack.addBuiltIn(*accuracy, "ShooterMoving", ModifierOrder::Movement, shooterMoving);
		stack.addBuiltIn(*accuracy, "TargetMoving", ModifierOrder::Movement, targetMoving);
		stack.addBuiltIn(*accuracy, "TargetSize", ModifierOrder::TargetSize, targetSize);
		stack.addBuiltIn(*accuracy, "Veterancy", ModifierOrder::Veterancy, veterancy);

		// A mid-range shot at a vehicle by a regular, nobody moving: every modifier is x1.
		shot.range = 150.0f;
		shot.distance = 75.0f;
		shot.targetSize = TargetSize::Vehicle;
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

TEST(BaseAccuracy, ComesFromTheWeaponsClass)
{
	WeaponFacts rifle;
	rifle.classPercent = 70;
	EXPECT_FLOAT_EQ(baseAccuracy(Rules(), rifle), 0.7f);
}

TEST(BaseAccuracy, TheWeaponsOwnBaseWinsOverItsClass)
{
	WeaponFacts sniper;
	sniper.classPercent = 70;
	sniper.weaponPercent = 95;
	EXPECT_FLOAT_EQ(baseAccuracy(Rules(), sniper), 0.95f);
}

TEST(BaseAccuracy, SomeWeaponsNeverRoll)
{
	const Rules rules;
	WeaponFacts unlisted;
	EXPECT_LT(baseAccuracy(rules, unlisted), 0.0f);	// a damage type with no class: flame, poison

	WeaponFacts perfect;
	perfect.classPercent = 70;
	perfect.weaponPercent = 100;
	EXPECT_LT(baseAccuracy(rules, perfect), 0.0f);

	WeaponFacts shell;
	shell.classPercent = 75;
	shell.damageRadius = rules.maxDamageRadius + 1.0f;
	EXPECT_LT(baseAccuracy(rules, shell), 0.0f);
	shell.damageRadius = rules.maxDamageRadius;
	EXPECT_FLOAT_EQ(baseAccuracy(rules, shell), 0.75f);

	WeaponFacts artillery;
	artillery.classPercent = 75;
	artillery.scatterRadius = 20.0f;
	EXPECT_LT(baseAccuracy(rules, artillery), 0.0f);

	WeaponFacts other;
	other.classPercent = 70;
	other.contact = true;
	EXPECT_LT(baseAccuracy(rules, other), 0.0f);
	other.contact = false;
	other.laser = true;
	EXPECT_LT(baseAccuracy(rules, other), 0.0f);
	other.laser = false;
	other.damageAtSelf = true;
	EXPECT_LT(baseAccuracy(rules, other), 0.0f);
}

TEST(BaseAccuracy, ZeroPercentStillRolls)
{
	WeaponFacts hopeless;
	hopeless.weaponPercent = 0;
	EXPECT_FLOAT_EQ(baseAccuracy(Rules(), hopeless), 0.0f);
}

// ---- The built-in modifiers ----------------------------------------------------------------------

TEST_F(HitChanceTest, AMidRangeShotAtAStillVehicleIsTheBase)
{
	EXPECT_FLOAT_EQ(chance(0.7f), 0.7f);
}

TEST_F(HitChanceTest, RangeBands)
{
	shot.distance = 40.0f;
	EXPECT_FLOAT_EQ(chance(0.6f), 0.6f * 1.15f);
	shot.distance = 100.0f;
	EXPECT_FLOAT_EQ(chance(0.6f), 0.6f);
	shot.distance = 101.0f;
	EXPECT_FLOAT_EQ(chance(0.6f), 0.6f * 0.85f);
	shot.distance = 150.0f;
	EXPECT_FLOAT_EQ(chance(0.6f), 0.6f * 0.85f);

	shot.range = 0.0f;	// no range known: mid
	EXPECT_FLOAT_EQ(chance(0.6f), 0.6f);
}

TEST_F(HitChanceTest, MovingMakesItHarder)
{
	shot.shooterMoving = true;
	EXPECT_FLOAT_EQ(chance(0.8f), 0.8f * 0.75f);
	shot.targetMoving = true;
	EXPECT_FLOAT_EQ(chance(0.8f), 0.8f * 0.75f * 0.85f);
}

TEST_F(HitChanceTest, TargetSize)
{
	shot.targetSize = TargetSize::Infantry;
	EXPECT_FLOAT_EQ(chance(0.7f), 0.7f * 0.9f);
	shot.targetSize = TargetSize::Aircraft;
	EXPECT_FLOAT_EQ(chance(0.7f), 0.7f * 0.9f);
	shot.targetSize = TargetSize::Building;
	EXPECT_FLOAT_EQ(chance(0.6f), 0.6f * 1.5f);
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
	// A moving veteran rifleman at long range against running infantry.
	shot.distance = 140.0f;
	shot.shooterMoving = true;
	shot.targetMoving = true;
	shot.targetSize = TargetSize::Infantry;
	shot.veterancy = 1;
	EXPECT_NEAR(chance(0.7f), 0.7f * 0.85f * 0.75f * 0.85f * 0.9f * 1.15f, 1e-6f);
}

TEST_F(HitChanceTest, AnotherSlicesModifierStacksWhileItIsOn)
{
	stack.add(*cover, "Cover", ModifierOrder::Cover, tenPointsOff);
	shot.veterancy = 2;
	EXPECT_FLOAT_EQ(chance(0.5f), 0.5f * 1.3f);

	registry.activate(CoH::FeatureSet::fromList("Accuracy,Cover"));
	EXPECT_FLOAT_EQ(chance(0.5f), (0.5f - 0.1f) * 1.3f);	// cover applies before veterancy
}

// ---- The clamp and the roll ----------------------------------------------------------------------

TEST_F(HitChanceTest, TheChanceIsKeptBetweenMinAndMax)
{
	shot.veterancy = 3;
	shot.targetSize = TargetSize::Building;
	EXPECT_FLOAT_EQ(chance(0.9f), 0.95f);

	shot = Shot();
	shot.range = 150.0f;
	shot.distance = 150.0f;
	shot.shooterMoving = true;
	shot.targetMoving = true;
	EXPECT_FLOAT_EQ(chance(0.05f), 0.05f);
	EXPECT_FLOAT_EQ(chance(0.0f), 0.05f);
}

TEST_F(HitChanceTest, WithoutLimitsTheChanceCanReachZeroAndCertainty)
{
	rules.minChance = 0.0f;
	rules.maxChance = 1.0f;
	EXPECT_FLOAT_EQ(chance(0.0f), 0.0f);
	shot.targetSize = TargetSize::Building;
	EXPECT_FLOAT_EQ(chance(0.9f), 1.0f);

	stack.add(*cover, "Cover", ModifierOrder::Cover, tenPointsOff);
	registry.activate(CoH::FeatureSet::fromList("Accuracy,Cover"));
	shot.targetSize = TargetSize::Vehicle;
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
