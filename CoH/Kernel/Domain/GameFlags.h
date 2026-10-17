// CoH Kernel: which flags a game plays with, and how the host's flags travel in the game options.
// Plain C++ with no engine headers, so it is unit tested.

#pragma once

#include "Kernel/Domain/FeatureSet.h"

#include <string>
#include <string_view>

namespace CoH
{

// Where the game being started comes from. It decides whose flags the game plays with.
enum class GameOrigin
{
	ShellMap,	///< the menu background: never any feature
	LocalGame,	///< skirmish, campaign, or a network game this machine hosts: the local flags
	RemoteGame,	///< a network game another machine hosts: the host's flags, from the game options
	Replay,		///< the flags the replay was recorded with, from its game options
	SaveGame,	///< the flags the save was made with
};

FeatureSet chooseGameFlags(GameOrigin origin, const FeatureSet &local, const FeatureSet &fromGameOptions,
	const FeatureSet &fromSave);

// The game options string ("US=1;M=...;SD=...;") carries the host's flags under this key.
// Network games send it to every player, and replays store it in their header.
inline constexpr std::string_view GameOptionKey = "CoH";

// "CoH=Sectors,Accuracy;", or nothing at all when no flag is on, so that the options string of a
// game without features stays exactly the original one.
std::string formatGameOption(const FeatureSet &flags);

// The flags under the CoH key; none when the key is missing, as in every retail replay.
FeatureSet findGameOption(std::string_view options);

} // namespace CoH
