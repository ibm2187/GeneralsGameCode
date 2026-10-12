// IBM Engine: reads the local feature flags.
// Features.ini lives in the user data folder next to Options.ini and holds one line per
// feature, for example "Squads = yes". The command line can override it for one run with
// "-feature Squads=1" or "-feature Squads=0".

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/UserPreferences.h"

#include "Features/FeatureFlags/FeatureFlagsInternal.h"

namespace IbmEngine
{

class FeaturePreferences : public UserPreferences
{
public:
	FeaturePreferences()
	{
		load("Features.ini");
	}
};

// Lists every known feature in Features.ini so the user can see what can be switched on.
static void writeMissingFeatures(FeaturePreferences &prefs)
{
	Bool changed = FALSE;
	for (Int i = 0; i < Feature_Count; ++i)
	{
		const char *name = getFeatureName((Feature)i);
		if (prefs.find(name) == prefs.end())
		{
			prefs.setBool(name, FALSE);
			changed = TRUE;
		}
	}

	if (changed)
		prefs.write();
}

static void applyCommandLine(FeatureMask &features)
{
	// The engine's own command line parser skips options it does not know, so this one is ours.
	for (Int arg = 1; arg + 1 < __argc; ++arg)
	{
		if (stricmp(__argv[arg], "-feature") != 0)
			continue;

		const char *setting = __argv[arg + 1];
		const char *equals = strchr(setting, '=');

		AsciiString name;
		Bool enable = TRUE;
		if (equals != nullptr)
		{
			name.set(setting, (int)(equals - setting));
			enable = atoi(equals + 1) != 0;
		}
		else
		{
			name.set(setting);
		}

		Feature feature;
		if (!findFeatureByName(name.str(), feature))
		{
			DEBUG_CRASH(("Unknown feature '%s' on the command line", name.str()));
			continue;
		}

		if (enable)
			features |= featureBit(feature);
		else
			features &= ~featureBit(feature);
	}
}

void initFeatureFlags()
{
	FeaturePreferences prefs;
	writeMissingFeatures(prefs);

	FeatureMask features = 0;
	for (Int i = 0; i < Feature_Count; ++i)
	{
		if (prefs.getBool(getFeatureName((Feature)i), FALSE))
			features |= featureBit((Feature)i);
	}

	applyCommandLine(features);

	setLocalFeatures(features);
	DEBUG_LOG(("IbmEngine: local feature flags = %X", features));
}

} // namespace IbmEngine
