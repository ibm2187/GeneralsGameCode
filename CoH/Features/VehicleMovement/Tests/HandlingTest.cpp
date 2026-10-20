// CoH VehicleMovement: the weight curves, the slope rule and Unit line names.

#include "Features/VehicleMovement/Domain/Handling.h"
#include "Features/VehicleMovement/Domain/Names.h"

#include <gtest/gtest.h>

using namespace CoH::VehicleMovement;

namespace
{

// The pack's curves, in short: 100% at 30 tons.
HandlingCurves packCurves()
{
	HandlingCurves curves;
	curves.speed = { { 5.0f, 1.15f }, { 30.0f, 1.0f }, { 70.0f, 0.8f } };
	curves.acceleration = { { 5.0f, 1.5f }, { 30.0f, 1.0f }, { 70.0f, 0.55f } };
	curves.braking = { { 5.0f, 1.4f }, { 30.0f, 1.0f }, { 70.0f, 0.6f } };
	curves.turnRate = { { 5.0f, 1.25f }, { 30.0f, 1.0f }, { 70.0f, 0.7f } };
	curves.slopePenalty = { { 5.0f, 0.3f }, { 30.0f, 0.8f }, { 70.0f, 1.4f } };
	curves.slopeMinSpeed = 0.4f;
	return curves;
}

} // namespace

TEST(VehicleMovementCurve, EmptyGivesTheFallback)
{
	const Curve curve;
	EXPECT_FLOAT_EQ(curve.at(30.0f, 1.0f), 1.0f);
	EXPECT_FLOAT_EQ(curve.at(30.0f, 0.0f), 0.0f);
}

TEST(VehicleMovementCurve, StraightLinesBetweenPoints)
{
	const Curve curve = { { 30.0f, 1.0f }, { 70.0f, 0.8f } };
	EXPECT_FLOAT_EQ(curve.at(30.0f, 1.0f), 1.0f);
	EXPECT_FLOAT_EQ(curve.at(50.0f, 1.0f), 0.9f);
	EXPECT_FLOAT_EQ(curve.at(70.0f, 1.0f), 0.8f);
}

TEST(VehicleMovementCurve, FlatBeyondTheEnds)
{
	const Curve curve = { { 5.0f, 1.2f }, { 70.0f, 0.8f } };
	EXPECT_FLOAT_EQ(curve.at(0.0f, 1.0f), 1.2f);
	EXPECT_FLOAT_EQ(curve.at(200.0f, 1.0f), 0.8f);
}

TEST(VehicleMovementCurve, PointsInAnyOrderAndReplaced)
{
	Curve curve;
	curve.add({ 70.0f, 0.8f });
	curve.add({ 5.0f, 1.2f });
	curve.add({ 30.0f, 0.5f });
	curve.add({ 30.0f, 1.0f });	// replaces the first 30
	EXPECT_FLOAT_EQ(curve.at(30.0f, 1.0f), 1.0f);
	EXPECT_FLOAT_EQ(curve.at(17.5f, 1.0f), 1.1f);
}

TEST(VehicleMovementHandling, MediumDrivesAsRetail)
{
	const Handling medium = handlingFor(30.0f, packCurves());
	EXPECT_FLOAT_EQ(medium.speed, 1.0f);
	EXPECT_FLOAT_EQ(medium.acceleration, 1.0f);
	EXPECT_FLOAT_EQ(medium.braking, 1.0f);
	EXPECT_FLOAT_EQ(medium.turnRate, 1.0f);
}

TEST(VehicleMovementHandling, HeavierIsSlowerEverywhere)
{
	const HandlingCurves curves = packCurves();
	const Handling light = handlingFor(6.0f, curves);
	const Handling heavy = handlingFor(45.0f, curves);
	const Handling super = handlingFor(70.0f, curves);
	EXPECT_GT(light.speed, 1.0f);
	EXPECT_GT(light.acceleration, light.speed);	// lights get off the mark quickest
	EXPECT_LT(heavy.speed, 1.0f);
	EXPECT_LT(super.speed, heavy.speed);
	EXPECT_LT(super.acceleration, heavy.acceleration);
	EXPECT_LT(super.braking, heavy.braking);
	EXPECT_LT(super.turnRate, heavy.turnRate);
	EXPECT_GT(super.slopePenalty, light.slopePenalty);
}

TEST(VehicleMovementHandling, UpgradeWeightSlowsALittle)
{
	const HandlingCurves curves = packCurves();
	const float crusader = 30.0f;
	const Handling plain = handlingFor(crusader, curves);
	const Handling armored = handlingFor(totalWeight(crusader, { 4.0f }), curves);
	EXPECT_LT(armored.speed, plain.speed);
	EXPECT_GT(armored.speed, 0.97f);
}

TEST(VehicleMovementHandling, NeverStops)
{
	HandlingCurves curves;
	curves.speed = { { 0.0f, 1.0f }, { 100.0f, -1.0f } };
	EXPECT_FLOAT_EQ(handlingFor(100.0f, curves).speed, MIN_FACTOR);
}

TEST(VehicleMovementHandling, WeightNeverNegative)
{
	EXPECT_FLOAT_EQ(totalWeight(30.0f, { 4.0f, 3.0f }), 37.0f);
	EXPECT_FLOAT_EQ(totalWeight(2.0f, { -5.0f }), 0.0f);
}

TEST(VehicleMovementSlope, FlatAndDownhillKeepSpeed)
{
	EXPECT_FLOAT_EQ(slopeSpeedFactor(0.0f, 1.0f, 0.4f), 1.0f);
	EXPECT_FLOAT_EQ(slopeSpeedFactor(-0.5f, 1.0f, 0.4f), 1.0f);
}

TEST(VehicleMovementSlope, UphillSlowsByPenalty)
{
	EXPECT_FLOAT_EQ(slopeSpeedFactor(0.25f, 0.8f, 0.4f), 0.8f);
	EXPECT_FLOAT_EQ(slopeSpeedFactor(0.25f, 0.0f, 0.4f), 1.0f);
}

TEST(VehicleMovementSlope, NeverBelowTheFloor)
{
	EXPECT_FLOAT_EQ(slopeSpeedFactor(2.0f, 1.4f, 0.4f), 0.4f);
	EXPECT_FLOAT_EQ(slopeSpeedFactor(2.0f, 1.4f, 0.0f), MIN_FACTOR);
}

TEST(VehicleMovementSlope, HeavyLosesMoreOnTheSameHill)
{
	const HandlingCurves curves = packCurves();
	const float grade = 0.3f;
	const float light = slopeSpeedFactor(grade, handlingFor(6.0f, curves).slopePenalty, curves.slopeMinSpeed);
	const float super = slopeSpeedFactor(grade, handlingFor(70.0f, curves).slopePenalty, curves.slopeMinSpeed);
	EXPECT_GT(light, super);
}

TEST(VehicleMovementNames, GeneralsVersionsMatchLess)
{
	EXPECT_EQ(nameMatch("ChinaTankOverlord", "ChinaTankOverlord"), 2);
	EXPECT_EQ(nameMatch("chinatankoverlord", "ChinaTankOverlord"), 2);
	EXPECT_EQ(nameMatch("ChinaTankOverlord", "Nuke_ChinaTankOverlord"), 1);
	EXPECT_EQ(nameMatch("ChinaTankOverlord", "ChinaTankOverlordGattlingCannon"), 0);
	EXPECT_EQ(nameMatch("", "ChinaTankOverlord"), 0);
}

TEST(VehicleMovementNames, ChallengeGeneralsPrefixes)
{
	EXPECT_EQ(nameMatch("GLATankScorpion", "GC_Chem_GLATankScorpion"), 1);
	EXPECT_EQ(nameMatch("TankOverlord", "Boss_TankOverlord"), 1);
	EXPECT_EQ(nameMatch("TankOverlord", "ChinaTankOverlord"), 0);
}
