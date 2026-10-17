// CoH Upkeep: how the slice's Engine and UI parts reach each other. Other slices use Upkeep.h.

#pragma once

class CommandButton;
class Player;
class ThingTemplate;
class UnicodeString;

namespace CoH::Upkeep
{

// Population.cpp: worked out from the player's objects each time it is asked, so the answer is
// always that of the current game state, the same on every machine.
struct Standing
{
	int population = 0;		///< units alive and units in production queues
	int buildingBonus = 0;	///< what the player's finished buildings add to the cap
	int cap = 0;
	int percent = 0;		///< the upkeep share at this population
};

bool isUpkeepGame();	///< the flag is on and the game has sectors
Standing standingOf(const Player *player);
int unitPopulation(const ThingTemplate *unit);	///< 0 for what isn't a unit or costs nothing
bool isNeverRefused(const ThingTemplate *unit);

// Rules.cpp: the hook subscribers.
bool allowsProduction(const Player *player, const ThingTemplate *build);
void takeUpkeep(const Player *player, unsigned int &amount);
int upkeepTaken(int playerIndex);	///< since the game started, for the log
void forgetUpkeepTaken();

// UI
void addPopulationToMoneyText(const Player *player, UnicodeString &text);
void addUpkeepToMoneyTooltip(const Player *player, UnicodeString &description);
void addPopulationToTooltip(const CommandButton *button, UnicodeString &description);

} // namespace CoH::Upkeep
