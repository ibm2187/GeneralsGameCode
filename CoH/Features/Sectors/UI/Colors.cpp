// CoH Sectors: see Colors.h.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Player.h"
#include "GameClient/GameClient.h"

#include "Features/Sectors/Engine/World.h"
#include "Features/Sectors/UI/Colors.h"

namespace CoH::Sectors
{

Color withAlpha(Color color, UnsignedByte alpha)
{
	UnsignedByte r, g, b, a;
	GameGetColorComponents(color, &r, &g, &b, &a);
	return GameMakeColor(r, g, b, alpha);
}

Color ownerColor(PlayerId owner, UnsignedByte alpha)
{
	const Player *player = findPlayer(owner);
	if (owner == NoPlayer || player == nullptr)
		return GameMakeColor(210, 210, 210, alpha);
	return withAlpha(player->getPlayerColor(), alpha);
}

bool blinkOn()
{
	return (TheGameClient->getFrame() / 8) % 2 == 0;
}

} // namespace CoH::Sectors
