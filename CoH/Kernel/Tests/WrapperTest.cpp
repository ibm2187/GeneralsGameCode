// CoH Kernel: tests for the C++20 wrapper's pure parts: frames, chance rolls, synced picks and INI
// field tables.

#include <gtest/gtest.h>

#include "Common/RandomValue.h"

#include "Kernel/Domain/Chance.h"
#include "Kernel/Domain/Frames.h"
#include "Kernel/Ini.h"
#include "Kernel/Random.h"

#include <array>
#include <cstddef>
#include <vector>

// ---- Frames -------------------------------------------------------------------------------------

static_assert(CoH::framesFromSeconds(1.0f) == 30);
static_assert(CoH::isEvery(60, 30));

TEST(KernelFrames, SecondsRoundUpToWholeFrames)
{
	EXPECT_EQ(CoH::framesFromSeconds(2.5f), 75u);
	EXPECT_EQ(CoH::framesFromSeconds(0.01f), 1u);
	EXPECT_EQ(CoH::framesFromSeconds(1.02f), 31u);
}

TEST(KernelFrames, FloatNoiseDoesNotAddAFrame)
{
	EXPECT_EQ(CoH::framesFromSeconds(0.1f), 3u);
	EXPECT_EQ(CoH::framesFromSeconds(0.7f), 21u);
	EXPECT_EQ(CoH::framesFromSeconds(1.1f), 33u);
}

TEST(KernelFrames, NoTimeIsNoFrames)
{
	EXPECT_EQ(CoH::framesFromSeconds(0.0f), 0u);
	EXPECT_EQ(CoH::framesFromSeconds(-3.0f), 0u);
	EXPECT_EQ(CoH::framesFromSeconds(1.0e12f), 4294967295u);
}

TEST(KernelFrames, FramesToSeconds)
{
	EXPECT_FLOAT_EQ(CoH::secondsFromFrames(45), 1.5f);
	EXPECT_FLOAT_EQ(CoH::secondsFromFrames(0), 0.0f);
}

TEST(KernelFrames, PeriodsStartOnMultiplesShiftedByTheOffset)
{
	EXPECT_TRUE(CoH::isEvery(0, 30));
	EXPECT_FALSE(CoH::isEvery(61, 30));
	EXPECT_TRUE(CoH::isEvery(58, 30, 2));
	EXPECT_FALSE(CoH::isEvery(60, 30, 2));
	EXPECT_FALSE(CoH::isEvery(0, 0));
}

// ---- Chance -------------------------------------------------------------------------------------

TEST(KernelChance, RollsBelowTheProbabilityPass)
{
	EXPECT_TRUE(CoH::rollPasses(0.24f, 0.25f));
	EXPECT_FALSE(CoH::rollPasses(0.25f, 0.25f));
	EXPECT_FALSE(CoH::rollPasses(1.0f, 0.999f));
}

TEST(KernelChance, NoneNeverPassesAndCertainAlwaysDoes)
{
	EXPECT_FALSE(CoH::rollPasses(0.0f, 0.0f));
	EXPECT_FALSE(CoH::rollPasses(0.0f, -1.0f));
	EXPECT_TRUE(CoH::rollPasses(1.0f, 1.0f));
	EXPECT_TRUE(CoH::rollPasses(1.0f, 2.0f));
}

// ---- Random (synced, so the same seed gives the same game) ---------------------------------------

namespace
{

// The next synced number, to see whether the draws before it moved the sequence.
Int nextDraw()
{
	return GameLogicRandomValue(0, 1000000);
}

} // namespace

TEST(KernelRandom, TheSameSeedPicksTheSame)
{
	const std::array<int, 5> items = { 10, 20, 30, 40, 50 };
	std::vector<int> first;
	std::vector<int> second;
	InitRandom(1234);
	for (int i = 0; i < 50; ++i)
		first.push_back(*CoH::pick(items));
	InitRandom(1234);
	for (int i = 0; i < 50; ++i)
		second.push_back(*CoH::pick(items));
	EXPECT_EQ(first, second);
}

TEST(KernelRandom, PicksAreFromTheItems)
{
	std::vector<int> items = { 1, 2, 3 };
	InitRandom(99);
	for (int i = 0; i < 100; ++i)
	{
		const int *picked = CoH::pick(items);
		ASSERT_NE(picked, nullptr);
		EXPECT_GE(picked, items.data());
		EXPECT_LT(picked, items.data() + items.size());
	}
}

TEST(KernelRandom, NoneOrOneItemDrawsNothing)
{
	std::vector<int> none;
	std::array<int, 1> one = { 7 };
	InitRandom(5);
	const Int expected = nextDraw();

	InitRandom(5);
	EXPECT_EQ(CoH::pick(none), nullptr);
	EXPECT_EQ(CoH::pick(one), &one[0]);
	EXPECT_EQ(CoH::between(4, 4), 4);
	EXPECT_EQ(nextDraw(), expected);
}

TEST(KernelRandom, ChanceDrawsOnceWhateverTheProbability)
{
	InitRandom(5);
	GameLogicRandomValueReal(0.0f, 1.0f);
	GameLogicRandomValueReal(0.0f, 1.0f);
	const Int expected = nextDraw();

	InitRandom(5);
	EXPECT_FALSE(CoH::chance(0.0f));
	EXPECT_TRUE(CoH::chance(1.0f));
	EXPECT_EQ(nextDraw(), expected);
}

TEST(KernelRandom, ChanceMatchesItsProbability)
{
	InitRandom(42);
	int passed = 0;
	for (int i = 0; i < 4000; ++i)
		passed += CoH::chance(0.25f) ? 1 : 0;
	EXPECT_NEAR(passed, 1000, 150);
}

// ---- INI fields ---------------------------------------------------------------------------------

namespace
{

struct Rules
{
	float bonus = 0.0f;
	unsigned int delayFrames = 0;
};

struct Settings
{
	int count = 0;
	Rules rules;
	float radius = 0.0f;
	float missChance = 0.0f;
	bool enabled = false;
	KindOfMaskType kinds;
};

void parseNothing(INI *, void *, void *, const void *)
{
}

} // namespace

static_assert(CoH::Ini::Parsable<float>);
static_assert(CoH::Ini::Parsable<KindOfMaskType>);
static_assert(!CoH::Ini::Parsable<double>);

TEST(KernelIni, FieldsFindTheirMembersAndParsers)
{
	const CoH::Ini::Table<Settings> table = {
		CoH::Ini::field("Count", &Settings::count),
		CoH::Ini::field("Bonus", &Settings::rules, &Rules::bonus),
		CoH::Ini::seconds("Delay", &Settings::rules, &Rules::delayFrames),
		CoH::Ini::percent("MissChance", &Settings::missChance),
		CoH::Ini::field("Enabled", &Settings::enabled),
		CoH::Ini::field("Kinds", &Settings::kinds),
		CoH::Ini::custom<Settings>("Extra", &parseNothing),
	};
	const FieldParse *fields = table;

	EXPECT_STREQ(fields[0].token, "Count");
	EXPECT_EQ(fields[0].parse, &INI::parseInt);
	EXPECT_EQ(fields[0].offset, (Int)offsetof(Settings, count));

	EXPECT_EQ(fields[1].parse, &INI::parseReal);
	EXPECT_EQ(fields[1].offset, (Int)offsetof(Settings, rules.bonus));

	EXPECT_EQ(fields[2].parse, &CoH::Ini::parseSecondsAsFrames);
	EXPECT_EQ(fields[2].offset, (Int)offsetof(Settings, rules.delayFrames));

	EXPECT_EQ(fields[3].parse, &INI::parsePercentToReal);
	EXPECT_EQ(fields[3].offset, (Int)offsetof(Settings, missChance));

	EXPECT_EQ(fields[4].parse, &INI::parseBool);
	EXPECT_EQ(fields[5].parse, &KindOfMaskType::parseFromINI);
	EXPECT_EQ(fields[5].offset, (Int)offsetof(Settings, kinds));

	EXPECT_EQ(fields[6].parse, &parseNothing);
	EXPECT_EQ(fields[6].offset, 0);
}

TEST(KernelIni, TheTableEndsLikeTheEnginesTables)
{
	const CoH::Ini::Table<Settings> table = { CoH::Ini::field("Radius", &Settings::radius) };
	const FieldParse *fields = table;
	EXPECT_EQ(fields[1].token, nullptr);
	EXPECT_EQ(fields[1].parse, nullptr);
}
