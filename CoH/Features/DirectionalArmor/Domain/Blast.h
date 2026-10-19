// CoH DirectionalArmor: a tank shell's high-explosive blast against soldiers, as pure code. Unit tested
// in Tests/BlastTest.cpp.
//
// Retail tank shells hurt only what is within their primary damage radius (5): one soldier. Like a
// shell in Company of Heroes, a shell with a Blast line also hurts every soldier within the blast's
// radius beyond that: 'atInner' of the blast's damage just outside the primary radius, falling in a
// straight line to 'atOuter' at the blast's edge. Vehicles and buildings aren't caught in it.

#pragma once

#include <optional>

namespace CoH::DirectionalArmor
{

struct BlastFalloff
{
	float atInner = 1.0f;
	float atOuter = 1.0f;
};

struct BlastShell
{
	float damage = 0.0f;	///< before the soldier's armor and InfantryDamage
	float radius = 0.0f;
};

// The blast damage for a soldier 'distance' from where the shell went off (to the edge of its body, as
// the engine measures a blast's reach), or nothing: inside the primary radius the shell's own damage
// hits it, and beyond the blast's radius nothing does.
std::optional<float> blastDamage(const BlastShell &shell, const BlastFalloff &falloff, float distance,
	float primaryRadius);

} // namespace CoH::DirectionalArmor
