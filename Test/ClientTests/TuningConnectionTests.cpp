#include "../../ClientLib/TuningConnection.h"

using ::testing::_;
using ::testing::AtLeast;

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

	inline static std::vector<SoftwareEndpoint::TuningService> s_safeTuningServices =
		{
			{
				.equipmentId = "SYSTEMID_CLIENTTEST_WS04_TUNS",
				.shortenId = "WS04_TUNS",
				.clientRequestAddress = {"127.0.0.1", 13335},
				.drivenSources = {},
				.singleLmControl = true
			}
		};

	inline static const SoftwareInfo s_softwareInfo = {E::SoftwareType::TuningClient, "SYSTEMID_CLIENTTEST_WS03_TUN", 1, 2, 3};
	inline static const SoftwareInfo s_safeSoftwareInfoA = {E::SoftwareType::TuningClient, "SYSTEMID_CLIENTTEST_WS04_TUNA", 1, 2, 3};
	inline static const SoftwareInfo s_safeSoftwareInfoB = {E::SoftwareType::TuningClient, "SYSTEMID_CLIENTTEST_WS04_TUNB", 1, 2, 3};
};


class MockITuningSignalManager: public ITuningSignalManager
{
public:
	MOCK_METHOD(bool, signalExists, (Hash hash), (const override));
	MOCK_METHOD(bool, signalExists, (const QString& appSignalId), (const override));

	MOCK_METHOD(AppSignalParam, signalParam, (Hash hash, bool* found), (const override));
	MOCK_METHOD(AppSignalParam, signalParam, (const QString& appSignalId, bool* found), (const override));

	MOCK_METHOD(bool, signalParam, (Hash hash, AppSignalParam* result), (const override));
	MOCK_METHOD(bool, signalParam, (const QString& appSignalId, AppSignalParam* result), (const override));

	MOCK_METHOD(TuningSignalState, state, (Hash hash, bool* found), (const override));
	MOCK_METHOD(TuningSignalState, state, (const QString& appSignalId, bool* found), (const override));

	MOCK_METHOD(TuningSignalState, state, (Hash hash, Hash tuningServiceHash, bool* found), (const override));
	MOCK_METHOD(TuningSignalState, state, (const QString& appSignalId, Hash tuningServiceHash, bool* found), (const override));

	MOCK_METHOD(QStringList, signalIdsByTag, (const QString& tag), (const override));
};

class MockITuningSignalUpdater : public ITuningSignalUpdater
{
public:
	MOCK_METHOD(void, reset, (), (override));

	MOCK_METHOD(std::vector<Hash>, signalHashes, (), (const, override));
	MOCK_METHOD(std::vector<Hash>, signalHashes, (const std::vector<Hash> lmEquipmentIdHashes), (const, override));

	MOCK_METHOD(void, invalidateSignalStates, (Hash tuningServiceHash), (override));

	MOCK_METHOD(void, setState, (const TuningSignalState& state, Hash tuningServiceHash), (override));
	MOCK_METHOD(void, setStates, (const std::vector<TuningSignalState>& states, Hash tuningServiceHash), (override));

	MOCK_METHOD(void, notifySignalParamsUpdated, (), (override));
};


TEST_F(TuningConnectionTests, connect)
{
	ILogFileStub logFile;

	//TuningSignalManager signalManager{s_softwareInfo.equipmentID(), &logFile};

	MockITuningSignalManager signalManager{};
	MockITuningSignalUpdater signalUpdater{};

	EXPECT_CALL(signalUpdater, invalidateSignalStates(::calcHash(s_tuningServices[0].equipmentId)))
			.Times(1);	// 1 times, when connection to TuningService is closed;

	EXPECT_CALL(signalUpdater, invalidateSignalStates(::calcHash(s_tuningServices[1].equipmentId)))
			.Times(1);	// 1 times, when connection to TuningService is closed;

	std::vector<Hash> lmHashes = {::calcHash(QStringLiteral("SYSTEMID_CLIENTTEST_CH11_MD00"))};
	EXPECT_CALL(signalUpdater, signalHashes(lmHashes))
			.Times(2);

	ClientLib::TuningUserManager userManager;
	TuningLog::TuningLogStub tuningLog{userManager, {}};

	{
		ClientLib::TuningConnection tc{signalManager, signalUpdater, &logFile, &tuningLog};
		tc.updateConnections(s_softwareInfo, s_tuningServices, true, TuningClientSettings::LmStatusFlagMode::None);

		// Wait for connection established
		//
		QElapsedTimer timer;
		timer.start();

		while (timer.hasExpired(5000) == false)
		{
			QCoreApplication::instance()->processEvents();
			QThread::msleep(10);

			// Wait for several replies
			//
			std::vector<Tcp::ConnectionState> connStates = tc.tcpTuningConnStates();
			if (std::all_of(connStates.begin(), connStates.end(), [](const auto& s) { return s.isConnected && s.replyCount > 2; }))
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

TEST_F(TuningConnectionTests, tuningSourceInfo)
{
	ILogFileStub logFile;

	TuningSignalManager signalManager{s_safeSoftwareInfoA.equipmentID(), &logFile};

	ClientLib::TuningUserManager userManager;
	TuningLog::TuningLogStub tuningLog{userManager, {}};

	Hash lmHash = {::calcHash(QStringLiteral("SYSTEMID_CLIENTTEST_CH12_MD00"))};

	{
		ClientLib::TuningConnection tc{signalManager, signalManager, &logFile, &tuningLog};
		tc.updateConnections(s_safeSoftwareInfoA, s_safeTuningServices, true, TuningClientSettings::LmStatusFlagMode::None);

		// Wait for connection established
		//
		{
			QElapsedTimer timer;
			timer.start();

			while (timer.hasExpired(5000) == false)
			{
				QCoreApplication::instance()->processEvents();
				QThread::msleep(10);

				// Wait for several replies
				//
				std::vector<Tcp::ConnectionState> connStates = tc.tcpTuningConnStates();
				if (std::all_of(connStates.begin(), connStates.end(), [](const auto& s) { return s.isConnected && s.replyCount > 2; }))
				{
					break;
				}
			}
		}

		// Check that one connection is established.
		//
		std::vector<Tcp::ConnectionState> connStates = tc.tcpTuningConnStates();

		ASSERT_EQ(connStates.size(), 1);

		EXPECT_TRUE(connStates[0].isConnected);

		EXPECT_EQ(connStates[0].peerAddr, s_safeTuningServices[0].clientRequestAddress);

		// Check that tuning source info is arrived

		std::vector<ClientLib::TuningSource> allSourcesInfo = tc.tuningSourcesInfo();
		EXPECT_EQ(allSourcesInfo.size(), 1);

		std::vector<ClientLib::TuningSource> sourceInfo = tc.tuningSourceInfo(lmHash);
		EXPECT_EQ(sourceInfo.size(), 1);

		for (const ClientLib::TuningSource& si : sourceInfo)
		{
			// Check the contents of TuningSource

			EXPECT_TRUE(si.id() != 0);
			EXPECT_EQ(si.valid(), true);

			EXPECT_EQ(lmHash, ::calcHash(si.equipmentId()));

			EXPECT_EQ(si.controllersCount(), 1);
			EXPECT_EQ(si.controllerEquipmentId(0), "SYSTEMID_CLIENTTEST_CH12_MD00_ETHERNET02");

			EXPECT_EQ(si.statesCount(), 1);

			const ::Network::TuningSourceState& st1 = si.state(0);
			const ::Network::TuningSourceState& st2 = si.state(::calcHash(si.controllerEquipmentId(0)));

			EXPECT_EQ(st1.sourceid(), st2.sourceid());

			EXPECT_FALSE(st1.isreply());	// Sources are not active, so isReply should be false
			EXPECT_FALSE(st2.isreply());
		}

		// Check source activation functions

		EXPECT_EQ(tc.tuningSourceStatesCount(lmHash), 1);

		EXPECT_EQ(tc.activatedTuningSourceStatesCount(lmHash), 0);	// No sources are active

		EXPECT_TRUE(tc.activateTuningSource(lmHash, true));

		// Wait for source is activated
		//
		{
			QElapsedTimer timer;
			timer.start();

			while (timer.hasExpired(5000) == false)
			{
				QCoreApplication::instance()->processEvents();
				QThread::msleep(10);

				if (tc.activatedTuningSourceStatesCount(lmHash) == 1)
				{
					break;
				}
			}
		}

		// Check that source has been activated
		//
		sourceInfo = tc.tuningSourceInfo(lmHash);
		EXPECT_EQ(sourceInfo.size(), 1);

		for (const ClientLib::TuningSource& si : sourceInfo)
		{
			const ::Network::TuningSourceState& st = si.state(::calcHash(si.controllerEquipmentId(0)));
			EXPECT_TRUE(st.isreply());
		}

		EXPECT_TRUE(tc.activateTuningSource(lmHash, false));

		// Wait for source is deactivated
		//
		{
			QElapsedTimer timer;
			timer.start();

			while (timer.hasExpired(5000) == false)
			{
				QCoreApplication::instance()->processEvents();
				QThread::msleep(10);

				if (tc.activatedTuningSourceStatesCount(lmHash) == 0)
				{
					break;
				}
			}
		}

		// Check that source has been deactivated
		//
		sourceInfo = tc.tuningSourceInfo(lmHash);
		EXPECT_EQ(sourceInfo.size(), 1);

		for (const ClientLib::TuningSource& si : sourceInfo)
		{
			const ::Network::TuningSourceState& st = si.state(::calcHash(si.controllerEquipmentId(0)));
			EXPECT_TRUE(st.isreply() == false);
		}
	}

	return;
}
