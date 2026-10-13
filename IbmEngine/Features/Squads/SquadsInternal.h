// IBM Engine: helpers shared inside the Squads slice only.

#pragma once

#include "Features/FeatureFlags/MemoryPoolGlue.h"

class IbmSquadUpdate;
class IbmSquadUpdateModuleData;
class Object;
class SpawnBehaviorModuleData;
class ThingTemplate;
enum WeaponSlotType CPP_11(: Int);

namespace IbmEngine
{

// The squad module of a leader, or nullptr if obj is not a squad leader.
IbmSquadUpdate *getSquad(const Object *obj);

inline Bool isSquadLeader(const Object *obj) { return getSquad(obj) != nullptr; }

// The leader of a squad member, or nullptr if obj is not a squad member.
Object *getSquadLeaderOf(const Object *obj);

// The SpawnBehavior data of a squad leader's template, which lists its soldiers, or nullptr.
const SpawnBehaviorModuleData *findSquadSpawnData(const ThingTemplate *leaderTemplate);

// TRUE if obj's own button set has a button that fires the weapon in this slot (Burton's knife,
// Kell's snipe, the bombers' Detonate). Many soldiers carry a weapon there; only heroes fire it by hand.
Bool hasFireWeaponButton(const Object *obj, WeaponSlotType slot);

// Brings back the squad's first lost member if it is near one of its ReinforceAt buildings.
// Returns TRUE if one came back.
Bool reinforceSquad(Object *leader, const IbmSquadUpdateModuleData *data);

// Calls func for every living member of a squad, in spawn order.
template <typename Func>
void forEachSquadMember(const Object *leader, Func func);

} // namespace IbmEngine

// ------------------------------------------------------------------------------------------------
#include "GameLogic/GameLogic.h"
#include "GameLogic/Module/SpawnBehavior.h"
#include "GameLogic/Object.h"

template <typename Func>
void IbmEngine::forEachSquadMember(const Object *leader, Func func)
{
	SpawnBehaviorInterface *spawn = leader->getSpawnBehaviorInterface();
	if (spawn == nullptr)
		return;

	// Copy first: func may kill members or give orders that change the list.
	const std::list<ObjectID> ids = spawn->getSlaveIDs();
	for (std::list<ObjectID>::const_iterator it = ids.begin(); it != ids.end(); ++it)
	{
		Object *member = TheGameLogic->findObjectByID(*it);
		if (member != nullptr && !member->isEffectivelyDead())
			func(member);
	}
}
