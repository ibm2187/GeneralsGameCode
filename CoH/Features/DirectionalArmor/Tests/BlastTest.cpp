// CoH DirectionalArmor: a tank shell's blast hurts the soldiers beside its target, falling off from its
// middle to its edge.

#include <gtest/gtest.h>

#include "Features/DirectionalArmor/Domain/Blast.h"

#include <vector>

namespace
{

using namespace CoH::DirectionalArmor;

const BlastFalloff pack = { .atInner = 3.0f, .atOuter = 1.0f };
const BlastShell crusader = { .damage = 20.0f, .radius = 20.0f };	// primary radius 5, as retail
constexpr float primary = 5.0f;

TEST(DirectionalArmorBlast, TheTargetGetsTheShellNotTheBlast)
{
	EXPECT_FALSE(blastDamage(crusader, pack, 0.0f, primary).has_value());
	EXPECT_FALSE(blastDamage(crusader, pack, 5.0f, primary).has_value());
}

TEST(DirectionalArmorBlast, SoldiersBesideTheTargetAreHurt)
{
	// A squad in formation around the soldier the tank shot at: the target at 0, the rest 4 to 18 away.
	const std::vector<float> squad = { 6.0f, 8.0f, 12.0f, 15.0f, 18.0f };
	for (const float distance : squad)
	{
		const std::optional<float> damage = blastDamage(crusader, pack, distance, primary);
		ASSERT_TRUE(damage.has_value()) << distance;
		EXPECT_GT(*damage, crusader.damage * pack.atOuter - 0.001f) << distance;
	}
}

TEST(DirectionalArmorBlast, FallsFromInnerToOuter)
{
	EXPECT_NEAR(*blastDamage(crusader, pack, 5.001f, primary), 60.0f, 0.01f);
	EXPECT_FLOAT_EQ(*blastDamage(crusader, pack, 12.5f, primary), 40.0f);
	EXPECT_FLOAT_EQ(*blastDamage(crusader, pack, 20.0f, primary), 20.0f);
	EXPECT_GT(*blastDamage(crusader, pack, 6.0f, primary), *blastDamage(crusader, pack, 15.0f, primary));
}

TEST(DirectionalArmorBlast, NothingBeyondTheEdgeOrWithoutABlast)
{
	EXPECT_FALSE(blastDamage(crusader, pack, 20.5f, primary).has_value());
	EXPECT_FALSE(blastDamage({}, pack, 8.0f, primary).has_value());
	EXPECT_FALSE(blastDamage({ .damage = 20.0f, .radius = 4.0f }, pack, 8.0f, primary).has_value());
}

} // namespace
