// CoH Kernel: what a slice's Engine code includes to declare itself and plug into the engine.
//
// A slice declares itself once, in its Engine code:
//
//     namespace { void init(CoH::Feature &self); void reset(); }
//     COH_FEATURE(Sectors, init, reset)
//
// and is listed in CoH/CMakeLists.txt with coh_add_feature(Sectors). The name is the slice's folder
// under CoH/Features, its flag, and the folder its INI pack installs to. In init it subscribes to the
// hub (Kernel/Hooks.h) and adds its modules and INI blocks.

#pragma once

#include "Common/ModuleFactory.h"

#include "Kernel/Hooks.h"

#define COH_FEATURE(Name, initFunction, resetFunction) \
	::CoH::FeatureInfo coh_feature_##Name() { return { #Name, initFunction, resetFunction }; }

namespace CoH
{

// A module the slice's INI can name in an Object, added to the module factory with the original ones.
void registerModule(NewModuleProc create, NewModuleDataProc createData, ModuleType type, const AsciiString &name,
	Int interfaceMask);

// An INI block the slice's pack can contain, such as CoHSectors. Packs load with INI_LOAD_CREATE_OVERRIDES.
void registerIniBlock(const char *token, INIBlockParse parse);

// Game state the slice saves with the game, under 'name', while the feature is on. It is saved after
// the objects, so on load its xfer runs once every object has its saved ID, and its loadPostProcess
// once the whole save is in. A save without it (made with the flag off, or before the slice saved
// anything) doesn't call it: the slice keeps the state a new game starts with. Register it in init.
void registerSaveData(const Feature &self, const char *name, Snapshot *data);

// Whether the game was started with this command-line option (for a slice's test options).
bool hasCommandLineOption(const char *option);

} // namespace CoH

// The CoH form of ModuleFactory's addModule.
#define COH_ADD_MODULE(classname) \
	::CoH::registerModule(classname::friend_newModuleInstance, classname::friend_newModuleData, \
		classname::getModuleType(), AsciiString(#classname), classname::getInterfaceMask())
