// CoH Squads: the game's squads in a save game. The Kernel loads this after the objects, so the
// members' saved IDs are theirs again; a member that isn't in the game leaves its slot empty.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Snapshot.h"
#include "Common/Xfer.h"
#include "GameLogic/GameLogic.h"

#include "Kernel/Log.h"

#include "Features/Squads/Engine/Internal.h"

namespace CoH::Squads
{

namespace
{

void xferBook(Xfer *xfer)
{
	SavedBook saved;
	if (xfer->getXferMode() == XFER_SAVE)
		saved = book().save();

	Int nextId = saved.nextId;
	xfer->xferInt(&nextId);
	saved.nextId = nextId;

	UnsignedShort squadCount = (UnsignedShort)saved.squads.size();
	xfer->xferUnsignedShort(&squadCount);
	saved.squads.resize(squadCount);
	for (SavedSquad &squad : saved.squads)
	{
		Int id = squad.id;
		xfer->xferInt(&id);
		squad.id = id;

		AsciiString unit = squad.unit.c_str();
		xfer->xferAsciiString(&unit);
		squad.unit = unit.str();

		UnsignedByte slotCount = (UnsignedByte)squad.slots.size();
		xfer->xferUnsignedByte(&slotCount);
		squad.slots.resize(slotCount);
		for (SavedSlot &slot : squad.slots)
		{
			ObjectID member = (ObjectID)slot.member;
			xfer->xferObjectID(&member);
			slot.member = (MemberId)member;
			Real maxHealth = slot.maxHealth;
			xfer->xferReal(&maxHealth);
			slot.maxHealth = maxHealth;
		}
	}

	if (xfer->getXferMode() == XFER_LOAD)
	{
		const int members = book().restore(saved,
			[](MemberId member) { return TheGameLogic->findObjectByID((ObjectID)member) != nullptr; });
		CoH::log("Squads: loaded %d squads with %d members", (int)book().squads().size(), members);
	}
}

void xferOpenSquads(Xfer *xfer)
{
	UnsignedShort count = (UnsignedShort)openSquads().size();
	xfer->xferUnsignedShort(&count);
	if (xfer->getXferMode() == XFER_SAVE)
	{
		for (const auto &entry : openSquads())
		{
			ObjectID factory = entry.first;
			Int squad = entry.second;
			xfer->xferObjectID(&factory);
			xfer->xferInt(&squad);
		}
		return;
	}

	openSquads().clear();
	for (UnsignedShort i = 0; i < count; ++i)
	{
		ObjectID factory = INVALID_ID;
		Int squad = 0;
		xfer->xferObjectID(&factory);
		xfer->xferInt(&squad);
		if (book().find(squad) != nullptr)
			openSquads()[factory] = squad;
	}
}

void xferSlotPositions(Xfer *xfer)
{
	UnsignedShort count = (UnsignedShort)slotPositions().size();
	xfer->xferUnsignedShort(&count);
	if (xfer->getXferMode() == XFER_SAVE)
	{
		for (auto &entry : slotPositions())
		{
			ObjectID member = (ObjectID)entry.first;
			xfer->xferObjectID(&member);
			xfer->xferCoord3D(&entry.second);
		}
		return;
	}

	slotPositions().clear();
	for (UnsignedShort i = 0; i < count; ++i)
	{
		ObjectID member = INVALID_ID;
		Coord3D position;
		xfer->xferObjectID(&member);
		xfer->xferCoord3D(&position);
		if (book().squadOf((MemberId)member) != 0)
			slotPositions()[(MemberId)member] = position;
	}
}

class SaveData : public Snapshot
{
protected:
	virtual void crc(Xfer * /*xfer*/) override { }

	// Version 1: the book (each squad's id, unit and slots), the factories' open squads, the slot
	// positions.
	virtual void xfer(Xfer *xfer) override
	{
		XferVersion currentVersion = 1;
		XferVersion version = currentVersion;
		xfer->xferVersion(&version, currentVersion);

		xferBook(xfer);
		xferOpenSquads(xfer);
		xferSlotPositions(xfer);
	}

	virtual void loadPostProcess() override { }
};

} // namespace

Snapshot *saveData()
{
	static SaveData data;
	return &data;
}

} // namespace CoH::Squads
