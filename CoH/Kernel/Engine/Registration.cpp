// CoH Kernel: the modules and INI blocks slices add. Slices add them in their init, at engine init;
// the module factory takes the modules when it starts, and the INI parser asks for a block's parser
// when the block isn't in its own table. Both are there whether a flag is on or not: only an active
// feature's pack names them.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Kernel/Engine/Internal.h"
#include "Kernel/Kernel.h"

#include <map>
#include <string>

namespace CoH
{

namespace
{

struct ModuleEntry
{
	NewModuleProc create;
	NewModuleDataProc createData;
	ModuleType type;
	AsciiString name;
	Int interfaceMask;
};

std::vector<ModuleEntry> &modules()
{
	static std::vector<ModuleEntry> entries;
	return entries;
}

std::map<std::string, INIBlockParse> &iniBlocks()
{
	static std::map<std::string, INIBlockParse> blocks;
	return blocks;
}

// ModuleFactory::addModuleInternal is protected, for the factory and its subclasses. A pointer to it,
// taken through a subclass, lets the Kernel add modules to the factory without changing the factory.
class ModuleFactoryAccess : public ModuleFactory
{
public:
	static void add(ModuleFactory &factory, const ModuleEntry &entry)
	{
		void (ModuleFactory::*addModuleInternal)(NewModuleProc, NewModuleDataProc, ModuleType, const AsciiString &, Int) =
			&ModuleFactoryAccess::addModuleInternal;
		(factory.*addModuleInternal)(entry.create, entry.createData, entry.type, entry.name, entry.interfaceMask);
	}
};

} // namespace

void registerModule(NewModuleProc create, NewModuleDataProc createData, ModuleType type, const AsciiString &name,
	Int interfaceMask)
{
	modules().push_back({ create, createData, type, name, interfaceMask });
}

void registerIniBlock(const char *token, INIBlockParse parse)
{
	DEBUG_ASSERTCRASH(iniBlocks().count(token) == 0, ("CoH: INI block '%s' is added twice", token));
	iniBlocks()[token] = parse;
}

void Registration::addModulesTo(ModuleFactory &factory)
{
	for (const ModuleEntry &entry : modules())
		ModuleFactoryAccess::add(factory, entry);
}

INIBlockParse Registration::findIniBlock(const char *token)
{
	const auto it = iniBlocks().find(token);
	return it != iniBlocks().end() ? it->second : nullptr;
}

} // namespace CoH
