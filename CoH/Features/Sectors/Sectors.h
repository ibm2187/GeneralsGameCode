// CoH Sectors: what other slices may ask about sectors. The only header of this slice others include.
//
// Supplies come from territory: the map is divided into sectors around capture points, units take
// points by standing on them, and each held sector joined to its owner's home pays money. Every
// answer here is "no sectors" while the Sectors flag is off or the game has none (the campaign).

#pragma once

class Player;
struct Coord3D;

namespace CoH::Sectors
{

// The game being played has sectors.
bool isActive();

// The index of the player owning the sector at a position; -1 for neutral ground or no sectors.
int ownerIndexAt(const Coord3D &position);

// How many sectors a player holds, and how many of them are joined to the player's home.
int heldSectors(const Player *player);
int connectedSectors(const Player *player);

// What the player's connected sectors pay per minute.
int incomePerMinute(const Player *player);

} // namespace CoH::Sectors
