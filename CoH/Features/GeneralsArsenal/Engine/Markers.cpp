// CoH GeneralsArsenal: the super weapon targets and air cover zones in play, for the UI. Their modules
// register here as they are made and unmade (also when a game loads), so the UI draws them without
// searching every object.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "GameLogic/Object.h"

#include "Features/GeneralsArsenal/Engine/AirCoverZoneUpdate.h"
#include "Features/GeneralsArsenal/Engine/Internal.h"
#include "Features/GeneralsArsenal/Engine/StrikeCountdownUpdate.h"

#include <algorithm>

namespace CoH::GeneralsArsenal
{

namespace
{

std::vector<StrikeCountdownUpdate *> &markers()
{
	static std::vector<StrikeCountdownUpdate *> modules;
	return modules;
}

std::vector<AirCoverZoneUpdate *> &zones()
{
	static std::vector<AirCoverZoneUpdate *> modules;
	return modules;
}

template <typename Module>
void forget(std::vector<Module *> &modules, Module *module)
{
	modules.erase(std::remove(modules.begin(), modules.end(), module), modules.end());
}

template <typename Module>
std::vector<Object *> objectsOf(const std::vector<Module *> &modules)
{
	std::vector<Object *> objects;
	for (Module *module : modules)
		objects.push_back(module->ownerObject());
	return objects;
}

} // namespace

void registerMarker(StrikeCountdownUpdate *marker) { markers().push_back(marker); }
void unregisterMarker(StrikeCountdownUpdate *marker) { forget(markers(), marker); }
void registerZone(AirCoverZoneUpdate *zone) { zones().push_back(zone); }
void unregisterZone(AirCoverZoneUpdate *zone) { forget(zones(), zone); }

std::vector<Object *> strikeMarkers()
{
	return objectsOf(markers());
}

std::vector<Object *> airCoverZones()
{
	return objectsOf(zones());
}

} // namespace CoH::GeneralsArsenal
