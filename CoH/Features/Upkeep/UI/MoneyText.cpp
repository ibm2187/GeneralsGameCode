// CoH Upkeep: population next to the money, gross income, upkeep and net income in the money
// tooltip, and on each unit's build button the population it takes and whether it still fits.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Player.h"
#include "Common/ThingTemplate.h"
#include "GameClient/ControlBar.h"
#include "GameClient/GameText.h"

#include "Features/Sectors/Sectors.h"
#include "Features/Upkeep/Engine/Config.h"
#include "Features/Upkeep/Engine/Internal.h"

namespace CoH::Upkeep
{

namespace
{

void addLine(UnicodeString &description, const UnicodeString &line)
{
	if (!description.isEmpty())
		description.concat(L"\n");
	description.concat(line);
}

} // namespace

void addPopulationToMoneyText(const Player *player, UnicodeString &text)
{
	if (!isUpkeepGame() || player == nullptr)
		return;
	const Standing standing = standingOf(player);
	UnicodeString population;
	if (standing.percent > 0)
		population.format(TheGameText->FETCH_OR_SUBSTITUTE("CoH:PopulationUpkeepDisplay", L"   Pop %d/%d (-%d%%)").str(),
			standing.population, standing.cap, standing.percent);
	else
		population.format(TheGameText->FETCH_OR_SUBSTITUTE("CoH:PopulationDisplay", L"   Pop %d/%d").str(),
			standing.population, standing.cap);
	text.concat(population);
}

void addUpkeepToMoneyTooltip(const Player *player, UnicodeString &description)
{
	if (!isUpkeepGame() || player == nullptr)
		return;
	const Standing standing = standingOf(player);
	const IncomeSplit split = splitIncome(Sectors::incomePerMinute(player), standing.percent);

	UnicodeString income;
	income.format(TheGameText->FETCH_OR_SUBSTITUTE("CoH:UpkeepIncome",
		L"Income per minute: $%d gross, -$%d upkeep (%d%%), $%d net").str(), split.gross, split.upkeep, standing.percent, split.net);
	UnicodeString population;
	population.format(TheGameText->FETCH_OR_SUBSTITUTE("CoH:UpkeepPopulation",
		L"Population %d of %d: units take population by their cost, units being made included. "
		L"Buildings and sectors held raise the cap.").str(), standing.population, standing.cap);

	addLine(description, L"");
	addLine(description, income);
	addLine(description, population);

	const int next = config().steps.nextStepPopulation(standing.population);
	if (next >= 0)
	{
		UnicodeString step;
		step.format(TheGameText->FETCH_OR_SUBSTITUTE("CoH:UpkeepNextStep", L"At population %d upkeep rises to %d%%.").str(),
			next, config().steps.percentAt(next));
		addLine(description, step);
	}
}

void addPopulationToTooltip(const CommandButton *button, UnicodeString &description)
{
	if (!isUpkeepGame() || button == nullptr || button->getCommandType() != GUI_COMMAND_UNIT_BUILD)
		return;
	const int cost = unitPopulation(button->getThingTemplate());
	const Player *player = TheControlBar ? TheControlBar->getCurrentlyViewedPlayer() : nullptr;
	if (cost <= 0 || player == nullptr)
		return;

	const Standing standing = standingOf(player);
	UnicodeString line;
	line.format(TheGameText->FETCH_OR_SUBSTITUTE("CoH:UnitPopulation", L"Population: %d").str(), cost);
	addLine(description, L"");
	addLine(description, line);
	if (!isNeverRefused(button->getThingTemplate()) && !fitsUnderCap(standing.population, cost, standing.cap))
	{
		UnicodeString full;
		full.format(TheGameText->FETCH_OR_SUBSTITUTE("CoH:PopulationCapReached",
			L"Population cap reached (%d of %d): hold more sectors or build more to raise it.").str(), standing.population, standing.cap);
		addLine(description, full);
	}
}

} // namespace CoH::Upkeep
