// CoH Kernel: a set of feature flag names, and the text formats flags travel in.
// Plain C++ with no engine headers, so it is unit tested.

#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace CoH
{

// Feature names compare without case, so "sectors" in Features.ini turns on "Sectors".
// The set keeps its names sorted, so every machine lists them in the same order.
class FeatureSet
{
public:
	FeatureSet() = default;

	// "Sectors,Accuracy" (spaces allowed). An empty list or "none" is no features.
	static FeatureSet fromList(std::string_view list);

	// Features.ini: one "Name = yes" or "Name = no" per line; ';' and '#' start comments.
	static FeatureSet fromSettingsFile(std::string_view text);

	// "Sectors,Accuracy"; empty when the set is empty.
	std::string toList() const;

	void insert(std::string_view name);
	bool contains(std::string_view name) const;
	bool empty() const { return m_names.empty(); }
	const std::vector<std::string> &names() const { return m_names; }

	bool operator==(const FeatureSet &other) const;
	bool operator!=(const FeatureSet &other) const { return !(*this == other); }

private:
	std::vector<std::string> m_names;
};

bool sameFeatureName(std::string_view a, std::string_view b);

} // namespace CoH
