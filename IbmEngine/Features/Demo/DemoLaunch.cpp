// IBM Engine: "-demo [map]" on the command line starts a skirmish straight from startup, skipping
// the menus: the player (with their skirmish preferences' name, side and colour) against one easy AI
// on a random side, with the Demo flag on. "-demoSide <PlayerTemplate>" (FactionGLA) picks the player's side. The map defaults to the Demo slice's blank IbmDemo map,
// which install.ps1 puts in the user data folder's Maps folder.
// See README.md.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/GlobalData.h"
#include "Common/MessageStream.h"
#include "Common/PlayerTemplate.h"
#include "Common/RandomValue.h"
#include "Common/SkirmishPreferences.h"
#include "GameClient/MapUtil.h"
#include "GameLogic/GameLogic.h"
#include "GameNetwork/GameInfo.h"

#include "Features/Demo/Demo.h"
#include "Features/FeatureFlags/FeatureFlags.h"

namespace
{

const char *const DEFAULT_DEMO_MAP = "IbmDemo\\IbmDemo.map";	///< in the user data folder's Maps folder

// The map after "-demo", "" for the default, or nullptr when "-demo" isn't on the command line.
const char *findDemoArgument()
{
	for (Int arg = 1; arg < __argc; ++arg)
	{
		if (stricmp(__argv[arg], "-demo") != 0)
			continue;
		if (arg + 1 < __argc && __argv[arg + 1][0] != '-')
			return __argv[arg + 1];
		return "";
	}
	return nullptr;
}

// "-demoSide FactionGLA": the player's side, instead of their skirmish preference.
Int findDemoSide(Int preferred)
{
	for (Int arg = 1; arg + 1 < __argc; ++arg)
	{
		if (stricmp(__argv[arg], "-demoSide") != 0)
			continue;
		const Int side = ThePlayerTemplateStore->getTemplateNumByName(__argv[arg + 1]);
		if (side >= 0)
			return side;
		DEBUG_LOG(("IbmEngine -demo: no side '%s'", __argv[arg + 1]));
	}
	return preferred;
}

} // namespace

//-------------------------------------------------------------------------------------------------
Bool IbmEngine::startDemoGame()
{
	const char *argument = findDemoArgument();
	if (argument == nullptr)
		return FALSE;

	if (TheMapCache == nullptr)
		return FALSE;

	// The game finds new maps only in the user data folder's Maps folder, and official ones by their
	// "Maps\..." path. A map given as neither is looked for in the user maps.
	AsciiString map = *argument != '\0' ? AsciiString(argument) : AsciiString(DEFAULT_DEMO_MAP);
	const MapMetaData *md = TheMapCache->findMap(map);
	if (md == nullptr)
	{
		AsciiString userMap;
		userMap.format("%s\\%s", TheMapCache->getUserMapDir().str(), map.str());
		md = TheMapCache->findMap(userMap);
		if (md != nullptr)
			map = userMap;
	}
	if (md == nullptr)
	{
		DEBUG_LOG(("IbmEngine -demo: map '%s' not found; showing the menus instead", map.str()));
		return FALSE;
	}

	TheWritableGlobalData->m_shellMapOn = FALSE;
	TheWritableGlobalData->m_playIntro = FALSE;

	// The same setup as the skirmish menu (SkirmishGameOptionsMenu's startPressed and reallyDoStart).
	if (TheSkirmishGameInfo == nullptr)
		TheSkirmishGameInfo = NEW SkirmishGameInfo;
	TheSkirmishGameInfo->init();
	TheSkirmishGameInfo->clearSlotList();
	TheSkirmishGameInfo->reset();
	TheSkirmishGameInfo->setLocalIP(TheSkirmishGameInfo->getSlot(0)->getIP());
	TheSkirmishGameInfo->enterGame();

	SkirmishPreferences prefs;

	GameSlot human;
	human.setName(prefs.getUserName());
	human.setState(SLOT_PLAYER, prefs.getUserName());
	human.setColor(prefs.getPreferredColor());
	human.setPlayerTemplate(findDemoSide(prefs.getPreferredFaction()));
	human.setStartPos(0);
	TheSkirmishGameInfo->setSlot(0, human);

	GameSlot ai;
	ai.setState(SLOT_EASY_AI);
	ai.setColor(-1);
	ai.setPlayerTemplate(PLAYERTEMPLATE_RANDOM);
	ai.setStartPos(1);
	TheSkirmishGameInfo->setSlot(1, ai);

	TheSkirmishGameInfo->setSeed(GetTickCount());
	TheSkirmishGameInfo->setFeatureFlags(getLocalFeatures() | featureBit(Feature_Demo));
	TheSkirmishGameInfo->setStartingCash(prefs.getStartingCash());
	TheSkirmishGameInfo->setSuperweaponRestriction(0);
	TheSkirmishGameInfo->setMap(map);
	TheSkirmishGameInfo->setMapCRC(md->m_CRC);
	TheSkirmishGameInfo->setMapSize(md->m_filesize);

	TheWritableGlobalData->m_mapName = map;
	TheSkirmishGameInfo->startGame(0);

	Int maxFPS = max(15, min(61, prefs.getInt("FPS", TheGlobalData->m_framesPerSecondLimit)));
	if (maxFPS > 60)
		maxFPS = 1000;

	InitRandom(TheSkirmishGameInfo->getSeed());
	GameMessage *msg = TheMessageStream->appendMessage(GameMessage::MSG_NEW_GAME);
	msg->appendIntegerArgument(GAME_SKIRMISH);
	msg->appendIntegerArgument(DIFFICULTY_NORMAL);
	msg->appendIntegerArgument(0);
	msg->appendIntegerArgument(maxFPS);

	DEBUG_LOG(("IbmEngine -demo: starting a skirmish on '%s'", map.str()));
	return TRUE;
}
