// CoH Sectors: capture by conquest. Units standing in a point's radius move its capture toward their
// side. Units of two enemy sides make it contested, and nothing moves. An enemy's point is neutralized
// first, then captured. More units capture faster, up to a cap. A point nobody stands on drifts back
// to where it was: an owned point to full hold, a neutral one to no progress.

#pragma once

#include "Types.h"

#include <vector>

namespace CoH::Sectors
{

struct PointState
{
	PlayerId owner = NoPlayer;
	PlayerId progressSide = NoPlayer;	///< whose progress 'progress' is; the owner while owned
	float progress = 0.0f;				///< 0..1: the owner's hold while owned, else progressSide's capture
};

// How many capturing units a player has inside a point's radius.
struct Presence
{
	PlayerId player = NoPlayer;
	int units = 0;
};

struct Occupation
{
	enum class Kind
	{
		Empty,		///< nobody inside
		Contested,	///< enemies of each other inside
		Held,		///< one side inside: 'side' and its allies
	};

	Kind kind = Kind::Empty;
	PlayerId side = NoPlayer;	///< the player with the most units on the holding side; ties go to the lower id
	int units = 0;				///< all the units of the holding side
};

Occupation resolveOccupation(const std::vector<Presence> &presences, const AreEnemies &areEnemies);

struct CaptureRules
{
	float captureSeconds = 20.0f;		///< one unit takes a neutral point in this time
	float neutralizeSeconds = 15.0f;	///< one unit neutralizes an enemy point in this time
	float recoverSeconds = 60.0f;		///< an empty point drifts all the way back in this time; 0: never
	int maxUnits = 5;					///< units beyond this don't capture any faster
	float extraUnitBonus = 0.25f;		///< each unit after the first adds this much speed
};

// How much faster than one unit the given number of units capture.
float unitSpeedFactor(int units, const CaptureRules &rules);

struct CaptureChange
{
	bool ownerChanged = false;
	PlayerId previousOwner = NoPlayer;
};

// Advances a point by 'seconds'. A locked point (a home whose command center stands) can't be taken
// from its owner: enemies on it change nothing.
CaptureChange advanceCapture(PointState &state, const Occupation &occupation, bool locked, const CaptureRules &rules,
	float seconds, const AreEnemies &areEnemies);

} // namespace CoH::Sectors
