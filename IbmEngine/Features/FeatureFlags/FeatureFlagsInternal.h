// IBM Engine: feature flag functions shared inside the FeatureFlags slice only.

#pragma once

#include "Features/FeatureFlags/FeatureFlags.h"

namespace IbmEngine
{

Bool findFeatureByName(const char *name, Feature &feature);
Bool featureHasPack(Feature feature);
void setLocalFeatures(FeatureMask features);
void setActiveFeatures(FeatureMask features);

} // namespace IbmEngine
