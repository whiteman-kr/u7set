// Functional tests for class ClientLib::Config controller
// ConfigurationService must be ranning on localhost and default port
//
#include "../../ClientLib/IAppSignalUpdater.h"
#include "../../ClientLib/IRecentAppSignals.h"
#include "../../ClientLib/AdsConnection.h"

using ::testing::_;
using ::testing::AtLeast;

namespace
{
	static const std::vector<SoftwareEndpoint::AppDataService> AppDataServices =
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


	class MockAppSignalUpdater : public ClientLib::IAppSignalUpdater
	{
	public:
		MOCK_METHOD(void, reset, (), (override));
		MOCK_METHOD(void, notifySignalParamsUpdated, (), (override));
		MOCK_METHOD(void, addSignal, (const AppSignalParam& appSignal, const QString& appDataServiceId), (override));
		MOCK_METHOD(void, addSignals, (const std::vector<AppSignalParam>& appSignals, const QString& appDataServiceId), (override));
		MOCK_METHOD(void, invalidateSignalStates, (Qt::HANDLE sourceThreadId), (override));
		MOCK_METHOD(void, setState, (const QString& appSignalId, const AppSignalState& state, Qt::HANDLE sourceThreadId), (override));
		MOCK_METHOD(void, setState, (Hash signalHash, const AppSignalState& state, Qt::HANDLE sourceThreadId), (override));
		MOCK_METHOD(void, setState, (const std::vector<AppSignalState>& states, Qt::HANDLE sourceThreadId), (override));
	};

	class MockRecentAppSignals : public ClientLib::IRecentAppSignals
	{
	public:
		MOCK_METHOD(void, addRecentAppSignal, (Hash h), (override));
		MOCK_METHOD(void, addRecentAppSignals, (const std::vector<Hash>& hashes), (override));
		MOCK_METHOD(std::vector<Hash>, recentlyUsedAppSignals, (const QString& appDataServivceId), (override));
	};
}


TEST(AdsConnectionTests, connectToAds)
{
	ILogFileStub log;
	MockAppSignalUpdater signalUpdater;

	SoftwareInfo softwareInfo;
	softwareInfo.init(E::SoftwareType::Monitor, "SYSTEMID_CLIENTTEST_WS03_MONITOR", 0, 0);

	EXPECT_CALL(signalUpdater, reset())
			.Times(1);	// 1 time in adsConnection.updateConnections

	EXPECT_CALL(signalUpdater, invalidateSignalStates(_))
			.Times(2);	// 2 times on disconnect two ADSs.

	{
		ClientLib::AdsConnection adsConnection{signalUpdater, nullptr, &log};
		adsConnection.updateConnections(softwareInfo, AppDataServices);

		// Wait for connection established
		//
		QElapsedTimer timer;
		timer.start();

		while (timer.hasExpired(3000) == false)
		{
			QCoreApplication::instance()->processEvents();
			QThread::msleep(10);

			std::vector<Tcp::ConnectionState> adsConnStates = adsConnection.tcpSignalConnStates();
			if (std::all_of(adsConnStates.begin(), adsConnStates.end(), [](const auto& s) { return s.isConnected; }))
			{
				break;
			}
		}

		// Check that two connections are established
		//
		std::vector<Tcp::ConnectionState> adsConnStates = adsConnection.tcpSignalConnStates();
		std::vector<Tcp::ConnectionState> adsRecntStates = adsConnection.recentSignalConnStates();

		ASSERT_EQ(adsConnStates.size(), 2);
		ASSERT_EQ(adsRecntStates.size(), 0);

		EXPECT_TRUE(adsConnStates[0].isConnected);
		EXPECT_TRUE(adsConnStates[1].isConnected);

		EXPECT_EQ(adsConnStates[0].peerAddr.toStdString(), AppDataServices[0].address.toStdString());
		EXPECT_EQ(adsConnStates[1].peerAddr.toStdString(), AppDataServices[1].address.toStdString());
	}

	return;
}
