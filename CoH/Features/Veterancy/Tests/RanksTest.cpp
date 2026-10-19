// CoH Veterancy: tests for the rank tables: classes, a unit's own lines, generals' versions, the tooltip.

#include <gtest/gtest.h>

#include "Features/Veterancy/Domain/Ranks.h"

using namespace CoH::Veterancy;

namespace
{

constexpr std::size_t at(Rank rank) { return static_cast<std::size_t>(rank); }

Rules someRules()
{
	Rules rules;
	RankTable &infantry = rules.classes[static_cast<std::size_t>(UnitClass::Infantry)];
	infantry[at(Rank::Veteran)].accuracy = 1.15f;
	infantry[at(Rank::Veteran)].damageTaken = 0.9f;
	infantry[at(Rank::Elite)].accuracy = 1.3f;
	infantry[at(Rank::Heroic)].accuracy = 1.5f;
	infantry[at(Rank::Heroic)].suppressionResistance = 0.6f;

	RankTable &vehicle = rules.classes[static_cast<std::size_t>(UnitClass::Vehicle)];
	vehicle[at(Rank::Veteran)].speed = 1.05f;
	vehicle[at(Rank::Veteran)].reload = 1.15f;
	return rules;
}

ObjectLine line(const char *name, Rank rank, BonusChange change)
{
	return { name, rank, change };
}

} // namespace

TEST(VeterancyRanks, RanksFollowTheEngineLevels)
{
	EXPECT_EQ(rankOf(0), Rank::Regular);
	EXPECT_EQ(rankOf(1), Rank::Veteran);
	EXPECT_EQ(rankOf(2), Rank::Elite);
	EXPECT_EQ(rankOf(3), Rank::Heroic);
	EXPECT_EQ(rankOf(-1), Rank::Regular);
	EXPECT_EQ(rankOf(7), Rank::Heroic);
}

TEST(VeterancyRanks, AHelicopterIsAnAircraft)
{
	EXPECT_EQ(classify({ .aircraft = true, .vehicle = true }), UnitClass::Aircraft);
	EXPECT_EQ(classify({ .vehicle = true }), UnitClass::Vehicle);
	EXPECT_EQ(classify({ .infantry = true }), UnitClass::Infantry);
	EXPECT_EQ(classify({}), UnitClass::Other);
}

TEST(VeterancyRanks, AUnitTakesItsClassTable)
{
	const RankTable table = tableFor(someRules(), "AmericaInfantryRanger", UnitClass::Infantry);
	EXPECT_FLOAT_EQ(table[at(Rank::Regular)].accuracy, 1.0f);
	EXPECT_FLOAT_EQ(table[at(Rank::Veteran)].accuracy, 1.15f);
	EXPECT_FLOAT_EQ(table[at(Rank::Veteran)].damageTaken, 0.9f);
	EXPECT_FLOAT_EQ(table[at(Rank::Elite)].accuracy, 1.3f);
	EXPECT_FLOAT_EQ(table[at(Rank::Heroic)].suppressionResistance, 0.6f);
	EXPECT_FLOAT_EQ(table[at(Rank::Heroic)].speed, 1.0f);
}

TEST(VeterancyRanks, AUnitOfNoClassHasNoBonuses)
{
	const RankTable table = tableFor(someRules(), "AmericaPowerPlant", UnitClass::Other);
	for (const RankBonus &bonus : table)
	{
		EXPECT_FLOAT_EQ(bonus.accuracy, 1.0f);
		EXPECT_FLOAT_EQ(bonus.reload, 1.0f);
		EXPECT_TRUE(changesOf(bonus).empty());
	}
}

TEST(VeterancyRanks, OwnLinesChangeOnlyWhatTheyName)
{
	Rules rules = someRules();
	rules.objects.push_back(line("AmericaVehicleHumvee", Rank::Elite, { .upgrade = std::string("Upgrade_AmericaBattleDrone") }));
	rules.objects.push_back(line("AmericaVehicleHumvee", Rank::Veteran, { .speed = 1.2f }));

	const RankTable table = tableFor(rules, "AmericaVehicleHumvee", UnitClass::Vehicle);
	EXPECT_FLOAT_EQ(table[at(Rank::Veteran)].speed, 1.2f);
	EXPECT_FLOAT_EQ(table[at(Rank::Veteran)].reload, 1.15f);	// the class's, kept
	EXPECT_EQ(table[at(Rank::Elite)].upgrade, "Upgrade_AmericaBattleDrone");
	EXPECT_TRUE(table[at(Rank::Veteran)].upgrade.empty());

	const RankTable other = tableFor(rules, "AmericaTankCrusader", UnitClass::Vehicle);
	EXPECT_FLOAT_EQ(other[at(Rank::Veteran)].speed, 1.05f);
	EXPECT_TRUE(other[at(Rank::Elite)].upgrade.empty());
}

TEST(VeterancyRanks, AGeneralsVersionTakesItsPlainUnitsLinesThenItsOwn)
{
	Rules rules = someRules();
	rules.objects.push_back(line("AirF_AmericaVehicleHumvee", Rank::Veteran, { .accuracy = 1.4f }));
	rules.objects.push_back(line("americavehiclehumvee", Rank::Veteran, { .accuracy = 1.25f, .sight = 1.3f }));

	const RankTable general = tableFor(rules, "AirF_AmericaVehicleHumvee", UnitClass::Vehicle);
	EXPECT_FLOAT_EQ(general[at(Rank::Veteran)].accuracy, 1.4f);	// its own wins
	EXPECT_FLOAT_EQ(general[at(Rank::Veteran)].sight, 1.3f);	// the plain unit's, without case
	EXPECT_FLOAT_EQ(general[at(Rank::Veteran)].reload, 1.15f);	// the class's

	const RankTable plain = tableFor(rules, "AmericaVehicleHumvee", UnitClass::Vehicle);
	EXPECT_FLOAT_EQ(plain[at(Rank::Veteran)].accuracy, 1.25f);
}

TEST(VeterancyRanks, BaseNameDropsTheGeneral)
{
	EXPECT_EQ(baseName("Tank_ChinaTankOverlord"), "ChinaTankOverlord");
	EXPECT_EQ(baseName("ChinaTankOverlord"), "ChinaTankOverlord");
	EXPECT_EQ(baseName("Odd_"), "Odd_");
}

TEST(VeterancyRanks, TheTooltipShowsWholePercents)
{
	RankBonus bonus;
	bonus.accuracy = 1.15f;
	bonus.damageTaken = 0.85f;
	bonus.suppressionResistance = 0.4f;
	bonus.speed = 1.0f;
	bonus.sight = 1.1f;
	bonus.reload = 1.5f;

	const std::vector<StatChange> changes = changesOf(bonus);
	ASSERT_EQ(changes.size(), 5u);
	EXPECT_EQ(changes[0].stat, Stat::Accuracy);
	EXPECT_EQ(changes[0].percent, 15);
	EXPECT_EQ(changes[1].stat, Stat::DamageTaken);
	EXPECT_EQ(changes[1].percent, -15);
	EXPECT_EQ(changes[2].stat, Stat::SuppressionResistance);
	EXPECT_EQ(changes[2].percent, 40);
	EXPECT_EQ(changes[3].stat, Stat::Sight);		// speed is unchanged, so not listed
	EXPECT_EQ(changes[3].percent, 10);
	EXPECT_EQ(changes[4].stat, Stat::Reload);
	EXPECT_EQ(changes[4].percent, 50);
}
