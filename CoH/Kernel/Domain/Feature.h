// CoH Kernel: the feature registry. Every slice registers once; the registry turns slices on and
// off per game from a set of flags. Plain C++ with no engine headers, so it is unit tested.

#pragma once

#include "Kernel/Domain/FeatureSet.h"

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace CoH
{

class Feature;

// What a slice tells the Kernel about itself (see COH_FEATURE in Kernel/Kernel.h).
// The name is the slice's flag and the folder its INI pack installs to.
struct FeatureInfo
{
	const char *name;
	void (*init)(Feature &self);	///< once at engine init: subscribe to hooks, add modules and INI blocks
	void (*reset)();				///< after every game: drop per-game state and INI tuning
};

class Feature
{
public:
	explicit Feature(const FeatureInfo &info);

	const std::string &name() const { return m_name; }

	// On for the game being played. Hooks only reach the subscribers of active features.
	bool isActive() const { return m_active; }

	void init();
	void reset();

private:
	friend class FeatureRegistry;

	std::string m_name;
	void (*m_init)(Feature &self);
	void (*m_reset)();
	bool m_active = false;
};

class FeatureRegistry
{
public:
	// Returns nullptr and adds nothing for a name that is empty or already registered.
	Feature *add(const FeatureInfo &info);

	Feature *find(std::string_view name) const;

	// The flags that name a registered feature, spelled as the feature spells its name.
	FeatureSet known(const FeatureSet &flags) const;

	// Turns on exactly the registered features named in the flags.
	void activate(const FeatureSet &flags);
	void deactivateAll();
	FeatureSet active() const;

	void initAll();
	void resetAll();

	const std::vector<std::unique_ptr<Feature>> &features() const { return m_features; }

private:
	std::vector<std::unique_ptr<Feature>> m_features;	///< owned through pointers, so subscribers can keep a Feature's address
};

} // namespace CoH
