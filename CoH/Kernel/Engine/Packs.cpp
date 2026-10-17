// CoH Kernel: INI packs. A slice's Data/INI folder installs to CoH\Packs\<Name> in the game folder
// (CoH/Scripts/install.ps1). At game start, after the base INI and before map.ini, the pack of every
// active feature loads like a map.ini does: with INI_LOAD_CREATE_OVERRIDES, so the objects, command
// sets, weapons and so on that it changes are overrides, and the engine drops them when the game
// resets. A feature without a pack folder loads nothing.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/INI.h"

#include "Kernel/Engine/Internal.h"

namespace CoH::Packs
{

namespace
{

void loadPack(const Feature &feature)
{
	AsciiString folder;
	folder.format("CoH\\Packs\\%s", feature.name().c_str());

	DEBUG_LOG(("CoH: loading the %s pack", feature.name().c_str()));
	INI ini;
	ini.loadDirectory(folder, INI_LOAD_CREATE_OVERRIDES, nullptr, INI::LoadFlags_SearchSubDirs);
}

} // namespace

void loadActive(const FeatureRegistry &registry)
{
	for (const std::unique_ptr<Feature> &feature : registry.features())
	{
		if (feature->isActive())
			loadPack(*feature);
	}
}

} // namespace CoH::Packs
