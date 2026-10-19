// CoH UnitScale: what other slices may ask about scaled units. The only header of this slice others
// include.
//
// Vehicles are drawn bigger and infantry smaller, so a tank is several soldiers long, as in CoH. The
// whole slice is its INI pack (Data/make_pack.py writes it from the retail INI): each unit's Scale,
// and its Geometry to match, so shots, spacing and selection follow the model. While the UnitScale
// flag is off nothing is scaled.

#pragma once

namespace CoH::UnitScale
{

// The game being played draws units at CoH proportions. Their geometry (bounding radius, height) is
// already scaled in their templates, so slices that read GeometryInfo see the new sizes either way.
bool isActive();

} // namespace CoH::UnitScale
