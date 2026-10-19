// CoH Veterancy: experience from damage dealt, and squads sharing one pool of it.
//
// Damage to an enemy earns its share of the victim's kill value; whole points go to the engine's
// ExperienceTracker, so the engine still decides the rank and its own bonuses. A squad's members
// end every frame with the same experience: what each earned since the last share is added to the
// pool, and a new member (made or reinforced) is raised to it. Both are saved with the game.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Player.h"
#include "Common/Snapshot.h"
#include "Common/Xfer.h"
#include "GameLogic/Damage.h"
#include "GameLogic/ExperienceTracker.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Module/BodyModule.h"
#include "GameLogic/Object.h"

#include "Kernel/Objects.h"
#include "Kernel/Time.h"

#include "Features/Squads/Squads.h"
#include "Features/Veterancy/Domain/Experience.h"
#include "Features/Veterancy/Engine/Config.h"
#include "Features/Veterancy/Engine/Internal.h"

#include <map>
#include <set>
#include <vector>

namespace CoH::Veterancy
{

namespace
{

// Fractions of a point each unit has earned from damage, by ID.
std::map<ObjectID, Real> &carried()
{
	static std::map<ObjectID, Real> carry;
	return carry;
}

// Each squad member's experience after the last share, by ID.
std::map<ObjectID, Int> &seen()
{
	static std::map<ObjectID, Int> members;
	return members;
}

// Squads whose members earned or changed rank this frame; shared at the end of it.
std::set<int> &changedSquads()
{
	static std::set<int> squads;
	return squads;
}

bool &sharing()
{
	static bool now = false;
	return now;
}

bool squadsShare()
{
	return config().squadsShare && Squads::isActive();
}

void noteSquadOf(const Object *unit)
{
	if (sharing() || !squadsShare())
		return;
	if (const int squad = Squads::squadOf(unit))
		changedSquads().insert(squad);
}

// As Object::scoreTheKill decides what earns experience: an enemy of a playable side that isn't
// under construction or hidden from the GUI, not its own player's.
bool earnsFrom(const Object &source, const Object &victim)
{
	const Player *victimPlayer = victim.getControllingPlayer();
	if (victimPlayer == nullptr || !victimPlayer->isPlayableSide())
		return false;
	if (victim.isKindOf(KINDOF_IGNORED_IN_GUI) || victim.testStatus(OBJECT_STATUS_UNDER_CONSTRUCTION))
		return false;
	if (source.getControllingPlayer() == victimPlayer || source.getRelationship(&victim) != ENEMIES)
		return false;
	const ExperienceTracker *tracker = source.getExperienceTracker();
	return tracker != nullptr && tracker->isAcceptingExperiencePoints();
}

void shareSquad(int squad)
{
	const std::vector<Object *> members = Squads::members(squad);
	if (members.empty())
		return;

	std::vector<MemberExperience> experience;
	experience.reserve(members.size());
	for (const Object *member : members)
	{
		const auto known = seen().find(member->getID());
		experience.push_back({
			.known = known != seen().end(),
			.seen = known != seen().end() ? known->second : 0,
			.now = member->getExperienceTracker()->getCurrentExperience(),
		});
	}
	const Int pool = squadPool(experience);

	sharing() = true;
	for (Object *member : members)
	{
		ExperienceTracker *tracker = member->getExperienceTracker();
		if (tracker->getCurrentExperience() != pool && tracker->isTrainable())
			tracker->setExperienceAndLevel(pool, FALSE);
		seen()[member->getID()] = pool;
	}
	sharing() = false;
}

// Squads this frame didn't mention: new members join at their squad's experience within a second.
void shareEverySquad()
{
	std::set<int> squads;
	for (const Object *object : allObjects())
	{
		if (const int squad = Squads::squadOf(object))
			squads.insert(squad);
	}
	for (int squad : squads)
		shareSquad(squad);
}

} // namespace

void onDamageDealt(Object *victim, const DamageInfo *damage)
{
	if (victim == nullptr || damage == nullptr || config().damageExperience <= 0.0f)
		return;
	Object *source = TheGameLogic->findObjectByID(damage->in.m_sourceID);
	if (source == nullptr || source == victim || !earnsFrom(*source, *victim))
		return;
	const BodyModuleInterface *body = victim->getBodyModule();
	if (body == nullptr)
		return;

	const Real gain = experienceFromDamage(config().damageExperience, damage->out.m_actualDamageClipped,
		body->getMaxHealth(), victim->getExperienceTracker()->getExperienceValue(source));
	const int points = wholePoints(carried()[source->getID()], gain);
	if (points <= 0)
		return;
	source->getExperienceTracker()->addExperiencePoints(points);
	noteSquadOf(source);
}

// The engine adds the kill's experience right after this; the squad shares it at the frame's end.
void onKillScored(Object *killer, const Object * /*victim*/)
{
	noteSquadOf(killer);
}

void onUnitRankChanged(const Object *unit)
{
	noteSquadOf(unit);
}

void shareSquadExperience()
{
	if (!squadsShare())
		return;
	if (everySeconds(1.0f))
	{
		shareEverySquad();
		changedSquads().clear();
		return;
	}
	const std::set<int> squads = std::move(changedSquads());
	changedSquads().clear();
	for (int squad : squads)
		shareSquad(squad);
}

void forgetUnit(const Object *unit)
{
	carried().erase(unit->getID());
	seen().erase(unit->getID());
}

void forgetExperience()
{
	carried().clear();
	seen().clear();
	changedSquads().clear();
	sharing() = false;
}

// ---- Saved with the game -----------------------------------------------------------------------

namespace
{

template <class Value, class XferValue>
void xferById(Xfer *xfer, std::map<ObjectID, Value> &values, XferValue xferValue)
{
	UnsignedInt count = (UnsignedInt)values.size();
	xfer->xferUnsignedInt(&count);
	if (xfer->getXferMode() == XFER_SAVE)
	{
		for (auto &[id, value] : values)
		{
			ObjectID saved = id;
			xfer->xferObjectID(&saved);
			xferValue(xfer, &value);
		}
		return;
	}
	values.clear();
	for (UnsignedInt i = 0; i < count; ++i)
	{
		ObjectID id = INVALID_ID;
		Value value {};
		xfer->xferObjectID(&id);
		xferValue(xfer, &value);
		if (TheGameLogic->findObjectByID(id) != nullptr)
			values[id] = value;
	}
}

class SaveData : public Snapshot
{
protected:
	virtual void crc(Xfer * /*xfer*/) override { }

	// Version 1: each unit's carried fraction of a point, each squad member's experience at the last share.
	virtual void xfer(Xfer *xfer) override
	{
		XferVersion currentVersion = 1;
		XferVersion version = currentVersion;
		xfer->xferVersion(&version, currentVersion);

		xferById(xfer, carried(), [](Xfer *x, Real *value) { x->xferReal(value); });
		xferById(xfer, seen(), [](Xfer *x, Int *value) { x->xferInt(value); });
	}

	virtual void loadPostProcess() override { }
};

} // namespace

Snapshot *saveData()
{
	static SaveData data;
	return &data;
}

} // namespace CoH::Veterancy
