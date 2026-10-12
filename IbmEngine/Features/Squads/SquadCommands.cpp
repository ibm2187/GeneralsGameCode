// IBM Engine: squad skills. You command the leader, but skills belong to members: Burton's C4, Black
// Lotus's hacks, Kell's kill-driver shot, the bombers' Detonate. The leader's button set shows them;
// the buttons are evaluated for the member that has the skill, and the order goes to that member,
// the nearest one if several have it, or all of them for a weapon fired without a target.
// Everything after the hand-over is the engine's normal code.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/MessageStream.h"
#include "Common/SpecialPower.h"
#include "GameClient/ControlBar.h"
#include "GameLogic/AI.h"
#include "GameLogic/PartitionManager.h"

#include "Features/Squads/Squads.h"
#include "Features/Squads/SquadUpdate.h"

namespace
{

// What an order needs from whoever carries it out: a special power or a weapon.
struct Skill
{
	const SpecialPowerTemplate *power;
	WeaponSlotType weaponSlot;
	Bool isWeapon;

	Skill() : power(nullptr), weaponSlot(PRIMARY_WEAPON), isWeapon(FALSE) { }

	Bool isHeldBy(const Object *obj) const
	{
		if (isWeapon)
			return obj->getWeaponInWeaponSlot(weaponSlot) != nullptr && IbmEngine::hasFireWeaponButton(obj, weaponSlot);
		return power != nullptr && obj->getSpecialPowerModule(power) != nullptr;
	}
};

// The squad's member that can use the skill, nearest to pos. Members inside a container can't.
Object *findMemberWithSkill(const Object *leader, const Skill &skill, const Coord3D &pos)
{
	Object *best = nullptr;
	Real bestDistSqr = 0.0f;

	IbmEngine::forEachSquadMember(leader, [&](Object *member)
	{
		if (member->isContained() || !skill.isHeldBy(member))
			return;

		const Real distSqr = ThePartitionManager->getDistanceSquared(member, &pos, FROM_CENTER_2D);
		if (best == nullptr || distSqr < bestDistSqr)
		{
			best = member;
			bestDistSqr = distSqr;
		}
	});

	return best;
}

} // namespace

//-------------------------------------------------------------------------------------------------
Bool IbmEngine::hasFireWeaponButton(const Object *obj, WeaponSlotType slot)
{
	const CommandSet *set = TheControlBar->findCommandSet(obj->getCommandSetString());
	if (set == nullptr)
		return FALSE;

	for (Int i = 0; i < MAX_COMMANDS_PER_SET; ++i)
	{
		const CommandButton *button = set->getCommandButton(i);
		if (button != nullptr && button->getCommandType() == GUI_COMMAND_FIRE_WEAPON && button->getWeaponSlot() == slot)
			return TRUE;
	}
	return FALSE;
}

//-------------------------------------------------------------------------------------------------
void IbmEngine::routeSquadCommand(const GameMessage *msg, AIGroup *group)
{
	if (group == nullptr)
		return;

	Skill skill;
	Coord3D target;
	Bool hasTarget = FALSE;
	ObjectID specificSource = INVALID_ID;

	// The argument layouts are the ones GameLogic's dispatcher reads.
	switch (msg->getType())
	{
		case GameMessage::MSG_DO_SPECIAL_POWER:
			skill.power = TheSpecialPowerStore->findSpecialPowerTemplateByID(msg->getArgument(0)->integer);
			specificSource = msg->getArgument(2)->objectID;
			break;

		case GameMessage::MSG_DO_SPECIAL_POWER_AT_LOCATION:
		{
			const Bool hasAngle = msg->getArgumentCount() >= 6;
			skill.power = TheSpecialPowerStore->findSpecialPowerTemplateByID(msg->getArgument(0)->integer);
			target = msg->getArgument(1)->location;
			hasTarget = TRUE;
			specificSource = msg->getArgument(hasAngle ? 5 : 4)->objectID;
			break;
		}

		case GameMessage::MSG_DO_SPECIAL_POWER_AT_OBJECT:
		{
			skill.power = TheSpecialPowerStore->findSpecialPowerTemplateByID(msg->getArgument(0)->integer);
			const Object *targetObj = TheGameLogic->findObjectByID(msg->getArgument(1)->objectID);
			if (targetObj != nullptr)
			{
				target = *targetObj->getPosition();
				hasTarget = TRUE;
			}
			specificSource = msg->getArgument(3)->objectID;
			break;
		}

		case GameMessage::MSG_DO_WEAPON:
			skill.isWeapon = TRUE;
			skill.weaponSlot = (WeaponSlotType)msg->getArgument(0)->integer;
			break;

		case GameMessage::MSG_DO_WEAPON_AT_LOCATION:
			skill.isWeapon = TRUE;
			skill.weaponSlot = (WeaponSlotType)msg->getArgument(0)->integer;
			target = msg->getArgument(1)->location;
			hasTarget = TRUE;
			break;

		case GameMessage::MSG_DO_WEAPON_AT_OBJECT:
		{
			skill.isWeapon = TRUE;
			skill.weaponSlot = (WeaponSlotType)msg->getArgument(0)->integer;
			const Object *targetObj = TheGameLogic->findObjectByID(msg->getArgument(1)->objectID);
			if (targetObj != nullptr)
			{
				target = *targetObj->getPosition();
				hasTarget = TRUE;
			}
			break;
		}

		default:
			return;
	}

	// An order for one specific source (a shortcut power) ignores the selection.
	if (specificSource != INVALID_ID && TheGameLogic->findObjectByID(specificSource) != nullptr)
		return;

	if (!skill.isWeapon && skill.power == nullptr)
		return;

	const Bool allHolders = msg->getType() == GameMessage::MSG_DO_WEAPON;

	// The squads in the selection, in selection order. Selecting a squad selects its members too.
	std::vector<Object *> leaders;
	const VecObjectID ids = group->getAllIDs();
	for (VecObjectID::const_iterator it = ids.begin(); it != ids.end(); ++it)
	{
		Object *obj = TheGameLogic->findObjectByID(*it);
		Object *leader = isSquadLeader(obj) ? obj : getSquadLeaderOf(obj);
		if (leader != nullptr && std::find(leaders.begin(), leaders.end(), leader) == leaders.end())
			leaders.push_back(leader);
	}

	for (std::vector<Object *>::iterator it = leaders.begin(); it != leaders.end(); ++it)
	{
		Object *leader = *it;

		// A power or weapon of the leader's own stays with the leader.
		if (skill.isHeldBy(leader))
			continue;

		// Who carries out the order: the nearest member with the skill or, for a weapon fired without
		// a target (the bombers' Detonate), every member with it.
		std::vector<Object *> chosen;
		if (allHolders)
		{
			forEachSquadMember(leader, [&](Object *member)
			{
				if (!member->isContained() && skill.isHeldBy(member))
					chosen.push_back(member);
			});
		}
		else if (Object *member = findMemberWithSkill(leader, skill, hasTarget ? target : *leader->getPosition()))
		{
			chosen.push_back(member);
		}
		if (chosen.empty())
			continue;

		// Add the members first: a group that becomes empty destroys itself.
		for (std::vector<Object *>::iterator c = chosen.begin(); c != chosen.end(); ++c)
		{
			if (!group->isMember(*c))
				group->add(*c);
		}

		if (group->isMember(leader))
			group->remove(leader);

		forEachSquadMember(leader, [&](Object *other)
		{
			if (std::find(chosen.begin(), chosen.end(), other) == chosen.end() && group->isMember(other))
				group->remove(other);
		});

		for (std::vector<Object *>::iterator c = chosen.begin(); c != chosen.end(); ++c)
			getSquad(leader)->setMemberOnOwnOrder((*c)->getID());
	}
}

//-------------------------------------------------------------------------------------------------
Bool IbmEngine::evaluateSquadCommand(const CommandButton *command, Object *&obj, CommandAvailability &availability)
{
	if (obj == nullptr || !obj->isKindOf(KINDOF_MOB_NEXUS) || !isSquadLeader(obj))
		return FALSE;

	Skill skill;
	switch (command->getCommandType())
	{
		case GUI_COMMAND_SPECIAL_POWER:
			skill.power = command->getSpecialPowerTemplate();
			if (skill.power == nullptr)
				return FALSE;
			break;

		case GUI_COMMAND_FIRE_WEAPON:
			skill.isWeapon = TRUE;
			skill.weaponSlot = command->getWeaponSlot();
			break;

		default:
			return FALSE;
	}

	if (skill.isHeldBy(obj))
		return FALSE;

	Object *member = findMemberWithSkill(obj, skill, *obj->getPosition());
	if (member == nullptr)
	{
		// The member with this skill is dead or inside something.
		availability = COMMAND_RESTRICTED;
		return TRUE;
	}

	obj = member;
	return FALSE;
}
