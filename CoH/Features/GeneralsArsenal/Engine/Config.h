// CoH GeneralsArsenal: the tuning, read from the pack. The CoHGeneralsArsenal block holds the earning
// rates and the AI's settings; one CoHPower block per General's power holds its cost, rank, airfield
// rule, loss penalty, levels and how the AI uses it. Back to these defaults after every game.

#pragma once

#include "Common/KindOf.h"

#include "Features/GeneralsArsenal/Domain/AiChoice.h"
#include "Features/GeneralsArsenal/Domain/CommandPoints.h"
#include "Features/GeneralsArsenal/Domain/PowerRules.h"

#include <map>
#include <string>

class INI;
class SpecialPowerTemplate;

namespace CoH::GeneralsArsenal
{

struct PowerConfig
{
	PowerRule rule;
	Int aiUse = 0;							///< index into AiUse, as INI reads it
	KindOfMaskType aiTargetKinds;			///< what a cover power is for (enemies of these kinds near the army)
	AsciiString description;				///< label of a line the tooltip adds, such as how a sortie works
};

struct Config
{
	EarnRates earn;
	Real maxLossDelaySeconds = 300.0f;		///< losses never push a power more than this past a full cooldown
	KindOfMaskType airfieldKinds;			///< a standing, finished structure of these kinds is an airfield
	KindOfMaskType aiBuildListRemovals;		///< taken out of AI build lists: the super weapon buildings

	Real aiThinkSeconds = 8.0f;
	AiPolicy aiPolicy;
	Int aiStrikeMinValue = 1200;			///< an enemy group worth less than this (build cost) isn't struck
	Real aiStrikeRadius = 150.0f;			///< the area a strike's value is counted in
	Real aiCoverRadius = 350.0f;			///< enemies this close to the AI's units call for cover
	Real aiFirstUseSeconds = 120.0f;		///< the AI uses no power before this much game time

	AsciiString ledgerObject;				///< the hidden object that keeps every player's command points

	std::map<std::string, PowerConfig> powers;	///< by special power template name

	Config();
};

const Config &config();
void parseConfigBlock(INI *ini);
void parsePowerBlock(INI *ini);
void resetConfig();

// The power's settings, or null for a power the pack doesn't name (it is used as in the original).
const PowerConfig *powerConfig(const SpecialPowerTemplate *power);

AiUse aiUseOf(const PowerConfig &power);

UnsignedInt secondsToLogicFrames(Real seconds);

} // namespace CoH::GeneralsArsenal
