// CoH Kernel: the registry of slices and the local flags, set up once at engine init.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Kernel/Engine/Internal.h"
#include "Kernel/Kernel.h"

namespace CoH::Kernel
{

namespace
{

FeatureSet &localFlagsStorage()
{
	static FeatureSet flags;
	return flags;
}

} // namespace

FeatureRegistry &registry()
{
	static FeatureRegistry theRegistry;
	return theRegistry;
}

const FeatureSet &localFlags()
{
	return localFlagsStorage();
}

void init()
{
	registerFeatures(registry());
	registerIniBlock("CoHText", PackText::parseBlock);

	// Only flags of features this build has: a stale name in Features.ini must not end up in the
	// game options of a game, where it would make the options string differ from the original.
	localFlagsStorage() = registry().known(readLocalFlags());
	DEBUG_LOG(("CoH: %d features, local flags '%s'", (Int)registry().features().size(), localFlags().toList().c_str()));

	registry().initAll();
}

} // namespace CoH::Kernel
