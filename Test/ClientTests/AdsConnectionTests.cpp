#include <ClientLib/AppSignalManager.h>
#include <ClientLib/IRecentAppSignals.h>
#include <ClientLib/AdsConnection.h>
#include "ConnectionPorts.h"

using ::testing::_;
using ::testing::AtLeast;
using ::testing::An;

namespace
{
	class MockAppSignalUpdater : public ClientLib::IAppSignalUpdater
	{
	public:
		MOCK_METHOD(void, reset, (), (override));
		MOCK_METHOD(void, notifySignalParamsUpdated, (), (override));
		MOCK_METHOD(void, addSignal, (const AppSignalParam& appSignal, const QString& appDataServiceId), (override));
		MOCK_METHOD(void, addSignals, (const std::vector<AppSignalParam>& appSignals, const QString& appDataServiceId), (override));
		MOCK_METHOD(void, invalidateSignalStates, (Qt::HANDLE sourceThreadId), (override));
		MOCK_METHOD(void, setState, (const QString& appSignalId, const AppSignalState& state, Hash dataServerHash, Qt::HANDLE sourceThreadId), (override));
		MOCK_METHOD(void, setState, (Hash signalHash, const AppSignalState& state, Hash dataServerHash, Qt::HANDLE sourceThreadId), (override));
		MOCK_METHOD(void, setState, (const std::vector<AppSignalState>& states, Hash dataServerHash, Qt::HANDLE sourceThreadId), (override));
	};

	class MockRecentAppSignals : public ClientLib::IRecentAppSignals
	{
	public:
		MOCK_METHOD(void, addRecentAppSignal, (Hash h), (override));
		MOCK_METHOD(void, addRecentAppSignals, (const std::vector<Hash>& hashes), (override));
		MOCK_METHOD(std::vector<Hash>, recentlyUsedAppSignals, (const QString& appDataServivceId), (override));
		MOCK_METHOD(bool, hasRecentlyUsedAppSignals, (), (override));
	};
}

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

	virtual void TearDown()
	{
	}

	inline static std::vector<SoftwareEndpoint::AppDataService> AppDataServices =
	{
		{
			.equipmentId = "SYSTEMID_CLIENTTEST_WS01_ADS",
			.shortenId = "WS01_ADS",
			.address = {"127.0.0.1", 13323},
			.realtimeAddress = {"127.0.0.1", 13324}
		},
		{
			.equipmentId = "SYSTEMID_CLIENTTEST_WS02_ADS",
			.shortenId = "WS02_ADS",
			.address = {"127.0.0.1", 13326},
			.realtimeAddress = {"127.0.0.1", 13327}}
	};
};


TEST_F(AdsConnectionTests, connectToAds)
{
	ILogFileStub log;
	MockAppSignalUpdater signalUpdater;
	MockRecentAppSignals recentlyUsedSignals;

	QString ads1{"SYSTEMID_CLIENTTEST_WS01_ADS"};
	QString ads2{"SYSTEMID_CLIENTTEST_WS02_ADS"};

	Hash dataServerHash1 = ::calcHash(ads1);
	Hash dataServerHash2 = ::calcHash(ads2);

	SoftwareInfo softwareInfo(E::SoftwareType::Monitor, "SYSTEMID_CLIENTTEST_WS03_MONITOR");

	// MockAppSignalUpdater
	//
	EXPECT_CALL(signalUpdater, notifySignalParamsUpdated())
			.Times(2);	// 2 times, once for each ADS whern all signals loaded (per ADS).

	EXPECT_CALL(signalUpdater, reset())
			.Times(1);	// 1 time in adsConnection.updateConnections.

	EXPECT_CALL(signalUpdater, invalidateSignalStates(_))
			.Times(4);	// 4 times on disconnect two ADSs * two recent connections.

	EXPECT_CALL(signalUpdater, addSignals(_, ads1))
			.Times(AtLeast(1));

	EXPECT_CALL(signalUpdater, addSignals(_, ads2))
			.Times(AtLeast(1));

	EXPECT_CALL(signalUpdater, setState(An<const std::vector<AppSignalState>&>(), dataServerHash1, _))
			.Times(AtLeast(1));

	EXPECT_CALL(signalUpdater, setState(An<const std::vector<AppSignalState>&>(), dataServerHash2, _))
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

	// Start
	//
	{
		ClientLib::AdsConnection adsConnection{signalUpdater, &recentlyUsedSignals, &log};
		adsConnection.updateConnections(softwareInfo, AppDataServices);

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
			std::vector<Tcp::ConnectionState> adsConnStates = adsConnection.tcpSignalConnStates();
			if (std::all_of(adsConnStates.begin(), adsConnStates.end(), [](const auto& s) { return s.replyCount > 20; }))
			{
				break;
			}
		}

		// Check that two connections are established.
		//
		std::vector<Tcp::ConnectionState> adsConnStates = adsConnection.tcpSignalConnStates();
		std::vector<Tcp::ConnectionState> adsRecntStates = adsConnection.recentSignalConnStates();

		ASSERT_EQ(adsConnStates.size(), 2);
		ASSERT_EQ(adsRecntStates.size(), 2);

		EXPECT_TRUE(adsConnStates[0].isConnected);
		EXPECT_TRUE(adsConnStates[1].isConnected);
		EXPECT_EQ(adsConnStates[0].peerAddr.toStdString(), AppDataServices[0].address.toStdString());
		EXPECT_EQ(adsConnStates[1].peerAddr.toStdString(), AppDataServices[1].address.toStdString());

		EXPECT_TRUE(adsRecntStates[0].isConnected);
		EXPECT_TRUE(adsRecntStates[1].isConnected);
		EXPECT_EQ(adsRecntStates[0].peerAddr.toStdString(), AppDataServices[0].address.toStdString());
		EXPECT_EQ(adsRecntStates[1].peerAddr.toStdString(), AppDataServices[1].address.toStdString());
	}

	return;
}

TEST_F(AdsConnectionTests, adsNoConnection)
{
	ILogFileStub log;
	MockAppSignalUpdater signalUpdater;

	SoftwareInfo softwareInfo(E::SoftwareType::Monitor, "SYSTEMID_CLIENTTEST_WS03_MONITOR");

	// MockAppSignalUpdater
	//
	EXPECT_CALL(signalUpdater, notifySignalParamsUpdated())
			.Times(0);

	EXPECT_CALL(signalUpdater, reset())
			.Times(1);	// 1 time in adsConnection.updateConnections.

	EXPECT_CALL(signalUpdater, invalidateSignalStates(_))
			.Times(0);

	EXPECT_CALL(signalUpdater, addSignals(_, _))
			.Times(0);

	EXPECT_CALL(signalUpdater, setState(_, _, _))
			.Times(0);

	// Start
	//
	{
		auto servers = AppDataServices;
		servers[0].address = {"192.178.12.90", 13323};		// Some unreachable addresses.
		servers[1].address = {"192.178.13.90", 13323};		//

		ClientLib::AdsConnection adsConnection{signalUpdater, nullptr, &log};
		adsConnection.updateConnections(softwareInfo, servers);

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
		std::vector<Tcp::ConnectionState> adsConnStates = adsConnection.tcpSignalConnStates();
		std::vector<Tcp::ConnectionState> adsRecntStates = adsConnection.recentSignalConnStates();

		ASSERT_EQ(adsConnStates.size(), 2);
		ASSERT_EQ(adsRecntStates.size(), 0);

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
	ClientLib::AppSignalManager signalManager{&log};
	SoftwareInfo softwareInfo(E::SoftwareType::Monitor, "SYSTEMID_CLIENTTEST_WS03_MONITOR");

	// Start
	//
	ClientLib::AdsConnection adsConnection{signalManager, &signalManager, &log};
	adsConnection.updateConnections(softwareInfo, AppDataServices);

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
		std::vector<Tcp::ConnectionState> adsConnStates = adsConnection.tcpSignalConnStates();
		if (std::all_of(adsConnStates.begin(), adsConnStates.end(), [](const auto& s) { return s.replyCount > 30; }))
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
	while (timer.hasExpired(4000) == false)
	{
		QThread::msleep(10);

		bool signalFound = false;
		AppSignalState state = signalManager.signalState("#SYSTEMID_CLIENTTEST_CH10_MD00_PI_BLINK", &signalFound);

		EXPECT_TRUE(signalFound);

		stateChanges += (lastState == state.value()) ? 0 : 1;
		lastState = state.value();
	}

	qDebug() << "TEST(AdsConnectionTests, receivesState): stateChanges of #SYSTEMID_CLIENTTEST_CH10_MD00_PI_BLINK: " << stateChanges;

	// The test machine can be loaded heavily, it will result in reduced numbers of state changes.
	//
	EXPECT_TRUE(stateChanges >= 3 && stateChanges < 12);

	return;
}
