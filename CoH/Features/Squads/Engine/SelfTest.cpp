// CoH Squads: an in-game self-test, only with -cohSquadsSelfTest (and -cohLog to see it).
//
// Play a skirmish as USA. A few seconds in, two Ranger squads, a Humvee and a garrisonable pub appear
// near the player's command center, and the test plays the player: it sends the same messages the
// player's clicks send (select, deselect, move, enter, exit, evacuate) and checks what game logic
// made of them. Each check writes "Squads test: PASS ..." or "FAIL ..." to the CoH log, then a
// summary. Its objects come from the test, not from messages, so its replay doesn't play back.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/MessageStream.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "GameClient/InGameUI.h"
#include "GameClient/View.h"
#include "GameLogic/AI.h"
#include "GameLogic/AIPathfind.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Module/ContainModule.h"
#include "GameLogic/Object.h"
#include "GameLogic/TerrainLogic.h"

#include "Kernel/Kernel.h"
#include "Kernel/Log.h"

#include "Features/Squads/Engine/Internal.h"
#include "Features/Squads/Squads.h"
#include "Features/Upkeep/Upkeep.h"

#include <algorithm>
#include <cmath>
#include <initializer_list>
#include <vector>

namespace CoH::Squads
{

namespace
{

const UnsignedInt kFirstFrame = 90;	///< the test starts 3 seconds into the game

struct TestState
{
	int stage = 0;
	UnsignedInt nextFrame = 0;
	int passed = 0;
	int failed = 0;
	Player *player = nullptr;
	int squadA = 0;
	int squadB = 0;
	ObjectID humvee = INVALID_ID;
	ObjectID pub = INVALID_ID;
	Coord3D goal;
	int populationBefore = 0;
	int populationAdded = 0;
};

TestState &test()
{
	static TestState theState;
	return theState;
}

void check(bool ok, const char *what)
{
	(ok ? test().passed : test().failed)++;
	CoH::log("Squads test: %s %s", ok ? "PASS" : "FAIL", what);
}

void waitFrames(UnsignedInt frames)
{
	test().nextFrame = TheGameLogic->getFrame() + frames;
	++test().stage;
}

Object *objectById(ObjectID id)
{
	return TheGameLogic->findObjectByID(id);
}

Coord3D onGround(Real x, Real y)
{
	Coord3D position;
	position.set(x, y, TheTerrainLogic->getGroundHeight(x, y));
	return position;
}

Object *spawn(const char *name, Team *team, const Coord3D &position)
{
	const ThingTemplate *kind = TheThingFactory->findTemplate(name);
	if (kind == nullptr || team == nullptr)
		return nullptr;
	Object *object = TheThingFactory->newObject(kind, team);
	object->setPosition(&position);
	TheAI->pathfinder()->addObjectToPathfindMap(object);
	return object;
}

int spawnSquad(const Coord3D &at)
{
	Squad &squad = book().create("AmericaInfantryRanger", 4);
	for (int i = 0; i < 4; ++i)
	{
		Object *ranger = spawn("AmericaInfantryRanger", test().player->getDefaultTeam(), onGround(at.x + i * 15.0f, at.y));
		if (ranger != nullptr)
			addMember(squad.id(), ranger);
	}
	return squad.id();
}

void findCommandCenter(Object *object, void *userData)
{
	if (object->isKindOf(KINDOF_COMMANDCENTER))
		*(Object **)userData = object;
}

// ---- What the player's clicks send ----

void sendSelect(const std::vector<Object *> &objects, bool newGroup)
{
	// What the client does on a click before it sends the message: select what was clicked on screen.
	if (newGroup)
		TheInGameUI->deselectAllDrawables();
	for (Object *object : objects)
		if (object->getDrawable() != nullptr)
			TheInGameUI->selectDrawable(object->getDrawable());
	GameMessage *message = TheMessageStream->appendMessage(GameMessage::MSG_CREATE_SELECTED_GROUP);
	message->appendBooleanArgument(newGroup);
	for (Object *object : objects)
		message->appendObjectIDArgument(object->getID());
}

void sendDeselect(Object *object)
{
	if (object->getDrawable() != nullptr)
		TheInGameUI->deselectDrawable(object->getDrawable());
	GameMessage *message = TheMessageStream->appendMessage(GameMessage::MSG_REMOVE_FROM_SELECTED_GROUP);
	message->appendObjectIDArgument(object->getID());
}

void sendMove(const Coord3D &goal)
{
	TheMessageStream->appendMessage(GameMessage::MSG_DO_MOVETO)->appendLocationArgument(goal);
}

void sendEnter(Object *container)
{
	GameMessage *message = TheMessageStream->appendMessage(GameMessage::MSG_ENTER);
	message->appendObjectIDArgument(INVALID_ID);
	message->appendObjectIDArgument(container->getID());
}

void sendExit(Object *rider)
{
	TheMessageStream->appendMessage(GameMessage::MSG_EXIT)->appendObjectIDArgument(rider->getID());
}

// ---- What game logic made of them ----

std::vector<ObjectID> selectedIds()
{
	std::vector<ObjectID> ids;
#if RETAIL_COMPATIBLE_AIGROUP
	AIGroup *group = TheAI->createGroup();
	test().player->getCurrentSelectionAsAIGroup(group);
	ids = group->getAllIDs();
	TheAI->destroyGroup(group);
#else
	AIGroupPtr group = TheAI->createGroup();
	test().player->getCurrentSelectionAsAIGroup(group.Peek());
	ids = group->getAllIDs();
	group->removeAll();
#endif
	std::sort(ids.begin(), ids.end());
	return ids;
}

std::vector<ObjectID> idsOf(std::initializer_list<int> squads)
{
	std::vector<ObjectID> ids;
	for (int squad : squads)
		for (Object *member : members(squad))
			ids.push_back(member->getID());
	std::sort(ids.begin(), ids.end());
	return ids;
}

bool isSelectionExactly(std::initializer_list<int> squads)
{
	const std::vector<ObjectID> selected = selectedIds();
	const std::vector<ObjectID> expected = idsOf(squads);
	if (selected != expected)
		CoH::log("Squads test: selection has %d objects, expected %d", (int)selected.size(), (int)expected.size());
	return selected == expected;
}

bool allInside(int squad, const Object *container)
{
	const std::vector<Object *> riders = members(squad);
	int inside = 0;
	for (const Object *rider : riders)
	{
		if (container != nullptr && rider->getContainedBy() == container)
		{
			++inside;
			continue;
		}
		const AIUpdateInterface *ai = rider->getAIUpdateInterface();
		const Coord3D *at = rider->getPosition();
		CoH::log("Squads test: member %d outside, %.0f from the container, AI state %d%s", (int)rider->getID(),
			container ? (Real)std::hypot(at->x - container->getPosition()->x, at->y - container->getPosition()->y) : -1.0f,
			ai ? (int)ai->getCurrentStateID() : -1, ai && ai->isIdle() ? ", idle" : "");
	}
	CoH::log("Squads test: %d of %d members inside", inside, (int)riders.size());
	return !riders.empty() && inside == (int)riders.size();
}

bool noneInside(int squad)
{
	for (const Object *rider : members(squad))
		if (rider->getContainedBy() != nullptr)
			return false;
	return !members(squad).empty();
}

bool isInFormationAt(int squad, const Coord3D &goal)
{
	const std::vector<Object *> squadMembers = members(squad);
	Real nearest = 1.0e9f;
	bool ok = squadMembers.size() == 4;
	for (size_t a = 0; a < squadMembers.size(); ++a)
	{
		const Coord3D *position = squadMembers[a]->getPosition();
		Coord3D slot;
		const bool hasSlot = formationSlot(squadMembers[a], &slot);
		const Real toSlot = hasSlot ? std::hypot(position->x - slot.x, position->y - slot.y) : -1.0f;
		const Real toGoal = std::hypot(position->x - goal.x, position->y - goal.y);
		CoH::log("Squads test: member %d at (%.0f, %.0f), %.0f from its slot, %.0f from the goal",
			(int)a, position->x, position->y, toSlot, toGoal);
		ok = ok && hasSlot && toSlot < 30.0f && toGoal < 70.0f;
		for (size_t b = a + 1; b < squadMembers.size(); ++b)
			nearest = std::min(nearest, (Real)std::hypot(position->x - squadMembers[b]->getPosition()->x,
				position->y - squadMembers[b]->getPosition()->y));
	}
	CoH::log("Squads test: nearest two members %.0f apart", nearest);
	return ok && nearest >= 12.0f;
}

// ---- The stages ----

void setUp()
{
	TestState &state = test();
	state.player = ThePlayerList->getLocalPlayer();
	Object *commandCenter = nullptr;
	if (state.player != nullptr)
		state.player->iterateObjects(findCommandCenter, &commandCenter);
	if (commandCenter == nullptr)
	{
		CoH::log("Squads test: no command center for the local player; play a skirmish as USA");
		state.stage = -1;
		return;
	}

	// Toward the middle of the map, clear of the base.
	Region3D extent;
	TheTerrainLogic->getExtent(&extent);
	const Coord3D *base = commandCenter->getPosition();
	Real dx = (extent.lo.x + extent.hi.x) * 0.5f - base->x;
	Real dy = (extent.lo.y + extent.hi.y) * 0.5f - base->y;
	const Real length = std::max((Real)1.0f, (Real)std::hypot(dx, dy));
	dx /= length;
	dy /= length;
	const Coord3D start = onGround(base->x + dx * 220.0f, base->y + dy * 220.0f);

	state.populationBefore = Upkeep::population(state.player);
	state.squadA = spawnSquad(start);
	state.squadB = spawnSquad(onGround(start.x - dy * 60.0f, start.y + dx * 60.0f));
	state.populationAdded = Upkeep::population(state.player) - state.populationBefore;
	Object *humvee = spawn("AmericaVehicleHumvee", state.player->getDefaultTeam(),
		onGround(start.x + dy * 90.0f, start.y - dx * 90.0f));
	Object *pub = spawn("GermanPub01", ThePlayerList->getNeutralPlayer()->getDefaultTeam(),
		onGround(base->x + dx * 650.0f, base->y + dy * 650.0f));
	state.humvee = humvee ? humvee->getID() : INVALID_ID;
	state.pub = pub ? pub->getID() : INVALID_ID;
	state.goal = onGround(start.x + dx * 300.0f, start.y + dy * 300.0f);

	const ThingTemplate *ranger = TheThingFactory->findTemplate("AmericaInfantryRanger");
	CoH::log("Squads test: set up squads %d and %d, Humvee %d, pub %d; a Ranger order costs %d and takes %d frames",
		state.squadA, state.squadB, (int)state.humvee, (int)state.pub, ranger->calcCostToBuild(state.player),
		ranger->calcTimeToBuild(state.player));
	check(ranger->calcCostToBuild(state.player) == 810, "a Ranger squad costs 810");
	check(members(state.squadA).size() == 4 && members(state.squadB).size() == 4, "both squads have 4 members");
	waitFrames(30);
}

void runStage()
{
	TestState &state = test();
	const std::vector<Object *> a = members(state.squadA);
	const std::vector<Object *> b = members(state.squadB);
	Object *humvee = objectById(state.humvee);
	Object *pub = objectById(state.pub);

	switch (state.stage)
	{
		case 1:
		{
			const int population = state.populationAdded;
			CoH::log("Squads test: the two squads added %d population", population);
			check(!Upkeep::isActive() || population == 2 * Upkeep::populationCost(TheThingFactory->findTemplate("AmericaInfantryRanger")),
				"each squad takes the population of one order");
			check(countsAsUnit(a[0]) && !countsAsUnit(a[1]), "only the leader counts as a unit");
			sendSelect({ a[2] }, true);
			waitFrames(30);
			break;
		}
		case 2:
			check(isSelectionExactly({ state.squadA }), "clicking one member selects its squad");
			sendSelect({ a[0], a[1], b[3] }, true);	// a box over parts of two squads
			waitFrames(30);
			break;
		case 3:
			check(isSelectionExactly({ state.squadA, state.squadB }), "a box over two squads selects both, each member once");
			sendDeselect(a[1]);
			waitFrames(30);
			break;
		case 4:
			check(isSelectionExactly({ state.squadB }), "deselecting one member deselects its squad");
			sendSelect({ a[3] }, true);
			sendMove(state.goal);
			waitFrames(450);
			break;
		case 5:
			check(isInFormationAt(state.squadA, state.goal), "a moved squad stands in its formation slots at the goal");
			if (humvee != nullptr)
				sendEnter(humvee);
			waitFrames(450);
			break;
		case 6:
			check(allInside(state.squadA, humvee), "the squad rides in the Humvee");
			check(humvee != nullptr && humvee->getContain()->isValidContainerFor(b[0], TRUE),
				"a squad in a Humvee takes one slot: another squad still fits");
			if (humvee != nullptr && !a.empty())
			{
				sendSelect({ humvee }, true);
				sendExit(a[1]);	// the passenger button of one member
			}
			waitFrames(300);
			break;
		case 7:
			check(noneInside(state.squadA), "one member getting out takes the squad out");
			sendSelect({ a[0] }, true);
			if (pub != nullptr)
				sendEnter(pub);
			waitFrames(900);
			break;
		case 8:
			check(allInside(state.squadA, pub), "the squad garrisons the pub");
			check(pub != nullptr && pub->getContain()->isValidContainerFor(b[0], TRUE),
				"a squad in a garrison takes one place: another squad still fits");
			if (pub != nullptr)
			{
				sendSelect({ pub }, true);
				TheMessageStream->appendMessage(GameMessage::MSG_EVACUATE);
			}
			waitFrames(300);
			break;
		case 9:
		{
			check(noneInside(state.squadA), "evacuating the pub lets the squad out");
			Object *oldLeader = leader(state.squadA);
			if (oldLeader != nullptr)
				oldLeader->kill();
			waitFrames(150);
			break;
		}
		case 10:
		{
			const std::vector<Object *> left = members(state.squadA);
			check(left.size() == 3 && missingMembers(state.squadA) == 1, "a lost member leaves an empty slot");
			check(!left.empty() && countsAsUnit(left[0]), "the next member leads");
			check(emptySlots(state.squadA).size() == 1 && emptySlots(state.squadA)[0] == 0 &&
				unitOfSlot(state.squadA, 0) == TheThingFactory->findTemplate("AmericaInfantryRanger"),
				"the empty slot is known for Reinforce");
			CoH::log("Squads test: done, %d passed, %d failed", state.passed, state.failed);
			state.stage = -1;
			break;
		}
		default:
			state.stage = -1;
			break;
	}
}

} // namespace

void runSelfTest()
{
	static const bool enabled = CoH::hasCommandLineOption("-cohSquadsSelfTest");
	if (!enabled || test().stage < 0)
		return;
	const UnsignedInt frame = TheGameLogic->getFrame();
	if (test().stage == 0)
	{
		if (frame >= kFirstFrame)
			setUp();
		return;
	}
	// Keep the camera on squad A, so a windowed test shows it.
	Object *squadLeader = leader(test().squadA);
	if (squadLeader != nullptr && TheTacticalView != nullptr && frame % 15 == 0)
		TheTacticalView->lookAt(squadLeader->getPosition());
	if (frame < test().nextFrame)
		return;
	if (members(test().squadA).empty())
	{
		check(false, "squad A is still there");
		test().stage = -1;
		return;
	}
	runStage();
}

void resetSelfTest()
{
	test() = TestState();
}

} // namespace CoH::Squads
