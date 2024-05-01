#include <ClientLib/TuningSignalManager.h>
#include <ClientLib/IRecentAppSignals.h>
#include <ClientLib/ITuningLog.h>
#include <ClientLib/TuningConnection.h>
#include "ConnectionPorts.h"

using ::testing::_;
using ::testing::AtLeast;

class TuningConnectionTests : public ::testing::Test
{
protected:
	virtual void SetUp()
	{
		s_tuningServices[0].clientRequestAddress.setPort(g_connectionPorts.tuningService1.clientRequestPort);
		s_tuningServices[1].clientRequestAddress.setPort(g_connectionPorts.tuningService2.clientRequestPort);

		s_safeTuningServices[0].clientRequestAddress.setPort(g_connectionPorts.tuningService3.clientRequestPort);

		protoSignalSet.Clear();

		{
			asFloat.setAppSignalID("#CLIENTTEST_TUNING_AF1");
			asFloat.setHash(::calcHash(asFloat.appSignalID()));
			asFloat.setSignalType(E::SignalType::Analog);
			asFloat.setAnalogSignalFormat(E::AnalogAppSignalFormat::Float32);
			TuningValue tvLow{{TuningValueType::Float}, 0};
			TuningValue tvHigh{{TuningValueType::Float}, 100};
			asFloat.setTuningLowBound(tvLow);
			asFloat.setTuningHighBound(tvHigh);
			asFloat.setTuningDefaultValue(tvLow);
			asFloat.setEnableTuning(true);
			asFloat.setLmEquipmentID("SYSTEMID_CLIENTTEST_CH11_MD00");

			::Proto::AppSignal* protoAppSignal = protoSignalSet.add_appsignal();
			asFloat.saveToProto(protoAppSignal);
		}

		{
			asInt.setAppSignalID("#CLIENTTEST_TUNING_ASI1");
			asInt.setHash(::calcHash(asInt.appSignalID()));
			asInt.setSignalType(E::SignalType::Analog);
			asInt.setAnalogSignalFormat(E::AnalogAppSignalFormat::SignedInt32);
			TuningValue tvLow{{TuningValueType::SignedInt32}, 0};
			TuningValue tvHigh{{TuningValueType::SignedInt32}, 100};
			asInt.setTuningLowBound(tvLow);
			asInt.setTuningHighBound(tvHigh);
			asInt.setTuningDefaultValue(tvLow);
			asInt.setEnableTuning(true);
			asInt.setLmEquipmentID("SYSTEMID_CLIENTTEST_CH11_MD00");

			::Proto::AppSignal* protoAppSignal = protoSignalSet.add_appsignal();
			asInt.saveToProto(protoAppSignal);
		}

		{
			asDiscrete.setAppSignalID("#CLIENTTEST_TUNING_D1");
			asDiscrete.setHash(::calcHash(asDiscrete.appSignalID()));
			asDiscrete.setSignalType(E::SignalType::Discrete);
			asDiscrete.setAnalogSignalFormat(E::AnalogAppSignalFormat::SignedInt32);
			TuningValue tvLow{{TuningValueType::Discrete}, 0};
			TuningValue tvHigh{{TuningValueType::Discrete}, 1};
			asDiscrete.setTuningLowBound(tvLow);
			asDiscrete.setTuningHighBound(tvHigh);
			asDiscrete.setTuningDefaultValue(tvLow);
			asDiscrete.setEnableTuning(true);
			asDiscrete.setLmEquipmentID("SYSTEMID_CLIENTTEST_CH11_MD00");

			::Proto::AppSignal* protoAppSignal = protoSignalSet.add_appsignal();
			asDiscrete.saveToProto(protoAppSignal);
		}
	}

	virtual void TearDown()
	{
	}

	AppSignal asFloat;
	AppSignal asInt;
	AppSignal asDiscrete;

	Proto::AppSignalSet protoSignalSet;

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

	inline static const SoftwareInfo s_softwareInfo = {E::SoftwareType::TuningClient, "SYSTEMID_CLIENTTEST_WS03_TUN"};
	inline static const SoftwareInfo s_safeSoftwareInfoA = {E::SoftwareType::TuningClient, "SYSTEMID_CLIENTTEST_WS04_TUNA"};
	inline static const SoftwareInfo s_safeSoftwareInfoB = {E::SoftwareType::TuningClient, "SYSTEMID_CLIENTTEST_WS04_TUNB"};
};


class MockITuningSignalManager: public ITuningSignalManager
{
public:
	MOCK_METHOD(bool, signalExists, (Hash hash), (const override));
	MOCK_METHOD(bool, signalExists, (const QString& appSignalId), (const override));
	MOCK_METHOD(bool, signalsExist, (const QStringList& signalIds), (const override));

	MOCK_METHOD(AppSignalParam, signalParam, (Hash hash, bool* found), (const override));
	MOCK_METHOD(AppSignalParam, signalParam, (const QString& appSignalId, bool* found), (const override));

	MOCK_METHOD(int, signalsCount, (), (const override));
	MOCK_METHOD(std::vector<Hash>, signalHashes, (), (const, override));
	MOCK_METHOD(std::vector<AppSignalParam>, signalList, (), (const override));

	MOCK_METHOD(TuningSignalState, state, (Hash hash, bool* found), (const override));
	MOCK_METHOD(TuningSignalState, state, (const QString& appSignalId, bool* found), (const override));

	MOCK_METHOD(TuningSignalState, state, (Hash hash, Hash tuningServiceHash, bool* found), (const override));
	MOCK_METHOD(TuningSignalState, state, (const QString& appSignalId, Hash tuningServiceHash, bool* found), (const override));

	MOCK_METHOD(void, state, (const std::vector<Hash>& appSignalHashes, std::vector<TuningSignalState>* result, int* found), (const override));
	MOCK_METHOD(void, state, (const std::vector<QString>& appSignalIds, std::vector<TuningSignalState>* result, int* found), (const override));

	MOCK_METHOD(QStringList, signalIdsByTag, (const QString& tag), (const override));
};

class MockITuningSignalUpdater : public ITuningSignalUpdater
{
public:
	MOCK_METHOD(void, reset, (), (override));

	MOCK_METHOD(std::vector<Hash>, signalHashes, (const std::vector<Hash> lmEquipmentIdHashes), (const, override));

	MOCK_METHOD(void, invalidateSignalStates, (Hash tuningServiceHash), (override));

	virtual void setState(const TuningSignalState& /*state*/, Hash /*tuningServiceHash*/) override
	{
	}
	virtual void setStates(const std::vector<TuningSignalState>& /*states*/, Hash /* tuningServiceHash*/) override
	{
	}

	MOCK_METHOD(void, notifySignalParamsUpdated, (), (override));
};

class IRecentAppSignalsStub : public ClientLib::IRecentAppSignals
{
public:
	virtual void addRecentAppSignal(Hash /*h*/) override
	{
	}
	virtual void addRecentAppSignals(const std::vector<Hash>& /*hashes*/) override
	{
	}
	virtual std::vector<Hash> recentlyUsedAppSignals(const QString& /*appDataServivceId*/) override	
	{
		return {};
	}
	virtual bool hasRecentlyUsedAppSignals() override
	{
		return false;
	}
};

TEST_F(TuningConnectionTests, connect)
{
	ILogFileStub logFile;

	MockITuningSignalManager signalManager{};
	MockITuningSignalUpdater signalUpdater{};
	IRecentAppSignalsStub recentAppSignals{};
	TuningAuthorizationStub tuningAuthorization;

	EXPECT_CALL(signalUpdater, invalidateSignalStates(::calcHash(s_tuningServices[0].equipmentId)))
			.Times(1);	// 1 times, when connection to TuningService is closed;

	EXPECT_CALL(signalUpdater, invalidateSignalStates(::calcHash(s_tuningServices[1].equipmentId)))
			.Times(1);	// 1 times, when connection to TuningService is closed;

	std::vector<Hash> lmHashes = {::calcHash(QStringLiteral("SYSTEMID_CLIENTTEST_CH11_MD00"))};
	EXPECT_CALL(signalUpdater, signalHashes(lmHashes))
			.Times(2);

	ClientLib::TuningLogStub tuningLog;

	{
		ClientLib::TuningConnection tc{signalManager, signalUpdater, recentAppSignals, tuningAuthorization, &logFile, &tuningLog};
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
			if (std::all_of(connStates.begin(), connStates.end(), [](const auto& s) { return s.isConnected && s.replyCount > 4; }))
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
	TuningAuthorizationStub tuningAuthorization;

	ClientLib::TuningSignalManager signalManager{s_safeSoftwareInfoA.equipmentID(), &logFile};

	ClientLib::TuningLogStub tuningLog;

	Hash lmHash = {::calcHash(QStringLiteral("SYSTEMID_CLIENTTEST_CH12_MD00"))};

	ClientLib::TuningConnection tc{signalManager, signalManager, signalManager, tuningAuthorization, &logFile, &tuningLog};
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
			if (std::all_of(connStates.begin(), connStates.end(), [](const auto& s) { return s.isConnected && s.replyCount > 4; }))
			{
				QThread::msleep(2000);
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

	// Wait for sources info arrives
	//
	std::vector<ClientLib::TuningSource> allSourcesInfo = tc.tuningSourcesInfo();
	{
		QElapsedTimer timer;
		timer.start();

		while (timer.hasExpired(5000) == false)
		{
			if (allSourcesInfo.size() > 0)
			{
				break;
			}
			QCoreApplication::instance()->processEvents();
			QThread::msleep(10);

			allSourcesInfo = tc.tuningSourcesInfo();
		}
	}
	EXPECT_EQ(allSourcesInfo.size(), 1);

	// Check that tuning source info for LM is arrived
	//
	std::vector<ClientLib::TuningSource> sourceInfo = tc.tuningSourceInfo(lmHash);
	EXPECT_EQ(sourceInfo.size(), 1);

	for (const ClientLib::TuningSource& si : sourceInfo)
	{
		// Check the contents of TuningSource

		EXPECT_TRUE(si.id() != 0);
		EXPECT_EQ(si.valid(), true);

		if (si.statesCount() == 0)
		{
			EXPECT_TRUE(si.statesCount() != 0);
			break;
		}

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
				QThread::msleep(2000);
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

		const ::Network::TuningSourceState& stp = si.previousState(::calcHash(si.controllerEquipmentId(0)));
		EXPECT_FALSE(stp.isreply());

		//Check that tuning sources have no errors

		for (int i = 0; i < si.controllersCount(); i++)
		{
			EXPECT_EQ(si.getErrorsCount(i), 0);
		}

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

	return;
}


TEST_F(TuningConnectionTests, activeClientInfo)
{
	ILogFileStub logFile;

	ClientLib::TuningSignalManager signalManagerA{s_safeSoftwareInfoA.equipmentID(), &logFile};
	ClientLib::TuningSignalManager signalManagerB{s_safeSoftwareInfoB.equipmentID(), &logFile};

	ClientLib::TuningLogStub tuningLog;
	TuningAuthorizationStub tuningAuthorization;

	Hash lmHash = {::calcHash(QStringLiteral("SYSTEMID_CLIENTTEST_CH12_MD00"))};

	// Create 2 tuning connections to the service
	//
	ClientLib::TuningConnection tcA{signalManagerA, signalManagerA, signalManagerA, tuningAuthorization, &logFile, &tuningLog};
	ClientLib::TuningConnection tcB{signalManagerB, signalManagerB, signalManagerB, tuningAuthorization, &logFile, &tuningLog};

	tcA.updateConnections(s_safeSoftwareInfoA, s_safeTuningServices, true, TuningClientSettings::LmStatusFlagMode::SOR);
	tcB.updateConnections(s_safeSoftwareInfoB, s_safeTuningServices, true, TuningClientSettings::LmStatusFlagMode::SOR);

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
			std::vector<Tcp::ConnectionState> connStatesA = tcA.tcpTuningConnStates();
			std::vector<Tcp::ConnectionState> connStatesB = tcB.tcpTuningConnStates();

			connStatesA.insert(connStatesA.end(), connStatesB.begin(), connStatesB.end());
			if (std::all_of(connStatesA.begin(), connStatesA.end(), [](const auto& s) { return s.isConnected && s.replyCount > 4; }))
			{
				QThread::msleep(2000);
				break;
			}
		}
	}

	// Check that active client is client SYSTEMID_CLIENTTEST_WS04_TUNA
	//
	QString infoA;
	QString infoB;

	// Set active client to SYSTEMID_CLIENTTEST_WS04_TUNA
	//
	EXPECT_TRUE(tcA.takeClientControl(lmHash));

	// Wait while active client becomes SYSTEMID_CLIENTTEST_WS04_TUNA
	//
	{
		QElapsedTimer timer;
		timer.start();

		while (timer.hasExpired(5000) == false)
		{
			QCoreApplication::instance()->processEvents();
			QThread::msleep(10);

			infoA = tcA.clientControlInfo();
			infoB = tcB.clientControlInfo();

			if (infoA.contains("active client is SYSTEMID_CLIENTTEST_WS04_TUNA") == true &&
				infoB.contains("active client is SYSTEMID_CLIENTTEST_WS04_TUNA") == true)
			{
				break;
			}
		}
	}

	EXPECT_TRUE(infoA.contains("active client is SYSTEMID_CLIENTTEST_WS04_TUNA"));
	EXPECT_TRUE(infoB.contains("active client is SYSTEMID_CLIENTTEST_WS04_TUNA"));

	// Set active client to SYSTEMID_CLIENTTEST_WS04_TUNB
	//
	EXPECT_TRUE(tcB.takeClientControl(lmHash));

	// Wait while active client becomes SYSTEMID_CLIENTTEST_WS04_TUNB
	//
	{
		QElapsedTimer timer;
		timer.start();

		while (timer.hasExpired(5000) == false)
		{
			QCoreApplication::instance()->processEvents();
			QThread::msleep(10);

			infoA = tcA.clientControlInfo();
			infoB = tcB.clientControlInfo();

			if(infoA.contains("active client is SYSTEMID_CLIENTTEST_WS04_TUNB") == true &&
					infoB.contains("active client is SYSTEMID_CLIENTTEST_WS04_TUNB") == true)
			{
				break;
			}
		}
	}

	EXPECT_TRUE(infoA.contains("active client is SYSTEMID_CLIENTTEST_WS04_TUNB"));
	EXPECT_TRUE(infoB.contains("active client is SYSTEMID_CLIENTTEST_WS04_TUNB"));

	// Deactivate tuning source
	//
	EXPECT_TRUE(tcB.activateTuningSource(lmHash, false));

	// Wait for source to be deactivated
	//
	{
		QElapsedTimer timer;
		timer.start();

		while (timer.hasExpired(5000) == false)
		{
			QCoreApplication::instance()->processEvents();
			QThread::msleep(10);

			if (tcB.activatedTuningSourceStatesCount(lmHash) == 0)
			{
				break;
			}
		}
	}

	EXPECT_EQ(tcB.activatedTuningSourceStatesCount(lmHash), 0);
}


TEST_F(TuningConnectionTests, writeAnalogSignals)
{
	ILogFileStub logFile;

	ClientLib::TuningSignalManager signalManager{s_softwareInfo.equipmentID(), &logFile};

	bool ok = signalManager.load(protoSignalSet);

	EXPECT_TRUE(ok);

	ClientLib::TuningLogStub tuningLog;
	TuningAuthorizationStub tuningAuthorization;

	// Create tuning connection to the service
	//
	ClientLib::TuningConnection tc{signalManager, signalManager, signalManager, tuningAuthorization, &logFile, &tuningLog};
	tc.updateConnections(s_softwareInfo, s_tuningServices, false/*autoApply*/, TuningClientSettings::LmStatusFlagMode::SOR);

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
			if (std::all_of(connStates.begin(), connStates.end(), [](const auto& s) { return s.isConnected && s.replyCount > 4; }))
			{
				break;
			}
		}
	}

	// Wait for all signals to become valid
	//
	{
		QElapsedTimer timer;
		timer.start();

		while (timer.hasExpired(5000) == false)
		{
			QCoreApplication::instance()->processEvents();
			QThread::msleep(10);

			bool allValid = true;
			for (int i = 0; i < protoSignalSet.appsignal_size(); i++)
			{
				QString appSignalID = QString::fromStdString(protoSignalSet.appsignal(i).appsignalid());

				TuningSignalState state = signalManager.state(appSignalID, nullptr);
				if (state.valid() == false)
				{
					allValid = false;
					break;
				}
			}
			if (allValid == true)
			{
				break;
			}
		}
	}

	// Write set of float values to #CLIENTTEST_TUNING_AF1
	//
	std::vector<float> floatValues{10.0f, 40.0f, 100.0f, 70.0f, 50.0f, 25.0f, 12.5f, 3.25f, 0.5f, 0.0f};

	for (int i = 0; i < floatValues.size(); i++)
	{
		TuningValue tv({TuningValueType::Float}, floatValues[i]);
		EXPECT_TRUE(tc.writeTuningSignal(asFloat.appSignalID(), tv));

		// Wait for signal is written
		//
		{
			QElapsedTimer timer;
			timer.start();

			TuningSignalState state;

			while (timer.hasExpired(5000) == false)
			{
				QCoreApplication::instance()->processEvents();
				QThread::msleep(10);

				state = signalManager.state(asFloat.appSignalID(), nullptr);
				if (state.valid() == true &&
						fabs(state.value().floatValue() - floatValues[i]) < std::numeric_limits<float>::epsilon())
				{
					break;
				}
			}

			EXPECT_FLOAT_EQ(state.value().floatValue(), floatValues[i]);
			EXPECT_TRUE(state.valid());
		}
	}

	// Write set of int values to #CLIENTTEST_TUNING_ASI1
	//
	std::vector<int> intValues{10, 40, 100, 70, 50, 25, 12, 3, 1, 0};

	for (int i = 0; i < intValues.size(); i++)
	{
		TuningValue tv({TuningValueType::SignedInt32}, intValues[i]);
		EXPECT_TRUE(tc.writeTuningSignal(asInt.appSignalID(), tv));

		// Wait for signal is written
		//
		{
			QElapsedTimer timer;
			timer.start();

			TuningSignalState state;

			while (timer.hasExpired(5000) == false)
			{
				QCoreApplication::instance()->processEvents();
				QThread::msleep(10);

				state = signalManager.state(asInt.appSignalID(), nullptr);
				if (state.valid() == true && fabs(state.value().int32Value() == intValues[i]))
				{
					break;
				}
			}

			EXPECT_EQ(state.value().int32Value(), intValues[i]);
			EXPECT_TRUE(state.valid());
		}
	}
}

TEST_F(TuningConnectionTests, applyAnalogSignals)
{
	ILogFileStub logFile;

	ClientLib::TuningSignalManager signalManager{s_softwareInfo.equipmentID(), &logFile};

	bool ok = signalManager.load(protoSignalSet);

	EXPECT_TRUE(ok);

	ClientLib::TuningLogStub tuningLog;
	TuningAuthorizationStub tuningAuthorization;

	// Create tuning connection to the service
	//
	ClientLib::TuningConnection tc{signalManager, signalManager, signalManager, tuningAuthorization, &logFile, &tuningLog};
	tc.updateConnections(s_softwareInfo, s_tuningServices, false/*autoApply*/, TuningClientSettings::LmStatusFlagMode::SOR);

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
			if (std::all_of(connStates.begin(), connStates.end(), [](const auto& s) { return s.isConnected && s.replyCount > 4; }))
			{
				break;
			}
		}
	}
	// Check that values are unapplied
	//
	std::vector<ClientLib::TuningSource> tss = tc.tuningSourcesInfo();
	EXPECT_EQ(tss.size(), 2);
	for (const ClientLib::TuningSource& ts : tss)
	{
		for (int i = 0; i < ts.statesCount(); i++)
		{
			EXPECT_TRUE(ts.state(i).hasunappliedparams());
		}
	}

	// Apply changes
	//
	std::vector<Hash> hashes = {::calcHash(asFloat.appSignalID()),
								::calcHash(asInt.appSignalID()),
								::calcHash(asDiscrete.appSignalID())};
	tc.applyTuningSignals(hashes);

	// Wait for changes are applied
	//
	{
		QElapsedTimer timer;
		timer.start();

		while (timer.hasExpired(5000) == false)
		{
			QCoreApplication::instance()->processEvents();
			QThread::msleep(10);

			tss = tc.tuningSourcesInfo();
			EXPECT_EQ(tss.size(), 2);

			bool unapplied = false;

			for (const ClientLib::TuningSource& ts : tss)
			{
				for (int i = 0; i < ts.statesCount(); i++)
				{
					if (ts.state(i).hasunappliedparams() == true)
					{
						unapplied = true;
					}
				}
			}
			if (unapplied == false)
			{
				break;
			}
		}
	}

	// Check that values are applied
	//
	tss = tc.tuningSourcesInfo();
	EXPECT_EQ(tss.size(), 2);
	for (const ClientLib::TuningSource& ts : tss)
	{
		for (int i = 0; i < ts.statesCount(); i++)
		{
			EXPECT_FALSE(ts.state(i).hasunappliedparams());
		}
	}
}

TEST_F(TuningConnectionTests, writeDiscreteSignals)
{
	ILogFileStub logFile;

	ClientLib::TuningSignalManager signalManager{s_softwareInfo.equipmentID(), &logFile};

	bool ok = signalManager.load(protoSignalSet);

	EXPECT_TRUE(ok);

	ClientLib::TuningLogStub tuningLog;
	TuningAuthorizationStub tuningAuthorization;

	// Create tuning connection to the service
	//
	ClientLib::TuningConnection tc{signalManager, signalManager, signalManager, tuningAuthorization, &logFile, &tuningLog};
	tc.updateConnections(s_softwareInfo, s_tuningServices, false/*autoApply*/, TuningClientSettings::LmStatusFlagMode::AccessKey);

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
			if (std::all_of(connStates.begin(), connStates.end(), [](const auto& s) { return s.isConnected && s.replyCount > 4; }))
			{
				break;
			}
		}
	}

	// Wait for all signals to become valid
	//
	{
		QElapsedTimer timer;
		timer.start();

		while (timer.hasExpired(5000) == false)
		{
			QCoreApplication::instance()->processEvents();
			QThread::msleep(10);

			bool allValid = true;
			for (int i = 0; i < protoSignalSet.appsignal_size(); i++)
			{
				QString appSignalID = QString::fromStdString(protoSignalSet.appsignal(i).appsignalid());

				TuningSignalState state = signalManager.state(appSignalID, nullptr);
				if (state.valid() == false)
				{
					allValid = false;
					break;
				}
			}
			if (allValid == true)
			{
				break;
			}
		}
	}

	// Write set of int values to #CLIENTTEST_TUNING_D1
	//
	std::vector<int> discreteValues{1, 0};

	for (int i = 0; i < discreteValues.size(); i++)
	{
		TuningValue tv({TuningValueType::Discrete}, discreteValues[i]);
		EXPECT_TRUE(tc.writeTuningSignal(asDiscrete.appSignalID(), tv));

		// Wait for signal is written
		//
		{
			QElapsedTimer timer;
			timer.start();

			TuningSignalState state;

			while (timer.hasExpired(5000) == false)
			{
				QCoreApplication::instance()->processEvents();
				QThread::msleep(10);

				state = signalManager.state(asDiscrete.appSignalID(), nullptr);
				if (state.valid() == true && fabs(state.value().discreteValue() == discreteValues[i]))
				{
					break;
				}
			}

			EXPECT_EQ(state.value().discreteValue(), discreteValues[i]);
			EXPECT_TRUE(state.valid());
		}
	}
}

TEST_F(TuningConnectionTests, applyDiscreteSignals)
{
	ILogFileStub logFile;

	ClientLib::TuningSignalManager signalManager{s_softwareInfo.equipmentID(), &logFile};

	bool ok = signalManager.load(protoSignalSet);

	EXPECT_TRUE(ok);

	ClientLib::TuningLogStub tuningLog;
	TuningAuthorizationStub tuningAuthorization;

	// Create tuning connection to the service
	//
	ClientLib::TuningConnection tc{signalManager, signalManager, signalManager, tuningAuthorization, &logFile, &tuningLog};
	tc.updateConnections(s_softwareInfo, s_tuningServices, false/*autoApply*/, TuningClientSettings::LmStatusFlagMode::SOR);

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
			if (std::all_of(connStates.begin(), connStates.end(), [](const auto& s) { return s.isConnected && s.replyCount > 4; }))
			{
				break;
			}
		}
	}
	// Check that values are unapplied
	//
	std::vector<ClientLib::TuningSource> tss = tc.tuningSourcesInfo();
	EXPECT_EQ(tss.size(), 2);
	for (const ClientLib::TuningSource& ts : tss)
	{
		for (int i = 0; i < ts.statesCount(); i++)
		{
			EXPECT_TRUE(ts.state(i).hasunappliedparams());
		}
	}

	// Apply changes
	//
	tc.applyTuningSignals();

	// Wait for changes are applied
	//
	{
		QElapsedTimer timer;
		timer.start();

		while (timer.hasExpired(5000) == false)
		{
			QCoreApplication::instance()->processEvents();
			QThread::msleep(10);

			tss = tc.tuningSourcesInfo();
			EXPECT_EQ(tss.size(), 2);

			bool unapplied = false;

			for (const ClientLib::TuningSource& ts : tss)
			{
				for (int i = 0; i < ts.statesCount(); i++)
				{
					if (ts.state(i).hasunappliedparams() == true)
					{
						unapplied = true;
					}
				}
			}
			if (unapplied == false)
			{
				break;
			}
		}
	}

	// Check that values are applied
	//
	tss = tc.tuningSourcesInfo();
	EXPECT_EQ(tss.size(), 2);
	for (const ClientLib::TuningSource& ts : tss)
	{
		for (int i = 0; i < ts.statesCount(); i++)
		{
			EXPECT_FALSE(ts.state(i).hasunappliedparams());
		}
	}
}
