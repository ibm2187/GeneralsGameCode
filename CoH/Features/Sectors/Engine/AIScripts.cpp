// CoH Sectors: the skirmish AI scripts are built around supply buildings. "Build the War Factory once
// a Supply Center is built" never comes true without them, and the AI stops building after its
// barracks. So their conditions ask about the same general's Barracks instead, the building the pack's
// prerequisites ask for too (Data/make_pack.py). Only conditions change; an action that builds a
// supply building finds nothing to build.
//
// The scripts are read from the map and SkirmishScripts.scb as every game loads, so this runs for new
// and loaded games alike.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "GameLogic/Scripts.h"
#include "GameLogic/SidesList.h"

#include "Kernel/Log.h"

#include "Features/Sectors/Engine/Config.h"
#include "Features/Sectors/Engine/Internal.h"

namespace CoH::Sectors
{

namespace
{

// The same general's Barracks for a supply building: AirF_AmericaSupplyCenter -> AirF_AmericaBarracks.
const ThingTemplate *replacementFor(const AsciiString &name)
{
	const ThingTemplate *original = TheThingFactory->findTemplate(name);
	if (original == nullptr || !original->isAnyKindOf(config().aiBuildListRemovals))
		return nullptr;

	for (const ScriptRename &rename : config().aiScriptRenames)
	{
		const char *found = strstr(name.str(), rename.from.str());
		if (found == nullptr)
			continue;

		AsciiString renamed;
		renamed.set(name.str(), (Int)(found - name.str()));
		renamed.concat(rename.to);
		renamed.concat(found + rename.from.getLength());
		if (const ThingTemplate *replacement = TheThingFactory->findTemplate(renamed))
			return replacement;
	}
	return nullptr;
}

int retargetCondition(Condition *condition)
{
	int changed = 0;
	for (Int i = 0; i < condition->getNumParameters(); ++i)
	{
		Parameter *parameter = condition->getParameter(i);
		if (parameter == nullptr || parameter->getParameterType() != Parameter::OBJECT_TYPE)
			continue;
		if (const ThingTemplate *replacement = replacementFor(parameter->getString()))
		{
			parameter->friend_setString(replacement->getName());
			++changed;
		}
	}
	return changed;
}

int retargetScript(Script *script)
{
	int changed = 0;
	for (OrCondition *any = script->getOrCondition(); any; any = any->getNextOrCondition())
	{
		for (Condition *condition = any->getFirstAndCondition(); condition; condition = condition->getNext())
			changed += retargetCondition(condition);
	}
	return changed;
}

int retargetList(ScriptList *list)
{
	int changed = 0;
	for (Script *script = list->getScript(); script; script = script->getNext())
		changed += retargetScript(script);
	for (ScriptGroup *group = list->getScriptGroup(); group; group = group->getNext())
	{
		for (Script *script = group->getScript(); script; script = script->getNext())
			changed += retargetScript(script);
	}
	return changed;
}

} // namespace

void retargetAIScripts()
{
	int changed = 0;
	for (Int i = 0; i < TheSidesList->getNumSides(); ++i)
	{
		if (ScriptList *list = TheSidesList->getSideInfo(i)->getScriptList())
			changed += retargetList(list);
	}
	CoH::log("Sectors: %d script conditions on supply buildings now ask about barracks", changed);
}

} // namespace CoH::Sectors
