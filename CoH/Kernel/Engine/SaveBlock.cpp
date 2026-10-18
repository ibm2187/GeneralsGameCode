// CoH Kernel: the CoH blocks of a save game.
//
// CHUNK_CoH holds the flags. GameState saves it before the map, so on load the flags are known before
// the game starts and loads the INI packs. A save without the block, such as one made by the original
// game, loads with every feature off.
//
// CHUNK_CoHFeatures holds the features' own saved state (CoH::registerSaveData). It is saved after the
// objects, so on load every object already has its saved ID. Each feature's data is a named block of
// its own: a save without a feature's data, or with data for a feature that isn't on, still loads.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Snapshot.h"
#include "Common/Xfer.h"

#include "Kernel/Engine/Internal.h"
#include "Kernel/Hooks.h"
#include "Kernel/Kernel.h"

#include <vector>

namespace CoH
{

namespace
{

class SaveBlock : public Snapshot
{
protected:
	virtual void crc( Xfer *xfer ) override { }

	// Version 1: the active flags as a list.
	virtual void xfer( Xfer *xfer ) override
	{
		XferVersion currentVersion = 1;
		XferVersion version = currentVersion;
		xfer->xferVersion( &version, currentVersion );

		AsciiString flags = Kernel::registry().active().toList().c_str();
		xfer->xferAsciiString( &flags );

		if (xfer->getXferMode() == XFER_LOAD)
			GameFlags::setSaved(FeatureSet::fromList(flags.str()));
	}

	virtual void loadPostProcess() override { }
};

struct SavePart
{
	const Feature *owner;
	AsciiString name;
	Snapshot *data;
};

std::vector<SavePart> &saveParts()
{
	static std::vector<SavePart> parts;
	return parts;
}

const SavePart *findActivePart(const AsciiString &name)
{
	for (const SavePart &part : saveParts())
	{
		if (part.owner->isActive() && part.name == name)
			return &part;
	}
	return nullptr;
}

class FeatureSaveBlock : public Snapshot
{
protected:
	virtual void crc( Xfer *xfer ) override { }

	// Version 1: the number of parts, then each part's name and its data in a block of its own.
	virtual void xfer( Xfer *xfer ) override
	{
		XferVersion currentVersion = 1;
		XferVersion version = currentVersion;
		xfer->xferVersion( &version, currentVersion );

		if (xfer->getXferMode() == XFER_SAVE)
		{
			UnsignedShort count = 0;
			for (const SavePart &part : saveParts())
			{
				if (part.owner->isActive())
					++count;
			}
			xfer->xferUnsignedShort( &count );

			for (const SavePart &part : saveParts())
			{
				if (!part.owner->isActive())
					continue;
				AsciiString name = part.name;
				xfer->xferAsciiString( &name );
				xfer->beginBlock();
				xfer->xferSnapshot( part.data );
				xfer->endBlock();
			}
			return;
		}

		UnsignedShort count = 0;
		xfer->xferUnsignedShort( &count );
		for (UnsignedShort i = 0; i < count; ++i)
		{
			AsciiString name;
			xfer->xferAsciiString( &name );
			const Int size = xfer->beginBlock();
			const SavePart *part = findActivePart(name);
			if (part == nullptr)
			{
				DEBUG_LOG(( "CoH: skipping saved data '%s', its feature isn't on", name.str() ));
				xfer->skip( size );
				continue;
			}
			xfer->xferSnapshot( part->data );
			xfer->endBlock();
		}
	}

	virtual void loadPostProcess() override { }
};

} // namespace

void registerSaveData(const Feature &self, const char *name, Snapshot *data)
{
	DEBUG_ASSERTCRASH(data != nullptr, ("CoH: saved data '%s' has no snapshot", name));
	for (const SavePart &part : saveParts())
	{
		if (part.name.compare(name) == 0)
		{
			DEBUG_CRASH(("CoH: saved data '%s' is registered twice", name));
			return;
		}
	}
	saveParts().push_back({ &self, AsciiString(name), data });
}

Snapshot *Hooks::saveBlock()
{
	static SaveBlock block;
	return &block;
}

Snapshot *Hooks::featureSaveBlock()
{
	static FeatureSaveBlock block;
	return &block;
}

} // namespace CoH
