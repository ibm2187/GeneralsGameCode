// CoH Squads: tests for member bookkeeping, squad types and container room.

#include <gtest/gtest.h>

#include "Features/Squads/Domain/Room.h"
#include "Features/Squads/Domain/SquadBook.h"
#include "Features/Squads/Domain/SquadTypes.h"

using namespace CoH::Squads;

TEST(SquadsBook, MembersFillSlotsInOrder)
{
	SquadBook book;
	Squad &squad = book.create("AmericaInfantryRanger", 4);
	EXPECT_TRUE(book.join(squad.id(), 11, 120.0f));
	EXPECT_TRUE(book.join(squad.id(), 12, 120.0f));
	EXPECT_EQ(squad.memberCount(), 2);
	EXPECT_EQ(squad.missingCount(), 2);
	EXPECT_EQ(squad.slotOf(12), 1);
	EXPECT_EQ(book.squadOf(11), squad.id());
	EXPECT_EQ(squad.leader(), 11u);
}

TEST(SquadsBook, AFullSquadTakesNoOneMore)
{
	SquadBook book;
	Squad &squad = book.create("AmericaInfantryRanger", 2);
	EXPECT_TRUE(book.join(squad.id(), 1, 100.0f));
	EXPECT_TRUE(book.join(squad.id(), 2, 100.0f));
	EXPECT_FALSE(book.join(squad.id(), 3, 100.0f));
	EXPECT_EQ(book.squadOf(3), 0);
}

TEST(SquadsBook, AMemberIsInOneSquadOnly)
{
	SquadBook book;
	const SquadId a = book.create("A", 2).id();
	const SquadId b = book.create("B", 2).id();
	EXPECT_TRUE(book.join(a, 7, 100.0f));
	EXPECT_FALSE(book.join(b, 7, 100.0f));
	EXPECT_NE(a, b);
}

TEST(SquadsBook, ALostMemberLeavesItsSlotEmpty)
{
	SquadBook book;
	Squad &squad = book.create("R", 3);
	const SquadId id = squad.id();
	book.join(id, 1, 100.0f);
	book.join(id, 2, 100.0f);
	book.join(id, 3, 100.0f);

	EXPECT_EQ(book.leave(1), 0);	// the squad goes on
	const Squad *left = book.find(id);
	ASSERT_NE(left, nullptr);
	EXPECT_EQ(left->memberIn(0), 0u);
	EXPECT_EQ(left->leader(), 2u);	// the next member speaks for the squad
	EXPECT_EQ(left->missingCount(), 1);
	EXPECT_FLOAT_EQ(left->fullHealth(), 300.0f);	// the bar still counts the lost member

	// A new member (Reinforce) takes the empty slot.
	EXPECT_TRUE(book.join(id, 4, 100.0f));
	EXPECT_EQ(book.find(id)->slotOf(4), 0);
}

TEST(SquadsBook, ASquadWithNoOneLeftIsGone)
{
	SquadBook book;
	const SquadId id = book.create("R", 2).id();
	book.join(id, 1, 100.0f);
	book.join(id, 2, 100.0f);
	EXPECT_EQ(book.leave(1), 0);
	EXPECT_EQ(book.leave(2), id);
	EXPECT_EQ(book.find(id), nullptr);
	EXPECT_EQ(book.leave(2), 0);	// leaving twice does nothing
}

TEST(SquadsBook, FullHealthCountsSlotsNotYetFilled)
{
	SquadBook book;
	Squad &squad = book.create("R", 4);
	book.join(squad.id(), 1, 120.0f);
	EXPECT_FLOAT_EQ(squad.fullHealth(), 480.0f);
}

TEST(SquadsTypes, AGeneralsVersionUsesTheBaseEntry)
{
	SquadTypes types;
	SquadType rangers;
	rangers.unit = "AmericaInfantryRanger";
	rangers.size = 4;
	types.add(rangers);

	ASSERT_NE(types.find("AmericaInfantryRanger"), nullptr);
	ASSERT_NE(types.find("AirF_AmericaInfantryRanger"), nullptr);
	EXPECT_EQ(types.find("AirF_AmericaInfantryRanger")->size, 4);
	EXPECT_EQ(types.find("AmericaInfantryPathfinder"), nullptr);
	EXPECT_EQ(types.find("Boss_InfantryRanger"), nullptr);
}

TEST(SquadsTypes, AnOwnEntryWinsAndPrefixesStripOneAtATime)
{
	SquadTypes types;
	SquadType rebels;
	rebels.unit = "GLAInfantryRebel";
	rebels.size = 5;
	types.add(rebels);
	SquadType chemRebels;
	chemRebels.unit = "Chem_GLAInfantryRebel";
	chemRebels.size = 3;
	types.add(chemRebels);

	EXPECT_EQ(types.find("Slth_GLAInfantryRebel")->size, 5);
	EXPECT_EQ(types.find("Chem_GLAInfantryRebel")->size, 3);
	EXPECT_EQ(types.find("GC_Chem_GLAInfantryRebel")->size, 3);
}

TEST(SquadsTypes, EscortsFollowTheOrderedUnitInTheSlots)
{
	SquadType kell;
	kell.unit = "GLAInfantryJarmenKell";
	kell.size = 1;
	kell.escorts.push_back({ "GLAInfantryRebel", 2 });
	EXPECT_EQ(kell.memberCount(), 3);
	EXPECT_EQ(kell.unitOfSlot(0), "GLAInfantryJarmenKell");
	EXPECT_EQ(kell.unitOfSlot(2), "GLAInfantryRebel");
	EXPECT_EQ(kell.unitOfSlot(3), "");
}

TEST(SquadsTypes, ASquadCostsItsMembersScaled)
{
	EXPECT_EQ(squadAmount(225, 1, 4, 0, 90), 810);		// four Rangers, 10% off
	EXPECT_EQ(squadAmount(300, 2, 6, 0, 90), 810);		// Red Guards come two per order
	EXPECT_EQ(squadAmount(1500, 1, 1, 300, 100), 1800);	// a hero with two 150 escorts
	EXPECT_EQ(squadAmount(150, 1, 1, 0, 100), 150);		// a squad of one costs the unit
	EXPECT_EQ(squadAmount(0, 1, 4, 0, 90), 0);
}

TEST(SquadsRoom, ASquadTakesTheRoomOfOneUnit)
{
	std::vector<Rider> inside;
	const Rider ranger = { 5, 1 };
	EXPECT_EQ(roomNeeded(ranger, inside), 1);
	inside.push_back(ranger);
	EXPECT_EQ(roomNeeded(ranger, inside), 0);	// its mates ride along
	inside.push_back(ranger);
	inside.push_back(ranger);
	EXPECT_EQ(roomUsed(inside), 1);

	const Rider loner = { 0, 1 };
	EXPECT_EQ(roomNeeded(loner, inside), 1);
	inside.push_back(loner);
	inside.push_back(loner);
	EXPECT_EQ(roomUsed(inside), 3);

	const Rider otherSquad = { 6, 2 };
	EXPECT_EQ(roomNeeded(otherSquad, inside), 2);
	inside.push_back(otherSquad);
	inside.push_back(otherSquad);
	EXPECT_EQ(roomUsed(inside), 5);
}
