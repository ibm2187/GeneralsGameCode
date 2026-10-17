// CoH Sectors: the money tooltip tells what the viewed player's sectors pay.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Player.h"
#include "Common/UnicodeString.h"
#include "GameClient/GameText.h"

#include "Features/Sectors/Engine/Internal.h"
#include "Features/Sectors/Engine/World.h"

namespace CoH::Sectors
{

void addIncomeToTooltip(const Player *player, UnicodeString &description)
{
	const World &w = world();
	if (!w.hasSectors() || player == nullptr)
		return;

	const PlayerId me = player->getPlayerIndex();
	Int held = 0;
	Int cutOff = 0;
	for (int i = 0; i < w.sectorCount(); ++i)
	{
		if (w.ownersAtRefresh()[i] != me)
			continue;
		++held;
		if (!w.isConnected(me, i))
			++cutOff;
	}

	UnicodeString line;
	line.format(TheGameText->FETCH_OR_SUBSTITUTE("CoH:SectorIncome", L"Sectors: +$%d per minute from %d held, %d cut off").str(),
		w.incomePerMinute(me), held, cutOff);
	if (!description.isEmpty())
		description.concat(L"\n");
	description.concat(line);
}

} // namespace CoH::Sectors
