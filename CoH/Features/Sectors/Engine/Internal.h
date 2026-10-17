// CoH Sectors: how the slice's Engine and UI parts reach each other. Other slices use Sectors.h.

#pragma once

#include "Features/Sectors/Domain/Points.h"

#include <vector>

struct Coord3D;
class Object;
class Player;
class ThingTemplate;
class UnicodeString;

namespace CoH::Sectors
{

// Setup.cpp
Bool isSectorGame();	///< skirmish and network games and their replays have sectors; the campaign doesn't
void setUpNewGame();	///< makes the points of a new game and empties the supplies
void emptySupplies(Object *object);

// AIBuildList.cpp
void adaptAIBuildList(Player *player);	///< no supply buildings, and nothing planned where it may not build
void logAIBuildList(Player *player, const std::vector<Start> &starts);

// Logic.cpp: one step of each, on the logic frame.
bool isCapturer(const Object *object);	///< a living unit of a playing player that counts on a point
void advancePoints();	///< capture, owner changes, the supply network
void payIncome();
void announceOwnerChange(int sector, int previousOwner, int newOwner);

// AI.cpp, AIScripts.cpp
void thinkForAIs();
void retargetAIScripts();	///< the AI scripts' conditions on supply buildings ask about barracks

// BuildRule.cpp
bool allowsBuildingAt(const ThingTemplate *build, const Coord3D *position, const Player *player);	///< the hook's answer; counts refusals
bool isBuildingAllowedAt(const ThingTemplate *build, const Coord3D *position, const Player *player);
int refusedBuildSpots(int player);	///< since the game started, for the log
void forgetRefusedBuildSpots();

// UI
void drawGround();
void drawRadar(int x, int y, int width, int height);
void addIncomeToTooltip(const Player *player, UnicodeString &description);

} // namespace CoH::Sectors
