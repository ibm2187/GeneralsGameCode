// IBM Engine: air power, the functions the engine's hooks call. With the AirPower feature on,
// players build no jets: airfields offer helicopters only (the pack's button sets), and air strikes
// come from general powers. The skirmish AI's teams that ask for a jet ask for a helicopter instead.

#pragma once

#include "Common/AsciiString.h"

class INI;

namespace IbmEngine
{

// TeamTemplateInfo: a team that asks for a jet gets the unit its "AIReplace" line names instead,
// in the same general's version when there is one ("AirF_AmericaJetRaptor" -> "AirF_AmericaVehicleComanche").
void replaceTeamJet(AsciiString &unitThingName);

// INI block "IbmAirPower": the settings, read from the AirPower pack at game start.
void parseAirPowerDefinition(INI *ini);

} // namespace IbmEngine
