#include <gtest/gtest.h>

#include <ArchV3Lib/Core.h>

#include "Common.h"

ArchV3::DbConnectionInfo dbConnInfo = {.host = "127.0.0.1", .port = 5433, .user = "u7arch", .password = "P2ssw0rd"};

class DbTests : public ::testing::Test
{
protected:
	static void SetUpTestSuite()
	{ 
		//ArchV3::Db::dropDatabases(dbConnInfo, "u7arch_test_%", logger);
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

TEST_F(DbTests, OpenCreatesDatabase)
{
	ArchV3::Db db("TEST_PROJECT", "APP_DATA_SRV", dbConnInfo, logger);

	ASSERT_TRUE(db.open());
	EXPECT_TRUE(db.isOpen());

	db.close();
	EXPECT_FALSE(db.isOpen());
}