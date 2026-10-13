// IBM Engine: watch zones, the functions the engine's hooks call. With the WatchZones feature on,
// airfields and tunnels have powers that watch a circle on the map for a while. A watch zone
// (IbmWatchZoneUpdate) calls in a jet for each enemy that enters; the jet follows that enemy
// and fires at it, so its missiles home in.

#pragma once

#include "Lib/BaseType.h"

class Object;

namespace IbmEngine
{

// DeliverPayloadAIUpdate::update, while the jet approaches: moves the jet's target and destination
// along with the enemy it was sent at. TRUE when they moved, so the jet must be told to fly there.
// Stops following when the enemy dies, hides or leaves the zone's follow range.
Bool followWatchTarget(const Object *jet, Coord3D &targetPos, Coord3D &moveToPos);

// DeliverPayload's firing: the enemy a watch zone's jet was sent at, or nullptr for other jets
// and when the enemy is gone.
Object *getWatchTarget(const Object *jet);

// Forgets the jets of the last game. Called when a game starts.
void resetWatchZones();

} // namespace IbmEngine
