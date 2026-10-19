// CoH DirectionalArmor: an in-game self-test, only with -cohArmorSelfTest (and -cohLog to see it).
//
// Three seconds into a skirmish it puts a Crusader of the local player beside its base and a tight
// group of five enemy Rangers in front of it, has the tank fire one shell at the middle Ranger, and
// just after that shell goes off logs every Ranger's health (later shells don't count). It passes
// when the blast hurt at least two of the Rangers beside the one it was fired at: tanks kill more than
// one soldier at a time.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "GameClient/View.h"
#include "GameLogic/AI.h"
#include "GameLogic/AIPathfind.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Module/BodyModule.h"
#include "GameLogic/Object.h"
#include "GameLogic/TerrainLogic.h"
#include "GameLogic/Weapon.h"

#include "Kernel/Kernel.h"
#include "Kernel/Log.h"
#include "Kernel/Objects.h"
#include "Kernel/Players.h"
#include "Kernel/Time.h"

#include "Features/DirectionalArmor/Engine/Internal.h"

#include <array>
#include <vector>

namespace CoH::DirectionalArmor
{

namespace
{

constexpr UnsignedInt StartFrame = 3 * FramesPerSecond;
constexpr UnsignedInt GiveUpAfterFrames = 10 * FramesPerSecond;
constexpr UnsignedInt SettleFrames = 2;	///< the blast's damage is dealt in the frame it goes off
constexpr Real TankToSquad = 100.0f;
constexpr std::array<Flat, 5> Formation = { { { 0, 0 }, { 0, 9 }, { 0, -9 }, { 8, 5 }, { 8, -5 } } };	///< the target first

struct TestState
{
	int stage = 0;			///< 0 waiting, 1 fired, 2 the shell went off, -1 done
	UnsignedInt checkFrame = 0;
	int blastsBefore = 0;
	ObjectID tank = INVALID_ID;
	std::vector<ObjectID> rangers;
	std::vector<Real> healthBefore;
};

TestState &test()
{
	static TestState theTest;
	return theTest;
}

Coord3D onGround(Real x, Real y)
{
	Coord3D position;
	position.set(x, y, TheTerrainLogic->getGroundHeight(x, y));
	return position;
}

Object *spawn(const char *name, Team *team, const Coord3D &position, Real angle)
{
	const ThingTemplate *kind = TheThingFactory->findTemplate(name);
	if (kind == nullptr || team == nullptr)
		return nullptr;
	Object *object = TheThingFactory->newObject(kind, team);
	object->setPosition(&position);
	object->setOrientation(angle);
	TheAI->pathfinder()->addObjectToPathfindMap(object);
	return object;
}

Real healthOf(ObjectID id)
{
	const Object *object = findObject(id);
	const BodyModuleInterface *body = object != nullptr ? object->getBodyModule() : nullptr;
	return body != nullptr && !object->isEffectivelyDead() ? body->getHealth() : 0.0f;
}

Player *enemyOf(const Player &local)
{
	for (Player *side : activeSides())
	{
		if (side != &local && isEnemy(local, *side))
			return side;
	}
	return nullptr;
}

const Object *commandCenterOf(const Player &player)
{
	for (const Object *object : objectsOf(player, { .anyKind = kinds(KINDOF_COMMANDCENTER) }))
		return object;
	return nullptr;
}

void setUp()
{
	TestState &t = test();
	Player *local = ThePlayerList->getLocalPlayer();
	Player *enemy = local != nullptr ? enemyOf(*local) : nullptr;
	const Object *base = local != nullptr ? commandCenterOf(*local) : nullptr;
	if (enemy == nullptr || base == nullptr)
	{
		log("DirectionalArmor test: no enemy or no base; skipped");
		t.stage = -1;
		return;
	}

	const Coord3D &home = *base->getPosition();
	const Coord3D tankAt = onGround(home.x + 150.0f, home.y + 150.0f);
	Object *tank = spawn("AmericaTankCrusader", local->getDefaultTeam(), tankAt, 0.0f);
	for (const Flat &offset : Formation)
	{
		Object *ranger = spawn("AmericaInfantryRanger", enemy->getDefaultTeam(),
			onGround(tankAt.x + TankToSquad + offset.x, tankAt.y + offset.y), PI);
		if (ranger != nullptr)
			t.rangers.push_back(ranger->getID());
	}
	if (tank == nullptr || tank->getAI() == nullptr || t.rangers.size() != Formation.size())
	{
		log("DirectionalArmor test: couldn't place the tank and the Rangers; skipped");
		t.stage = -1;
		return;
	}

	for (const ObjectID id : t.rangers)
		t.healthBefore.push_back(healthOf(id));
	t.tank = tank->getID();
	tank->getAI()->aiAttackObject(findObject(t.rangers.front()), 1, CMD_FROM_AI);
	if (TheTacticalView != nullptr)
		TheTacticalView->lookAt(&tankAt);
	t.blastsBefore = blastsSoFar();
	t.checkFrame = now() + GiveUpAfterFrames;
	t.stage = 1;
	log("DirectionalArmor test: a Crusader fires one shell at the middle of five Rangers");
}

void check()
{
	TestState &t = test();
	int bystandersHurt = 0;
	for (size_t i = 0; i < t.rangers.size(); ++i)
	{
		const Real health = healthOf(t.rangers[i]);
		const bool hurt = health < t.healthBefore[i];
		if (i > 0 && hurt)
			++bystandersHurt;
		log("DirectionalArmor test:   Ranger %d (%s) %.0f -> %.0f%s", (int)i, i == 0 ? "the target" : "beside it",
			t.healthBefore[i], health, health <= 0.0f ? " (dead)" : "");
	}
	const bool passed = bystandersHurt >= 2;
	log("DirectionalArmor test: %s, the blast hurt %d of the 4 Rangers beside the target", passed ? "PASSED" : "FAILED",
		bystandersHurt);
	t.stage = -1;
}

} // namespace

void runSelfTest()
{
	static const bool enabled = hasCommandLineOption("-cohArmorSelfTest");
	TestState &t = test();
	if (!enabled || t.stage < 0)
		return;
	if (t.stage == 0 && now() >= StartFrame)
		setUp();
	else if (t.stage == 1 && blastsSoFar() > t.blastsBefore)
	{
		t.stage = 2;
		t.checkFrame = now() + SettleFrames;
	}
	else if (t.stage >= 1 && now() >= t.checkFrame)
		check();
}

void resetSelfTest()
{
	test() = TestState();
}

} // namespace CoH::DirectionalArmor
