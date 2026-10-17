// CoH Squads: formation slots. A squad that is sent somewhere spreads its members over slots around
// the goal: rows across the way it is heading, the first row in front, every other row shifted half a
// space so the men behind see past the men in front. Cover (later) moves slots onto cover.

#pragma once

namespace CoH::Squads
{

struct FormationRules
{
	float spacing = 22.0f;	///< distance between neighbours, in world units (an infantryman is about 10 across)
	int rowWidth = 3;		///< members in one row; more go in the rows behind
};

// A slot's place in the squad's own frame: 'forward' along the heading, 'left' across it.
struct SlotOffset
{
	float forward = 0.0f;
	float left = 0.0f;
};

// The slot of member 'index' (0 first) in a squad of 'count'. The slots of a squad centre on the goal.
SlotOffset formationSlot(int index, int count, const FormationRules &rules);

struct Point
{
	float x = 0.0f;
	float y = 0.0f;
};

// The heading from 'from' to 'to' in radians (0 is +x, counter-clockwise); 'fallback' when they meet.
float headingFrom(Point from, Point to, float fallback);

// Where a slot is on the map, for a squad going to 'goal' with 'heading'.
Point slotPosition(Point goal, float heading, SlotOffset slot);

} // namespace CoH::Squads
