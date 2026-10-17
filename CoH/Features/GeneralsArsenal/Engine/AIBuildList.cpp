// CoH GeneralsArsenal: the super weapon buildings come out of the skirmish AI's build list. Their
// powers are General's powers now and the buildings can't be built; an entry the AI can't place would
// hold up the rest of its building. Its scripts may add them again, so this runs on every AI step.
// Its jets need no change: a team it can't train (no factory builds the unit) is never picked.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Player.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "GameLogic/SidesList.h"

#include "Kernel/Log.h"

#include "Features/GeneralsArsenal/Engine/Config.h"
#include "Features/GeneralsArsenal/Engine/Internal.h"

namespace CoH::GeneralsArsenal
{

void removeAIBuildListEntries(Player *player)
{
	BuildListInfo *head = player->getBuildList();
	BuildListInfo *kept = nullptr;
	BuildListInfo *lastKept = nullptr;

	for (BuildListInfo *info = head; info;)
	{
		BuildListInfo *next = info->getNext();
		const ThingTemplate *plan = TheThingFactory->findTemplate(info->getTemplateName());
		const Bool remove = plan && plan->isAnyKindOf(config().aiBuildListRemovals);

		if (remove)
		{
			CoH::log("GeneralsArsenal: %ls's AI no longer builds %s", player->getPlayerDisplayName().str(), info->getTemplateName().str());
			info->setNextBuildList(nullptr);
			if (info != head)
				deleteInstance(info);
		}
		else
		{
			if (lastKept)
				lastKept->setNextBuildList(info);
			else
				kept = info;
			lastKept = info;
		}
		info = next;
	}
	if (lastKept)
		lastKept->setNextBuildList(nullptr);

	// setBuildList deletes the old head, and only it once it is cut loose.
	if (kept != head)
		player->setBuildList(kept);
}

} // namespace CoH::GeneralsArsenal
