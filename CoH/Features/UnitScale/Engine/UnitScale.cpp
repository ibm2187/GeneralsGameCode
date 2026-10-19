// CoH UnitScale: the feature. Everything it changes is in its INI pack, which the Kernel loads only
// while the flag is on; the code only answers isActive and runs the showcase test option
// (Showcase.cpp). See ../README.md.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Kernel/Kernel.h"

#include "Features/UnitScale/Engine/Internal.h"
#include "Features/UnitScale/UnitScale.h"

namespace CoH::UnitScale
{

namespace
{

const Feature *&theFeature()
{
	static const Feature *feature = nullptr;
	return feature;
}

void init(Feature &self)
{
	theFeature() = &self;
	initShowcase(self);
}

void reset()
{
}

} // namespace

bool isActive()
{
	return theFeature() != nullptr && theFeature()->isActive();
}

} // namespace CoH::UnitScale

COH_FEATURE(UnitScale, CoH::UnitScale::init, CoH::UnitScale::reset)
