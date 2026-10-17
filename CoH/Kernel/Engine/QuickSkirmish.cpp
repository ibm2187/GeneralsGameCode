// CoH Kernel: a quick skirmish for play-testing, started straight from startup without the menus
// (CoH/scripts/skirmish.ps1). On the command line:
//
//     -cohSkirmish [map]       start a skirmish; the map is a folder under Maps ("Tournament Desert",
//                              the default) or a map path the map list knows
//     -cohSide <template>      the player's side, such as FactionAmericaAirForceGeneral; default: the
//                              skirmish preferences' side
//     -cohAI easy|medium|hard  the one AI opponent, on a random side; default: medium
//
// The game plays with the local flags, so "-coh Sectors" turns Sectors on. It is set up like the
// skirmish menu sets up a game (SkirmishGameOptionsMenu), so it records a replay like one.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/GlobalData.h"
#include "Common/MessageStream.h"
#include "Common/PlayerTemplate.h"
#include "Common/RandomValue.h"
#include "Common/SkirmishPreferences.h"
#include "GameClient/MapUtil.h"
#include "GameLogic/GameLogic.h"
#include "GameNetwork/GameInfo.h"

#include "Kernel/Domain/Arguments.h"
#include "Kernel/Domain/Text.h"
#include "Kernel/Engine/Internal.h"

namespace CoH::QuickSkirmish
{

namespace
{

const char *const DEFAULT_MAP = "Tournament Desert";

// "Tournament Desert" is Maps\Tournament Desert\Tournament Desert.map, among the game's maps or the
// user's. A full map path is used as it is.
AsciiString findMapPath(const std::string &map)
{
	const AsciiString given = map.empty() ? DEFAULT_MAP : map.c_str();
	if (TheMapCache->findMap(given) != nullptr)
		return given;

	AsciiString gameMap;
	gameMap.format("Maps\\%s\\%s.map", given.str(), given.str());
	if (TheMapCache->findMap(gameMap) != nullptr)
		return gameMap;

	AsciiString userMap;
	userMap.format("%s\\%s\\%s.map", TheMapCache->getUserMapDir().str(), given.str(), given.str());
	if (TheMapCache->findMap(userMap) != nullptr)
		return userMap;

	return AsciiString::TheEmptyString;
}

Int findPlayerSide(const std::vector<std::string> &arguments, Int preferred)
{
	const std::optional<std::string> side = findOption(arguments, "-cohSide");
	if (!side || side->empty())
		return preferred;

	const Int found = ThePlayerTemplateStore->getTemplateNumByName(side->c_str());
	return found >= 0 ? found : preferred;
}

SlotState findOpponent(const std::vector<std::string> &arguments)
{
	const std::optional<std::string> ai = findOption(arguments, "-cohAI");
	if (ai && Text::equalsNoCase(*ai, "easy"))
		return SLOT_EASY_AI;
	if (ai && Text::equalsNoCase(*ai, "hard"))
		return SLOT_BRUTAL_AI;
	return SLOT_MED_AI;
}

void setUpGame(const AsciiString &map, const MapMetaData &mapData, const std::vector<std::string> &arguments)
{
	if (TheSkirmishGameInfo == nullptr)
		TheSkirmishGameInfo = NEW SkirmishGameInfo;
	TheSkirmishGameInfo->init();
	TheSkirmishGameInfo->clearSlotList();
	TheSkirmishGameInfo->reset();
	TheSkirmishGameInfo->setLocalIP(TheSkirmishGameInfo->getSlot(0)->getIP());
	TheSkirmishGameInfo->enterGame();

	SkirmishPreferences prefs;

	GameSlot player;
	player.setName(prefs.getUserName());
	player.setState(SLOT_PLAYER, prefs.getUserName());
	player.setColor(prefs.getPreferredColor());
	player.setPlayerTemplate(findPlayerSide(arguments, prefs.getPreferredFaction()));
	player.setStartPos(0);
	TheSkirmishGameInfo->setSlot(0, player);

	GameSlot opponent;
	opponent.setState(findOpponent(arguments));
	opponent.setColor(-1);
	opponent.setPlayerTemplate(PLAYERTEMPLATE_RANDOM);
	opponent.setStartPos(1);
	TheSkirmishGameInfo->setSlot(1, opponent);

	TheSkirmishGameInfo->setSeed(GetTickCount());
	TheSkirmishGameInfo->setStartingCash(prefs.getStartingCash());
	TheSkirmishGameInfo->setSuperweaponRestriction(0);
	TheSkirmishGameInfo->setMap(map);
	TheSkirmishGameInfo->setMapCRC(mapData.m_CRC);
	TheSkirmishGameInfo->setMapSize(mapData.m_filesize);
}

void sendNewGame()
{
	TheWritableGlobalData->m_mapName = TheSkirmishGameInfo->getMap();
	TheSkirmishGameInfo->startGame(0);

	InitRandom(TheSkirmishGameInfo->getSeed());
	GameMessage *msg = TheMessageStream->appendMessage(GameMessage::MSG_NEW_GAME);
	msg->appendIntegerArgument(GAME_SKIRMISH);
	msg->appendIntegerArgument(DIFFICULTY_NORMAL);
	msg->appendIntegerArgument(0);
	msg->appendIntegerArgument(TheGlobalData->m_framesPerSecondLimit);
}

std::optional<std::string> findMapArgument()
{
	return findOption(Kernel::processArguments(), "-cohSkirmish");
}

} // namespace

void prepareIfAsked()
{
	if (!findMapArgument())
		return;

	TheWritableGlobalData->m_shellMapOn = FALSE;
	TheWritableGlobalData->m_playIntro = FALSE;
	TheWritableGlobalData->m_playSizzle = FALSE;
}

void startIfAsked()
{
	const std::optional<std::string> map = findMapArgument();
	if (!map || TheMapCache == nullptr)
		return;

	const AsciiString mapPath = findMapPath(*map);
	const MapMetaData *mapData = mapPath.isEmpty() ? nullptr : TheMapCache->findMap(mapPath);
	if (mapData == nullptr)
	{
		DEBUG_LOG(("CoH: -cohSkirmish found no map '%s'; showing the menus instead", map->c_str()));
		return;
	}

	setUpGame(mapPath, *mapData, Kernel::processArguments());
	sendNewGame();
}

} // namespace CoH::QuickSkirmish
