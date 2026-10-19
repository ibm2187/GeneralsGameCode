// CoH Veterancy: tests that the rank's accuracy replaces Accuracy's built-in veterancy rather than
// stacking on it, and combines with the other hit modifiers (Cover's, say) as Accuracy.h says.
//
// The slice's code uses Accuracy only through Accuracy.h. This test also builds Accuracy's modifier
// stack itself, to check the hand-over as Accuracy really applies it.

#include <gtest/gtest.h>

#include "Features/Accuracy/Accuracy.h"
#include "Features/Accuracy/Domain/ModifierStack.h"
#include "Features/Veterancy/Domain/Ranks.h"
#include "Kernel/Domain/Feature.h"
#include "Kernel/Domain/FeatureSet.h"

using namespace CoH::Accuracy;
using CoH::Veterancy::Rank;
using CoH::Veterancy::RankTable;

namespace
{

RankTable &table()
{
	static RankTable ranks {};
	return ranks;
}

// As Engine/Veterancy.cpp's rankAccuracy, with the shot's rank standing for the shooter's.
HitModifier rankAccuracy(const Shot &shot)
{
	return scaleChance(table()[static_cast<std::size_t>(CoH::Veterancy::rankOf(shot.veterancy))].accuracy);
}

HitModifier builtInVeterancy(const Shot &shot) { return scaleChance(1.0f + 0.15f * shot.veterancy); }
HitModifier inCover(const Shot &) { return scaleChance(0.7f); }

const CoH::FeatureInfo AccuracyInfo = { "Accuracy", nullptr, nullptr };
const CoH::FeatureInfo CoverInfo = { "Cover", nullptr, nullptr };
const CoH::FeatureInfo VeterancyInfo = { "Veterancy", nullptr, nullptr };

class VeterancyAccuracy : public ::testing::Test
{
protected:
	void SetUp() override
	{
		table() = {};
		table()[1].accuracy = 1.10f;
		table()[2].accuracy = 1.30f;
		table()[3].accuracy = 1.60f;

		accuracy = registry.add(AccuracyInfo);
		cover = registry.add(CoverInfo);
		veterancy = registry.add(VeterancyInfo);

		// Veterancy's init may run before Accuracy adds its built-in one: it still stands in.
		stack.add(*veterancy, "Veterancy", ModifierOrder::Veterancy, rankAccuracy);
		stack.addBuiltIn(*accuracy, "Veterancy", ModifierOrder::Veterancy, builtInVeterancy);
		stack.add(*cover, "Cover", ModifierOrder::Cover, inCover);
	}

	void turnOn(const char *flags) { registry.activate(CoH::FeatureSet::fromList(flags)); }

	CoH::FeatureRegistry registry;
	CoH::Feature *accuracy = nullptr;
	CoH::Feature *cover = nullptr;
	CoH::Feature *veterancy = nullptr;
	ModifierStack stack;
	Shot shot;
};

} // namespace

TEST_F(VeterancyAccuracy, TheRankReplacesTheBuiltInVeterancy)
{
	turnOn("Accuracy,Veterancy");
	shot.veterancy = 2;
	EXPECT_FLOAT_EQ(stack.apply(0.5f, shot), 0.5f * 1.30f);	// not 0.5 x 1.30 x 1.30, nor x 1.30
	EXPECT_EQ(stack.activeNames(), std::vector<std::string>({ "Veterancy" }));
}

TEST_F(VeterancyAccuracy, WithVeterancyOffTheBuiltInOneIsBack)
{
	turnOn("Accuracy");
	shot.veterancy = 2;
	EXPECT_FLOAT_EQ(stack.apply(0.5f, shot), 0.5f * 1.30f);	// the built-in +15% a rank
	shot.veterancy = 3;
	EXPECT_FLOAT_EQ(stack.apply(0.5f, shot), 0.5f * 1.45f);
}

TEST_F(VeterancyAccuracy, ARegularIsUnchanged)
{
	turnOn("Accuracy,Veterancy");
	shot.veterancy = 0;
	EXPECT_FLOAT_EQ(stack.apply(0.4f, shot), 0.4f);
}

TEST_F(VeterancyAccuracy, TheRankCombinesWithCover)
{
	turnOn("Accuracy,Cover,Veterancy");
	shot.veterancy = 3;
	EXPECT_FLOAT_EQ(stack.apply(0.5f, shot), 0.5f * 0.7f * 1.60f);
	EXPECT_EQ(stack.activeNames(), std::vector<std::string>({ "Cover", "Veterancy" }));
}
