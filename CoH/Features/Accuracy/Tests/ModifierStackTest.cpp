// CoH Accuracy: tests for the hit modifier stack: its order, features turning off, and stand-ins.

#include <gtest/gtest.h>

#include "Features/Accuracy/Domain/ModifierStack.h"
#include "Kernel/Domain/Feature.h"

using namespace CoH::Accuracy;

namespace
{

HitModifier half(const Shot &) { return scaleChance(0.5f); }
HitModifier tenPointsOff(const Shot &) { return addToChance(-0.1f); }
HitModifier twentyPointsOn(const Shot &) { return addToChance(0.2f); }
HitModifier noChange(const Shot &) { return {}; }
HitModifier byVeterancy(const Shot &shot) { return scaleChance(1.0f + 0.1f * shot.veterancy); }

const CoH::FeatureInfo AccuracyInfo = { "Accuracy", nullptr, nullptr };
const CoH::FeatureInfo CoverInfo = { "Cover", nullptr, nullptr };
const CoH::FeatureInfo VeterancyInfo = { "Veterancy", nullptr, nullptr };

class ModifierStackTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		accuracy = registry.add(AccuracyInfo);
		cover = registry.add(CoverInfo);
		veterancy = registry.add(VeterancyInfo);
		turnOn("Accuracy,Cover,Veterancy");
	}

	void turnOn(const char *flags)
	{
		registry.activate(CoH::FeatureSet::fromList(flags));
	}

	CoH::FeatureRegistry registry;
	CoH::Feature *accuracy = nullptr;
	CoH::Feature *cover = nullptr;
	CoH::Feature *veterancy = nullptr;
	ModifierStack stack;
	Shot shot;
};

} // namespace

TEST_F(ModifierStackTest, AnEmptyStackLeavesTheBase)
{
	EXPECT_FLOAT_EQ(stack.apply(0.7f, shot), 0.7f);
	EXPECT_TRUE(stack.activeNames().empty());
}

TEST_F(ModifierStackTest, ModifiersApplyByOrderThenName)
{
	// Added out of order on purpose.
	stack.add(*cover, "Zeta", 100, tenPointsOff);
	stack.addBuiltIn(*accuracy, "Range", 100, half);
	stack.add(*cover, "Cover", 400, twentyPointsOn);
	stack.addBuiltIn(*accuracy, "Alpha", 100, noChange);

	const std::vector<std::string> expected = { "Alpha", "Range", "Zeta", "Cover" };
	EXPECT_EQ(stack.activeNames(), expected);

	// ((0.8 * 1) * 0.5 - 0.1) + 0.2
	EXPECT_FLOAT_EQ(stack.apply(0.8f, shot), 0.5f);
}

TEST_F(ModifierStackTest, TheOrderDecidesForModifiersThatAdd)
{
	stack.add(*cover, "Half", 200, half);
	stack.add(*cover, "Bonus", 100, twentyPointsOn);
	EXPECT_FLOAT_EQ(stack.apply(0.4f, shot), 0.3f);	// (0.4 + 0.2) * 0.5, not 0.4 * 0.5 + 0.2
}

TEST_F(ModifierStackTest, AModifierCountsOnlyWhileItsFeatureIsOn)
{
	stack.addBuiltIn(*accuracy, "Range", 100, half);
	stack.add(*cover, "Cover", 400, tenPointsOff);

	turnOn("Accuracy");
	EXPECT_FLOAT_EQ(stack.apply(0.8f, shot), 0.4f);
	EXPECT_EQ(stack.activeNames(), std::vector<std::string>{ "Range" });
}

TEST_F(ModifierStackTest, AnAddedModifierStandsInForTheBuiltInOneOfTheSameName)
{
	stack.add(*veterancy, "Veterancy", 600, half);	// added before the built-in one, as a slice listed earlier would be
	stack.addBuiltIn(*accuracy, "Veterancy", 600, byVeterancy);
	shot.veterancy = 2;

	EXPECT_FLOAT_EQ(stack.apply(0.8f, shot), 0.4f);
	EXPECT_EQ(stack.activeNames(), std::vector<std::string>{ "Veterancy" });

	// With the Veterancy slice off, the built-in one is back.
	turnOn("Accuracy");
	EXPECT_FLOAT_EQ(stack.apply(0.5f, shot), 0.6f);
}

TEST_F(ModifierStackTest, BetweenAddedModifiersTheLaterOneWins)
{
	stack.add(*cover, "Cover", 400, half);
	stack.add(*veterancy, "Cover", 400, tenPointsOff);
	EXPECT_FLOAT_EQ(stack.apply(0.8f, shot), 0.7f);

	turnOn("Accuracy,Cover");
	EXPECT_FLOAT_EQ(stack.apply(0.8f, shot), 0.4f);
}

TEST_F(ModifierStackTest, NamelessOrEmptyModifiersAreIgnored)
{
	stack.add(*cover, "", 100, half);
	stack.add(*cover, "Nothing", 100, nullptr);
	EXPECT_FLOAT_EQ(stack.apply(0.8f, shot), 0.8f);
}
