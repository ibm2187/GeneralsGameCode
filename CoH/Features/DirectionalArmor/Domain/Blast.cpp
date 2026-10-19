// CoH DirectionalArmor: see Blast.h.

#include "Features/DirectionalArmor/Domain/Blast.h"

#include <algorithm>

namespace CoH::DirectionalArmor
{

float blastScale(const BlastFalloff &falloff, float distance, float primaryRadius, float blastRadius)
{
	if (distance <= primaryRadius || blastRadius <= primaryRadius)
		return 1.0f;
	const float along = std::clamp((distance - primaryRadius) / (blastRadius - primaryRadius), 0.0f, 1.0f);
	return falloff.atInner + (falloff.atOuter - falloff.atInner) * along;
}

} // namespace CoH::DirectionalArmor
