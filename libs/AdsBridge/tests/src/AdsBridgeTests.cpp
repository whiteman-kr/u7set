#include "TestSettings.h"
#include <AdsBridge/AdsBridge.h>
#include <chrono>
#include <exception>
#include <fstream>
#include <sstream>
#include <string>
#include <syncstream>
#include <thread>

using ::testing::_;
using ::testing::AtLeast;
using ::testing::An;

namespace
{
#if 1
	const int ConnectionsPerServer = 1; // 1 for GrpcBased connection.
#else
	const int ConnectionsPerServer = 2; // 2 for TcpBased connection (States, Recents)
#endif

	// AdsBridge Equipment ID used in the tests.
	//
	const auto g_equipmentId{"SYSTEMID_CLIENTTEST_WS03_ADSBRIDGE"};

	// Test log handler writes received data to std::cerr.
	//
	extern "C" void logHandler(MatsLogLevel level, const char* message)
	{
		const char* logLevelString = nullptr;
		switch (level)
		{
		case MATS_LOG_LEVEL_DEBUG:
			logLevelString = "DBG: ";
			break;
		case MATS_LOG_LEVEL_WARNING:
			logLevelString = "WRN: ";
			break;
		case MATS_LOG_LEVEL_ERROR:
			logLevelString = "ERR: ";
			break;
		default:
			logLevelString = "UNK: ";
			std::terminate();
		}

		std::osyncstream{std::cerr} << logLevelString << message << "\n";
		return;
	}

	// Test log handler saves received data to these globals.
	//
	MatsLogLevel g_testLogHandlerLevel = MATS_LOG_LEVEL_ERROR;
	std::string g_testLogHandlerMessage;

	extern "C" void testLogHandler(MatsLogLevel level, const char* message)
	{
		g_testLogHandlerLevel = level;
		g_testLogHandlerMessage = message;
		return;
	}
} // namespace

class AdsBridgeTests : public ::testing::Test
{
protected:
	virtual void SetUp() override
	{
		AdsInit(g_equipmentId);

		AdsSetLogHandler(&logHandler);
		AdsSetLogLevel(MATS_LOG_LEVEL_DEBUG);

		return;
	}

	virtual void TearDown() override { AdsShutdown(); }

	struct AdsConnection
	{
		std::string equipmentId;
		std::string address;
		int port;
	};

	static inline std::vector<AdsConnection> AppDataServices = {
		{.equipmentId = "SYSTEMID_CLIENTTEST_WS01_ADS", .address = {"127.0.0.1"}, .port = 13323},
		{.equipmentId = "SYSTEMID_CLIENTTEST_WS02_ADS", .address = {"127.0.0.1"}, .port = 13326}};
};

TEST_F(AdsBridgeTests, ApiVersion)
{
	EXPECT_EQ(::AdsGetInterfaceVersion(), 0x00000001);
	return;
}

TEST_F(AdsBridgeTests, LogHandler)
{
	AdsSetLogHandler(&testLogHandler);
	AdsSetLogLevel(MATS_LOG_LEVEL_DEBUG);

	AdsTestLogHandler(MATS_LOG_LEVEL_DEBUG, "Test debug message");
	EXPECT_EQ(g_testLogHandlerLevel, MATS_LOG_LEVEL_DEBUG);
	EXPECT_EQ(g_testLogHandlerMessage, "Test debug message");

	AdsTestLogHandler(MATS_LOG_LEVEL_WARNING, "Test warning message");
	EXPECT_EQ(g_testLogHandlerLevel, MATS_LOG_LEVEL_WARNING);
	EXPECT_EQ(g_testLogHandlerMessage, "Test warning message");

	AdsTestLogHandler(MATS_LOG_LEVEL_ERROR, "Test error message");
	EXPECT_EQ(g_testLogHandlerLevel, MATS_LOG_LEVEL_ERROR);
	EXPECT_EQ(g_testLogHandlerMessage, "Test error message");

	// Test AdsSetLogLevel
	//
	AdsSetLogLevel(MATS_LOG_LEVEL_WARNING); // Debug messages should not be logged.

	g_testLogHandlerLevel = MATS_LOG_LEVEL_ERROR;
	g_testLogHandlerMessage.clear();

	AdsTestLogHandler(MATS_LOG_LEVEL_DEBUG, "Test debug message");
	EXPECT_EQ(g_testLogHandlerLevel, MATS_LOG_LEVEL_ERROR);
	EXPECT_EQ(g_testLogHandlerMessage, "");

	AdsTestLogHandler(MATS_LOG_LEVEL_WARNING, "Test warning message");
	EXPECT_EQ(g_testLogHandlerLevel, MATS_LOG_LEVEL_WARNING);
	EXPECT_EQ(g_testLogHandlerMessage, "Test warning message");

	// --
	//
	AdsSetLogLevel(MATS_LOG_LEVEL_ERROR); // Debug and Warning messages should not be logged.

	g_testLogHandlerLevel = MATS_LOG_LEVEL_ERROR;
	g_testLogHandlerMessage.clear();

	AdsTestLogHandler(MATS_LOG_LEVEL_DEBUG, "Test debug message");
	EXPECT_EQ(g_testLogHandlerLevel, MATS_LOG_LEVEL_ERROR);
	EXPECT_EQ(g_testLogHandlerMessage, "");

	AdsTestLogHandler(MATS_LOG_LEVEL_WARNING, "Test warning message");
	EXPECT_EQ(g_testLogHandlerLevel, MATS_LOG_LEVEL_ERROR);
	EXPECT_EQ(g_testLogHandlerMessage, "");

	AdsTestLogHandler(MATS_LOG_LEVEL_ERROR, "Test error message");
	EXPECT_EQ(g_testLogHandlerLevel, MATS_LOG_LEVEL_ERROR);
	EXPECT_EQ(g_testLogHandlerMessage, "Test error message");

	return;
}

TEST_F(AdsBridgeTests, AdsInit)
{
	// First calls happens in AdsBridgeTests::AdsBridgeTests()
	//

	// Second call should fail
	//
	ASSERT_FALSE(AdsInit(g_equipmentId));
	return;
}

TEST_F(AdsBridgeTests, AdsGetSoftwareId)
{
	// Test AdsInit
	//
	auto id = AdsGetSoftwareId();
	EXPECT_STREQ(id, g_equipmentId);

	return;
}


TEST_F(AdsBridgeTests, AdsLoadConfiguration)
{
	ASSERT_TRUE(AdsLoadConfiguration(TestSettings::ConfigurationFile.c_str()));
	EXPECT_STREQ(AdsGetSoftwareId(), g_equipmentId);

	AdsConnect(); // AdsGetConnectionCount() will return connections count only after AdsConnect().
	EXPECT_EQ(AdsGetTcpConnectionCount(), ConnectionsPerServer * 2);
	AdsCloseConnection();

	return;
}

TEST_F(AdsBridgeTests, AdsSetConfiguration)
{
	// Load data from the configuration file (CTestSettings::ConfigurationFile).
	//
	std::string configurationData;

	std::ifstream configFile(TestSettings::ConfigurationFile);
	EXPECT_TRUE(configFile.is_open());

	std::stringstream buffer;
	buffer << configFile.rdbuf();
	configurationData = buffer.str();
	configFile.close();
	EXPECT_FALSE(configurationData.empty());

	EXPECT_TRUE(AdsSetConfiguration(configurationData.data(), configurationData.size()));

	EXPECT_STREQ(AdsGetSoftwareId(), g_equipmentId);

	AdsConnect(); // AdsGetConnectionCount() will return connections count only after AdsConnect().
	EXPECT_EQ(AdsGetTcpConnectionCount(), ConnectionsPerServer * 2);

	AdsCloseConnection();
	return;
}

TEST_F(AdsBridgeTests, AdsSetConfigurationProfile)
{
	ASSERT_TRUE(AdsLoadConfiguration(TestSettings::ConfigurationFile.c_str()));

	EXPECT_TRUE(AdsSetConfigurationProfile("Default"));
	EXPECT_STREQ(AdsGetSoftwareId(), g_equipmentId);

	AdsConnect(); // AdsGetConnectionCount() will return connections count only after AdsConnect().
	EXPECT_EQ(AdsGetTcpConnectionCount(), ConnectionsPerServer * 2);
	AdsCloseConnection();

	EXPECT_TRUE(AdsSetConfigurationProfile("linux_code_coverage"));
	EXPECT_STREQ(AdsGetSoftwareId(), g_equipmentId);

	AdsConnect(); // AdsGetConnectionCount() will return connections count only after AdsConnect().
	EXPECT_EQ(AdsGetTcpConnectionCount(), ConnectionsPerServer * 2);
	AdsCloseConnection();

	EXPECT_FALSE(AdsSetConfigurationProfile("NoProfile"));

	return;
}

TEST_F(AdsBridgeTests, AdsAddService)
{
	{
		const char* adsEquipmentId = "SYSTEMID_CLIENTTEST_WS01_ADS";
		const char* address = "127.0.0.1";
		int port = 13323;

		AdsAddService(adsEquipmentId, address, port);

		AdsConnect(); // AdsGetConnectionCount() will return connections count only after AdsConnect().
		EXPECT_EQ(AdsGetTcpConnectionCount(), ConnectionsPerServer * 1);

		// After close connection should not be any TcpConnection.
		//
		AdsCloseConnection();
		EXPECT_EQ(AdsGetTcpConnectionCount(), 0);
	}

	{
		const char* adsEquipmentId = "SYSTEMID_CLIENTTEST_WS02_ADS";
		const char* address = "127.0.0.2";
		int port = 13323 + 1;

		AdsAddService(adsEquipmentId, address, port);

		AdsConnect(); // AdsGetConnectionCount() will return connections count only after AdsConnect().
		EXPECT_EQ(AdsGetTcpConnectionCount(), ConnectionsPerServer * 2);

		// After close connection should not be any TcpConnection.
		//
		AdsCloseConnection();
		EXPECT_EQ(AdsGetTcpConnectionCount(), 0);
	}

	return;
}

TEST_F(AdsBridgeTests, AdsGetTcpConnectionStatuses)
{
	ASSERT_TRUE(AdsLoadConfiguration(TestSettings::ConfigurationFile.c_str()));
	ASSERT_TRUE(AdsSetConfigurationProfile(TestSettings::SettingsProfile.c_str()));

	AdsConnect(); // AdsGetConnectionCount() will return connections count only after AdsConnect().

	constexpr int ConnectionCount = ConnectionsPerServer * 2;
	struct AdsConnectionStatus connectionStatus[ConnectionCount]{};

	// Wait for the connections to be established. timeout is 10 seconds.
	//
	for (int i = 0; i < 100; i++)
	{
		ASSERT_EQ(AdsGetTcpConnectionCount(), ConnectionCount);
		ASSERT_TRUE(AdsGetTcpConnectionStatuses(connectionStatus, ConnectionCount));

		bool allConnected = std::all_of(std::begin(connectionStatus),
										std::end(connectionStatus),
										[](const auto& cs)
										{
											return cs.status;
										});
		if (allConnected)
		{
			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	ASSERT_EQ(AdsGetTcpConnectionCount(), ConnectionCount);
	ASSERT_TRUE(AdsGetTcpConnectionStatuses(connectionStatus, ConnectionCount));

	EXPECT_TRUE(connectionStatus[0].status);
	EXPECT_TRUE(connectionStatus[1].status);

	if (ConnectionsPerServer == 2)
	{
		EXPECT_TRUE(connectionStatus[2].status);
		EXPECT_TRUE(connectionStatus[3].status);
	}

	EXPECT_EQ(connectionStatus[0].setConnectionResult, ADS_SET_CONNECTION_RESULT_OK);
	EXPECT_EQ(connectionStatus[1].setConnectionResult, ADS_SET_CONNECTION_RESULT_OK);

	if (ConnectionsPerServer == 2)
	{
		EXPECT_EQ(connectionStatus[2].setConnectionResult, ADS_SET_CONNECTION_RESULT_OK);
		EXPECT_EQ(connectionStatus[3].setConnectionResult, ADS_SET_CONNECTION_RESULT_OK);
	}

	return;
}

TEST_F(AdsBridgeTests, AdsSignalParamsAndStatesLoaded)
{
	ASSERT_TRUE(AdsLoadConfiguration(TestSettings::ConfigurationFile.c_str()));
	ASSERT_TRUE(AdsSetConfigurationProfile(TestSettings::SettingsProfile.c_str()));

	AdsConnect();

	// Wait for the connections to be established. timeout is 10 seconds.
	//
	for (int i = 0; i < 100; i++)
	{
		if (AdsSignalParamsLoaded() == true && AdsSignalStatesLoaded() == true)
		{
			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	EXPECT_TRUE(AdsSignalParamsLoaded());
	EXPECT_TRUE(AdsSignalStatesLoaded());

	return;
}

TEST_F(AdsBridgeTests, AdsCalcHash)
{
	auto ch = AdsCalcHash("#ABCD_EFG_DEF");
	EXPECT_EQ(ch, ch = 0x592BB7B5BDE546ACull);
}

TEST_F(AdsBridgeTests, AdsGetSignalList)
{
	ASSERT_TRUE(AdsLoadConfiguration(TestSettings::ConfigurationFile.c_str()));
	ASSERT_TRUE(AdsSetConfigurationProfile(TestSettings::SettingsProfile.c_str()));

	AdsConnect();

	// Wait for the connections to be established. timeout is 10 seconds.
	//
	for (int i = 0; i < 100; i++)
	{
		if (AdsSignalParamsLoaded() == true)
		{
			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	ASSERT_TRUE(AdsSignalParamsLoaded());

	size_t signalCount = AdsGetSignalCount();
	EXPECT_GE(signalCount, 5); // We have at least 5 signals in the test configuration.

	auto signalHashes = std::make_unique<MatsSignalHash[]>(signalCount);
	std::fill(signalHashes.get(), signalHashes.get() + signalCount, 0x1122334455667788);

	EXPECT_TRUE(AdsGetSignalList(signalHashes.get(), signalCount));

	// Check that no signal hash is 0x1122334455667788.
	//
	EXPECT_FALSE(std::any_of(signalHashes.get(),
							 signalHashes.get() + signalCount,
							 [](const auto& sh)
							 {
								 return sh == 0x1122334455667788;
							 }));

	return;
}

TEST_F(AdsBridgeTests, AdsGetSignalParams)
{
	ASSERT_TRUE(AdsLoadConfiguration(TestSettings::ConfigurationFile.c_str()));
	ASSERT_TRUE(AdsSetConfigurationProfile(TestSettings::SettingsProfile.c_str()));

	AdsConnect();

	// Wait for the connections to be established. timeout is 10 seconds.
	//
	for (int i = 0; i < 100; i++)
	{
		if (AdsSignalParamsLoaded() == true)
		{
			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	ASSERT_TRUE(AdsSignalParamsLoaded());

	size_t signalCount = AdsGetSignalCount();
	EXPECT_GE(signalCount, 5); // We have at least 5 signals in the test configuration.

	auto signalHashes = std::make_unique<MatsSignalHash[]>(signalCount);
	EXPECT_TRUE(AdsGetSignalList(signalHashes.get(), signalCount));

	auto signalParams = std::make_unique<MatsAppSignalParam[]>(signalCount);
	std::fill(signalParams.get(), signalParams.get() + signalCount, MatsAppSignalParam{});

	EXPECT_TRUE(AdsGetSignalParams(signalHashes.get(), signalParams.get(), signalCount));

	// Check that all signal parameters are loaded.
	//
	EXPECT_FALSE(std::any_of(signalParams.get(),
							 signalParams.get() + signalCount,
							 [](const auto& sp)
							 {
								 return sp.hash == 0;
							 }));

	return;
}

TEST_F(AdsBridgeTests, AdsGetSignalStates)
{
	ASSERT_TRUE(AdsLoadConfiguration(TestSettings::ConfigurationFile.c_str()));
	ASSERT_TRUE(AdsSetConfigurationProfile(TestSettings::SettingsProfile.c_str()));

	AdsConnect();

	// Wait for the connections to be established. timeout is 10 seconds.
	//
	for (int i = 0; i < 100; i++)
	{
		if (AdsSignalStatesLoaded() == true)
		{
			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	ASSERT_TRUE(AdsSignalStatesLoaded());

	size_t signalCount = AdsGetSignalCount();
	EXPECT_GE(signalCount, 5); // We have at least 5 signals in the test configuration.

	auto signalHashes = std::make_unique<MatsSignalHash[]>(signalCount);
	EXPECT_TRUE(AdsGetSignalList(signalHashes.get(), signalCount));

	auto signalStates = std::make_unique<MatsAppSignalState[]>(signalCount);
	std::fill(signalStates.get(), signalStates.get() + signalCount, MatsAppSignalState{});

	EXPECT_EQ(AdsGetSignalStates(signalHashes.get(), signalStates.get(), signalCount), signalCount);

	// Check that all signal states are loaded.
	//
	EXPECT_FALSE(std::any_of(signalStates.get(),
							 signalStates.get() + signalCount,
							 [](const auto& ss)
							 {
								 return ss.hash == 0;
							 }));

	return;
}

TEST_F(AdsBridgeTests, AdsGetSignalStatesRequestSignal)
{
	ASSERT_TRUE(AdsLoadConfiguration(TestSettings::ConfigurationFile.c_str()));
	ASSERT_TRUE(AdsSetConfigurationProfile(TestSettings::SettingsProfile.c_str()));

	AdsConnect();

	// Wait for the connections to be established. timeout is 10 seconds.
	//
	for (int i = 0; i < 100; i++)
	{
		if (AdsSignalStatesLoaded() == true)
		{
			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	ASSERT_TRUE(AdsSignalStatesLoaded());

	MatsSignalHash hash = AdsCalcHash("#SYSTEMID_CLIENTTEST_CH10_MD00_PI_BLINK"); // Blink is a signal that changes its state every 500 ms.
	MatsAppSignalState state{};

	double lastState = 0.0;
	int stateChanges = 0;

	// get signals states for 10 seconds, value should be different (0 or 1) at least 5 times.
	//
	for (int i = 0; i < 200; i++)
	{
		EXPECT_EQ(AdsGetSignalStates(&hash, &state, 1), 1);
		EXPECT_EQ(state.hash, hash);

		if (state.value != lastState)
		{
			lastState = state.value;
			stateChanges++;
		}

		if (stateChanges >= 5)
		{
			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}

	EXPECT_GE(stateChanges, 5);

	return;
}