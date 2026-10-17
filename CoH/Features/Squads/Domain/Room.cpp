// CoH Squads: see Room.h.

#include "Features/Squads/Domain/Room.h"

namespace CoH::Squads
{

namespace
{

bool isSquadIn(SquadId squad, const std::vector<Rider> &riders, size_t before)
{
	for (size_t i = 0; i < before && i < riders.size(); ++i)
	{
		if (riders[i].squad == squad)
			return true;
	}
	return false;
}

} // namespace

int roomUsed(const std::vector<Rider> &inside)
{
	int used = 0;
	for (size_t i = 0; i < inside.size(); ++i)
	{
		const Rider &rider = inside[i];
		if (rider.squad != 0 && isSquadIn(rider.squad, inside, i))
			continue;
		used += rider.room;
	}
	return used;
}

int roomNeeded(const Rider &rider, const std::vector<Rider> &inside)
{
	if (rider.squad != 0 && isSquadIn(rider.squad, inside, inside.size()))
		return 0;
	return rider.room;
}

} // namespace CoH::Squads
