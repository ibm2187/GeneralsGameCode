// CoH Squads: see Formation.h.

#include "Features/Squads/Domain/Formation.h"

#include <cmath>

namespace CoH::Squads
{

SlotOffset formationSlot(int index, int count, const FormationRules &rules)
{
	SlotOffset slot;
	if (count <= 1 || index < 0 || index >= count)
		return slot;

	const int rowWidth = rules.rowWidth > 0 ? rules.rowWidth : 1;
	const int rows = (count + rowWidth - 1) / rowWidth;
	const int row = index / rowWidth;
	const int column = index % rowWidth;
	const int inThisRow = (row == rows - 1) ? count - row * rowWidth : rowWidth;

	// Rows from the front, centred on the goal; members across the row, centred on the heading.
	slot.forward = ((rows - 1) * 0.5f - row) * rules.spacing;
	slot.left = ((inThisRow - 1) * 0.5f - column) * rules.spacing;
	if (row % 2 == 1)
		slot.left += rules.spacing * 0.5f;
	return slot;
}

float headingFrom(Point from, Point to, float fallback)
{
	const float dx = to.x - from.x;
	const float dy = to.y - from.y;
	if (dx * dx + dy * dy < 1.0f)
		return fallback;
	return std::atan2(dy, dx);
}

Point slotPosition(Point goal, float heading, SlotOffset slot)
{
	const float c = std::cos(heading);
	const float s = std::sin(heading);
	// 'forward' runs along (c, s); 'left' along (-s, c).
	Point position;
	position.x = goal.x + slot.forward * c - slot.left * s;
	position.y = goal.y + slot.forward * s + slot.left * c;
	return position;
}

} // namespace CoH::Squads
