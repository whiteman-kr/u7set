#include "../../ClientLib/AdsSourceStateConnection.h"
#include "ConnectionPorts.h"


class AdsSourceStateConnectionTests : public ::testing::Test
{
protected:
	virtual void SetUp()
	{
		s_appDataServices[0].address.setPort(g_connectionPorts.ads1.clientRequestPort);
		s_appDataServices[0].realtimeAddress.setPort(g_connectionPorts.ads1.rtTrendsRequestPort);

		s_appDataServices[1].address.setPort(g_connectionPorts.ads2.clientRequestPort);
		s_appDataServices[1].realtimeAddress.setPort(g_connectionPorts.ads2.rtTrendsRequestPort);
	}

	virtual void TearDown()
	{
	}

	inline static std::vector<SoftwareEndpoint::AppDataService> s_appDataServices =
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


TEST_F(AdsSourceStateConnectionTests, connectToAds)
{
	ILogFileStub log;

	SoftwareInfo softwareInfo(E::SoftwareType::Monitor, "SYSTEMID_CLIENTTEST_WS03_MONITOR");

	// Start
	//
	ClientLib::AdsSourceStateConnection adsConnection{&log};
	adsConnection.updateConnections(softwareInfo, s_appDataServices);

	// Wait for connection established
	//
	QElapsedTimer timer;
	timer.start();

	while (timer.hasExpired(3000) == false)
	{
		QCoreApplication::instance()->processEvents();
		QThread::msleep(10);

		std::vector<Tcp::ConnectionState> adsConnStates = adsConnection.adsConnectionStates();
		if (std::all_of(adsConnStates.begin(), adsConnStates.end(), [](const auto& s) { return s.replyCount > 10; }))
		{
			break;
		}
	}

	// Check that two connections are established.
	//
	std::vector<Tcp::ConnectionState> adsConnStates = adsConnection.adsConnectionStates();

	ASSERT_EQ(adsConnStates.size(), 2);

	EXPECT_TRUE(adsConnStates[0].isConnected);
	EXPECT_TRUE(adsConnStates[1].isConnected);
	EXPECT_EQ(adsConnStates[0].peerAddr.toStdString(), s_appDataServices[0].address.toStdString());
	EXPECT_EQ(adsConnStates[1].peerAddr.toStdString(), s_appDataServices[1].address.toStdString());

	return;
}

TEST_F(AdsSourceStateConnectionTests, adsNoConnection)
{
	ILogFileStub log;

	SoftwareInfo softwareInfo(E::SoftwareType::Monitor, "SYSTEMID_CLIENTTEST_WS03_MONITOR");

	// Start
	//
	auto servers = s_appDataServices;
	servers[0].address = {"192.178.12.90", 13323};		// Some unreachable addresses.
	servers[1].address = {"192.178.13.90", 13323};		//

	ClientLib::AdsSourceStateConnection adsConnection{&log};
	adsConnection.updateConnections(softwareInfo, servers);

	// Wait for connection established
	//
	QElapsedTimer timer;
	timer.start();

	while (timer.hasExpired(3000) == false)
	{
		QCoreApplication::instance()->processEvents();
		QThread::msleep(10);

		std::vector<Tcp::ConnectionState> adsConnStates = adsConnection.adsConnectionStates();
		if (std::all_of(adsConnStates.begin(), adsConnStates.end(), [](const auto& s) { return s.replyCount > 10; }))
		{
			break;
		}
	}

	// Check that two connections are established.
	//
	std::vector<Tcp::ConnectionState> adsConnStates = adsConnection.adsConnectionStates();

	ASSERT_EQ(adsConnStates.size(), 2);

	EXPECT_FALSE(adsConnStates[0].isConnected);
	EXPECT_FALSE(adsConnStates[1].isConnected);

	return;
}

TEST_F(AdsSourceStateConnectionTests, receiveSourceStates)
{
	ILogFileStub log;

	SoftwareInfo softwareInfo(E::SoftwareType::Monitor, "SYSTEMID_CLIENTTEST_WS03_MONITOR");

	// Start
	//
	ClientLib::AdsSourceStateConnection adsConnection{&log};
	adsConnection.updateConnections(softwareInfo, s_appDataServices);

	// Wait for connection established
	//
	QElapsedTimer timer;
	timer.start();

	while (timer.hasExpired(3000) == false)
	{
		QCoreApplication::instance()->processEvents();
		QThread::msleep(10);

		std::vector<Tcp::ConnectionState> adsConnStates = adsConnection.adsConnectionStates();
		if (std::all_of(adsConnStates.begin(), adsConnStates.end(), [](const auto& s) { return s.replyCount > 20; }))
		{
			break;
		}
	}

	// Check that two connections are established.
	//
	std::vector<Tcp::ConnectionState> adsConnStates = adsConnection.adsConnectionStates();

	ASSERT_EQ(adsConnStates.size(), 2);
	EXPECT_TRUE(adsConnStates[0].isConnected);
	EXPECT_TRUE(adsConnStates[1].isConnected);

	// Get source states
	//
	std::vector<ClientLib::AppDataSourceState> sources = adsConnection.appDataSourceStates();

	// Tests work only with ClientTests part in the project database. No simulator tests interference.
	//
	std::erase_if(sources, [](const auto& source) {	return source.equipmentId().contains("CLIENTTEST") == false;});

	for (const ClientLib::AppDataSourceState& source : sources)
	{
		EXPECT_TRUE(source.valid());
		EXPECT_EQ(source.state.receivesdata(), true);
		EXPECT_GT(source.state.receivedframescount(), 0);
	}

	return;
}
