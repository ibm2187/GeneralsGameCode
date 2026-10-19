// CoH UnitScale: what the slice's Engine files share. Not for other slices; they include UnitScale.h.

#pragma once

namespace CoH
{
class Feature;
}

namespace CoH::UnitScale
{

// The -cohUnitScaleShowcase test option (Showcase.cpp).
void initShowcase(Feature &self);

} // namespace CoH::UnitScale
