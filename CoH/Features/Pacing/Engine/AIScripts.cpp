// CoH Pacing: the skirmish AI's scripted money. The retail scripts give the AI money on top of its
// income: $2,200 at the start on every difficulty, and on Hard $10,000, $10,000, $15,000 and $20,000 as
// its _ESCALATION counter climbs (one a 10 seconds, more once its enemy has a tech building or a super
// weapon): about $55,000 by minute 13. Against sector income that is more than half the AI's money, so
// AIScriptMoney scales every gift a script gives to "<This Player>".
//
// The scripts are read from the map and SkirmishScripts.scb as every game loads (a save keeps only
// which scripts are active), so this runs for new and loaded games alike and never scales twice.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "GameLogic/Scripts.h"
#include "GameLogic/SidesList.h"

#include "Kernel/Log.h"

#include "Features/Pacing/Domain/Pacing.h"
#include "Features/Pacing/Engine/Internal.h"

namespace CoH::Pacing
{

namespace
{

struct Scaled
{
	int gifts = 0;
	int before = 0;
	int after = 0;
};

bool isGiftToSelf(ScriptAction &action)
{
	if (action.getActionType() != ScriptAction::PLAYER_GIVE_MONEY || action.getNumParameters() < 2)
		return false;
	const Parameter *player = action.getParameter(0);
	const Parameter *amount = action.getParameter(1);
	return player != nullptr && amount != nullptr && player->getString() == THIS_PLAYER && amount->getInt() > 0;
}

void scaleActions(ScriptAction *action, float multiplier, Scaled &scaled)
{
	for (; action != nullptr; action = action->getNext())
	{
		if (!isGiftToSelf(*action))
			continue;
		Parameter *amount = action->getParameter(1);
		const int gift = scaledIncome(static_cast<unsigned int>(amount->getInt()), multiplier);
		++scaled.gifts;
		scaled.before += amount->getInt();
		scaled.after += gift;
		amount->friend_setInt(gift);
	}
}

void scaleScripts(Script *script, float multiplier, Scaled &scaled)
{
	for (; script != nullptr; script = script->getNext())
	{
		scaleActions(script->getAction(), multiplier, scaled);
		scaleActions(script->getFalseAction(), multiplier, scaled);
	}
}

} // namespace

void scaleAIScriptMoney(float multiplier)
{
	if (multiplier == 1.0f)
		return;
	Scaled scaled;
	for (Int i = 0; i < TheSidesList->getNumSides(); ++i)
	{
		ScriptList *list = TheSidesList->getSideInfo(i)->getScriptList();
		if (list == nullptr)
			continue;
		scaleScripts(list->getScript(), multiplier, scaled);
		for (ScriptGroup *group = list->getScriptGroup(); group != nullptr; group = group->getNext())
			scaleScripts(group->getScript(), multiplier, scaled);
	}
	log("Pacing: %d script gifts of money to the AI scaled from $%d to $%d", scaled.gifts, scaled.before, scaled.after);
}

} // namespace CoH::Pacing
