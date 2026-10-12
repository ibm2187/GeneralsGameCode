// IBM Engine: feature flag registry and state.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Features/FeatureFlags/FeatureFlags.h"
#include "Features/FeatureFlags/FeatureFlagsInternal.h"

namespace IbmEngine
{

static const char *const s_featureNames[Feature_Count] =
{
	"Squads",
	"SquadsDemo",
	"Accuracy",
};

// Whether the feature has an INI pack in Data\INI\Features\<Name>\.
static const Bool s_featureHasPack[Feature_Count] =
{
	TRUE,	// Squads
	FALSE,	// SquadsDemo
	TRUE,	// Accuracy
};

static FeatureMask s_localFeatures = 0;
static FeatureMask s_activeFeatures = 0;

const char *getFeatureName(Feature feature)
{
	DEBUG_ASSERTCRASH(feature >= 0 && feature < Feature_Count, ("Invalid feature %d", feature));
	return s_featureNames[feature];
}

Bool featureHasPack(Feature feature)
{
	return s_featureHasPack[feature];
}

Bool findFeatureByName(const char *name, Feature &feature)
{
	for (Int i = 0; i < Feature_Count; ++i)
	{
		if (stricmp(name, s_featureNames[i]) == 0)
		{
			feature = (Feature)i;
			return TRUE;
		}
	}
	return FALSE;
}

FeatureMask getLocalFeatures()
{
	return s_localFeatures;
}

void setLocalFeatures(FeatureMask features)
{
	s_localFeatures = features;
}

FeatureMask getActiveFeatures()
{
	return s_activeFeatures;
}

void setActiveFeatures(FeatureMask features)
{
	s_activeFeatures = features;
}

Bool isFeatureActive(Feature feature)
{
	return (s_activeFeatures & featureBit(feature)) != 0;
}

} // namespace IbmEngine
