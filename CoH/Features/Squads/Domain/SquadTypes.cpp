// CoH Squads: see SquadTypes.h.

#include "Features/Squads/Domain/SquadTypes.h"

namespace CoH::Squads
{

int SquadType::memberCount() const
{
	int count = size;
	for (const Escort &escort : escorts)
		count += escort.count;
	return count;
}

std::string SquadType::unitOfSlot(int slot) const
{
	if (slot < 0)
		return std::string();
	if (slot < size)
		return unit;
	slot -= size;
	for (const Escort &escort : escorts)
	{
		if (slot < escort.count)
			return escort.unit;
		slot -= escort.count;
	}
	return std::string();
}

void SquadTypes::add(const SquadType &type)
{
	for (SquadType &existing : m_types)
	{
		if (existing.unit == type.unit)
		{
			existing = type;
			return;
		}
	}
	m_types.push_back(type);
}

const SquadType *SquadTypes::findExact(const std::string &unit) const
{
	for (const SquadType &type : m_types)
	{
		if (type.unit == unit)
			return &type;
	}
	return nullptr;
}

const SquadType *SquadTypes::find(const std::string &unit) const
{
	// Strip one prefix at a time, so "GC_Chem_GLAInfantryRebel" finds "Chem_GLAInfantryRebel" first.
	std::string name = unit;
	while (!name.empty())
	{
		if (const SquadType *type = findExact(name))
			return type;
		const std::string::size_type underscore = name.find('_');
		if (underscore == std::string::npos)
			return nullptr;
		name = name.substr(underscore + 1);
	}
	return nullptr;
}

int squadAmount(int orderAmount, int orderQuantity, int size, int escortAmount, int percent)
{
	if (orderQuantity < 1)
		orderQuantity = 1;
	// (orderAmount * size / orderQuantity + escortAmount) * percent / 100, in whole numbers.
	const long long numerator = ((long long)orderAmount * size + (long long)escortAmount * orderQuantity) * percent;
	const long long denominator = (long long)orderQuantity * 100;
	if (numerator <= 0)
		return 0;
	return (int)((numerator + denominator / 2) / denominator);
}

} // namespace CoH::Squads
