// CoH Sectors: tests for capture by conquest: progress math, contesting and neutralizing.

#include <gtest/gtest.h>

#include "Features/Sectors/Domain/Capture.h"

using namespace CoH::Sectors;

namespace
{

// Players 0 and 1 are allies; 2 is everyone's enemy.
bool areEnemies(PlayerId a, PlayerId b)
{
	if (a == b)
		return false;
	return a == 2 || b == 2;
}

CaptureRules rules()
{
	CaptureRules r;
	r.captureSeconds = 10.0f;
	r.neutralizeSeconds = 5.0f;
	r.recoverSeconds = 20.0f;
	r.maxUnits = 3;
	r.extraUnitBonus = 0.5f;
	return r;
}

Occupation held(PlayerId side, int units)
{
	return { Occupation::Kind::Held, side, units };
}

} // namespace

TEST(SectorsCapture, NobodyInsideIsEmpty)
{
	EXPECT_EQ(resolveOccupation({}, areEnemies).kind, Occupation::Kind::Empty);
	EXPECT_EQ(resolveOccupation({ { 0, 0 }, { NoPlayer, 4 } }, areEnemies).kind, Occupation::Kind::Empty);
}

TEST(SectorsCapture, EnemiesInsideContest)
{
	const Occupation occupation = resolveOccupation({ { 0, 3 }, { 2, 1 } }, areEnemies);
	EXPECT_EQ(occupation.kind, Occupation::Kind::Contested);
}

TEST(SectorsCapture, AlliesHoldTogetherUnderTheStrongest)
{
	const Occupation occupation = resolveOccupation({ { 0, 1 }, { 1, 2 } }, areEnemies);
	EXPECT_EQ(occupation.kind, Occupation::Kind::Held);
	EXPECT_EQ(occupation.side, 1);
	EXPECT_EQ(occupation.units, 3);

	EXPECT_EQ(resolveOccupation({ { 1, 2 }, { 0, 2 } }, areEnemies).side, 0);	// a tie goes to the lower id
}

TEST(SectorsCapture, SpeedGrowsWithUnitsUpToTheCap)
{
	EXPECT_FLOAT_EQ(unitSpeedFactor(0, rules()), 0.0f);
	EXPECT_FLOAT_EQ(unitSpeedFactor(1, rules()), 1.0f);
	EXPECT_FLOAT_EQ(unitSpeedFactor(2, rules()), 1.5f);
	EXPECT_FLOAT_EQ(unitSpeedFactor(3, rules()), 2.0f);
	EXPECT_FLOAT_EQ(unitSpeedFactor(9, rules()), 2.0f);
}

TEST(SectorsCapture, OneUnitTakesANeutralPointInTheCaptureTime)
{
	PointState state;
	CaptureChange change = advanceCapture(state, held(2, 1), false, rules(), 5.0f, areEnemies);
	EXPECT_FALSE(change.ownerChanged);
	EXPECT_EQ(state.owner, NoPlayer);
	EXPECT_EQ(state.progressSide, 2);
	EXPECT_FLOAT_EQ(state.progress, 0.5f);

	change = advanceCapture(state, held(2, 1), false, rules(), 5.0f, areEnemies);
	EXPECT_TRUE(change.ownerChanged);
	EXPECT_EQ(change.previousOwner, NoPlayer);
	EXPECT_EQ(state.owner, 2);
	EXPECT_FLOAT_EQ(state.progress, 1.0f);
}

TEST(SectorsCapture, MoreUnitsCaptureFaster)
{
	PointState state;
	advanceCapture(state, held(0, 3), false, rules(), 2.5f, areEnemies);
	EXPECT_FLOAT_EQ(state.progress, 0.5f);
}

TEST(SectorsCapture, ContestedPointsDontMove)
{
	PointState state;
	advanceCapture(state, held(0, 1), false, rules(), 3.0f, areEnemies);
	const float before = state.progress;

	const Occupation contested = { Occupation::Kind::Contested, NoPlayer, 0 };
	EXPECT_FALSE(advanceCapture(state, contested, false, rules(), 100.0f, areEnemies).ownerChanged);
	EXPECT_FLOAT_EQ(state.progress, before);
	EXPECT_EQ(state.progressSide, 0);
}

TEST(SectorsCapture, AnEnemyPointIsNeutralizedFirstThenCaptured)
{
	PointState state = { 0, 0, 1.0f };

	CaptureChange change = advanceCapture(state, held(2, 1), false, rules(), 4.0f, areEnemies);
	EXPECT_FALSE(change.ownerChanged);
	EXPECT_EQ(state.owner, 0);
	EXPECT_NEAR(state.progress, 0.2f, 1e-5f);

	change = advanceCapture(state, held(2, 1), false, rules(), 1.0f, areEnemies);
	EXPECT_TRUE(change.ownerChanged);
	EXPECT_EQ(change.previousOwner, 0);
	EXPECT_EQ(state.owner, NoPlayer);
	EXPECT_FLOAT_EQ(state.progress, 0.0f);

	advanceCapture(state, held(2, 1), false, rules(), 10.0f, areEnemies);
	EXPECT_EQ(state.owner, 2);
}

TEST(SectorsCapture, AnEnemysProgressIsUndoneBeforeOwnProgress)
{
	PointState state = { NoPlayer, 2, 0.5f };
	advanceCapture(state, held(0, 1), false, rules(), 2.0f, areEnemies);
	EXPECT_EQ(state.progressSide, 2);
	EXPECT_NEAR(state.progress, 0.3f, 1e-5f);

	advanceCapture(state, held(0, 1), false, rules(), 3.0f, areEnemies);
	EXPECT_EQ(state.progressSide, NoPlayer);
	EXPECT_FLOAT_EQ(state.progress, 0.0f);

	advanceCapture(state, held(0, 1), false, rules(), 1.0f, areEnemies);
	EXPECT_EQ(state.progressSide, 0);
	EXPECT_NEAR(state.progress, 0.1f, 1e-5f);
}

TEST(SectorsCapture, AlliesDontTakeEachOthersPoints)
{
	PointState state = { 0, 0, 0.5f };
	EXPECT_FALSE(advanceCapture(state, held(1, 2), false, rules(), 2.0f, areEnemies).ownerChanged);
	EXPECT_EQ(state.owner, 0);
	EXPECT_GT(state.progress, 0.5f);	// they restore the ally's hold
}

TEST(SectorsCapture, ALockedHomeCantBeTaken)
{
	PointState state = { 0, 0, 1.0f };
	EXPECT_FALSE(advanceCapture(state, held(2, 5), true, rules(), 100.0f, areEnemies).ownerChanged);
	EXPECT_EQ(state.owner, 0);
	EXPECT_FLOAT_EQ(state.progress, 1.0f);
}

TEST(SectorsCapture, EmptyPointsDriftBack)
{
	PointState owned = { 0, 0, 0.5f };
	advanceCapture(owned, {}, false, rules(), 5.0f, areEnemies);
	EXPECT_FLOAT_EQ(owned.progress, 0.75f);

	PointState neutral = { NoPlayer, 2, 0.2f };
	advanceCapture(neutral, {}, false, rules(), 10.0f, areEnemies);
	EXPECT_FLOAT_EQ(neutral.progress, 0.0f);
	EXPECT_EQ(neutral.progressSide, NoPlayer);

	CaptureRules noRecovery = rules();
	noRecovery.recoverSeconds = 0.0f;
	PointState kept = { NoPlayer, 2, 0.2f };
	advanceCapture(kept, {}, false, noRecovery, 10.0f, areEnemies);
	EXPECT_FLOAT_EQ(kept.progress, 0.2f);
}
