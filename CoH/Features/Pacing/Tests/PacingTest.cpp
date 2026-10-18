// CoH Pacing: tests for unit costs, build times and the AI's handicap.

#include <gtest/gtest.h>

#include "Features/Pacing/Domain/Pacing.h"

using namespace CoH::Pacing;

namespace
{

Rules sampleRules()
{
	Rules rules;
	rules.classes[static_cast<std::size_t>(UnitClass::Infantry)] = { 1.5f, 1.25f };
	rules.classes[static_cast<std::size_t>(UnitClass::Vehicle)] = { 2.0f, 1.5f };
	rules.classes[static_cast<std::size_t>(UnitClass::Aircraft)] = { 2.0f, 2.0f };
	rules.objects.push_back({ "AmericaVehicleHumvee", { 1.25f, 1.0f } });
	rules.objects.push_back({ "Tank_ChinaTankOverlord", { 3.0f, 3.0f } });
	return rules;
}

} // namespace

TEST(PacingClassify, ExemptWinsThenStructureThenAircraft)
{
	EXPECT_EQ(classify({ .exempt = true, .vehicle = true }), UnitClass::Unchanged);
	EXPECT_EQ(classify({ .structure = true, .infantry = true }), UnitClass::Structure);
	EXPECT_EQ(classify({ .aircraft = true, .vehicle = true }), UnitClass::Aircraft);	// helicopters
	EXPECT_EQ(classify({ .vehicle = true }), UnitClass::Vehicle);
	EXPECT_EQ(classify({ .infantry = true }), UnitClass::Infantry);
	EXPECT_EQ(classify({}), UnitClass::Unchanged);
}

TEST(PacingBaseName, DropsTheGeneralsPrefix)
{
	EXPECT_EQ(baseName("Tank_ChinaTankOverlord"), "ChinaTankOverlord");
	EXPECT_EQ(baseName("ChinaTankOverlord"), "ChinaTankOverlord");
	EXPECT_EQ(baseName("Odd_"), "Odd_");
}

TEST(PacingScale, ClassUnlessTheObjectHasItsOwnLine)
{
	const Rules rules = sampleRules();
	EXPECT_FLOAT_EQ(scaleFor(rules, "AmericaInfantryRanger", UnitClass::Infantry).cost, 1.5f);
	EXPECT_FLOAT_EQ(scaleFor(rules, "AmericaTankCrusader", UnitClass::Vehicle).buildTime, 1.5f);
	EXPECT_FLOAT_EQ(scaleFor(rules, "AmericaVehicleHumvee", UnitClass::Vehicle).cost, 1.25f);
	EXPECT_FLOAT_EQ(scaleFor(rules, "americavehiclehumvee", UnitClass::Vehicle).cost, 1.25f);
	EXPECT_FLOAT_EQ(scaleFor(rules, "AmericaDozer", UnitClass::Unchanged).cost, 1.0f);
	EXPECT_FLOAT_EQ(scaleFor(rules, "AmericaBarracks", UnitClass::Structure).cost, 1.0f);	// no structure multiplier set
}

TEST(PacingScale, AGeneralsVersionUsesThePlainUnitsLine)
{
	const Rules rules = sampleRules();
	EXPECT_FLOAT_EQ(scaleFor(rules, "AirF_AmericaVehicleHumvee", UnitClass::Vehicle).cost, 1.25f);
	EXPECT_FLOAT_EQ(scaleFor(rules, "Tank_ChinaTankOverlord", UnitClass::Vehicle).cost, 3.0f);	// its own line
	EXPECT_FLOAT_EQ(scaleFor(rules, "ChinaTankOverlord", UnitClass::Vehicle).cost, 2.0f);	// the class
}

TEST(PacingScaled, RoundsToTheStep)
{
	EXPECT_EQ(scaled(225, 1.5f, 5), 340);	// 337.5
	EXPECT_EQ(scaled(900, 1.5f, 5), 1350);
	EXPECT_EQ(scaled(300, 1.0f, 5), 300);
	EXPECT_EQ(scaled(0, 2.0f, 5), 0);		// free stays free
	EXPECT_EQ(scaled(10, 0.01f, 5), 5);		// a cost stays at least one step
	EXPECT_EQ(scaled(150, 1.25f), 188);		// frames: 187.5
}

TEST(PacingIncome, ScalesDownAndKeepsWholeDollars)
{
	EXPECT_EQ(scaledIncome(50, 1.0f), 50u);
	EXPECT_EQ(scaledIncome(75, 0.8f), 60u);
	EXPECT_EQ(scaledIncome(75, 0.5f), 37u);
	EXPECT_EQ(scaledIncome(75, 0.0f), 0u);
	EXPECT_EQ(scaledIncome(100, 1.2f), 120u);
}
