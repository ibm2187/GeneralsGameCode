// CoH Squads: the tuning, read from the CoHSquads block of the pack (Data/INI/10_Squads.ini). Back to
// these defaults after every game.

#pragma once

#include "Features/Squads/Domain/Formation.h"
#include "Features/Squads/Domain/SquadTypes.h"

class INI;

namespace CoH::Squads
{

struct Config
{
	SquadTypes types;			///< each Squad line adds one; none means no squads
	FormationRules formation;
	int costPercent = 90;		///< a squad costs its members' price times this
	int timePercent = 60;		///< and takes their build time times this
};

const Config &config();
void parseConfigBlock(INI *ini);
void resetConfig();

} // namespace CoH::Squads
