// CoH Kernel: see Feature.h.

#include "Kernel/Domain/Feature.h"

namespace CoH
{

Feature::Feature(const FeatureInfo &info)
	: m_name(info.name)
	, m_init(info.init)
	, m_reset(info.reset)
{
}

void Feature::init()
{
	if (m_init)
		m_init(*this);
}

void Feature::reset()
{
	if (m_reset)
		m_reset();
}

Feature *FeatureRegistry::add(const FeatureInfo &info)
{
	if (info.name == nullptr || *info.name == '\0' || find(info.name) != nullptr)
		return nullptr;

	m_features.push_back(std::make_unique<Feature>(info));
	return m_features.back().get();
}

Feature *FeatureRegistry::find(std::string_view name) const
{
	for (const std::unique_ptr<Feature> &feature : m_features)
	{
		if (sameFeatureName(feature->name(), name))
			return feature.get();
	}
	return nullptr;
}

FeatureSet FeatureRegistry::known(const FeatureSet &flags) const
{
	FeatureSet result;
	for (const std::string &flag : flags.names())
	{
		if (const Feature *feature = find(flag))
			result.insert(feature->name());
	}
	return result;
}

void FeatureRegistry::activate(const FeatureSet &flags)
{
	for (const std::unique_ptr<Feature> &feature : m_features)
		feature->m_active = flags.contains(feature->name());
}

void FeatureRegistry::deactivateAll()
{
	activate(FeatureSet());
}

FeatureSet FeatureRegistry::active() const
{
	FeatureSet result;
	for (const std::unique_ptr<Feature> &feature : m_features)
	{
		if (feature->isActive())
			result.insert(feature->name());
	}
	return result;
}

void FeatureRegistry::initAll()
{
	for (const std::unique_ptr<Feature> &feature : m_features)
		feature->init();
}

void FeatureRegistry::resetAll()
{
	for (const std::unique_ptr<Feature> &feature : m_features)
		feature->reset();
}

} // namespace CoH
