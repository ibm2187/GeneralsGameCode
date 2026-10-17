// CoH Accuracy: how the slice's Engine and UI parts reach each other. Other slices use Accuracy.h.

#pragma once

#include "Kernel/Hooks.h"

#include "Features/Accuracy/Domain/ModifierStack.h"

struct Coord3D;

namespace CoH
{
class Feature;
}

namespace CoH::Accuracy
{

// Accuracy.cpp
const Feature *accuracyFeature();	///< null before init

// Shots.cpp: the roll, from the hub's weaponFiringAtObject.
ModifierStack &modifiers();
void addBuiltInModifiers(const Feature &self);
void rollShot(Hooks::WeaponShot &shot);

struct Tally
{
	int rolls = 0;
	int hits = 0;
};
const Tally &tally();	///< this game's rolls, for the log
void forgetTally();

// UI/MissPuff.cpp
void showMissPuff(const Coord3D &spot);

} // namespace CoH::Accuracy
