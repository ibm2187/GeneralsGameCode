// CoH Veterancy: the tuning, read from the CoHVeterancy block of the pack (Data/INI/10_Veterancy.ini),
// and each unit type's rank table, worked out once per game. Back to these defaults after every game.

#pragma once

#include "Features/Veterancy/Domain/Ranks.h"

class INI;
class Object;
class ThingTemplate;

namespace CoH::Veterancy
{

struct Config
{
	Rules rules;
	float damageExperience = 0.0f;	///< share of a victim's kill value earned per full health of damage
	bool squadsShare = true;		///< a squad's members share one experience pool
};

const Config &config();
void parseConfigBlock(INI *ini);
void resetConfig();

const RankTable &tableOf(const ThingTemplate *unit);
const RankBonus &bonusOf(const Object *unit);	///< at its rank now; all ones for none

} // namespace CoH::Veterancy
