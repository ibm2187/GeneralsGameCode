// CoH DirectionalArmor: a shell's blast against soldiers falls off from its middle to its edge.

#include <gtest/gtest.h>

#include "Features/DirectionalArmor/Domain/Blast.h"

namespace
{

using namespace CoH::DirectionalArmor;

const BlastFalloff pack = { .atInner = 3.0f, .atOuter = 1.0f };

TEST(DirectionalArmorBlast, DirectHitsAreNotScaled)
{
	EXPECT_FLOAT_EQ(blastScale(pack, 0.0f, 5.0f, 20.0f), 1.0f);
	EXPECT_FLOAT_EQ(blastScale(pack, 5.0f, 5.0f, 20.0f), 1.0f);
}

TEST(DirectionalArmorBlast, FallsFromInnerToOuter)
{
	EXPECT_NEAR(blastScale(pack, 5.001f, 5.0f, 20.0f), 3.0f, 0.001f);
	EXPECT_FLOAT_EQ(blastScale(pack, 12.5f, 5.0f, 20.0f), 2.0f);
	EXPECT_FLOAT_EQ(blastScale(pack, 20.0f, 5.0f, 20.0f), 1.0f);
	EXPECT_FLOAT_EQ(blastScale(pack, 30.0f, 5.0f, 20.0f), 1.0f);	// beyond the edge: the edge's
}

TEST(DirectionalArmorBlast, NoBlastNoScale)
{
	EXPECT_FLOAT_EQ(blastScale(pack, 8.0f, 5.0f, 5.0f), 1.0f);
	EXPECT_FLOAT_EQ(blastScale(pack, 8.0f, 5.0f, 0.0f), 1.0f);
	EXPECT_FLOAT_EQ(blastScale({}, 8.0f, 5.0f, 20.0f), 1.0f);	// no falloff configured
}

} // namespace
