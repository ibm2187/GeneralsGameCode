// CoH DirectionalArmor: sparks and a clang where a shot bounced off a vehicle (DeflectFX, an FXList the
// pack defines), and "Deflected" floating over it for anti-tank weapons. Only an effect: it draws no
// synced random numbers, and it shows only where the local player can see.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "GameClient/Color.h"
#include "GameClient/FXList.h"
#include "GameClient/GameText.h"
#include "GameClient/InGameUI.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"

#include "Features/DirectionalArmor/Engine/Config.h"
#include "Features/DirectionalArmor/Engine/Internal.h"

#include <cmath>

namespace CoH::DirectionalArmor
{

namespace
{

bool localPlayerSees(const Coord3D &spot)
{
	const Player *localPlayer = ThePlayerList != nullptr ? ThePlayerList->getLocalPlayer() : nullptr;
	return localPlayer == nullptr
		|| ThePartitionManager->getShroudStatusForPlayer(localPlayer->getPlayerIndex(), &spot) == CELLSHROUD_CLEAR;
}

// On the hull, on the side the shot came from, about half way up.
Coord3D hullPoint(const Object &victim, Flat towardsShot)
{
	const GeometryInfo &geometry = victim.getGeometryInfo();
	Coord3D spot = *victim.getPosition();
	const float length = std::sqrt(towardsShot.x * towardsShot.x + towardsShot.y * towardsShot.y);
	if (length > 0.0f)
	{
		const float reach = geometry.getMinorRadius() > 0.0f ? geometry.getMinorRadius() : geometry.getMajorRadius();
		spot.x += towardsShot.x / length * reach;
		spot.y += towardsShot.y / length * reach;
	}
	spot.z += geometry.getMaxHeightAbovePosition() * 0.5f;
	return spot;
}

} // namespace

void showDeflection(const Object &victim, Flat towardsShot, float penetration)
{
	const Config &c = config();
	const Coord3D spot = hullPoint(victim, towardsShot);
	if (!localPlayerSees(spot))
		return;

	if (!c.deflectFX.isEmpty() && TheFXListStore != nullptr)
		FXList::doFXPos(TheFXListStore->findFXList(c.deflectFX.str()), &spot);

	if (c.deflectTextPenetration > 0.0f && penetration >= c.deflectTextPenetration && TheInGameUI != nullptr
		&& TheGameText != nullptr)
	{
		Coord3D above = spot;
		above.z += 10.0f;
		TheInGameUI->addFloatingText(TheGameText->fetch("GUI:CoHDeflected"), &above, GameMakeColor(255, 220, 90, 255));
	}
}

} // namespace CoH::DirectionalArmor
