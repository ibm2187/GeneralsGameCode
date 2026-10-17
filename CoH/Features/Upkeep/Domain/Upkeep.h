// CoH Upkeep: the share of income a player's army costs. It grows with population in steps: from
// each step's population on, that step's share of every income is taken.

#pragma once

#include <vector>

namespace CoH::Upkeep
{

struct UpkeepStep
{
	int population = 0;	///< from this population on...
	int percent = 0;	///< ...this share of income goes to upkeep
};

class UpkeepSteps
{
public:
	// Adds a step; steps may come in any order. A second step at the same population replaces the first.
	void add(UpkeepStep step);
	void clear() { m_steps.clear(); }
	bool isEmpty() const { return m_steps.empty(); }
	const std::vector<UpkeepStep> &steps() const { return m_steps; }

	// The share taken at 'population': that of the highest step reached, 0 below every step.
	int percentAt(int population) const;

	// The population of the next step above 'population', or -1 when it is at the last one.
	int nextStepPopulation(int population) const;

private:
	std::vector<UpkeepStep> m_steps;	///< by population, lowest first
};

struct IncomeSplit
{
	int gross = 0;
	int upkeep = 0;
	int net = 0;
};

// How an income of 'gross' splits at 'percent' upkeep. Upkeep is rounded down, so the player keeps
// the change; the share is held between 0 and 100.
IncomeSplit splitIncome(int gross, int percent);

// The AI grows its army to 'population' only while upkeep there stays at or below 'maxPercent', so
// most of its income is left for rebuilding and its economy never stalls.
bool aiMayGrowTo(int population, const UpkeepSteps &steps, int maxPercent);

} // namespace CoH::Upkeep
