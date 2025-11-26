#include "ConnectionPorts.h"
#include <AdsConnectionLib/AdsConnection.h>
#include <AdsConnectionLib/IRecentAppSignals.h>
#include <ClientLib/AppSignalManager.h>
#include <ClientLib/LoggerStdAdapter.h>
#include <ClientLib/ServiceEndpoint.h>

#include "../OnlineLib/SoftwareEndpoint.h"
#include "../OnlineLib/SoftwareInfo.h"


using ::testing::_;
using ::testing::AtLeast;
using ::testing::An;

using SourceIdType = ::ClientLib::IAppSignalUpdater::SourceIdType;

namespace
{
	class MockAppSignalUpdater : public ClientLib::IAppSignalUpdater
	{
	public:
		MOCK_METHOD(void, reset, (), (override));
		MOCK_METHOD(void, notifySignalParamsUpdated, (), (override));
		MOCK_METHOD(void, addSignals, (std::span<const ::Proto::AppSignal> appSignals, const std::string& appDataServiceId), (override));
		MOCK_METHOD(void, invalidateSignalStates, (SourceIdType sourceThreadId), (override));
		MOCK_METHOD(void,
					setStates,
					(std::span<const Proto::AppSignalState> states, Hash dataServerHash, SourceIdType sourceThreadId),
					(override));
	};

	class MockRecentAppSignals : public ClientLib::IRecentAppSignals
	{
	public:
		MOCK_METHOD(void, addRecentAppSignal, (Hash h), (override));
		MOCK_METHOD(void, addRecentAppSignals, (std::span<const Hash> hashes), (override));
		MOCK_METHOD(std::vector<Hash>, recentlyUsedAppSignals, (const std::string& appDataServivceId), (override));
		MOCK_METHOD(bool, hasRecentlyUsedAppSignals, (), (override));
	};
} // namespace

class AdsConnectionTests : public ::testing::Test
{
protected:
	virtual void SetUp()
	{
		AppDataServices[0].address.setPort(g_connectionPorts.ads1.clientRequestPort);
		AppDataServices[0].realtimeAddress.setPort(g_connectionPorts.ads1.rtTrendsRequestPort);

		AppDataServices[1].address.setPort(g_connectionPorts.ads2.clientRequestPort);
		AppDataServices[1].realtimeAddress.setPort(g_connectionPorts.ads2.rtTrendsRequestPort);
	}

	virtual void TearDown() {}

	// clang-format off
	inline static std::vector<SoftwareEndpoint::AppDataService> AppDataServices = {
		{
			.equipmentId = "SYSTEMID_CLIENTTEST_WS01_ADS_RC1",
			.shortenId = "WS01_ADS",
			.address = {"127.0.0.1", 13323},
			.realtimeAddress = {"127.0.0.1", 13324}
		},
		{
			.equipmentId = "SYSTEMID_CLIENTTEST_WS02_ADS_RC1",
			.shortenId = "WS02_ADS",
			.address = {"127.0.0.1", 13326},
			.realtimeAddress = {"127.0.0.1", 13327}
		}
	};
	// clang-format on
};


TEST_F(AdsConnectionTests, connectToAds)
{
	ILogFileStub log;
	MockAppSignalUpdater signalUpdater;
	MockRecentAppSignals recentlyUsedSignals;

	std::string ads1{"SYSTEMID_CLIENTTEST_WS01_ADS_RC1"};
	std::string ads2{"SYSTEMID_CLIENTTEST_WS02_ADS_RC1"};

	Hash dataServerHash1 = ::calcHash(ads1);
	Hash dataServerHash2 = ::calcHash(ads2);

	SoftwareInfo softwareInfo(E::SoftwareType::Monitor, "SYSTEMID_CLIENTTEST_WS03_MONITOR");
	Network::SoftwareInfo networkSoftwareInfo;
	softwareInfo.serializeTo(&networkSoftwareInfo);

	// MockAppSignalUpdater
	//
	// clang-format off
	EXPECT_CALL(signalUpdater, notifySignalParamsUpdated())
			.Times(2);	// 2 times, once for each ADS when all signals loaded (per ADS).

	EXPECT_CALL(signalUpdater, reset())
			.Times(1);	// 1 time in adsConnection.updateConnections.

	EXPECT_CALL(signalUpdater, invalidateSignalStates(_))
			.Times(4);	// 4 times on disconnect two ADSs * two recent connections.

	EXPECT_CALL(signalUpdater, addSignals(_, ads1))
			.Times(AtLeast(1));

	EXPECT_CALL(signalUpdater, addSignals(_, ads2))
			.Times(AtLeast(1));

	EXPECT_CALL(signalUpdater, setStates(An<std::span<const Proto::AppSignalState>>(), dataServerHash1, _))
			.Times(AtLeast(1));

	EXPECT_CALL(signalUpdater, setStates(An<std::span<const Proto::AppSignalState>>(), dataServerHash2, _))
			.Times(AtLeast(1));

	// MockRecentAppSignals
	//
	EXPECT_CALL(recentlyUsedSignals, addRecentAppSignal(_))
			.Times(0);

	EXPECT_CALL(recentlyUsedSignals, addRecentAppSignals(_))
			.Times(0);

	EXPECT_CALL(recentlyUsedSignals, recentlyUsedAppSignals(ads1))
			.Times(AtLeast(1));

	EXPECT_CALL(recentlyUsedSignals, recentlyUsedAppSignals(ads2))
			.Times(AtLeast(1));
	// clang-format on

	// Start
	//
	{
		ClientLib::LoggerStdAdapter loggerAdapter{log};

		ClientLib::AdsConnection adsConnection{signalUpdater, &recentlyUsedSignals, nullptr, loggerAdapter};
		adsConnection.updateConnections(networkSoftwareInfo, toServiceEndpoint(AppDataServices));

		// Wait for connection established
		//
		QElapsedTimer timer;
		timer.start();

		while (timer.hasExpired(10'000) == false)
		{
			QCoreApplication::instance()->processEvents();
			QThread::yieldCurrentThread();

			// Wait for 20 replies, so all signals are loaded and some states are received.
			//
			auto adsConnStates = adsConnection.connectionStates();
			if (std::all_of(adsConnStates.begin(),
							adsConnStates.end(),
							[](const auto& s)
							{
								return s.replyCount > 20;
							}))
			{
				break;
			}
		}

		// Check that two connections are established.
		//
		auto adsConnStates = adsConnection.connectionStates();

		ASSERT_EQ(adsConnStates.size(), 2 * adsConnection.connectionsPerServer());

		EXPECT_TRUE(adsConnStates[0].isConnected);
		EXPECT_EQ(adsConnStates[0].peerAddr, toServiceEndpoint(AppDataServices[0]).address);

		EXPECT_TRUE(adsConnStates[1].isConnected);
		EXPECT_EQ(adsConnStates[1].peerAddr, toServiceEndpoint(AppDataServices[1]).address);

		if (adsConnection.connectionsPerServer() == 2)
		{
			EXPECT_TRUE(adsConnStates[2].isConnected);
			EXPECT_EQ(adsConnStates[2].peerAddr, toServiceEndpoint(AppDataServices[0]).address);

			EXPECT_TRUE(adsConnStates[3].isConnected);
			EXPECT_EQ(adsConnStates[3].peerAddr, toServiceEndpoint(AppDataServices[1]).address);
		}
	}

	return;
}

TEST_F(AdsConnectionTests, adsNoConnection)
{
	ILogFileStub log;
	ClientLib::LoggerStdAdapter loggerAdapter{log};
	MockAppSignalUpdater signalUpdater;

	SoftwareInfo softwareInfo(E::SoftwareType::Monitor, "SYSTEMID_CLIENTTEST_WS03_MONITOR");
	Network::SoftwareInfo networkSoftwareInfo;
	softwareInfo.serializeTo(&networkSoftwareInfo);

	// MockAppSignalUpdater
	//
	// clang-format off
	EXPECT_CALL(signalUpdater, notifySignalParamsUpdated())
		.Times(0);

	EXPECT_CALL(signalUpdater, reset())
		.Times(1);	// 1 time in adsConnection.updateConnections.

	EXPECT_CALL(signalUpdater, invalidateSignalStates(_))
		.Times(0);

	EXPECT_CALL(signalUpdater, addSignals(_, _))
		.Times(0);

	EXPECT_CALL(signalUpdater, setStates(_, _, _))
		.Times(0);
	// clang-format on

	// Start
	//
	{
		auto servers = AppDataServices;
		servers[0].address = {"192.178.12.90", 13323}; // Some unreachable addresses.
		servers[1].address = {"192.178.13.90", 13323}; //

		ClientLib::AdsConnection adsConnection{signalUpdater, nullptr, nullptr, loggerAdapter};
		adsConnection.updateConnections(networkSoftwareInfo, toServiceEndpoint(servers));

		// Wait for connection established
		//
		QElapsedTimer timer;
		timer.start();

		while (timer.hasExpired(3000) == false)
		{
			QCoreApplication::instance()->processEvents();
			QThread::msleep(10);
		}

		// Check that two connections are established.
		//
		auto adsConnStates = adsConnection.connectionStates();

		ASSERT_EQ(adsConnStates.size(), 2 * adsConnection.connectionsPerServer());

		EXPECT_FALSE(adsConnStates[0].isConnected);
		EXPECT_FALSE(adsConnStates[1].isConnected);
	}

	return;
}

TEST_F(AdsConnectionTests, receivesState)
{
	// Test: Connect to AppSignalManager and check that signal "#SYSTEMID_CLIENTTEST_CH10_MD00_PI_BLINK"
	//	     is blinking (1 Hz)
	//
	ILogFileStub log;
	ClientLib::LoggerStdAdapter loggerAdapter{log};
	ClientLib::AppSignalManager signalManager{&log};

	SoftwareInfo softwareInfo(E::SoftwareType::Monitor, "SYSTEMID_CLIENTTEST_WS03_MONITOR");
	Network::SoftwareInfo networkSoftwareInfo;
	softwareInfo.serializeTo(&networkSoftwareInfo);

	// Start
	//
	ClientLib::AdsConnection adsConnection{signalManager, &signalManager, nullptr, loggerAdapter};
	adsConnection.updateConnections(networkSoftwareInfo, toServiceEndpoint(AppDataServices));

	// Wait for connection established
	//
	QElapsedTimer timer;
	timer.start();
	bool connected = false;

	while (timer.hasExpired(3000) == false)
	{
		QCoreApplication::instance()->processEvents();
		QThread::msleep(10);

		// Wait for 30 replies, so all signals are loaded and some states are received.
		//
		auto adsConnStates = adsConnection.connectionStates();
		if (std::all_of(adsConnStates.begin(),
						adsConnStates.end(),
						[](const auto& s)
						{
							return s.replyCount > 30;
						}))
		{
			connected = true;
			break;
		}
	}

	ASSERT_TRUE(connected);

	// Check that signal "#SYSTEMID_CLIENTTEST_CH10_MD00_PI_BLINK" is blinking with frequency 1 Hz1.
	//
	timer.restart();
	double lastState = 0;
	int stateChanges = 0;
	while (timer.hasExpired(10000) == false && stateChanges < 3)
	{
		auto state = signalManager.signalState("#SYSTEMID_CLIENTTEST_CH10_MD00_PI_BLINK");

		ASSERT_TRUE(state.has_value());

		stateChanges += (lastState == state->value()) ? 0 : 1;
		lastState = state->value();

		QThread::yieldCurrentThread();
	}

	qDebug() << "TEST(AdsConnectionTests, receivesState): stateChanges of #SYSTEMID_CLIENTTEST_CH10_MD00_PI_BLINK: " << stateChanges;

	// The test machine can be loaded heavily, it will result in reduced numbers of state changes.
	//
	EXPECT_TRUE(stateChanges >= 3);

	return;
}
