// CoH Accuracy: a puff of dirt where a missed bullet lands. Only an effect: it draws no synced random
// numbers, and it shows only where the local player can see, so it gives nothing away under the shroud.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "GameClient/FXList.h"
#include "GameLogic/PartitionManager.h"

#include "Features/Accuracy/Engine/Config.h"
#include "Features/Accuracy/Engine/Internal.h"

namespace CoH::Accuracy
{

void showMissPuff(const Coord3D &spot)
{
	const AsciiString &fxName = config().missFX;
	if (fxName.isEmpty() || TheFXListStore == nullptr)
		return;

	const Player *localPlayer = ThePlayerList->getLocalPlayer();
	if (localPlayer != nullptr
		&& ThePartitionManager->getShroudStatusForPlayer(localPlayer->getPlayerIndex(), &spot) != CELLSHROUD_CLEAR)
		return;

	FXList::doFXPos(TheFXListStore->findFXList(fxName.str()), &spot);
}

} // namespace CoH::Accuracy
