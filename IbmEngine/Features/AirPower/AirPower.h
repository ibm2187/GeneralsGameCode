// IBM Engine: air power, the functions the engine's hooks call. With the AirPower feature on,
// players build no jets: airfields offer helicopters only (the pack's button sets), and air strikes
// come from general powers. The skirmish AI's teams that ask for a jet ask for a helicopter instead.

#pragma once

#include "Common/AsciiString.h"

class INI;
class Player;

namespace IbmEngine
{

// TeamTemplateInfo: a team that asks for a jet gets the unit its "AIReplace" line names instead,
// in the same general's version when there is one ("AirF_AmericaJetRaptor" -> "AirF_AmericaVehicleComanche").
void replaceTeamJet(AsciiString &unitThingName);

// ActionManager::canDoSpecialPowerAtLocation, for damaging powers (strikes, bombs, missiles): TRUE
// when the spot must be in sight of one of the player's units right now, not just explored
// ("StrikesNeedSight = Yes"). Only for human players: the skirmish AI's scripts aim on their own.
Bool strikeNeedsSight(const Player *player);

// INI block "IbmAirPower": the settings, read from the AirPower pack at game start.
void parseAirPowerDefinition(INI *ini);

} // namespace IbmEngine
