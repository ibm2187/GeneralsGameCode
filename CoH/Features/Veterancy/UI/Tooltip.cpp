// CoH Veterancy: a unit's build button lists what each rank gives it, one line a rank:
//     Veteran: +15% accuracy, -10% damage taken, 20% suppression resistance, +15% fire rate

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/ThingTemplate.h"
#include "Common/Upgrade.h"
#include "GameClient/ControlBar.h"
#include "GameClient/GameText.h"

#include "Features/Veterancy/Engine/Config.h"
#include "Features/Veterancy/Engine/Internal.h"

namespace CoH::Veterancy
{

namespace
{

void addLine(UnicodeString &description, const UnicodeString &line)
{
	if (!description.isEmpty())
		description.concat(L"\n");
	description.concat(line);
}

UnicodeString text(const char *label, const wchar_t *fallback)
{
	return TheGameText->FETCH_OR_SUBSTITUTE(label, fallback);
}

UnicodeString rankName(Rank rank)
{
	switch (rank)
	{
		case Rank::Veteran: return text("CoH:VeterancyVeteran", L"Veteran");
		case Rank::Elite: return text("CoH:VeterancyElite", L"Elite");
		case Rank::Heroic: return text("CoH:VeterancyHeroic", L"Heroic");
		default: return text("CoH:VeterancyRegular", L"Regular");
	}
}

UnicodeString changeText(const StatChange &change)
{
	UnicodeString format;
	switch (change.stat)
	{
		case Stat::Accuracy: format = text("CoH:VeterancyAccuracy", L"%+d%% accuracy"); break;
		case Stat::DamageTaken: format = text("CoH:VeterancyDamageTaken", L"%+d%% damage taken"); break;
		case Stat::SuppressionResistance: format = text("CoH:VeterancySuppression", L"%d%% suppression resistance"); break;
		case Stat::Speed: format = text("CoH:VeterancySpeed", L"%+d%% speed"); break;
		case Stat::Sight: format = text("CoH:VeterancySight", L"%+d%% sight"); break;
		case Stat::Reload: format = text("CoH:VeterancyReload", L"%+d%% fire rate"); break;
	}
	UnicodeString line;
	line.format(format.str(), change.percent);
	return line;
}

UnicodeString upgradeText(const std::string &name)
{
	const UpgradeTemplate *upgrade = TheUpgradeCenter->findUpgrade(name.c_str());
	if (upgrade == nullptr)
		return UnicodeString::TheEmptyString;
	UnicodeString line;
	line.format(text("CoH:VeterancyUpgrade", L"gains %s").str(),
		TheGameText->fetch(upgrade->getDisplayNameLabel()).str());
	return line;
}

UnicodeString rankLine(Rank rank, const RankBonus &bonus)
{
	UnicodeString line = rankName(rank);
	line.concat(L": ");
	bool first = true;
	const auto add = [&](const UnicodeString &part) {
		if (part.isEmpty())
			return;
		if (!first)
			line.concat(L", ");
		line.concat(part);
		first = false;
	};
	for (const StatChange &change : changesOf(bonus))
		add(changeText(change));
	if (!bonus.upgrade.empty())
		add(upgradeText(bonus.upgrade));
	return first ? UnicodeString::TheEmptyString : line;
}

} // namespace

void addRanksToTooltip(const CommandButton *button, UnicodeString &description)
{
	if (button == nullptr || button->getCommandType() != GUI_COMMAND_UNIT_BUILD)
		return;
	const ThingTemplate *unit = button->getThingTemplate();
	if (unit == nullptr || !unit->isTrainable())
		return;

	const RankTable &table = tableOf(unit);
	bool headed = false;
	for (std::size_t i = 1; i < RankCount; ++i)
	{
		const UnicodeString line = rankLine(static_cast<Rank>(i), table[i]);
		if (line.isEmpty())
			continue;
		if (!headed)
		{
			addLine(description, L"");
			headed = true;
		}
		addLine(description, line);
	}
}

} // namespace CoH::Veterancy
