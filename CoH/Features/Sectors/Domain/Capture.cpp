// CoH Sectors: see Capture.h.

#include "Capture.h"

#include <algorithm>

namespace CoH::Sectors
{

namespace
{

float perSecond(float seconds)
{
	return seconds > 0.0f ? 1.0f / seconds : 1.0f;
}

void recover(PointState &state, const CaptureRules &rules, float seconds)
{
	if (rules.recoverSeconds <= 0.0f)
		return;

	const float step = seconds / rules.recoverSeconds;
	if (state.owner != NoPlayer)
	{
		state.progress = std::min(1.0f, state.progress + step);
		return;
	}

	state.progress = std::max(0.0f, state.progress - step);
	if (state.progress == 0.0f)
		state.progressSide = NoPlayer;
}

CaptureChange neutralize(PointState &state, float amount)
{
	state.progress -= amount;
	if (state.progress > 0.0f)
		return {};

	const CaptureChange change = { true, state.owner };
	state.owner = NoPlayer;
	state.progressSide = NoPlayer;
	state.progress = 0.0f;
	return change;
}

CaptureChange captureNeutral(PointState &state, PlayerId side, float amount, const AreEnemies &areEnemies)
{
	// Someone else's progress on a neutral point is taken back down before this side builds its own.
	const bool othersProgress = state.progressSide != NoPlayer && state.progressSide != side &&
		areEnemies(state.progressSide, side);
	if (othersProgress)
	{
		state.progress -= amount;
		if (state.progress <= 0.0f)
		{
			state.progress = 0.0f;
			state.progressSide = NoPlayer;
		}
		return {};
	}

	state.progressSide = side;
	state.progress += amount;
	if (state.progress < 1.0f)
		return {};

	state.owner = side;
	state.progress = 1.0f;
	return { true, NoPlayer };
}

} // namespace

Occupation resolveOccupation(const std::vector<Presence> &presences, const AreEnemies &areEnemies)
{
	const auto counts = [](const Presence &presence) { return presence.units > 0 && presence.player != NoPlayer; };

	for (size_t a = 0; a < presences.size(); ++a)
	{
		for (size_t b = a + 1; b < presences.size(); ++b)
		{
			if (counts(presences[a]) && counts(presences[b]) && areEnemies(presences[a].player, presences[b].player))
				return { Occupation::Kind::Contested, NoPlayer, 0 };
		}
	}

	Occupation occupation;
	int sideUnits = 0;
	for (const Presence &presence : presences)
	{
		if (!counts(presence))
			continue;

		occupation.kind = Occupation::Kind::Held;
		occupation.units += presence.units;
		const bool strongest = presence.units > sideUnits ||
			(presence.units == sideUnits && presence.player < occupation.side);
		if (occupation.side == NoPlayer || strongest)
		{
			occupation.side = presence.player;
			sideUnits = presence.units;
		}
	}
	return occupation;
}

float unitSpeedFactor(int units, const CaptureRules &rules)
{
	if (units <= 0)
		return 0.0f;
	const int counted = std::min(units, std::max(1, rules.maxUnits));
	return 1.0f + rules.extraUnitBonus * (float)(counted - 1);
}

CaptureChange advanceCapture(PointState &state, const Occupation &occupation, bool locked, const CaptureRules &rules,
	float seconds, const AreEnemies &areEnemies)
{
	if (occupation.kind == Occupation::Kind::Contested)
		return {};

	if (occupation.kind == Occupation::Kind::Empty)
	{
		recover(state, rules, seconds);
		return {};
	}

	const float speed = unitSpeedFactor(occupation.units, rules) * seconds;
	const PlayerId side = occupation.side;

	if (state.owner == NoPlayer)
		return captureNeutral(state, side, speed * perSecond(rules.captureSeconds), areEnemies);

	if (!areEnemies(side, state.owner))
	{
		state.progress = std::min(1.0f, state.progress + speed * perSecond(rules.captureSeconds));
		return {};
	}

	if (locked)
		return {};
	return neutralize(state, speed * perSecond(rules.neutralizeSeconds));
}

} // namespace CoH::Sectors
