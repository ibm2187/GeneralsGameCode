// CoH Squads: one selection, one order. Selecting a member selects its squad; a squad sent somewhere
// spreads over formation slots; a member that leaves a transport takes its squad with it.
//
// Selection works on the player's order messages in game logic, so orders reach the whole squad on
// every machine, and the local player's own clicks select the mates on screen too. Moves are
// collected while the frame's orders arrive and re-issued, one slot per member, at the end of it.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Player.h"
#include "Common/ThingTemplate.h"
#include "GameLogic/AI.h"
#include "GameLogic/AIPathfind.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Object.h"
#include "GameLogic/TerrainLogic.h"

#include "Kernel/Log.h"

#include "Features/Squads/Domain/Formation.h"
#include "Features/Squads/Squads.h"
#include "Features/Squads/Engine/Config.h"
#include "Features/Squads/Engine/Internal.h"

#include <map>
#include <set>
#include <vector>

namespace CoH::Squads
{

namespace
{

struct Move
{
	ObjectID member = INVALID_ID;
	Coord3D goal;
	CommandSourceType source = CMD_FROM_AI;
	bool attackMove = false;
	int maxShots = 0;
};

struct Exit
{
	ObjectID member = INVALID_ID;
	ObjectID container = INVALID_ID;
};

struct Orders
{
	bool issuing = false;	///< the slice is giving orders itself: they aren't collected again
	std::map<SquadId, std::vector<Move>> moves;
	std::map<SquadId, std::vector<Exit>> exits;
	std::map<MemberId, Coord3D> slotPositions;
};

Orders &orders()
{
	static Orders theOrders;
	return theOrders;
}

void collectMove(SquadId squad, Object *member, const Coord3D &goal, const AICommandParms *command, bool attackMove)
{
	Move move;
	move.member = member->getID();
	move.goal = goal;
	move.source = command->m_cmdSource;
	move.attackMove = attackMove;
	move.maxShots = command->m_intValue;
	orders().moves[squad].push_back(move);
}

Coord3D averageOf(const std::vector<Coord3D> &points)
{
	Coord3D sum;
	sum.zero();
	for (const Coord3D &point : points)
		sum.add(point);
	if (!points.empty())
		sum.scale(1.0f / points.size());
	return sum;
}

// The members that were sent, in slot order, still alive and outside any container.
std::vector<Object *> moversOf(const Squad &squad, const std::vector<Move> &moves)
{
	std::vector<Object *> movers;
	for (Object *member : livingMembers(squad))
	{
		if (member->getContainedBy() != nullptr)
			continue;
		for (const Move &move : moves)
		{
			if (move.member == member->getID())
			{
				movers.push_back(member);
				break;
			}
		}
	}
	return movers;
}

void carryOutMoves()
{
	for (const auto &entry : orders().moves)
	{
		const Squad *squad = book().find(entry.first);
		const std::vector<Move> &moves = entry.second;
		// A single member sent somewhere (stepping aside, a skill) goes where the engine sent it.
		if (squad == nullptr || moves.size() < 2)
			continue;
		const std::vector<Object *> movers = moversOf(*squad, moves);
		if (movers.size() < 2)
			continue;

		std::vector<Coord3D> goals;
		for (const Move &move : moves)
			goals.push_back(move.goal);
		const Move &first = moves.front();
		sendInFormation(movers, averageOf(goals), first.source, first.attackMove, first.maxShots);
	}
	orders().moves.clear();
}

// A member that leaves a container takes the mates that rode with it.
void carryOutExits()
{
	for (const auto &entry : orders().exits)
	{
		const Squad *squad = book().find(entry.first);
		if (squad == nullptr)
			continue;
		std::set<ObjectID> leaving;
		for (const Exit &exit : entry.second)
			leaving.insert(exit.member);

		for (const Exit &exit : entry.second)
		{
			Object *container = TheGameLogic->findObjectByID(exit.container);
			if (container == nullptr)
				continue;
			for (Object *mate : livingMembers(*squad))
			{
				AIUpdateInterface *ai = mate->getAIUpdateInterface();
				if (ai == nullptr || mate->getContainedBy() != container || leaving.count(mate->getID()) != 0)
					continue;
				leaving.insert(mate->getID());
				orders().issuing = true;
				ai->aiExit(container, CMD_FROM_AI);
				orders().issuing = false;
			}
		}
	}
	orders().exits.clear();
}

// The player's selection doesn't check for doubles, and a message naming several members of a squad
// selects some of them twice: once for a mate named earlier, once for itself. Taking each member out
// twice (each removal takes one) and putting it back once leaves every member in it once.
void takeOutOfSelection(Object *member, PlayerMaskType mask, Bool affectClient)
{
	TheGameLogic->deselectObject(member, mask, FALSE);
	TheGameLogic->deselectObject(member, mask, affectClient);
}

} // namespace

void onPlayerSelection(Player *player, Object *object, bool selected)
{
	const Squad *squad = squadOfObject(object);
	if (squad == nullptr)
		return;
	const PlayerMaskType mask = player->getPlayerMask();
	for (Object *member : livingMembers(*squad))
	{
		// Mates riding in a transport stay as they are, as the engine leaves riders.
		if (member->getContainedBy() != object->getContainedBy())
			continue;
		takeOutOfSelection(member, mask, !selected && member->isLocallyControlled());
		if (selected)
			TheGameLogic->selectObject(member, FALSE, mask, member->isLocallyControlled());
	}
}

void onAiCommand(Object *object, const AICommandParms *command)
{
	if (orders().issuing || command == nullptr)
		return;
	const Squad *squad = squadOfObject(object);
	if (squad == nullptr)
		return;

	switch (command->m_cmd)
	{
		case AICMD_MOVE_TO_POSITION:
			collectMove(squad->id(), object, command->m_pos, command, false);
			break;
		case AICMD_ATTACKMOVE_TO_POSITION:
			collectMove(squad->id(), object, command->m_pos, command, true);
			break;
		case AICMD_FOLLOW_PATH:	// how a group of infantry is sent
			if (!command->m_coords.empty())
				collectMove(squad->id(), object, command->m_coords.back(), command, false);
			break;
		case AICMD_EXIT:
			if (command->m_obj != nullptr)
				orders().exits[squad->id()].push_back({ object->getID(), command->m_obj->getID() });
			break;
		case AICMD_ENTER:
			if (CoH::isLogging() && command->m_obj != nullptr && object == leader(squad->id()))
				CoH::log("Squads: frame %u squad %d (%s, player %d) sent into %s", TheGameLogic->getFrame(), squad->id(),
					squad->unit().c_str(), object->getControllingPlayer()->getPlayerIndex(),
					command->m_obj->getTemplate()->getName().str());
			break;
		default:
			break;
	}
}

void carryOutOrders()
{
	carryOutMoves();
	carryOutExits();
}

void sendInFormation(const std::vector<Object *> &movers, const Coord3D &goal,
	CommandSourceType source, bool attackMove, int maxShots)
{
	if (movers.empty())
		return;
	std::vector<Coord3D> positions;
	for (const Object *mover : movers)
		positions.push_back(*mover->getPosition());
	const Coord3D center = averageOf(positions);
	const float heading = headingFrom({ center.x, center.y }, { goal.x, goal.y }, movers.front()->getOrientation());

	const int count = (int)movers.size();
	for (int i = 0; i < count; ++i)
	{
		Object *mover = movers[i];
		AIUpdateInterface *ai = mover->getAIUpdateInterface();
		if (ai == nullptr)
			continue;
		const Point spot = slotPosition({ goal.x, goal.y }, heading, formationSlot(i, count, config().formation));
		Coord3D position;
		position.set(spot.x, spot.y, TheTerrainLogic->getGroundHeight(spot.x, spot.y));
		TheAI->pathfinder()->adjustDestination(mover, ai->getLocomotorSet(), &position);

		orders().issuing = true;
		if (attackMove)
			ai->aiAttackMoveToPosition(&position, maxShots, source);
		else
			ai->aiMoveToPosition(&position, source);
		orders().issuing = false;
		orders().slotPositions[(MemberId)mover->getID()] = position;
	}
}

bool lastSlotPosition(MemberId member, Coord3D *position)
{
	const auto found = orders().slotPositions.find(member);
	if (found == orders().slotPositions.end())
		return false;
	if (position != nullptr)
		*position = found->second;
	return true;
}

void forgetMember(MemberId member)
{
	orders().slotPositions.erase(member);
}

void forgetOrders()
{
	orders().moves.clear();
	orders().exits.clear();
	orders().slotPositions.clear();
	orders().issuing = false;
}

} // namespace CoH::Squads
