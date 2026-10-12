// IBM Engine: air power. See AirPower.h and README.md.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/INI.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"

#include "Features/AirPower/AirPower.h"
#include "Features/FeatureFlags/FeatureFlags.h"

namespace
{

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
	{ "AIReplace",	parseAIReplaceLine,	nullptr, 0 },
	{ nullptr, nullptr, nullptr, 0 }
};

} // namespace

//-------------------------------------------------------------------------------------------------
void IbmEngine::parseAirPowerDefinition(INI *ini)
{
	// Each game's pack sets everything again.
	s_aiReplace.clear();
	Int unused = 0;
	ini->initFromINI(&unused, s_fieldParse);
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
