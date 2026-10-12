// IBM Engine: the "ObjectCopy NewName SourceName" INI block.
// ObjectReskin can only change an object's model and size. ObjectCopy copies everything,
// then parses the full object field table, so a pack can change weapons, behaviors, cost or
// KindOf without repeating the original definition. In override mode (feature packs,
// map.ini) modules must be changed with AddModule, RemoveModule and ReplaceModule.
// A WeaponSet or ArmorSet in the copy replaces all of the source's sets.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/INI.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"

#include "Features/FeatureFlags/FeatureFlags.h"

namespace IbmEngine
{

void parseObjectCopyDefinition(INI *ini)
{
	const AsciiString name = ini->getNextToken();
	const AsciiString sourceName = ini->getNextToken();

	const ThingTemplate *source = TheThingFactory->findTemplate(sourceName, FALSE);
	if (source == nullptr)
	{
		DEBUG_CRASH(("[LINE: %d in '%s'] ObjectCopy %s: source object %s not found. It must be defined first.",
			ini->getLineNum(), ini->getFilename().str(), name.str(), sourceName.str()));
		throw INI_INVALID_DATA;
	}

	if (TheThingFactory->findTemplate(name, FALSE) != nullptr)
	{
		DEBUG_CRASH(("[LINE: %d in '%s'] ObjectCopy %s: an object with this name already exists.",
			ini->getLineNum(), ini->getFilename().str(), name.str()));
		throw INI_INVALID_DATA;
	}

	ThingTemplate *thingTemplate = TheThingFactory->newTemplate(name);
	thingTemplate->copyFrom(source);

	// copyFrom copies the source's override chain too. The copy starts its own.
	thingTemplate->setNextOverride(nullptr);
	if (ini->getLoadType() == INI_LOAD_CREATE_OVERRIDES)
	{
		// Removed again when the game ends, like every other override.
		thingTemplate->markAsOverride();
	}

	// Lets the first WeaponSet, ArmorSet or module of a kind replace the copied ones.
	thingTemplate->setCopiedFromDefault();

	ini->initFromINI(thingTemplate, thingTemplate->getFieldParse());

	thingTemplate->validate();

	if (ini->getLoadType() == INI_LOAD_CREATE_OVERRIDES)
		thingTemplate->resolveNames();
}

} // namespace IbmEngine
