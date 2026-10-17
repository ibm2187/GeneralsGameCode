// CoH GeneralsArsenal: what other slices may ask about General's powers and command points. The only
// header of this slice others include.
//
// Fixed-wing aircraft and super weapons are General's powers, paid per use with command points earned
// in combat. Every answer here is "none" while the GeneralsArsenal flag is off.

#pragma once

class Player;
class SpecialPowerTemplate;

namespace CoH::GeneralsArsenal
{

// The game being played keeps command points.
bool isActive();

// The player's command points, as shown.
int commandPoints(const Player *player);

// The power's command point cost; 0 for a power the pack doesn't price.
int powerCost(const SpecialPowerTemplate *power);

} // namespace CoH::GeneralsArsenal
