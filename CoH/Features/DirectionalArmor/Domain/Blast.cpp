// CoH DirectionalArmor: see Blast.h.

#include "Features/DirectionalArmor/Domain/Blast.h"

#include <algorithm>

namespace CoH::DirectionalArmor
{

std::optional<float> blastDamage(const BlastShell &shell, const BlastFalloff &falloff, float distance,
	float primaryRadius)
{
	if (shell.damage <= 0.0f || distance <= primaryRadius || distance > shell.radius)
		return std::nullopt;
	const float along = std::clamp((distance - primaryRadius) / (shell.radius - primaryRadius), 0.0f, 1.0f);
	return shell.damage * (falloff.atInner + (falloff.atOuter - falloff.atInner) * along);
}

} // namespace CoH::DirectionalArmor
