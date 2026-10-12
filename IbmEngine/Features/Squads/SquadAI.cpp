// IBM Engine: the skirmish AI builds squads. AI build orders name units in the map's and the
// skirmish scripts' team templates, and a factory only builds what its buttons offer. With squads
// on, the barracks offer squads instead of basic infantry, so each team template that asks for a
// basic infantry unit asks for its squad instead (IbmSquadUpdate's AIReplaces), in numbers that
// give about as many soldiers as before.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"

#include "Features/FeatureFlags/FeatureFlags.h"
#include "Features/Squads/Squads.h"
#include "Features/Squads/SquadUpdate.h"

//-------------------------------------------------------------------------------------------------
void IbmEngine::replaceTeamUnitWithSquad(AsciiString &unitThingName, Int &minUnits, Int &maxUnits)
{
	if (!isFeatureActive(Feature_Squads) || unitThingName.isEmpty())
		return;

	for (const ThingTemplate *t = TheThingFactory->firstTemplate(); t != nullptr; t = t->friend_getNextTemplate())
	{
		// Squad leaders are mob leaders; this skips almost every template cheaply.
		if (!t->isKindOf(KINDOF_MOB_NEXUS))
			continue;

		const ModuleInfo &modules = t->getBehaviorModuleInfo();
		for (Int i = 0; i < modules.getCount(); ++i)
		{
			if (modules.getNthName(i) != "IbmSquadUpdate")
				continue;

			const IbmSquadUpdateModuleData *data = static_cast<const IbmSquadUpdateModuleData *>(modules.getNthData(i));
			if (data->m_aiReplaces.isNotEmpty() && data->m_aiReplaces == unitThingName)
			{
				unitThingName = t->getName();

				const SpawnBehaviorModuleData *spawn = findSquadSpawnData(t);
				const Int size = spawn != nullptr ? max(1, spawn->m_spawnNumberData) : 1;
				minUnits = (minUnits + size - 1) / size;
				maxUnits = max(1, (maxUnits + size - 1) / size);
				return;
			}
		}
	}
}
