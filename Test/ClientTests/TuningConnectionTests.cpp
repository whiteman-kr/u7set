#include "../../ClientLib/TuningConnection.h"

class TuningConnectionTests : public ::testing::Test
{
protected:
	virtual void SetUp()
	{
		s_tuningServices[0].clientRequestAddress.setPort(g_connectionPorts.tuningService1.clientRequestPort);
		s_tuningServices[1].clientRequestAddress.setPort(g_connectionPorts.tuningService2.clientRequestPort);
	}

	virtual void TearDown()
	{
	}

	inline static std::vector<SoftwareEndpoint::TuningService> s_tuningServices =
		{
			{
				.equipmentId = "SYSTEMID_CLIENTTEST_WS01_TUNS",
				.shortenId = "WS01_TUNS",
				.clientRequestAddress = {"127.0.0.1", 13333},
				.drivenSources = {},
				.singleLmControl = false
			},
			{
				.equipmentId = "SYSTEMID_CLIENTTEST_WS02_TUNS",
				.shortenId = "WS02_TUNS",
				.clientRequestAddress = {"127.0.0.1", 13334},
				.drivenSources = {},
				.singleLmControl = false
			}
		};

	inline static const SoftwareInfo s_softwareInfo = {E::SoftwareType::TuningClient, "SYSTEMID_CLIENTTEST_WS03_TUN", 1, 2, 3};
};

TEST_F(TuningConnectionTests, connect)
{
	TuningSignalManager signalManager;
	ClientLib::TuningUserManager userManager;

	ILogFileStub logFile;
	TuningLog::TuningLogStub tuningLog{userManager, {}};

	{
		ClientLib::TuningConnection tc{signalManager, &logFile, &tuningLog};
		tc.updateConnections(s_softwareInfo, s_tuningServices, true, TuningClientSettings::LmStatusFlagMode::None);

		// Wait for connection established
		//
		QElapsedTimer timer;
		timer.start();

		while (timer.hasExpired(5000) == false)
		{
			QCoreApplication::instance()->processEvents();
			QThread::msleep(10);

			// Wait for 20 replies, so all signals are loaded and some states are received.
			//
			std::vector<Tcp::ConnectionState> connStates = tc.tcpTuningConnStates();
			if (std::all_of(connStates.begin(), connStates.end(), [](const auto& s) { return s.isConnected; }))
			{
				break;
			}
		}

		// Check that two connections are established.
		//
		std::vector<Tcp::ConnectionState> connStates = tc.tcpTuningConnStates();

		ASSERT_EQ(connStates.size(), 2);

		EXPECT_TRUE(connStates[0].isConnected);
		EXPECT_TRUE(connStates[1].isConnected);

		EXPECT_EQ(connStates[0].peerAddr, s_tuningServices[0].clientRequestAddress);
		EXPECT_EQ(connStates[1].peerAddr, s_tuningServices[1].clientRequestAddress);
	}

	return;
}
