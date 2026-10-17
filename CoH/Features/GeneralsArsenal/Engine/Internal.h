// CoH GeneralsArsenal: how the slice's Engine and UI parts reach each other. Other slices use
// GeneralsArsenal.h.

#pragma once

#include "Features/GeneralsArsenal/Domain/CommandPoints.h"
#include "Features/GeneralsArsenal/Domain/PowerRules.h"

#include <vector>

class AirCoverZoneUpdate;
class CommandButton;
class CommandPointsLedger;
class DamageInfo;
class StrikeCountdownUpdate;
class Object;
class Player;
class SpecialPowerTemplate;
class UnicodeString;
struct Coord3D;

namespace CoH::GeneralsArsenal
{

struct PowerConfig;

// Ledger.cpp: the hidden object that keeps the command points. Its module registers itself as it is
// made and unmade, so a loaded game finds it too (object IDs aren't final while a save loads).
void createLedger();					///< for a new game
void registerLedger(CommandPointsLedger *ledger);
void unregisterLedger(CommandPointsLedger *ledger);
Wallet *walletOf(const Player *player);	///< null without a ledger
int pointsOf(const Player *player);

// Earning.cpp
void earnForKill(Object *killer, const Object *victim);
void earnForDamage(Object *victim, const DamageInfo *damage);

// Powers.cpp
void refreshAirfields();				///< which players have an airfield standing; once a second
bool hasAirfield(const Player *player);
PlayerStanding standingFor(const Player *player, const PowerConfig &power, const Object *source,
	const SpecialPowerTemplate *powerTemplate);
bool allowsPower(const Object *source, const SpecialPowerTemplate *power);	///< the specialPowerUse veto
void payForPower(Object *source, const SpecialPowerTemplate *power, const Coord3D *target);
void grantPowerSciences();				///< each player's power sciences, by rank
void penalizeLoss(Player *player, const SpecialPowerTemplate *power, const Object *lost);	///< a plane of it was shot down
void tagPlane(Object *object);			///< every object created: a plane of the power fired this moment
void forgetLastTrigger();				///< the game ends
const Object *findPowerSource(const Player *player, const SpecialPowerTemplate *power);	///< the object of the player that has it, readiest first

// AI.cpp, AIBuildList.cpp
void thinkForAIs();
void logAIPowers();						///< each AI's powers and why it can or can't use them, for the log
void removeAIBuildListEntries(Player *player);

// Markers.cpp: the super weapon targets and air cover zones in play, for the UI. Their modules
// register themselves as they are made and unmade.
void registerMarker(StrikeCountdownUpdate *marker);
void unregisterMarker(StrikeCountdownUpdate *marker);
void registerZone(AirCoverZoneUpdate *zone);
void unregisterZone(AirCoverZoneUpdate *zone);
std::vector<Object *> strikeMarkers();
std::vector<Object *> airCoverZones();

// UI
void addPointsToMoneyText(const Player *player, UnicodeString &text);
void addPointsToMoneyTooltip(const Player *player, UnicodeString &description);
void addPowerToTooltip(const CommandButton *button, UnicodeString &description);
void drawMarkersOnGround();
void releaseMarkerText();				///< the game ends
void drawMarkersOnRadar(int x, int y, int width, int height);

} // namespace CoH::GeneralsArsenal
