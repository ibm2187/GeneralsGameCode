// CoH Pacing: what other slices may ask about unit prices and the AI's handicap. The only header of
// this slice others include.
//
// Units cost more and take longer to build, by class (infantry, vehicle, aircraft, structure) or by
// their own line, and the skirmish AI's income and build times scale with its difficulty. The engine
// asks through the hub's buildCost, buildTime and income, so prices change everywhere at once. Every
// answer here is "unchanged" while the Pacing flag is off.

#pragma once

class Player;
class ThingTemplate;

namespace CoH::Pacing
{

// The game being played scales prices and handicaps the AI.
bool isActive();

// How many times its retail cost and build time 'build' takes in this game, before the AI's handicap.
float costMultiplier(const ThingTemplate *build);
float buildTimeMultiplier(const ThingTemplate *build);

// The share of its income the player keeps under its handicap; 1 for humans.
float incomeMultiplier(const Player *player);

} // namespace CoH::Pacing
