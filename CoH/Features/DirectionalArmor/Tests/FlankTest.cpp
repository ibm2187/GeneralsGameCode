// CoH DirectionalArmor: when and where the skirmish AI flanks.

#include <gtest/gtest.h>

#include "Features/DirectionalArmor/Domain/Flank.h"

#include <cmath>

namespace
{

using namespace CoH::DirectionalArmor;

// A Crusader at the origin facing east, a Battlemaster's gun.
FlankSituation tankDuel(Flat attacker)
{
	return {
		.target = { 0.0f, 0.0f },
		.targetFacing = { 1.0f, 0.0f },
		.attacker = attacker,
		.arcs = arcsFromDegrees(90.0f, 90.0f),
		.armor = { .front = 130.0f, .side = 90.0f, .rear = 55.0f },
		.penetration = { .atNear = 120.0f, .atFar = 90.0f },
		.range = 150.0f,
	};
}

float distance(Flat a, Flat b)
{
	return std::sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

TEST(DirectionalArmorFlank, FromTheFrontGoesRoundToTheSide)
{
	const FlankRules rules;
	const std::optional<Flat> spot = flankSpot(tankDuel({ 140.0f, 20.0f }), rules);
	ASSERT_TRUE(spot.has_value());
	EXPECT_NEAR(distance(*spot, { 0.0f, 0.0f }), 150.0f * rules.rangeShare, 0.01f);
	EXPECT_LT(spot->x, 0.0f);	// swung back behind the target's middle
	EXPECT_GT(spot->y, 0.0f);	// on the side the attacker is already on
	EXPECT_NE(sideHit({ 1.0f, 0.0f }, *spot, arcsFromDegrees(90.0f, 90.0f)), Side::Front);
}

TEST(DirectionalArmorFlank, KeepsToTheAttackersSide)
{
	const std::optional<Flat> spot = flankSpot(tankDuel({ 140.0f, -20.0f }), FlankRules());
	ASSERT_TRUE(spot.has_value());
	EXPECT_LT(spot->y, 0.0f);
}

TEST(DirectionalArmorFlank, NotWhenAlreadyOnTheSideOrRear)
{
	EXPECT_FALSE(flankSpot(tankDuel({ 0.0f, 120.0f }), FlankRules()).has_value());
	EXPECT_FALSE(flankSpot(tankDuel({ -120.0f, 0.0f }), FlankRules()).has_value());
}

TEST(DirectionalArmorFlank, NotWhenTheFrontGetsThroughWellEnough)
{
	FlankSituation situation = tankDuel({ 30.0f, 0.0f });	// point-blank: 114 against 130, 88%
	EXPECT_FALSE(flankSpot(situation, FlankRules()).has_value());
	situation.penetration = { .atNear = 200.0f, .atFar = 200.0f };
	situation.attacker = { 140.0f, 0.0f };
	EXPECT_FALSE(flankSpot(situation, FlankRules()).has_value());
}

TEST(DirectionalArmorFlank, NotWhenTheFlankGainsTooLittle)
{
	FlankSituation situation = tankDuel({ 140.0f, 0.0f });
	situation.armor = { .front = 130.0f, .side = 130.0f, .rear = 130.0f };	// the same all round
	EXPECT_FALSE(flankSpot(situation, FlankRules()).has_value());
}

TEST(DirectionalArmorFlank, NotWhenTheDriveIsTooLong)
{
	FlankRules rules;
	rules.maxTravel = 50.0f;
	EXPECT_FALSE(flankSpot(tankDuel({ 140.0f, 20.0f }), rules).has_value());
}

TEST(DirectionalArmorFlank, NoFacingNoFlank)
{
	FlankSituation situation = tankDuel({ 140.0f, 20.0f });
	situation.targetFacing = { 0.0f, 0.0f };
	EXPECT_FALSE(flankSpot(situation, FlankRules()).has_value());
}

} // namespace
