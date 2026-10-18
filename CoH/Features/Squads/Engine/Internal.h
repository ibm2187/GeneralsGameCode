// CoH Squads: what the slice's Engine and UI files share. Not for other slices: they use Squads.h.

#pragma once

#include "Common/GameCommon.h"

#include "Features/Squads/Domain/SquadBook.h"
#include "Features/Squads/Domain/SquadTypes.h"

#include <map>
#include <vector>

class Object;
class Player;
class Snapshot;
class ThingTemplate;
struct AICommandParms;
struct Coord3D;
struct IRegion2D;

namespace CoH::Squads
{

// ---- The squads of this game (Book.cpp) ----

bool isSquadsGame();		///< the flag is on and the pack lists squads
void startGame();
void endGame();
SquadBook &book();

Object *memberObject(MemberId member);	///< null when it is gone
std::vector<Object *> livingMembers(const Squad &squad);
Squad *squadOfObject(const Object *object);
void onObjectDestroyed(Object *object);

// The squad made when this unit is ordered; null when it is made alone or squads are off.
const SquadType *squadTypeOf(const ThingTemplate *unit);

// ---- Production (Production.cpp) ----

void onUnitProduced(Object *factory, Object *unit);
void adjustBuildCost(const Player *player, const ThingTemplate *unit, int &cost);
void adjustBuildTime(const Player *player, const ThingTemplate *unit, int &frames);
int rawSquadCost(const ThingTemplate *unit);	///< no player modifiers; the unit's own cost when no squad
void startProduction();
void forgetProduction();

// The squad each factory is making, while an order that makes several units (Red Guards) still has
// units to come: they join it rather than start squads of their own.
std::map<ObjectID, SquadId> &openSquads();

// ---- Orders (Orders.cpp) ----

void onPlayerSelection(Player *player, Object *object, bool selected);
void onAiCommand(Object *object, const AICommandParms *command);
void carryOutOrders();		///< each logic frame: formation moves and following mates out
void sendInFormation(const std::vector<Object *> &movers, const Coord3D &goal, CommandSourceType source,
	bool attackMove, int maxShots);
bool lastSlotPosition(MemberId member, Coord3D *position);
std::map<MemberId, Coord3D> &slotPositions();	///< where each member's formation slot last put it
void forgetMember(MemberId member);
void forgetOrders();

// ---- Containers (Containers.cpp) ----

void onContainRoom(const Object *container, const Object *rider, int &used, int &needed);
bool isRiderShown(const Object *container, const Object *rider);	///< the squad's first rider there speaks for it

// ---- Saving (Save.cpp) ----

Snapshot *saveData();	///< the book, the open orders and the slot positions, loaded after the objects

// ---- UI (UI/HealthBar.cpp) ----

void onHealthBar(const Object *object, const IRegion2D *bar, float &health, float &maxHealth);

// ---- The in-game self-test, with -cohSquadsSelfTest (SelfTest.cpp) ----

void startSelfTest(bool loadedGame);	///< a loaded game checks the squads of the test's save
void runSelfTest();
void resetSelfTest();

} // namespace CoH::Squads
