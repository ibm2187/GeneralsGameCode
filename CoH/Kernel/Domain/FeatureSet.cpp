// CoH Kernel: see FeatureSet.h.

#include "Kernel/Domain/FeatureSet.h"
#include "Kernel/Domain/Text.h"

#include <algorithm>

namespace CoH
{

namespace
{

bool isYes(std::string_view value)
{
	return Text::equalsNoCase(value, "yes") || Text::equalsNoCase(value, "true") || value == "1";
}

std::string_view withoutComment(std::string_view line)
{
	const size_t comment = line.find_first_of(";#");
	return comment == std::string_view::npos ? line : line.substr(0, comment);
}

} // namespace

bool sameFeatureName(std::string_view a, std::string_view b)
{
	return Text::equalsNoCase(a, b);
}

FeatureSet FeatureSet::fromList(std::string_view list)
{
	FeatureSet set;
	if (Text::equalsNoCase(Text::trim(list), "none"))
		return set;

	for (std::string_view name : Text::split(list, ','))
		set.insert(name);
	return set;
}

FeatureSet FeatureSet::fromSettingsFile(std::string_view text)
{
	FeatureSet set;
	for (std::string_view line : Text::split(text, '\n'))
	{
		const std::string_view setting = withoutComment(line);
		const size_t equals = setting.find('=');
		if (equals == std::string_view::npos)
			continue;

		if (isYes(Text::trim(setting.substr(equals + 1))))
			set.insert(setting.substr(0, equals));
	}
	return set;
}

std::string FeatureSet::toList() const
{
	std::string list;
	for (const std::string &name : m_names)
	{
		if (!list.empty())
			list += ',';
		list += name;
	}
	return list;
}

void FeatureSet::insert(std::string_view name)
{
	const std::string_view trimmed = Text::trim(name);
	if (trimmed.empty() || contains(trimmed))
		return;

	const auto position = std::find_if(m_names.begin(), m_names.end(),
		[trimmed](const std::string &existing) { return Text::lessNoCase(trimmed, existing); });
	m_names.insert(position, std::string(trimmed));
}

bool FeatureSet::contains(std::string_view name) const
{
	return std::any_of(m_names.begin(), m_names.end(),
		[name](const std::string &existing) { return sameFeatureName(existing, name); });
}

bool FeatureSet::operator==(const FeatureSet &other) const
{
	return std::equal(m_names.begin(), m_names.end(), other.m_names.begin(), other.m_names.end(),
		[](const std::string &a, const std::string &b) { return sameFeatureName(a, b); });
}

} // namespace CoH
