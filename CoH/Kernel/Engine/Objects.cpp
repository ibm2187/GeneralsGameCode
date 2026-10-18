// CoH Kernel: see Kernel/Objects.h.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Player.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"

#include "Kernel/Objects.h"

#include <utility>

namespace CoH
{

namespace
{

// The whole filter as one partition filter.
class QueryFilter : public PartitionFilter
{
public:
	explicit QueryFilter(const ObjectFilter &filter) : m_filter(filter) { }

	virtual Bool allow(Object *other) override { return matches(*other, m_filter); }
#if defined(RTS_DEBUG)
	virtual const char *debugGetName() override { return "CoH::QueryFilter"; }
#endif

private:
	const ObjectFilter &m_filter;
};

IterOrderType iterOrder(Order order)
{
	return order == Order::NearestFirst ? ITER_SORTED_NEAR_TO_FAR : ITER_FASTEST;
}

} // namespace

bool matches(const Object &object, const ObjectFilter &filter)
{
	if (!filter.includeDead && object.isEffectivelyDead())
		return false;
	if (filter.owner && object.getControllingPlayer() != filter.owner)
		return false;
	if (filter.anyKind.any() && !object.isAnyKindOf(filter.anyKind))
		return false;
	if (object.isAnyKindOf(filter.noKind))
		return false;
	if (filter.enemiesOf && filter.enemiesOf->getRelationship(&object) != ENEMIES)
		return false;
	if (filter.alliesOf && filter.alliesOf->getRelationship(&object) != ALLIES)
		return false;
	return true;
}

// ---- ObjectRange --------------------------------------------------------------------------------

ObjectRange::ObjectRange(ObjectIterator *source) : m_source(source)
{
}

ObjectRange::ObjectRange(ObjectRange &&other) noexcept : m_source(std::exchange(other.m_source, nullptr))
{
}

ObjectRange &ObjectRange::operator=(ObjectRange &&other) noexcept
{
	if (this != &other)
	{
		deleteInstance(m_source);
		m_source = std::exchange(other.m_source, nullptr);
	}
	return *this;
}

ObjectRange::~ObjectRange()
{
	deleteInstance(m_source);
}

ObjectRange::iterator ObjectRange::begin()
{
	return m_source ? iterator(m_source, m_source->first()) : iterator();
}

// ---- Queries ------------------------------------------------------------------------------------

ObjectRange objectsNear(const Coord3D &center, Real radius, const ObjectFilter &filter, Order order)
{
	QueryFilter query(filter);
	PartitionFilter *filters[] = { &query, nullptr };
	return ObjectRange(ThePartitionManager->iterateObjectsInRange(&center, radius, FROM_CENTER_2D, filters,
		iterOrder(order)));
}

ObjectRange objectsNear(const Object &center, Real radius, const ObjectFilter &filter, Order order)
{
	QueryFilter query(filter);
	PartitionFilter *filters[] = { &query, nullptr };
	return ObjectRange(ThePartitionManager->iterateObjectsInRange(&center, radius, FROM_CENTER_2D, filters,
		iterOrder(order)));
}

std::vector<Object *> objectsOf(const Player &player, const ObjectFilter &filter)
{
	struct Search
	{
		const ObjectFilter &filter;
		std::vector<Object *> found;
	} search{ filter, {} };

	player.iterateObjects([](Object *object, void *userData)
	{
		Search &search = *static_cast<Search *>(userData);
		if (matches(*object, search.filter))
			search.found.push_back(object);
	}, &search);
	return std::move(search.found);
}

std::vector<Object *> allObjects(const ObjectFilter &filter)
{
	std::vector<Object *> found;
	for (Object *object = TheGameLogic->getFirstObject(); object; object = object->getNextObject())
	{
		if (matches(*object, filter))
			found.push_back(object);
	}
	return found;
}

Object *findObject(ObjectID id)
{
	return TheGameLogic->findObjectByID(id);
}

Object *findLiving(ObjectID id)
{
	Object *object = findObject(id);
	return object && !object->isEffectivelyDead() ? object : nullptr;
}

} // namespace CoH
