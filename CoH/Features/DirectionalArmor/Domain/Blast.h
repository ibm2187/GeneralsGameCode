// CoH DirectionalArmor: how hard a vehicle's shell blast hits a soldier caught in it, as pure code.
// Unit tested in Tests/BlastTest.cpp.
//
// A shell hits whatever is within its primary radius with its full damage and the rest of its blast
// with its splash (the weapon's SecondaryDamage). Against soldiers the splash is scaled by how near the
// shell they were, as in Company of Heroes: 'atInner' just outside the primary radius, falling in a
// straight line to 'atOuter' at the blast's edge.

#pragma once

namespace CoH::DirectionalArmor
{

struct BlastFalloff
{
	float atInner = 1.0f;
	float atOuter = 1.0f;
};

// The scale for a soldier 'distance' from where the shell went off (to the edge of its body, as the
// engine measures blast reach). 1 inside the primary radius (a direct hit) and for a weapon with no
// blast beyond it.
float blastScale(const BlastFalloff &falloff, float distance, float primaryRadius, float blastRadius);

} // namespace CoH::DirectionalArmor
