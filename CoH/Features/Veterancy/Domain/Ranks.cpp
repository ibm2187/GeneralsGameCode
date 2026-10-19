// CoH Veterancy: see Ranks.h.

#include "Features/Veterancy/Domain/Ranks.h"

#include <algorithm>

namespace CoH::Veterancy
{

namespace
{

constexpr char lower(char c)
{
	return (c >= 'A' && c <= 'Z') ? char(c - 'A' + 'a') : c;
}

bool sameName(std::string_view a, std::string_view b)
{
	return std::ranges::equal(a, b, [](char x, char y) { return lower(x) == lower(y); });
}

void applyLines(const Rules &rules, std::string_view name, RankTable &table)
{
	for (const ObjectLine &line : rules.objects)
	{
		if (sameName(line.name, name))
			line.change.applyTo(table[static_cast<std::size_t>(line.rank)]);
	}
}

// The percent a multiplier adds or takes, rounded half away from zero: 1.2 is 20, 0.85 is -15.
int percentOf(float multiplier)
{
	const float percent = (multiplier - 1.0f) * 100.0f;
	return static_cast<int>(percent + (percent < 0.0f ? -0.5f : 0.5f));
}

} // namespace

void BonusChange::applyTo(RankBonus &bonus) const
{
	if (accuracy)
		bonus.accuracy = *accuracy;
	if (damageTaken)
		bonus.damageTaken = *damageTaken;
	if (suppressionResistance)
		bonus.suppressionResistance = *suppressionResistance;
	if (speed)
		bonus.speed = *speed;
	if (sight)
		bonus.sight = *sight;
	if (reload)
		bonus.reload = *reload;
	if (upgrade)
		bonus.upgrade = *upgrade;
}

std::string_view baseName(std::string_view name)
{
	const std::size_t underscore = name.find('_');
	if (underscore == std::string_view::npos || underscore + 1 >= name.size())
		return name;
	return name.substr(underscore + 1);
}

RankTable tableFor(const Rules &rules, std::string_view name, UnitClass unitClass)
{
	RankTable table {};
	if (unitClass != UnitClass::Other)
		table = rules.classes[static_cast<std::size_t>(unitClass)];
	const std::string_view plain = baseName(name);
	if (plain.size() != name.size())
		applyLines(rules, plain, table);
	applyLines(rules, name, table);
	return table;
}

std::vector<StatChange> changesOf(const RankBonus &bonus)
{
	std::vector<StatChange> changes;
	const auto add = [&](Stat stat, int percent) {
		if (percent != 0)
			changes.push_back({ stat, percent });
	};
	add(Stat::Accuracy, percentOf(bonus.accuracy));
	add(Stat::DamageTaken, percentOf(bonus.damageTaken));
	add(Stat::SuppressionResistance, percentOf(1.0f + bonus.suppressionResistance));
	add(Stat::Speed, percentOf(bonus.speed));
	add(Stat::Sight, percentOf(bonus.sight));
	add(Stat::Reload, percentOf(bonus.reload));
	return changes;
}

} // namespace CoH::Veterancy
