// CoH GeneralsArsenal: command points next to the money, how they are earned in the money tooltip,
// and on each General's power button what it costs, the rank it needs and why it can't be used now
// (locked by rank, no airfield, cooling down, too expensive).

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Player.h"
#include "Common/SpecialPower.h"
#include "GameClient/ControlBar.h"
#include "GameClient/GameText.h"

#include "Features/GeneralsArsenal/Engine/Config.h"
#include "Features/GeneralsArsenal/Engine/Internal.h"

namespace CoH::GeneralsArsenal
{

namespace
{

void addLine(UnicodeString &description, const UnicodeString &line)
{
	if (!description.isEmpty())
		description.concat(L"\n");
	description.concat(line);
}

UnicodeString stateLine(PowerState state, const PowerConfig &power, const PlayerStanding &standing)
{
	UnicodeString line;
	switch (state)
	{
		case PowerState::LockedByRank:
			line.format(TheGameText->FETCH_OR_SUBSTITUTE("CoH:PowerLockedByRank", L"Locked: needs rank %d").str(), power.rule.minRank);
			break;
		case PowerState::NeedsAirfield:
			line = TheGameText->FETCH_OR_SUBSTITUTE("CoH:PowerNeedsAirfield", L"Needs an Airfield");
			break;
		case PowerState::CoolingDown:
			line = TheGameText->FETCH_OR_SUBSTITUTE("CoH:PowerCoolingDown", L"Cooling down");
			break;
		case PowerState::TooExpensive:
			line.format(TheGameText->FETCH_OR_SUBSTITUTE("CoH:PowerTooExpensive", L"Not enough command points (%d of %d)").str(),
				standing.points, power.rule.cost);
			break;
		default:
			line = TheGameText->FETCH_OR_SUBSTITUTE("CoH:PowerReady", L"Ready");
			break;
	}
	return line;
}

} // namespace

void addPointsToMoneyText(const Player *player, UnicodeString &text)
{
	if (walletOf(player) == nullptr)
		return;
	UnicodeString points;
	points.format(TheGameText->FETCH_OR_SUBSTITUTE("CoH:CommandPointsDisplay", L"   CP %d").str(), pointsOf(player));
	text.concat(points);
}

void addPointsToMoneyTooltip(const Player *player, UnicodeString &description)
{
	if (walletOf(player) == nullptr)
		return;
	UnicodeString line;
	line.format(TheGameText->FETCH_OR_SUBSTITUTE("CoH:CommandPointsTooltip",
		L"Command points (CP): %d. Earned in combat, from the experience your units gain by damaging and destroying "
		L"enemies. General's powers cost CP each time they are used.").str(), pointsOf(player));
	addLine(description, L"");
	addLine(description, line);
}

void addPowerToTooltip(const CommandButton *button, UnicodeString &description)
{
	const SpecialPowerTemplate *power = button ? button->getSpecialPowerTemplate() : nullptr;
	const PowerConfig *settings = powerConfig(power);
	const Player *player = TheControlBar ? TheControlBar->getCurrentlyViewedPlayer() : nullptr;
	if (settings == nullptr || player == nullptr || walletOf(player) == nullptr)
		return;

	const PlayerStanding standing = standingFor(player, *settings, findPowerSource(player, power), power);
	if (settings->description.isNotEmpty())
	{
		addLine(description, L"");
		addLine(description, TheGameText->fetch(settings->description));
	}

	UnicodeString terms;
	terms.format(TheGameText->FETCH_OR_SUBSTITUTE("CoH:PowerTerms", L"Cost: %d CP   Rank %d   Cooldown %d s").str(),
		settings->rule.cost, settings->rule.minRank, (Int)(power->getReloadTime() / LOGICFRAMES_PER_SECOND));
	addLine(description, L"");
	addLine(description, terms);
	if (settings->rule.lossPenaltySeconds > 0.0f)
	{
		UnicodeString loss;
		loss.format(TheGameText->FETCH_OR_SUBSTITUTE("CoH:PowerLossPenalty", L"Each aircraft lost: +%d s cooldown").str(),
			(Int)settings->rule.lossPenaltySeconds);
		addLine(description, loss);
	}
	addLine(description, stateLine(stateOf(settings->rule, standing), *settings, standing));
}

} // namespace CoH::GeneralsArsenal
