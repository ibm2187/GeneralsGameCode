// CoH Squads: tests for formation slots.

#include <gtest/gtest.h>

#include "Features/Squads/Domain/Formation.h"

#include <cmath>
#include <vector>

using namespace CoH::Squads;

namespace
{
const float kPi = 3.14159265f;
}

TEST(SquadsFormation, ALoneMemberStandsOnTheGoal)
{
	const SlotOffset slot = formationSlot(0, 1, FormationRules());
	EXPECT_FLOAT_EQ(slot.forward, 0.0f);
	EXPECT_FLOAT_EQ(slot.left, 0.0f);
}

TEST(SquadsFormation, OneRowSpreadsAcrossTheHeading)
{
	FormationRules rules;
	rules.spacing = 20.0f;
	rules.rowWidth = 3;
	EXPECT_FLOAT_EQ(formationSlot(0, 3, rules).left, 20.0f);
	EXPECT_FLOAT_EQ(formationSlot(1, 3, rules).left, 0.0f);
	EXPECT_FLOAT_EQ(formationSlot(2, 3, rules).left, -20.0f);
	for (int i = 0; i < 3; ++i)
		EXPECT_FLOAT_EQ(formationSlot(i, 3, rules).forward, 0.0f);
}

TEST(SquadsFormation, MoreMembersMakeStaggeredRowsBehind)
{
	FormationRules rules;
	rules.spacing = 20.0f;
	rules.rowWidth = 3;
	// Six: two rows of three, centred on the goal; the back row shifted by half a space.
	EXPECT_FLOAT_EQ(formationSlot(0, 6, rules).forward, 10.0f);
	EXPECT_FLOAT_EQ(formationSlot(3, 6, rules).forward, -10.0f);
	EXPECT_FLOAT_EQ(formationSlot(4, 6, rules).left, 10.0f);
	// Four: the back row has one member, behind the middle and shifted.
	EXPECT_FLOAT_EQ(formationSlot(3, 4, rules).left, 10.0f);
}

TEST(SquadsFormation, NoTwoSlotsMeet)
{
	const FormationRules rules;
	for (int count = 1; count <= 8; ++count)
	{
		std::vector<SlotOffset> slots;
		for (int i = 0; i < count; ++i)
			slots.push_back(formationSlot(i, count, rules));
		for (int a = 0; a < count; ++a)
			for (int b = a + 1; b < count; ++b)
			{
				const float df = slots[a].forward - slots[b].forward;
				const float dl = slots[a].left - slots[b].left;
				EXPECT_GE(std::sqrt(df * df + dl * dl), rules.spacing * 0.5f) << count << " " << a << " " << b;
			}
	}
}

TEST(SquadsFormation, SlotsTurnWithTheHeading)
{
	SlotOffset ahead;
	ahead.forward = 10.0f;
	const Point goal = { 100.0f, 50.0f };
	const Point east = slotPosition(goal, 0.0f, ahead);
	EXPECT_NEAR(east.x, 110.0f, 0.001f);
	EXPECT_NEAR(east.y, 50.0f, 0.001f);
	const Point north = slotPosition(goal, kPi * 0.5f, ahead);
	EXPECT_NEAR(north.x, 100.0f, 0.001f);
	EXPECT_NEAR(north.y, 60.0f, 0.001f);

	SlotOffset left;
	left.left = 10.0f;
	const Point leftOfEast = slotPosition(goal, 0.0f, left);
	EXPECT_NEAR(leftOfEast.y, 60.0f, 0.001f);	// left of heading east is north
}

TEST(SquadsFormation, HeadingFollowsTheWayToTheGoal)
{
	EXPECT_NEAR(headingFrom({ 0, 0 }, { 0, 100 }, 1.0f), kPi * 0.5f, 0.001f);
	EXPECT_FLOAT_EQ(headingFrom({ 5, 5 }, { 5, 5 }, 1.0f), 1.0f);	// already there: keep the old heading
}
