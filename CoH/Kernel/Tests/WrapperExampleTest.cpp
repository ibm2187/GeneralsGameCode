// CoH Kernel: the feature example in CoH/README.md (Writing a feature, in C++20), compiled here so it
// keeps building as the wrapper changes. Nothing runs it: it needs a game.

#include "Precompiled/PreRTS.h"	// the engine's own code gets it from its include path

#include <gtest/gtest.h>

#include "GameLogic/Object.h"

#include "Kernel/Ini.h"
#include "Kernel/Kernel.h"
#include "Kernel/Objects.h"
#include "Kernel/Players.h"
#include "Kernel/Random.h"
#include "Kernel/Time.h"

#include <ranges>

// ---- As in the README ---------------------------------------------------------------------------

namespace CoH::FieldAid
{

namespace
{

struct Config	// the CoHFieldAid block of Data/INI/FieldAid.ini; these are the defaults
{
	Real radius = 150.0f;
	Real healChance = 0.5f;
	Real healAmount = 10.0f;
	UnsignedInt intervalFrames = framesFromSeconds(2.0f);
	KindOfMaskType medicKinds = kinds(KINDOF_HEAL_PAD);
};

Config &config()
{
	static Config theConfig;
	return theConfig;
}

void parseConfigBlock(INI *ini)
{
	static const Ini::Table<Config> fields = {
		Ini::field("Radius", &Config::radius),
		Ini::percent("HealChance", &Config::healChance),	// "50%"
		Ini::field("HealAmount", &Config::healAmount),
		Ini::seconds("Interval", &Config::intervalFrames),	// "2.0", kept as 60 frames
		Ini::field("MedicKinds", &Config::medicKinds),
	};
	fields.read(ini, config());
}

void onLogicFrame()
{
	const Config &settings = config();
	if (!isEvery(now(), settings.intervalFrames))
		return;
	for (Player *player : activeSides())
	{
		for (Object *medic : objectsOf(*player, { .anyKind = settings.medicKinds }))
		{
			for (Object *patient : objectsNear(*medic, settings.radius, { .alliesOf = medic, .anyKind = kinds(KINDOF_INFANTRY) }))
			{
				if (chance(settings.healChance))	// synced: one draw per patient, in the engine's order
					patient->attemptHealing(settings.healAmount, medic);
			}
		}
	}
}

void init(Feature &self)
{
	registerIniBlock("CoHFieldAid", parseConfigBlock);
	Hooks::logicFrame().subscribe(self, onLogicFrame);
}

void reset()
{
	config() = Config();
}

} // namespace

} // namespace CoH::FieldAid

COH_FEATURE(FieldAid, CoH::FieldAid::init, CoH::FieldAid::reset)

// ---- The ranges work with std::views ------------------------------------------------------------

static_assert(std::ranges::input_range<CoH::ObjectRange &>);
static_assert(std::ranges::input_range<decltype(CoH::activeSides())>);

TEST(KernelExample, TheReadmeFeatureKeepsItsDefaults)
{
	CoH::FieldAid::reset();
	EXPECT_EQ(CoH::FieldAid::config().intervalFrames, 60u);
	EXPECT_TRUE(CoH::FieldAid::config().medicKinds.test(KINDOF_HEAL_PAD));
}
