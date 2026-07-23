#include <gtest/gtest.h>

#include <ArchV3Lib/Core.h>

#include "Common.h"


class CoreTests : public ::testing::Test
{
protected:
	static void SetUpTestSuite()
	{ 
		dropDatabases(TEST_PROJECT_DB_PATTERN);
	}

	static void TearDownTestSuite()
	{
		//	ArchV3::Db::dropDatabases(dbConnInfo, "u7arch_test_%", logger);
	}

	void SetUp() override
	{
		// before each test
	}

	void TearDown() override
	{
		// after each test
	}
};

TEST_F(CoreTests, Init)
{
	ArchV3::Core core("D:/Archive", *achInfoV3Data.get(), dbConnInfo, logger);

	EXPECT_TRUE(core.init());
}