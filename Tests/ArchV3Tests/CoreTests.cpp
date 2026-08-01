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

const QString CLIENT_ID1("SYSTEMID_RACK01_WS00_ADSV3");

const std::vector<QString> discretes = {
	"#LM1_DS01", 
	"#LM1_DS02", 
	"#LM1_DS03"
};

const std::vector<QString> analogs = {
	"#LM1_RES01", 
	"#LM1_RES02", 
	"#LM1_RES03"
};

TEST_F(CoreTests, Init)
{
	ArchV3::Core core("D:/Archive", *achInfoV3Data.get(), dbConnInfo, logger);

	EXPECT_TRUE(core.init());
}

TEST_F(CoreTests, PushArchData)
{
	dropDatabases(COMPILER_TESTS_PROJECT_DB_PATTERN);

	ArchV3::Core core("D:/Archive", *achInfoV3Data.get(), dbConnInfo, logger);

	EXPECT_TRUE(core.init());

	Network::SaveAppSignalsStatesToArchiveRequest request;

	request.set_clientequipmentid(CLIENT_ID1.toStdString());

	for (const QString& d : discretes)
	{
		SimpleAppSignalState state;

		state.hash = calcHash(d);
		state.value = 1;
		state.flags.all = 3;
		state.time.system.timeStamp = currentMSecsUTC();
		state.time.plant.timeStamp = state.time.system.timeStamp - 5;
		state.time.local.timeStamp = state.time.system.timeStamp + 60 * 2 * 1000;

		Proto::AppSignalState* ps = request.add_appsignalstates();

		state.save(ps);
	}

	QByteArray data;
	data.resize(static_cast<int>(request.ByteSizeLong()));

	request.SerializeToArray(data.data(), data.size());

	core.pushArchData(CLIENT_ID1, data.constData(), static_cast<size_t>(data.size()));

	QThread::msleep(500000);
}