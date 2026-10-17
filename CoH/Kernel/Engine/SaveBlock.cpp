// CoH Kernel: the CoH block of a save game. GameState saves it before the map, so on load the flags
// are known before the game starts and loads the INI packs. A save without the block, such as one
// made by the original game, loads with every feature off.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Snapshot.h"
#include "Common/Xfer.h"

#include "Kernel/Engine/Internal.h"
#include "Kernel/Hooks.h"

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

} // namespace

Snapshot *Hooks::saveBlock()
{
	static SaveBlock block;
	return &block;
}

} // namespace CoH
