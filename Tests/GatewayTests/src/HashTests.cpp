#include <GatewayClientLib/GwHash.hpp>

#include <gtest/gtest.h>


TEST(GwHashTests, calcHash)
{
	auto hash = Radiy::calcHash("SUBSYSBVB15");
	ASSERT_EQ(hash, 0x1d1fb9bbf73ffa9);
}

TEST(GwHashTests, calcHashComposed)
{
	auto hash = Radiy::calcHash("SUBSYS");
	hash = Radiy::calcHash("BVB15", hash);
	ASSERT_EQ(hash, 0x1d1fb9bbf73ffa9);
}