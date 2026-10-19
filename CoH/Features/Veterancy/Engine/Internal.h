// CoH Veterancy: how the slice's engine parts reach each other.

#pragma once

class CommandButton;
class DamageInfo;
class Object;
class Snapshot;
class UnicodeString;

namespace CoH::Veterancy
{

// ---- Experience.cpp: experience from damage, and squads sharing it ------------------------------

void onDamageDealt(Object *victim, const DamageInfo *damage);
void onKillScored(Object *killer, const Object *victim);
void onUnitRankChanged(const Object *unit);	///< its squad shares the new experience
void shareSquadExperience();					///< each logic frame
void forgetUnit(const Object *unit);
void forgetExperience();
Snapshot *saveData();

// ---- UI/Tooltip.cpp ----------------------------------------------------------------------------

void addRanksToTooltip(const CommandButton *button, UnicodeString &description);

} // namespace CoH::Veterancy
