// CoH Kernel: which flags each game plays with, so every player and every replay use the host's set.
//
// The game options string carries the flags of whoever hosts the game. A machine that hosts the game
// (skirmish, campaign, the host of a network game) writes its local flags; a network client or a
// replay keeps the flags it read from the options string and writes those back, so a client's own
// replay records the host's flags. A save game stores the flags in its CoH save block.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Recorder.h"
#include "GameLogic/GameLogic.h"
#include "GameNetwork/GameInfo.h"

#include "Kernel/Domain/GameFlags.h"
#include "Kernel/Engine/Internal.h"

#include <map>

namespace CoH::GameFlags
{

namespace
{

// The flags last read from each game's options string. A GameInfo is reused for game after game,
// and every options string read overwrites its entry, so an entry is never older than the options.
std::map<const GameInfo *, FeatureSet> &receivedFlags()
{
	static std::map<const GameInfo *, FeatureSet> flags;
	return flags;
}

FeatureSet &savedFlags()
{
	static FeatureSet flags;
	return flags;
}

FeatureSet receivedFlagsOf(const GameInfo *game)
{
	const auto it = receivedFlags().find(game);
	return it != receivedFlags().end() ? it->second : FeatureSet();
}

bool isReplayBeingPlayed(const GameInfo *game)
{
	return TheRecorder != nullptr && TheRecorder->isPlaybackMode() && game == TheRecorder->getGameInfo();
}

bool isHostedHere(const GameInfo *game)
{
	if (game == TheSkirmishGameInfo || game == TheChallengeGameInfo)
		return true;
	return game->isInGame() && game->amIHost();
}

GameOrigin originOf(const GameInfo *game, bool loadingSaveGame)
{
	if (loadingSaveGame)
		return GameOrigin::SaveGame;
	if (TheGameLogic->isInShellGame())
		return GameOrigin::ShellMap;
	if (game == nullptr)
		return GameOrigin::LocalGame;	// a campaign mission, or a map started from the command line
	if (isReplayBeingPlayed(game))
		return GameOrigin::Replay;
	return isHostedHere(game) ? GameOrigin::LocalGame : GameOrigin::RemoteGame;
}

FeatureSet flagsOf(const GameInfo *game)
{
	if (!isReplayBeingPlayed(game) && isHostedHere(game))
		return Kernel::localFlags();
	return receivedFlagsOf(game);
}

} // namespace

AsciiString optionsFor(const GameInfo *game)
{
	return AsciiString(formatGameOption(flagsOf(game)).c_str());
}

void onOptionsParsed(const GameInfo *game, const AsciiString &options)
{
	receivedFlags()[game] = findGameOption(options.str());
}

FeatureSet forNewGame(bool loadingSaveGame)
{
	const GameOrigin origin = originOf(TheGameInfo, loadingSaveGame);
	const FeatureSet fromGameOptions = TheGameInfo != nullptr ? receivedFlagsOf(TheGameInfo) : FeatureSet();
	return chooseGameFlags(origin, Kernel::localFlags(), fromGameOptions, savedFlags());
}

void setSaved(const FeatureSet &flags)
{
	savedFlags() = flags;
}

void clearSaved()
{
	savedFlags() = FeatureSet();
}

} // namespace CoH::GameFlags
