// CoH Accuracy: the answers of the public header, Accuracy.h.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Kernel/Domain/Feature.h"

#include "Features/Accuracy/Accuracy.h"
#include "Features/Accuracy/Engine/Internal.h"

namespace CoH::Accuracy
{

bool isActive()
{
	const Feature *feature = accuracyFeature();
	return feature != nullptr && feature->isActive();
}

void addHitModifier(const Feature &owner, const char *name, int order, ModifierFunction modifier)
{
	if (name != nullptr)
		modifiers().add(owner, name, order, modifier);
}

} // namespace CoH::Accuracy
