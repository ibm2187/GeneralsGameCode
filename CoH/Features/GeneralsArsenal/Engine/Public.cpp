// CoH GeneralsArsenal: GeneralsArsenal.h, for other slices.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Player.h"
#include "Common/PlayerList.h"

#include "Features/GeneralsArsenal/Engine/Config.h"
#include "Features/GeneralsArsenal/Engine/Internal.h"
#include "Features/GeneralsArsenal/GeneralsArsenal.h"

namespace CoH::GeneralsArsenal
{

// A ledger exists only while the flag is on: it is made at game start by this slice.
bool isActive()
{
	return ThePlayerList != nullptr && walletOf(ThePlayerList->getNeutralPlayer()) != nullptr;
}

int commandPoints(const Player *player)
{
	return pointsOf(player);
}

int powerCost(const SpecialPowerTemplate *power)
{
	const PowerConfig *settings = powerConfig(power);
	return settings ? settings->rule.cost : 0;
}

} // namespace CoH::GeneralsArsenal
