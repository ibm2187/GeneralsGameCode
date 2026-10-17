// CoH Upkeep: see Upkeep.h.

#include "Features/Upkeep/Domain/Upkeep.h"

#include <algorithm>

namespace CoH::Upkeep
{

void UpkeepSteps::add(UpkeepStep step)
{
	for (UpkeepStep &existing : m_steps)
	{
		if (existing.population == step.population)
		{
			existing = step;
			return;
		}
	}
	const auto higher = std::find_if(m_steps.begin(), m_steps.end(),
		[&](const UpkeepStep &existing) { return existing.population > step.population; });
	m_steps.insert(higher, step);
}

int UpkeepSteps::percentAt(int population) const
{
	int percent = 0;
	for (const UpkeepStep &step : m_steps)
	{
		if (population < step.population)
			break;
		percent = step.percent;
	}
	return percent;
}

int UpkeepSteps::nextStepPopulation(int population) const
{
	for (const UpkeepStep &step : m_steps)
	{
		if (step.population > population)
			return step.population;
	}
	return -1;
}

IncomeSplit splitIncome(int gross, int percent)
{
	IncomeSplit split;
	split.gross = std::max(gross, 0);
	split.upkeep = split.gross * std::clamp(percent, 0, 100) / 100;
	split.net = split.gross - split.upkeep;
	return split;
}

bool aiMayGrowTo(int population, const UpkeepSteps &steps, int maxPercent)
{
	return steps.percentAt(population) <= maxPercent;
}

} // namespace CoH::Upkeep
