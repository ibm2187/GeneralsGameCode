// CoH Kernel: tests for the feature registry and the hook events.

#include <gtest/gtest.h>

#include "Kernel/Domain/Event.h"
#include "Kernel/Domain/Feature.h"

namespace
{

int initCalls = 0;
int resetCalls = 0;
CoH::Feature *initialized = nullptr;

void countInit(CoH::Feature &self)
{
	++initCalls;
	initialized = &self;
}

void countReset()
{
	++resetCalls;
}

const CoH::FeatureInfo Sectors = { "Sectors", countInit, countReset };
const CoH::FeatureInfo Accuracy = { "Accuracy", nullptr, nullptr };

class FeatureRegistryTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		initCalls = 0;
		resetCalls = 0;
		initialized = nullptr;
	}

	CoH::FeatureRegistry registry;
};

} // namespace

TEST_F(FeatureRegistryTest, FindsAddedFeaturesWithoutCase)
{
	CoH::Feature *sectors = registry.add(Sectors);
	ASSERT_NE(sectors, nullptr);
	EXPECT_EQ(sectors->name(), "Sectors");
	EXPECT_EQ(registry.find("sectors"), sectors);
	EXPECT_EQ(registry.find("Accuracy"), nullptr);
}

TEST_F(FeatureRegistryTest, RefusesASecondFeatureWithTheSameName)
{
	registry.add(Sectors);
	EXPECT_EQ(registry.add({ "SECTORS", nullptr, nullptr }), nullptr);
	EXPECT_EQ(registry.add({ "", nullptr, nullptr }), nullptr);
	EXPECT_EQ(registry.features().size(), 1u);
}

TEST_F(FeatureRegistryTest, FeaturesStartOff)
{
	EXPECT_FALSE(registry.add(Sectors)->isActive());
	EXPECT_TRUE(registry.active().empty());
}

TEST_F(FeatureRegistryTest, ActivateTurnsOnExactlyTheNamedFeatures)
{
	CoH::Feature *sectors = registry.add(Sectors);
	CoH::Feature *accuracy = registry.add(Accuracy);

	registry.activate(CoH::FeatureSet::fromList("accuracy,Unknown"));
	EXPECT_FALSE(sectors->isActive());
	EXPECT_TRUE(accuracy->isActive());
	EXPECT_EQ(registry.active(), CoH::FeatureSet::fromList("Accuracy"));

	registry.activate(CoH::FeatureSet::fromList("Sectors"));
	EXPECT_TRUE(sectors->isActive());
	EXPECT_FALSE(accuracy->isActive());

	registry.deactivateAll();
	EXPECT_TRUE(registry.active().empty());
}

TEST_F(FeatureRegistryTest, KnownKeepsRegisteredNamesInTheirOwnSpelling)
{
	registry.add(Sectors);
	registry.add(Accuracy);
	EXPECT_EQ(registry.known(CoH::FeatureSet::fromList("SECTORS,Squads,accuracy")).toList(), "Accuracy,Sectors");
}

TEST_F(FeatureRegistryTest, InitPassesTheFeatureItselfAndResetRunsForEveryFeature)
{
	CoH::Feature *sectors = registry.add(Sectors);
	registry.add(Accuracy);	// no init or reset functions: skipped

	registry.initAll();
	EXPECT_EQ(initCalls, 1);
	EXPECT_EQ(initialized, sectors);

	registry.resetAll();
	EXPECT_EQ(resetCalls, 1);
}

namespace
{

int framesSeen = 0;
CoH::Feature *lastObject = nullptr;

void countFrame()
{
	++framesSeen;
}

void rememberObject(CoH::Feature *object)
{
	lastObject = object;
}

} // namespace

TEST_F(FeatureRegistryTest, EventsReachOnlyTheSubscribersOfActiveFeatures)
{
	CoH::Feature *sectors = registry.add(Sectors);
	CoH::Feature *accuracy = registry.add(Accuracy);
	CoH::Event<> frame;
	frame.subscribe(*sectors, countFrame);
	frame.subscribe(*accuracy, countFrame);
	framesSeen = 0;

	frame.raise();
	EXPECT_EQ(framesSeen, 0);

	registry.activate(CoH::FeatureSet::fromList("Sectors"));
	frame.raise();
	EXPECT_EQ(framesSeen, 1);

	registry.activate(CoH::FeatureSet::fromList("Sectors,Accuracy"));
	frame.raise();
	EXPECT_EQ(framesSeen, 3);
}

TEST_F(FeatureRegistryTest, EventsPassTheirArguments)
{
	CoH::Feature *sectors = registry.add(Sectors);
	CoH::Event<CoH::Feature *> created;
	created.subscribe(*sectors, rememberObject);
	registry.activate(CoH::FeatureSet::fromList("Sectors"));
	lastObject = nullptr;

	created.raise(sectors);
	EXPECT_EQ(lastObject, sectors);
}
