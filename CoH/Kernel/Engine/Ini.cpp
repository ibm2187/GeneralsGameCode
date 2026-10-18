// CoH Kernel: see Kernel/Ini.h.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Kernel/Domain/Frames.h"
#include "Kernel/Ini.h"

namespace CoH::Ini
{

void parseSecondsAsFrames(INI *ini, void * /*instance*/, void *store, const void * /*userData*/)
{
	*static_cast<UnsignedInt *>(store) = framesFromSeconds(INI::scanReal(ini->getNextToken()));
}

} // namespace CoH::Ini
