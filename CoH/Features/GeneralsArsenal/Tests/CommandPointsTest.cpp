// CoH GeneralsArsenal: tests for earning and spending command points.

#include <gtest/gtest.h>

#include "Features/GeneralsArsenal/Domain/CommandPoints.h"

using namespace CoH::GeneralsArsenal;

TEST(GeneralsArsenalPoints, AKillEarnsAShareOfTheVictimsExperience)
{
	EarnRates rates;
	rates.perKillExperience = 0.1f;
	EXPECT_FLOAT_EQ(earnedForKill(150, rates), 15.0f);
	EXPECT_FLOAT_EQ(earnedForKill(0, rates), 0.0f);
	EXPECT_FLOAT_EQ(earnedForKill(-20, rates), 0.0f);
}

TEST(GeneralsArsenalPoints, DamageEarnsByTheShareOfHealthTaken)
{
	EarnRates rates;
	rates.perDamageExperience = 0.2f;
	EXPECT_FLOAT_EQ(earnedForDamage(50.0f, 200.0f, 100, rates), 5.0f);	// a quarter of a 100 xp unit
	EXPECT_FLOAT_EQ(earnedForDamage(500.0f, 200.0f, 100, rates), 20.0f);	// overkill counts as the full health
	EXPECT_FLOAT_EQ(earnedForDamage(0.0f, 200.0f, 100, rates), 0.0f);
	EXPECT_FLOAT_EQ(earnedForDamage(50.0f, 0.0f, 100, rates), 0.0f);
}

TEST(GeneralsArsenalPoints, FractionsAddUpToWholePoints)
{
	Wallet wallet;
	for (int i = 0; i < 10; ++i)
		wallet.earn(0.25f, 1000.0f);
	EXPECT_EQ(wallet.whole(), 2);
	EXPECT_FLOAT_EQ(wallet.points(), 2.5f);
}

TEST(GeneralsArsenalPoints, PointsStopAtTheCap)
{
	Wallet wallet;
	wallet.earn(80.0f, 100.0f);
	wallet.earn(80.0f, 100.0f);
	EXPECT_FLOAT_EQ(wallet.points(), 100.0f);
}

TEST(GeneralsArsenalPoints, APowerIsPaidInFullOrNotAtAll)
{
	Wallet wallet;
	wallet.earn(59.9f, 1000.0f);
	EXPECT_FALSE(wallet.canAfford(60));
	EXPECT_FALSE(wallet.spend(60));
	EXPECT_FLOAT_EQ(wallet.points(), 59.9f);

	wallet.earn(0.1f, 1000.0f);
	EXPECT_TRUE(wallet.spend(60));
	EXPECT_EQ(wallet.whole(), 0);
}

TEST(GeneralsArsenalPoints, AFreePowerCostsNothing)
{
	Wallet wallet;
	EXPECT_TRUE(wallet.spend(0));
	EXPECT_FLOAT_EQ(wallet.points(), 0.0f);
}

TEST(GeneralsArsenalPoints, AChargeNeverGoesBelowZero)
{
	Wallet wallet;
	wallet.earn(30.0f, 1000.0f);
	wallet.charge(50);
	EXPECT_FLOAT_EQ(wallet.points(), 0.0f);
}
