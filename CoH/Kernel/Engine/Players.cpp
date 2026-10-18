// CoH Kernel: see Kernel/Players.h.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "GameLogic/Object.h"

#include "Kernel/Players.h"

namespace CoH
{

bool isEnemy(const Player &viewer, const Player &other)
{
	return viewer.getRelationship(other.getDefaultTeam()) == ENEMIES;
}

bool isAlly(const Player &viewer, const Player &other)
{
	return viewer.getRelationship(other.getDefaultTeam()) == ALLIES;
}

bool isEnemy(const Object &viewer, const Object &other)
{
	return viewer.getRelationship(&other) == ENEMIES;
}

bool isAlly(const Object &viewer, const Object &other)
{
	return viewer.getRelationship(&other) == ALLIES;
}

} // namespace CoH
