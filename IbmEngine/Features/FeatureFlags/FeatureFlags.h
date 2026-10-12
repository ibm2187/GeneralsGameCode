// IBM Engine: feature flags.
// A feature flag switches one gameplay feature on or off. Logic features are fixed when a
// game starts, travel with the game options (replays, LAN games) and may load an INI pack
// from Data\INI\Features\<Name>\ on top of the base game.

#pragma once

class INI;

namespace IbmEngine
{

enum Feature
{
	Feature_Squads,
	Feature_SquadsDemo,	///< test setup at game start (Squads slice, SquadDemo.cpp); needs Squads, has no pack
	Feature_Accuracy,	///< direct fire can miss; veterancy, range, movement and cover change the odds
	Feature_Economy,	///< fewer, tougher units: vehicles and aircraft cost more, build slower, move slower, last longer

	Feature_Count
};

typedef UnsignedInt FeatureMask;

inline FeatureMask featureBit(Feature feature) { return 1u << feature; }

// The name used in Features.ini, on the command line and for the INI pack folder.
const char *getFeatureName(Feature feature);

// Reads the local flags from Features.ini in the user data folder, then applies
// "-feature Name=1" or "-feature Name=0" from the command line. Called once at engine init.
void initFeatureFlags();

// The flags this PC would use for a game it hosts or plays alone.
FeatureMask getLocalFeatures();

// The flags of the game in progress. Logic code must test these, never the local flags.
FeatureMask getActiveFeatures();
Bool isFeatureActive(Feature feature);

// Decides the active flags for a game that is starting and loads their INI packs.
// Called by GameLogic::startNewGame before the map loads. gameMode is a GameMode value.
void onStartNewGame(Int gameMode);

// INI block "ObjectCopy NewName SourceName": copies a whole object, then parses the
// following fields on top of it, including AddModule, RemoveModule and ReplaceModule.
void parseObjectCopyDefinition(INI *ini);

} // namespace IbmEngine
