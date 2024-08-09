#include "ConnectionPorts.h"
#include <ClientLib/RtDataProvider.h>

using ::testing::_;
using ::testing::Eq;
using ::testing::Not;
using ::testing::Matcher;
using ::testing::AtLeast;
using ::testing::Return;

namespace
{
	class MockSignalDataServer : public ClientLib::ISignalDataServer
	{
	public:
		MOCK_METHOD(QStringList, dataServiceIds, (const QString& appSignalId), (const, override));
		MOCK_METHOD(bool, dataServiceHasSignal, (const QString& serviceEquipmentId, const QString& signalId), (const, override));
		MOCK_METHOD(bool, dataServiceHasSignal, (const QString& serviceEquipmentId, Hash signalHash), (const, override));
		MOCK_METHOD(std::vector<Hash>, dataServiceSignals, (const QString& serviceEquipmentId), (const, override));
	};
} // namespace


class RtDataProviderTests : public ::testing::Test
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

	inline static std::vector<SoftwareEndpoint::AppDataService> AppDataServices = {{.equipmentId = "SYSTEMID_CLIENTTEST_WS01_ADS_RC1",
																					.shortenId = "WS01_ADS",
																					.address = {"127.0.0.1", 13323},
																					.realtimeAddress = {"127.0.0.1", 13324}},
																				   {.equipmentId = "SYSTEMID_CLIENTTEST_WS02_ADS_RC1",
																					.shortenId = "WS02_ADS",
																					.address = {"127.0.0.1", 13326},
																					.realtimeAddress = {"127.0.0.1", 13327}}};

	ILogFileStub log;
};


TEST_F(RtDataProviderTests, connectToAds)
{
	MockSignalDataServer signalDataServer;
	SoftwareInfo softwareInfo(E::SoftwareType::Monitor, "SYSTEMID_CLIENTTEST_WS03_MONITOR");

	// Start
	//
	ClientLib::RtDataProvider adsRtConnection{signalDataServer, &log};
	adsRtConnection.updateConnections(softwareInfo, AppDataServices);

	// Wait for connection established
	//
	QElapsedTimer timer;
	timer.start();

	while (adsRtConnection.allConnected(std::chrono::milliseconds{20}) == false && timer.hasExpired(5'000) == false)
	{
		QCoreApplication::instance()->processEvents();
		QThread::yieldCurrentThread();
	}

	ASSERT_TRUE(adsRtConnection.allConnected(std::chrono::milliseconds{20}));

	// Check that two connections are established.
	//
	auto connStates = adsRtConnection.statistics();
	ASSERT_EQ(connStates.isConnected, 2);

	adsRtConnection.clear(); // Stop connection
	EXPECT_EQ(adsRtConnection.size(), 0);

	connStates = adsRtConnection.statistics();
	EXPECT_EQ(connStates.isConnected, 0);

	return;
}

TEST_F(RtDataProviderTests, adsNoConnection)
{
	MockSignalDataServer signalDataServer;
	SoftwareInfo softwareInfo(E::SoftwareType::Monitor, "SYSTEMID_CLIENTTEST_WS03_MONITOR");

	// Start
	//
	auto servers = AppDataServices;
	servers[0].realtimeAddress = {"192.178.12.90", 43323}; // Some unreachable addresses.
	servers[1].realtimeAddress = {"192.178.13.90", 43323}; //

	ClientLib::RtDataProvider adsRtConnection{signalDataServer, &log};
	adsRtConnection.updateConnections(softwareInfo, servers);

	EXPECT_EQ(adsRtConnection.size(), 2);

	QElapsedTimer timer;
	timer.start();

	while (adsRtConnection.allConnected(std::chrono::milliseconds{20}) == false && timer.hasExpired(2'000) == false)
	{
		QCoreApplication::instance()->processEvents();
		QThread::yieldCurrentThread();
	}

	// Connection should not be established.
	//
	ASSERT_FALSE(adsRtConnection.allConnected(std::chrono::milliseconds{20}));

	auto connStates = adsRtConnection.statistics();
	ASSERT_EQ(connStates.isConnected, 0);

	return;
}

TEST_F(RtDataProviderTests, ads1Connection)
{
	MockSignalDataServer signalDataServer;

	SoftwareInfo softwareInfo(E::SoftwareType::Monitor, "SYSTEMID_CLIENTTEST_WS03_MONITOR");

	// Start
	//
	auto servers = AppDataServices;
	servers[1].realtimeAddress = {"192.178.13.90", 43323}; // Make one address unreachable.

	ClientLib::RtDataProvider adsRtConnection{signalDataServer, &log};
	adsRtConnection.updateConnections(softwareInfo, servers);

	EXPECT_EQ(adsRtConnection.size(), 2);

	QElapsedTimer timer;
	timer.start();

	while (adsRtConnection.allConnected(std::chrono::milliseconds{20}) == false && timer.hasExpired(5'000) == false)
	{
		QCoreApplication::instance()->processEvents();
		QThread::yieldCurrentThread();
	}

	// Connection should not be established.
	//
	ASSERT_FALSE(adsRtConnection.allConnected(std::chrono::milliseconds{20}));

	// Only one connection should be established.
	//
	auto connStates = adsRtConnection.statistics();
	ASSERT_EQ(connStates.isConnected, 1);

	return;
}

TEST_F(RtDataProviderTests, discreteData)
{
	QString softwareId{"#SYSTEMID_CLIENTTEST_WS03_MONITOR"};

	QString appSignalId{"#CT_RT_NOT_0101"}; // Expected pattern is 01010101....

	MockSignalDataServer signalDataServer;

	EXPECT_CALL(signalDataServer, dataServiceHasSignal(_, Matcher<const QString&>(Not(Eq(appSignalId)))))
		.Times(0);

	EXPECT_CALL(signalDataServer, dataServiceHasSignal(_, Matcher<const QString&>(Eq(appSignalId))))
		.Times(AtLeast(20))
		.WillRepeatedly(Return(true));

	// Start connection
	//
	SoftwareInfo softwareInfo(E::SoftwareType::Monitor, softwareId);

	ClientLib::RtDataProvider adsRtConnection{signalDataServer, &log};
	adsRtConnection.updateConnections(softwareInfo, AppDataServices);

	EXPECT_EQ(adsRtConnection.size(), 2);

	{
		QElapsedTimer timer;
		timer.start();

		while (adsRtConnection.allConnected(std::chrono::milliseconds{20}) == false && timer.hasExpired(5'000) == false)
		{
			QCoreApplication::instance()->processEvents();
			QThread::yieldCurrentThread();
		}

		ASSERT_TRUE(adsRtConnection.allConnected(std::chrono::milliseconds{20}));
	}

	// Check signal patters.
	//
	adsRtConnection.setData(E::RtTrendsSamplePeriod::sp_5ms, {appSignalId});
	adsRtConnection.setSamplePeriod(E::RtTrendsSamplePeriod::sp_10s); // Should not influence to the data as discrete data is not apperture based.

	std::mutex dataMutex;
	std::deque<TrendLib::TrendStateItem> states0; // States received from AppDataServices[0]
	std::deque<TrendLib::TrendStateItem> states1; // States received from AppDataServices[1]

	class Receiver : public QObject
	{
		std::mutex& dataMutex;
		std::deque<TrendLib::TrendStateItem>& states0;
		std::deque<TrendLib::TrendStateItem>& states1;

	public:
		Receiver(std::mutex& dataMutex, std::deque<TrendLib::TrendStateItem>& states0, std::deque<TrendLib::TrendStateItem>& states1) :
			dataMutex(dataMutex),
			states0(states0),
			states1(states1)
		{
		}

	public slots:
		void dataReceiver(QString sourceEquipmentId,
						  std::shared_ptr<TrendLib::RealtimeData> data,
						  E::RtTrendsSamplePeriod /*samplePeriod*/,
						  TrendLib::TrendStateItem /*minState*/,
						  TrendLib::TrendStateItem /*maxState*/)
		{
			ASSERT_TRUE(data);
			EXPECT_TRUE(sourceEquipmentId == AppDataServices[0].equipmentId || sourceEquipmentId == AppDataServices[1].equipmentId);

			Hash expectedHash = calcHash(QString("#CT_RT_NOT_0101"));

			std::scoped_lock l(dataMutex);

			if (sourceEquipmentId == AppDataServices[0].equipmentId)
			{
				for (const auto& chunk : data->signalData)
				{
					EXPECT_EQ(chunk.appSignalHash, expectedHash);
					copy(begin(chunk.states), end(chunk.states), back_inserter(states0)); // insert states to the end of the states0
				}
			}

			if (sourceEquipmentId == AppDataServices[1].equipmentId)
			{
				for (const auto& chunk : data->signalData)
				{
					EXPECT_EQ(chunk.appSignalHash, expectedHash);
					copy(begin(chunk.states), end(chunk.states), back_inserter(states1)); // insert states to the end of the states1
				}
			}
		}
	};

	Receiver receiver{dataMutex, states0, states1};

	receiver.connect(&adsRtConnection, &ClientLib::RtDataProvider::dataReady, &receiver, &Receiver::dataReceiver, Qt::DirectConnection);

	QElapsedTimer timer;
	timer.start();

	TrendLib::TrendStateItem lastState0{};
	TrendLib::TrendStateItem lastState1{};
	int statesProcessed0{};
	int statesProcessed1{};

	while (timer.hasExpired(3'000) == false)
	{
		QCoreApplication::instance()->processEvents();
		QThread::yieldCurrentThread();

		std::scoped_lock l(dataMutex);
		while (states0.empty() == false)
		{
			auto state = states0.front();
			states0.pop_front();

			// qDebug() << state.plant << " " << state.value;

			if (lastState0.plant == 0)
			{
				lastState0 = state;
				continue;
			}

			if (lastState0.plant != state.plant)
			{
				EXPECT_EQ(state.plant - lastState0.plant, 5);
				EXPECT_NE(state.value, lastState0.value);

				statesProcessed0++;
			}

			lastState0 = state;
			EXPECT_TRUE(lastState0.value == 0.0 || lastState0.value == 1.0);
		}

		while (states1.empty() == false)
		{
			auto state = states1.front();
			states1.pop_front();

			// qDebug() << state.plant << " " << state.value;

			if (lastState1.plant == 0)
			{
				lastState1 = state;
				continue;
			}

			if (lastState1.plant != state.plant)
			{
				EXPECT_EQ(state.plant - lastState1.plant, 5);
				EXPECT_NE(state.value, lastState1.value);

				statesProcessed1++;
			}

			lastState1 = state;
			EXPECT_TRUE(lastState1.value == 0.0 || lastState1.value == 1.0);
		}
	}

	EXPECT_GT(statesProcessed0, 10);
	EXPECT_GT(statesProcessed1, 10);

	return;
}

TEST_F(RtDataProviderTests, mixedData)
{
	QString softwareId{"#SYSTEMID_CLIENTTEST_WS03_MONITOR"};

	QString discreteSignalId{"#CT_RT_NOT_0101"}; // Expected pattern is 01010101....
	QString intSignalId{"#CT_RT_ADDSI2"};        // Expected pattern is 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, ...
	QString floatSignalId{
		"#CT_RT_ADDFP"}; // Expected pattern is 0.01, 0.02, 0.03, 0.04, 0.05, 0.06, 0.07, 0.08, 0.09, 0.10, 0.11, 0.12, ...

	MockSignalDataServer signalDataServer;

	EXPECT_CALL(signalDataServer, dataServiceHasSignal(_, Matcher<const QString&>(Eq(discreteSignalId))))
		.Times(AtLeast(20))
		.WillRepeatedly(Return(true));

	EXPECT_CALL(signalDataServer, dataServiceHasSignal(_, Matcher<const QString&>(Eq(intSignalId))))
		.Times(AtLeast(20))
		.WillRepeatedly(Return(true));

	EXPECT_CALL(signalDataServer, dataServiceHasSignal(_, Matcher<const QString&>(Eq(floatSignalId))))
		.Times(AtLeast(20))
		.WillRepeatedly(Return(true));

	// Start connection
	//
	SoftwareInfo softwareInfo(E::SoftwareType::Monitor, softwareId);

	ClientLib::RtDataProvider adsRtConnection{signalDataServer, &log};
	adsRtConnection.updateConnections(softwareInfo, AppDataServices);

	EXPECT_EQ(adsRtConnection.size(), 2);

	{
		QElapsedTimer timer;
		timer.start();

		while (adsRtConnection.allConnected(std::chrono::milliseconds{20}) == false && timer.hasExpired(5'000) == false)
		{
			QCoreApplication::instance()->processEvents();
			QThread::yieldCurrentThread();
		}

		ASSERT_TRUE(adsRtConnection.allConnected(std::chrono::milliseconds{20}));
	}

	// Check signal patters.
	//
	adsRtConnection.setData(E::RtTrendsSamplePeriod::sp_5ms, QStringList{discreteSignalId, intSignalId, floatSignalId});

	struct Data
	{
		using States = std::deque<TrendLib::TrendStateItem>;
		std::map<Hash, States> states; // Key is mix of adsEqiupmentId and signalId, value is states

		Hash mapKey(const QString adsEqiupmentId, QString signalId) { return mapKey(adsEqiupmentId, ::calcHash(signalId)); }

		Hash mapKey(const QString& adsEqiupmentId, Hash signalHash) { return ::calcHash(adsEqiupmentId, signalHash); }

		States& getStates(const QString& adsEqiupmentId, QString signalId) { return states[mapKey(adsEqiupmentId, signalId)]; }

		States& getStates(const QString& adsEqiupmentId, Hash signalHash) { return states[mapKey(adsEqiupmentId, signalHash)]; }
	};

	std::mutex dataMutex;
	Data receivedData;

	class Receiver : public QObject
	{
		std::mutex& dataMutex;
		Data& receivedData;

	public:
		Receiver(std::mutex& dataMutex, Data& receivedData) :
			dataMutex(dataMutex),
			receivedData(receivedData)
		{
		}

	public slots:
		void dataReceiver(QString sourceEquipmentId,
						  std::shared_ptr<TrendLib::RealtimeData> data,
						  E::RtTrendsSamplePeriod /*samplePeriod*/,
						  TrendLib::TrendStateItem /*minState*/,
						  TrendLib::TrendStateItem /*maxState*/)
		{
			ASSERT_TRUE(data);
			EXPECT_TRUE(sourceEquipmentId == AppDataServices[0].equipmentId || sourceEquipmentId == AppDataServices[1].equipmentId);

			std::scoped_lock l(dataMutex);

			for (const auto& chunk : data->signalData)
			{
				auto& states = receivedData.getStates(sourceEquipmentId, chunk.appSignalHash);
				copy(begin(chunk.states), end(chunk.states), back_inserter(states)); // insert states to the end of the states0
			}
		}
	};

	Receiver receiver{dataMutex, receivedData};

	receiver.connect(&adsRtConnection, &ClientLib::RtDataProvider::dataReady, &receiver, &Receiver::dataReceiver, Qt::DirectConnection);

	QElapsedTimer timer;
	timer.start();

	TrendLib::TrendStateItem lastState0{};
	TrendLib::TrendStateItem lastState1{};

	while (timer.hasExpired(3'000) == false)
	{
		QCoreApplication::instance()->processEvents();
		QThread::yieldCurrentThread();
	}

	adsRtConnection.clear(); // Stop connections.

	std::scoped_lock l(dataMutex);

	EXPECT_EQ(receivedData.states.size(), 6);

	// Ads1, discrete signal
	{
		auto& states = receivedData.getStates(AppDataServices[0].equipmentId, discreteSignalId);
		EXPECT_GT(states.size(), 10);

		TrendLib::TrendStateItem lastState{};
		for (const auto& state : states)
		{
			if (lastState.plant == 0)
			{
				lastState = state;
				continue;
			}

			if (lastState.plant != state.plant)
			{
				EXPECT_EQ(state.plant - lastState.plant, 5);
				EXPECT_NE(state.value, lastState.value);
			}

			lastState = state;
			EXPECT_TRUE(lastState.value == 0.0 || lastState.value == 1.0);
		}

		states.clear();
	}

	// Ads2, discrete signals
	{
		auto& states = receivedData.getStates(AppDataServices[1].equipmentId, discreteSignalId);
		EXPECT_GT(states.size(), 10);

		TrendLib::TrendStateItem lastState{};
		for (const auto& state : states)
		{
			if (lastState.plant == 0)
			{
				lastState = state;
				continue;
			}

			if (lastState.plant != state.plant)
			{
				EXPECT_EQ(state.plant - lastState.plant, 5);
				EXPECT_NE(state.value, lastState.value);
			}

			lastState = state;
			EXPECT_TRUE(lastState.value == 0.0 || lastState.value == 1.0);
		}

		states.clear();
	}

	// Ads1, intSignalId
	{
		auto& states = receivedData.getStates(AppDataServices[0].equipmentId, intSignalId);
		EXPECT_GT(states.size(), 10);

		TrendLib::TrendStateItem lastState{};
		for (const auto& state : states)
		{
			if (lastState.plant == 0)
			{
				lastState = state;
				continue;
			}

			if (lastState.plant != state.plant)
			{
				EXPECT_EQ(state.plant - lastState.plant, 5);
				EXPECT_EQ(state.value, lastState.value + 2);
			}

			lastState = state;
		}

		states.clear();
	}

	// Ads2, intSignalId
	{
		auto& states = receivedData.getStates(AppDataServices[1].equipmentId, intSignalId);
		EXPECT_GT(states.size(), 10);

		TrendLib::TrendStateItem lastState{};
		for (const auto& state : states)
		{
			if (lastState.plant == 0)
			{
				lastState = state;
				continue;
			}

			if (lastState.plant != state.plant)
			{
				EXPECT_EQ(state.plant - lastState.plant, 5);
				EXPECT_EQ(state.value, lastState.value + 2);
			}

			lastState = state;
		}

		states.clear();
	}

	// Ads1, floatSignalId
	{
		auto& states = receivedData.getStates(AppDataServices[0].equipmentId, floatSignalId);
		EXPECT_GT(states.size(), 10);

		TrendLib::TrendStateItem lastState{};
		for (const auto& state : states)
		{
			if (lastState.plant == 0)
			{
				lastState = state;
				continue;
			}

			if (lastState.plant != state.plant)
			{
				EXPECT_EQ(state.plant - lastState.plant, 5);
				EXPECT_NEAR(state.value, lastState.value + 0.01, 0.00001);
			}

			lastState = state;
		}

		states.clear();
	}

	// Ads2, floatSignalId
	{
		auto& states = receivedData.getStates(AppDataServices[1].equipmentId, floatSignalId);
		EXPECT_GT(states.size(), 10);

		TrendLib::TrendStateItem lastState{};
		for (const auto& state : states)
		{
			if (lastState.plant == 0)
			{
				lastState = state;
				continue;
			}

			if (lastState.plant != state.plant)
			{
				EXPECT_EQ(state.plant - lastState.plant, 5);
				EXPECT_NEAR(state.value, lastState.value + 0.01, 0.00001);
			}

			lastState = state;
		}

		states.clear();
	}

	// Check that no left data.
	//
	for (auto& d : receivedData.states)
	{
		EXPECT_TRUE(d.second.empty());
	}

	return;
}