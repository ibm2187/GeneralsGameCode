// CoH Kernel: tests for flag sets, the game options key and the Kernel's command-line options.

#include <gtest/gtest.h>

#include "Kernel/Domain/Arguments.h"
#include "Kernel/Domain/FeatureSet.h"
#include "Kernel/Domain/GameFlags.h"

using CoH::FeatureSet;

TEST(FeatureSet, ListsAreTrimmedSortedAndWithoutDuplicates)
{
	const FeatureSet set = FeatureSet::fromList(" Sectors, accuracy ,,SECTORS");
	EXPECT_EQ(set.toList(), "accuracy,Sectors");
	EXPECT_TRUE(set.contains("Accuracy"));
	EXPECT_FALSE(set.contains("Squads"));
}

TEST(FeatureSet, EmptyAndNoneAreNoFeatures)
{
	EXPECT_TRUE(FeatureSet::fromList("").empty());
	EXPECT_TRUE(FeatureSet::fromList(" None ").empty());
	EXPECT_EQ(FeatureSet().toList(), "");
}

TEST(FeatureSet, EqualityIgnoresCase)
{
	EXPECT_EQ(FeatureSet::fromList("Sectors,Accuracy"), FeatureSet::fromList("accuracy,sectors"));
	EXPECT_NE(FeatureSet::fromList("Sectors"), FeatureSet::fromList("Sectors,Accuracy"));
}

TEST(FeatureSet, SettingsFileTurnsOnTheYesLines)
{
	const char *text =
		"; CoH features for local games\r\n"
		"Sectors = yes\r\n"
		"Accuracy = No\r\n"
		"Squads=TRUE   # comment\r\n"
		"Cover = 1\n"
		"Broken line\n"
		"Retreat = yes ; until it works\n";
	EXPECT_EQ(FeatureSet::fromSettingsFile(text).toList(), "Cover,Retreat,Sectors,Squads");
	EXPECT_TRUE(FeatureSet::fromSettingsFile("").empty());
}

TEST(GameFlags, NoFlagsLeaveTheOptionsStringAlone)
{
	EXPECT_EQ(CoH::formatGameOption(FeatureSet()), "");
}

TEST(GameFlags, FlagsRoundTripThroughTheOptionsString)
{
	const FeatureSet flags = FeatureSet::fromList("Sectors,Accuracy");
	const std::string options = "US=1;M=00maps/tournament desert;MC=1A2B;SD=42;S=HPlayer,0,0,TT,-1,0,0,0,1:;" +
		CoH::formatGameOption(flags);
	EXPECT_EQ(CoH::formatGameOption(flags), "CoH=Accuracy,Sectors;");
	EXPECT_EQ(CoH::findGameOption(options), flags);
}

TEST(GameFlags, OptionsWithoutTheKeyHaveNoFlags)
{
	EXPECT_TRUE(CoH::findGameOption("US=1;M=00maps/x;SD=42;S=O:O:;").empty());
	EXPECT_TRUE(CoH::findGameOption("").empty());
	EXPECT_TRUE(CoH::findGameOption("CoHX=Sectors;").empty());
}

TEST(GameFlags, EachGameOriginPlaysWithItsOwnFlags)
{
	const FeatureSet local = FeatureSet::fromList("Sectors");
	const FeatureSet host = FeatureSet::fromList("Accuracy");
	const FeatureSet saved = FeatureSet::fromList("Squads");

	EXPECT_TRUE(CoH::chooseGameFlags(CoH::GameOrigin::ShellMap, local, host, saved).empty());
	EXPECT_EQ(CoH::chooseGameFlags(CoH::GameOrigin::LocalGame, local, host, saved), local);
	EXPECT_EQ(CoH::chooseGameFlags(CoH::GameOrigin::RemoteGame, local, host, saved), host);
	EXPECT_EQ(CoH::chooseGameFlags(CoH::GameOrigin::Replay, local, host, saved), host);
	EXPECT_EQ(CoH::chooseGameFlags(CoH::GameOrigin::SaveGame, local, host, saved), saved);
}

TEST(Arguments, FindsAnOptionsValue)
{
	const std::vector<std::string> arguments = { "-win", "-COH", "Sectors,Accuracy", "-cohSkirmish" };
	EXPECT_EQ(CoH::findOption(arguments, "-coh"), std::optional<std::string>("Sectors,Accuracy"));
	EXPECT_EQ(CoH::findOption(arguments, "-cohSkirmish"), std::optional<std::string>(""));
	EXPECT_EQ(CoH::findOption(arguments, "-cohSide"), std::nullopt);
}

TEST(Arguments, AnOptionFollowedByAnotherOptionHasNoValue)
{
	const std::vector<std::string> arguments = { "-cohSkirmish", "-coh", "none", "-coh", "Sectors" };
	EXPECT_EQ(CoH::findOption(arguments, "-cohSkirmish"), std::optional<std::string>(""));
	EXPECT_EQ(CoH::findOption(arguments, "-coh"), std::optional<std::string>("Sectors"));
}
