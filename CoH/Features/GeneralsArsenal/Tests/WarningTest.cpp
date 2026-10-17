// CoH GeneralsArsenal: tests for the super weapon warning timer.

#include <gtest/gtest.h>

#include "Features/GeneralsArsenal/Domain/Warning.h"

using namespace CoH::GeneralsArsenal;

TEST(GeneralsArsenalWarning, ItFiresAfterTheDelay)
{
	const WarningTimer timer(100, 600);	// 20 s at 30 fps
	EXPECT_EQ(timer.fireFrame(), 700u);
	EXPECT_FALSE(timer.isDue(699));
	EXPECT_TRUE(timer.isDue(700));
	EXPECT_TRUE(timer.isDue(5000));
}

TEST(GeneralsArsenalWarning, TheCountRoundsUp)
{
	const WarningTimer timer(0, 600);
	EXPECT_EQ(timer.secondsLeft(0, 30), 20);
	EXPECT_EQ(timer.secondsLeft(1, 30), 20);
	EXPECT_EQ(timer.secondsLeft(570, 30), 1);
	EXPECT_EQ(timer.secondsLeft(599, 30), 1);
	EXPECT_EQ(timer.secondsLeft(600, 30), 0);
}

TEST(GeneralsArsenalWarning, ProgressGoesFromZeroToOne)
{
	const WarningTimer timer(100, 200);
	EXPECT_FLOAT_EQ(timer.progress(50), 0.0f);
	EXPECT_FLOAT_EQ(timer.progress(200), 0.5f);
	EXPECT_FLOAT_EQ(timer.progress(300), 1.0f);
	EXPECT_FLOAT_EQ(WarningTimer(100, 0).progress(100), 1.0f);
}

TEST(GeneralsArsenalWarning, TheCountdownReadsMinutesAndSeconds)
{
	EXPECT_EQ(formatCountdown(65), "1:05");
	EXPECT_EQ(formatCountdown(9), "0:09");
	EXPECT_EQ(formatCountdown(0), "0:00");
	EXPECT_EQ(formatCountdown(-3), "0:00");
}
