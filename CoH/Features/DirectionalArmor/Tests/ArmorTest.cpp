// CoH DirectionalArmor: the side a shot hits, penetration by range, the odds, deflection, names.

#include <gtest/gtest.h>

#include "Features/DirectionalArmor/Domain/Armor.h"
#include "Features/DirectionalArmor/Domain/Names.h"

#include <cmath>

namespace
{

using namespace CoH::DirectionalArmor;

const Arcs quarters = arcsFromDegrees(90.0f, 90.0f);
const Flat east = { 1.0f, 0.0f };
const HitRules plainHits = { .deflectedShare = 0.1f };
const HitRules packHits = { .deflectedShare = 0.05f, .sideDamage = { .front = 0.75f, .side = 1.25f, .rear = 1.5f } };

// From the vehicle, the direction 'degrees' counterclockwise from east.
Flat at(float degrees)
{
	const double radians = degrees * 3.14159265358979 / 180.0;
	return { (float)std::cos(radians) * 50.0f, (float)std::sin(radians) * 50.0f };	// test only: library cos is fine here
}

TEST(DirectionalArmorCos, MatchesTheLibraryOverAWholeTurn)
{
	for (float degrees = -720.0f; degrees <= 720.0f; degrees += 7.5f)
		EXPECT_NEAR(cosOfDegrees(degrees), std::cos(degrees * 3.14159265358979 / 180.0), 2e-6) << degrees;
}

TEST(DirectionalArmorCos, KnownAngles)
{
	EXPECT_NEAR(cosOfDegrees(0.0f), 1.0f, 1e-6f);
	EXPECT_NEAR(cosOfDegrees(45.0f), 0.70710677f, 1e-6f);
	EXPECT_NEAR(cosOfDegrees(90.0f), 0.0f, 1e-6f);
	EXPECT_NEAR(cosOfDegrees(180.0f), -1.0f, 1e-6f);
}

TEST(DirectionalArmorSide, QuartersAroundTheHull)
{
	EXPECT_EQ(sideHit(east, at(0.0f), quarters), Side::Front);
	EXPECT_EQ(sideHit(east, at(40.0f), quarters), Side::Front);
	EXPECT_EQ(sideHit(east, at(-40.0f), quarters), Side::Front);
	EXPECT_EQ(sideHit(east, at(50.0f), quarters), Side::Side);
	EXPECT_EQ(sideHit(east, at(90.0f), quarters), Side::Side);
	EXPECT_EQ(sideHit(east, at(-90.0f), quarters), Side::Side);
	EXPECT_EQ(sideHit(east, at(130.0f), quarters), Side::Side);
	EXPECT_EQ(sideHit(east, at(140.0f), quarters), Side::Rear);
	EXPECT_EQ(sideHit(east, at(180.0f), quarters), Side::Rear);
	EXPECT_EQ(sideHit(east, at(-140.0f), quarters), Side::Rear);
}

TEST(DirectionalArmorSide, FollowsTheFacingNotTheMap)
{
	const Flat north = { 0.0f, 3.0f };	// need not be unit length
	EXPECT_EQ(sideHit(north, at(90.0f), quarters), Side::Front);
	EXPECT_EQ(sideHit(north, at(0.0f), quarters), Side::Side);
	EXPECT_EQ(sideHit(north, at(-90.0f), quarters), Side::Rear);
}

TEST(DirectionalArmorSide, ArcWidthsComeFromTheIni)
{
	const Arcs wideFront = arcsFromDegrees(180.0f, 60.0f);
	EXPECT_EQ(sideHit(east, at(85.0f), wideFront), Side::Front);
	EXPECT_EQ(sideHit(east, at(140.0f), wideFront), Side::Side);
	EXPECT_EQ(sideHit(east, at(155.0f), wideFront), Side::Rear);

	const Arcs noArcs = arcsFromDegrees(0.0f, 0.0f);
	EXPECT_EQ(sideHit(east, at(1.0f), noArcs), Side::Side);
	EXPECT_EQ(sideHit(east, at(0.0f), noArcs), Side::Front);	// dead ahead still is the front
}

TEST(DirectionalArmorSide, NoDirectionHitsTheFront)
{
	EXPECT_EQ(sideHit(east, { 0.0f, 0.0f }, quarters), Side::Front);
	EXPECT_EQ(sideHit({ 0.0f, 0.0f }, at(180.0f), quarters), Side::Front);
}

TEST(DirectionalArmorPenetration, FallsFromNearToFar)
{
	const Penetration rocket = { .atNear = 55.0f, .atFar = 40.0f };
	EXPECT_FLOAT_EQ(penetrationAt(rocket, 0.0f, 175.0f), 55.0f);
	EXPECT_FLOAT_EQ(penetrationAt(rocket, 87.5f, 175.0f), 47.5f);
	EXPECT_FLOAT_EQ(penetrationAt(rocket, 175.0f, 175.0f), 40.0f);
	EXPECT_FLOAT_EQ(penetrationAt(rocket, 400.0f, 175.0f), 40.0f);	// beyond the range: far
	EXPECT_FLOAT_EQ(penetrationAt(rocket, 50.0f, 0.0f), 55.0f);		// no range: near
}

TEST(DirectionalArmorOdds, PenetrationOverArmor)
{
	EXPECT_FLOAT_EQ(penetrationChance(55.0f, 130.0f), 55.0f / 130.0f);
	EXPECT_FLOAT_EQ(penetrationChance(130.0f, 130.0f), 1.0f);
	EXPECT_FLOAT_EQ(penetrationChance(200.0f, 130.0f), 1.0f);
	EXPECT_FLOAT_EQ(penetrationChance(0.0f, 130.0f), 0.0f);
	EXPECT_FLOAT_EQ(penetrationChance(10.0f, 0.0f), 1.0f);	// no armor never stops a shot
}

TEST(DirectionalArmorOdds, RocketsBounceOffTheFrontAndGetThroughTheRear)
{
	// The pack's rocket soldier (a shaped charge: the same at any range) against the pack's Crusader.
	const Penetration rocket = { .atNear = 55.0f, .atFar = 55.0f };
	const ArmorValues crusader = { .front = 160.0f, .side = 85.0f, .rear = 45.0f };
	const float pushing = penetrationAt(rocket, 87.5f, 175.0f);
	EXPECT_LT(penetrationChance(pushing, crusader.front), 0.4f);	// most bounce off the front
	EXPECT_GT(penetrationChance(pushing, crusader.side), 0.5f);
	EXPECT_GT(penetrationChance(pushing, crusader.rear), 0.85f);	// most get through the rear
}

TEST(DirectionalArmorDeflection, ShareOfTheDamage)
{
	EXPECT_FLOAT_EQ(damageScale(true, 0.1f), 1.0f);
	EXPECT_FLOAT_EQ(damageScale(false, 0.1f), 0.1f);
	EXPECT_FLOAT_EQ(damageScale(false, 0.0f), 0.0f);
	EXPECT_FLOAT_EQ(damageScale(false, 2.0f), 1.0f);
	EXPECT_FLOAT_EQ(damageScale(false, -1.0f), 0.0f);
}

TEST(DirectionalArmorDeflection, ResolveRollsOnceAndScales)
{
	const ArmorValues armor = { .front = 100.0f, .side = 80.0f, .rear = 40.0f };
	int asked = 0;
	float askedChance = -1.0f;
	const auto failing = [&](float chance) { ++asked; askedChance = chance; return false; };

	const Hit bounced = resolveHit(Side::Front, armor, 60.0f, plainHits, failing);
	EXPECT_EQ(asked, 1);
	EXPECT_FLOAT_EQ(askedChance, 0.6f);
	EXPECT_FALSE(bounced.penetrated);
	EXPECT_FLOAT_EQ(bounced.armor, 100.0f);
	EXPECT_FLOAT_EQ(bounced.damageScale, 0.1f);

	const Hit through = resolveHit(Side::Rear, armor, 60.0f, plainHits, [&](float chance) { ++asked; return chance >= 1.0f; });
	EXPECT_EQ(asked, 2);	// asked even at a sure chance, so the number of rolls never depends on tuning
	EXPECT_TRUE(through.penetrated);
	EXPECT_FLOAT_EQ(through.chance, 1.0f);
	EXPECT_FLOAT_EQ(through.damageScale, 1.0f);
}

TEST(DirectionalArmorDeflection, ManyRollsMatchTheOdds)
{
	// A deterministic spread of rolls from 0 to 1, as the synced random numbers would give.
	const ArmorValues armor = { .front = 130.0f, .side = 90.0f, .rear = 55.0f };
	int through = 0;
	const int shots = 1000;
	for (int i = 0; i < shots; ++i)
	{
		const float roll = (i + 0.5f) / shots;
		through += resolveHit(Side::Front, armor, 52.0f, plainHits, [&](float chance) { return roll < chance; }).penetrated;
	}
	EXPECT_NEAR(through / (float)shots, 52.0f / 130.0f, 0.002f);
}

TEST(DirectionalArmorSideDamage, FrontLessSideAndRearMore)
{
	const ArmorValues armor = { .front = 100.0f, .side = 50.0f, .rear = 25.0f };
	const auto through = [](float) { return true; };
	const auto bounce = [](float) { return false; };
	EXPECT_FLOAT_EQ(resolveHit(Side::Front, armor, 200.0f, packHits, through).damageScale, 0.75f);
	EXPECT_FLOAT_EQ(resolveHit(Side::Side, armor, 200.0f, packHits, through).damageScale, 1.25f);
	EXPECT_FLOAT_EQ(resolveHit(Side::Rear, armor, 200.0f, packHits, through).damageScale, 1.5f);
	EXPECT_FLOAT_EQ(resolveHit(Side::Front, armor, 10.0f, packHits, bounce).damageScale, 0.05f * 0.75f);
	EXPECT_FLOAT_EQ(resolveHit(Side::Rear, armor, 10.0f, packHits, bounce).damageScale, 0.05f * 1.5f);
}

TEST(DirectionalArmorSideDamage, ExpectedDamageOfARocket)
{
	// The pack's rocket soldier (55 at any range) against the pack's Crusader (160 / 85 / 45).
	const float front = expectedDamage(penetrationChance(55.0f, 160.0f), Side::Front, packHits);
	const float side = expectedDamage(penetrationChance(55.0f, 85.0f), Side::Side, packHits);
	const float rear = expectedDamage(penetrationChance(55.0f, 45.0f), Side::Rear, packHits);
	EXPECT_LT(front, 0.3f);
	EXPECT_GT(side, 0.8f);
	EXPECT_FLOAT_EQ(rear, 1.5f);
	EXPECT_FLOAT_EQ(expectedDamage(0.5f, Side::Side, plainHits), 0.55f);
	EXPECT_FLOAT_EQ(expectedDamage(2.0f, Side::Front, plainHits), 1.0f);
}

TEST(DirectionalArmorNames, OwnLineAndGeneralsVersions)
{
	EXPECT_EQ(nameMatch("OverlordTankGun", "OverlordTankGun"), 2);
	EXPECT_EQ(nameMatch("overlordtankgun", "OverlordTankGun"), 2);
	EXPECT_EQ(nameMatch("OverlordTankGun", "Nuke_OverlordTankGun"), 1);
	EXPECT_EQ(nameMatch("ChinaTankEmperor", "Tank_ChinaTankEmperor"), 1);
	EXPECT_EQ(nameMatch("OverlordTankGun", "OverlordTankGunX"), 0);
	EXPECT_EQ(nameMatch("MarauderTankGun", "REDMarauderTankGun"), 0);	// no underscore: another weapon
	EXPECT_EQ(nameMatch("TankGun", "_TankGun"), 0);						// nothing before the underscore
	EXPECT_EQ(nameMatch("", "Anything"), 0);
	EXPECT_TRUE(namesCover("BattleMasterTankGun", "Tank_BattleMasterTankGun"));
}

} // namespace
