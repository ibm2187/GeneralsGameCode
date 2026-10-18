// CoH Kernel: economy stats for measuring the game's pace in unattended test games. With
// "-cohStats [seconds]" and -cohLog on the command line, every 'seconds' of game time (default 60)
// CoH.log gets one line per side:
//
//     CoH stats: 300s player 2 FactionChina income 4210/min (total 19800) built 41 lost 12 army 23 worth 9850 cash 1200
//
// income is what the side was paid since the last line (after upkeep), per minute; built and lost are
// the score screen's unit counts; army is the side's living units (infantry, vehicles, aircraft; not
// structures, drones, dozers or harvesters) and worth their retail build cost. It only reads, so it
// works the same with every flag off, and never changes the game.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Money.h"
#include "Common/Player.h"
#include "Common/PlayerTemplate.h"
#include "Common/ThingTemplate.h"
#include "GameLogic/Object.h"

#include "Kernel/Domain/Arguments.h"
#include "Kernel/Engine/Internal.h"
#include "Kernel/Log.h"
#include "Kernel/Objects.h"
#include "Kernel/Players.h"
#include "Kernel/Time.h"

#include <array>
#include <charconv>

namespace CoH::Stats
{

namespace
{

struct Income
{
	unsigned int total = 0;
	unsigned int sinceLastLine = 0;
};

std::array<Income, MAX_PLAYER_COUNT> incomes;

// The seconds between lines; 0 when not asked.
unsigned int intervalSeconds()
{
	static const unsigned int seconds = []
	{
		const std::optional<std::string> value = findOption(Kernel::processArguments(), "-cohStats");
		if (!value || !isLogging())
			return 0u;
		unsigned int parsed = 60;
		std::from_chars(value->data(), value->data() + value->size(), parsed);
		return parsed > 0 ? parsed : 60u;
	}();
	return seconds;
}

void logSide(Player &player, unsigned int seconds)
{
	const ObjectFilter army = {
		.anyKind = kinds(KINDOF_INFANTRY, KINDOF_VEHICLE, KINDOF_AIRCRAFT),
		.noKind = kinds(KINDOF_STRUCTURE, KINDOF_DRONE, KINDOF_DOZER, KINDOF_HARVESTER),
	};
	int armyCount = 0;
	int armyWorth = 0;
	for (const Object *unit : objectsOf(player, army))
	{
		++armyCount;
		armyWorth += unit->getTemplate()->friend_getBuildCost();
	}

	Income &income = incomes[player.getPlayerIndex()];
	ScoreKeeper *score = player.getScoreKeeper();
	const AsciiString side = player.getPlayerTemplate() ? player.getPlayerTemplate()->getName() : AsciiString("none");
	log("CoH stats: %us player %d %s income %u/min (total %u) built %d lost %d army %d worth %d cash %u",
		seconds, player.getPlayerIndex(), side.str(), income.sinceLastLine * 60 / intervalSeconds(),
		income.total, score->getTotalUnitsBuilt(), score->getTotalUnitsLost(), armyCount, armyWorth,
		player.getMoney()->countMoney());
	income.sinceLastLine = 0;
}

} // namespace

void onIncome(const Player &player, unsigned int amount)
{
	if (intervalSeconds() == 0)
		return;
	Income &income = incomes[player.getPlayerIndex()];
	income.total += amount;
	income.sinceLastLine += amount;
}

void logIfDue()
{
	const unsigned int seconds = intervalSeconds();
	if (seconds == 0 || now() == 0 || !everySeconds(Real(seconds)))
		return;
	for (Player *player : sides())
		logSide(*player, now() / LOGICFRAMES_PER_SECOND);
}

void reset()
{
	incomes = {};
}

} // namespace CoH::Stats
