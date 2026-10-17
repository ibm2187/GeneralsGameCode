// CoH Sectors: the colors sectors are drawn in: each owner's player color, a pale grey for neutral.

#pragma once

#include "GameClient/Color.h"

#include "Features/Sectors/Domain/Types.h"

namespace CoH::Sectors
{

Color ownerColor(PlayerId owner, UnsignedByte alpha);
Color withAlpha(Color color, UnsignedByte alpha);

// Visual blinking for contested points, from the client's frame count (never the logic's).
bool blinkOn();

} // namespace CoH::Sectors
