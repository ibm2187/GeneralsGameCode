// IBM Engine: air power. See AirPower.h and README.md.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/INI.h"
#include "Common/Player.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"

#include "Features/AirPower/AirPower.h"
#include "Features/FeatureFlags/FeatureFlags.h"

namespace
{

//-------------------------------------------------------------------------------------------------
// The IbmAirPower block's single values. Defaults match the pack.
struct AirPowerSettings
{
	Bool strikesNeedSight;	///< damaging powers need the spot in sight, not just explored

	AirPowerSettings()
	{
		strikesNeedSight = TRUE;
	}
};

AirPowerSettings s_settings;
std::map<AsciiString, AsciiString> s_aiReplace;	///< "AIReplace = AmericaJetRaptor AmericaVehicleComanche" lines

// "AIReplace = AmericaJetRaptor AmericaVehicleComanche"
void parseAIReplaceLine(INI *ini, void * /*instance*/, void * /*store*/, const void * /*userData*/)
{
	const AsciiString from = ini->getNextToken();
	const AsciiString to = ini->getNextToken();
	s_aiReplace[from] = to;
}

const FieldParse s_fieldParse[] =
{
	{ "AIReplace",			parseAIReplaceLine,	nullptr, 0 },
	{ "StrikesNeedSight",	INI::parseBool,		nullptr, offsetof(AirPowerSettings, strikesNeedSight) },
	{ nullptr, nullptr, nullptr, 0 }
};

} // namespace

//-------------------------------------------------------------------------------------------------
void IbmEngine::parseAirPowerDefinition(INI *ini)
{
	// Each game's pack sets everything again.
	s_settings = AirPowerSettings();
	s_aiReplace.clear();
	ini->initFromINI(&s_settings, s_fieldParse);
}

//-------------------------------------------------------------------------------------------------
Bool IbmEngine::strikeNeedsSight(const Player *player)
{
	return isFeatureActive(Feature_AirPower) && s_settings.strikesNeedSight
		&& player != nullptr && player->getPlayerType() == PLAYER_HUMAN;
}

//-------------------------------------------------------------------------------------------------
void IbmEngine::replaceTeamJet(AsciiString &unitThingName)
{
	if (!isFeatureActive(Feature_AirPower) || unitThingName.isEmpty())
		return;

	// The name, or the name after a general's prefix ("AirF_", "GC_Chem_").
	const char *full = unitThingName.str();
	for (const char *name = full; name != nullptr && *name != '\0'; )
	{
		std::map<AsciiString, AsciiString>::const_iterator it = s_aiReplace.find(AsciiString(name));
		if (it != s_aiReplace.end())
		{
			// Keep the general's prefix when that version of the replacement exists.
			AsciiString prefixed;
			prefixed.set(full, (Int)(name - full));
			prefixed.concat(it->second);
			unitThingName = TheThingFactory->findTemplate(prefixed, FALSE) != nullptr ? prefixed : it->second;
			return;
		}

		name = strchr(name, '_');
		if (name != nullptr)
			++name;
	}
}
