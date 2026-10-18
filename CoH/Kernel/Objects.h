// CoH Kernel: finding objects, for C++20 feature code. Queries take an ObjectFilter, written with
// designated initializers, and return ranges for a range-for or std::views:
//
//     for (Object *enemy : CoH::objectsNear(*unit, 150.0f, { .enemiesOf = unit }))
//         suppress(*enemy);
//
//     const auto tanks = CoH::objectsOf(*player, { .anyKind = CoH::kinds(KINDOF_VEHICLE) });
//
//     if (Object *jet = CoH::findLiving(m_jetID))
//         ...
//
// Every order here is the same on every machine, so game logic may depend on it.

#pragma once

#include "Common/GameType.h"
#include "Common/KindOf.h"
#include "GameLogic/ObjectIter.h"

#include <concepts>
#include <cstddef>
#include <iterator>
#include <vector>

class Object;
class Player;

namespace CoH
{

// Which objects a query keeps. The empty filter keeps every living object.
struct ObjectFilter
{
	const Object *enemiesOf = nullptr;	///< only objects this one sees as enemies
	const Object *alliesOf = nullptr;	///< only objects this one sees as allies
	const Player *owner = nullptr;		///< only objects this player controls
	KindOfMaskType anyKind;				///< only objects of at least one of these kinds; empty keeps every kind
	KindOfMaskType noKind;				///< no objects of any of these kinds
	bool includeDead = false;			///< also objects that are dying (isEffectivelyDead)
};

// The kinds as a mask: CoH::kinds(KINDOF_INFANTRY, KINDOF_VEHICLE).
template <std::same_as<KindOfType>... Kinds>
KindOfMaskType kinds(Kinds... kindList)
{
	KindOfMaskType mask;
	(mask.set(kindList), ...);
	return mask;
}

bool matches(const Object &object, const ObjectFilter &filter);

// The objects of a partition search. It owns the engine's iterator and deletes it when it goes out of
// scope, so there is no deleteInstance to forget. Each begin() starts over; objects that die during
// the loop are still visited, so check isEffectivelyDead after changing the world.
class ObjectRange
{
public:
	class iterator
	{
	public:
		using value_type = Object *;
		using difference_type = std::ptrdiff_t;

		iterator() = default;
		iterator(ObjectIterator *source, Object *current) : m_source(source), m_current(current) { }

		Object *operator*() const { return m_current; }
		iterator &operator++() { m_current = m_source->next(); return *this; }
		void operator++(int) { ++*this; }
		bool operator==(std::default_sentinel_t) const { return m_current == nullptr; }

	private:
		ObjectIterator *m_source = nullptr;
		Object *m_current = nullptr;
	};

	explicit ObjectRange(ObjectIterator *source);	///< takes ownership
	ObjectRange(ObjectRange &&other) noexcept;
	ObjectRange &operator=(ObjectRange &&other) noexcept;
	ObjectRange(const ObjectRange &) = delete;
	ObjectRange &operator=(const ObjectRange &) = delete;
	~ObjectRange();

	iterator begin();
	std::default_sentinel_t end() const { return {}; }

private:
	ObjectIterator *m_source;
};

static_assert(std::input_iterator<ObjectRange::iterator>);

enum class Order
{
	Any,			///< the engine's order (the same on every machine, but not sorted)
	NearestFirst
};

// Objects within 'radius' of a point or of an object's center, measured flat (2D). The object itself
// is never in it.
ObjectRange objectsNear(const Coord3D &center, Real radius, const ObjectFilter &filter = {}, Order order = Order::Any);
ObjectRange objectsNear(const Object &center, Real radius, const ObjectFilter &filter = {}, Order order = Order::Any);

// Every object a player has, in its teams' order (Player::iterateObjects).
std::vector<Object *> objectsOf(const Player &player, const ObjectFilter &filter = {});

// Every object in the game, of any player, in the game logic's order. A whole-map search: prefer
// objectsNear or objectsOf where they fit.
std::vector<Object *> allObjects(const ObjectFilter &filter = {});

// The object with this ID, or nullptr when there is none (INVALID_ID, or destroyed).
Object *findObject(ObjectID id);

// The same, but also nullptr while the object is dying: for orders, targets and members.
Object *findLiving(ObjectID id);

} // namespace CoH
