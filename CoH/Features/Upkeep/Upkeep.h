// CoH Upkeep: what other slices may ask about population and upkeep. The only header of this slice
// others include.
//
// Each unit takes population; the cap rises with buildings and sectors held, and upkeep takes a
// share of income that grows with population. Every answer here is "none" while the Upkeep flag is
// off or the game has no sectors (the campaign).

#pragma once

class Player;
class ThingTemplate;

namespace CoH::Upkeep
{

// The game being played counts population and takes upkeep.
bool isActive();

// The population the player's units take, those being made included, and the player's cap.
int population(const Player *player);
int populationCap(const Player *player);

// The share of income upkeep takes from the player now, in percent.
int upkeepPercent(const Player *player);

// The population one unit of this kind takes; 0 for structures and free units.
int populationCost(const ThingTemplate *unit);

} // namespace CoH::Upkeep
