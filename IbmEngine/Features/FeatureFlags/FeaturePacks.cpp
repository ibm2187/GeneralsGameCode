// IBM Engine: decides a starting game's feature flags and loads their INI packs.
// A pack is a folder Data\INI\Features\<Name>\ of INI files. It loads in override mode, the
// mode map.ini uses, so it can add new objects and change existing ones, and the game removes
// it again when the game ends. Files load in name order, so packs number their files.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/FileSystem.h"
#include "Common/INI.h"
#include "GameClient/ControlBar.h"
#include "GameClient/GameText.h"
#include "GameLogic/GameLogic.h"
#include "GameNetwork/GameInfo.h"

#include "Features/FeatureFlags/FeatureFlagsInternal.h"

namespace IbmEngine
{

static void loadFeaturePack(Feature feature)
{
	const char *name = getFeatureName(feature);

	AsciiString dir;
	dir.format("Data\\INI\\Features\\%s", name);

	// Strings first: object names (DisplayName) are looked up while the INI is parsed.
	// Same mechanism as map.str. A map that has its own map.str replaces these strings.
	AsciiString strFile;
	strFile.format("%s\\%s.str", dir.str(), name);
	if (TheFileSystem->doesFileExist(strFile.str()))
		TheGameText->initMapStringFile(strFile);

	INI ini;
	const UnsignedInt filesRead = ini.loadDirectory(dir, INI_LOAD_CREATE_OVERRIDES, nullptr);
	DEBUG_LOG(("IbmEngine: loaded %u INI files of feature pack '%s'", filesRead, name));
	DEBUG_ASSERTCRASH(filesRead > 0, ("Feature '%s' is on, but its pack %s is missing or empty", name, dir.str()));
	(void)filesRead; // Only used by debug builds.
}

void onStartNewGame(Int gameMode)
{
	FeatureMask features;

	if (gameMode == GAME_SHELL)
		features = 0; // The main menu's background map always plays the base game.
	else if (TheGameInfo != nullptr)
		features = TheGameInfo->getFeatureFlags(); // Skirmish, LAN and replays use the game's options.
	else
		features = getLocalFeatures();

	setActiveFeatures(features);
	DEBUG_LOG(("IbmEngine: active feature flags = %X (game mode %d)", features, gameMode));

	Bool loadedPack = FALSE;
	for (Int i = 0; i < Feature_Count; ++i)
	{
		if (features & featureBit((Feature)i))
		{
			loadFeaturePack((Feature)i);
			loadedPack = TRUE;
		}
	}

	// Button images are normally resolved once, after the startup INI load. Buttons a pack
	// adds need the same step, or they show no image.
	if (loadedPack && TheControlBar != nullptr)
		TheControlBar->postProcessCommands();
}

} // namespace IbmEngine
