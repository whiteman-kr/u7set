#include "../../OnlineLib/SoftwareInfo.h"
#include "../Proto/AppSignal.pb.h"

#include <ClientLib/TuningSignalManager.h>

class TuningSignalManagerTests : public ::testing::Test
{
protected:
	virtual void SetUp()
	{
		protoSignalSet.Clear();

		{
			as1.setAppSignalID("#SIGNAL1");
			as1.setHash(::calcHash(as1.appSignalID()));
			as1.setEnableTuning(true);
			as1.setLmEquipmentID("LM1");
			as1.setTagsStr("tag1");

			::Proto::AppSignal* protoAppSignal = protoSignalSet.add_appsignal();
			as1.saveToProto(protoAppSignal);
		}

		{
			as2.setAppSignalID("#SIGNAL2");
			as2.setHash(::calcHash(as2.appSignalID()));
			as2.setEnableTuning(true);
			as2.setLmEquipmentID("LM1");
			as2.setTagsStr("tag2");

			::Proto::AppSignal* protoAppSignal = protoSignalSet.add_appsignal();
			as2.saveToProto(protoAppSignal);
		}

		{
			as3.setAppSignalID("#SIGNAL3");
			as3.setHash(::calcHash(as3.appSignalID()));
			as3.setEnableTuning(true);
			as3.setLmEquipmentID("LM2");

			::Proto::AppSignal* protoAppSignal = protoSignalSet.add_appsignal();
			as3.saveToProto(protoAppSignal);
		}


		return;
	}

	virtual void TearDown()
	{
	}

	AppSignal as1;
	AppSignal as2;
	AppSignal as3;

	Proto::AppSignalSet protoSignalSet;

	inline static const SoftwareInfo s_softwareInfo = {E::SoftwareType::TuningClient, "SYSTEMID_CLIENTTEST_WS01_TUN"};
	inline static const QString s_tuningServiceId1 = "SYSTEMID_CLIENTTEST_WS01_TUNS";
	inline static const QString s_tuningServiceId2 = "SYSTEMID_CLIENTTEST_WS02_TUNS";

};


TEST_F(TuningSignalManagerTests, tuningValue)
{
	// Test base values and types
	//
	TuningValue v1(TuningValueType::Float);
	EXPECT_EQ(v1.type(), TuningValueType::Float);
	v1.setFloatValue(700);
	EXPECT_FLOAT_EQ(v1.floatValue(), 700);
	EXPECT_DOUBLE_EQ(v1.rawDouble(), 700);

	TuningValue v2(TuningValueType::Float, 256.0);
	EXPECT_EQ(v2.type(), TuningValueType::Float);

	TuningValue v3(QVariant(50));
	EXPECT_EQ(v3.type(), TuningValueType::SignedInt32);
	v3.setInt32Value(700);
	EXPECT_EQ(v3.int32Value(), 700);

	TuningValue v4(QVariant(true));
	EXPECT_EQ(v4.type(), TuningValueType::Discrete);
	EXPECT_TRUE(v4.typeStr().contains("Discrete"));
	v4.setDiscreteValue(false);
	EXPECT_EQ(v4.discreteValue(), false);

	EXPECT_EQ(v4.toVariant().typeId(), QMetaType::Int);

	// Test creating from string and creating from double functions
	//
	TuningValue v5;
	v5.setType(TuningValueType::SignedInt32);
	v5.fromString("7");
	EXPECT_TRUE(v5.tuningValueTypeString().contains("SignedInt32"));

	TuningValue v6 = TuningValue::createFromDouble(E::SignalType::Analog, E::AnalogAppSignalFormat::SignedInt32, 7);
	EXPECT_EQ(v6.type(), TuningValueType::SignedInt32);

	// Test comparators
	//
	EXPECT_TRUE(v5 == v6);

	v6.setInt32Value(10);
	EXPECT_TRUE(v5 <= v6);
	EXPECT_TRUE(v5 < v6);

	v6.setInt32Value(2);
	EXPECT_TRUE(v5 >= v6);
	EXPECT_TRUE(v5 > v6);

	EXPECT_TRUE(v5 != v6);

	EXPECT_EQ(v5.rawInt64(), 7);

	// Test proto functions
	//
	Proto::TuningValue message;

	TuningValue v7(TuningValueType::Discrete, true);
	v7.save(&message);

	TuningValue v8;
	bool ok = v8.load(message);
	EXPECT_TRUE(ok);

	EXPECT_EQ(v7, v8);
	return;
}

TEST_F(TuningSignalManagerTests, loadFromBinary)
{
	ILogFileStub logFile;
	ClientLib::TuningSignalManager tsm{s_softwareInfo.equipmentID(), &logFile};

	QSignalSpy loadSpy{&tsm, &ClientLib::TuningSignalManager::signalsLoaded};

	std::string buffer;
	protoSignalSet.SerializeToString(&buffer);
	QByteArray ba = QByteArray::fromRawData(buffer.data(), buffer.size());

	bool ok = tsm.load(ba);

	EXPECT_TRUE(ok);
	EXPECT_EQ(tsm.signalsCount(), protoSignalSet.appsignal_size());

	EXPECT_EQ(loadSpy.size(), 1);

	ClientLib::TuningSignalManager tsm2{s_softwareInfo.equipmentID(), &logFile};
	ba.fill(0x55, 1024);
	ok = tsm2.load(ba);
	EXPECT_FALSE(ok);

	return;
}

TEST_F(TuningSignalManagerTests, loadFromProto)
{
	ILogFileStub logFile;
	ClientLib::TuningSignalManager tsm{s_softwareInfo.equipmentID(), &logFile};

	QSignalSpy loadSpy{&tsm, &ClientLib::TuningSignalManager::signalsLoaded};

	bool ok = tsm.load(protoSignalSet);

	EXPECT_TRUE(ok);
	EXPECT_EQ(tsm.signalsCount(), protoSignalSet.appsignal_size());

	EXPECT_EQ(loadSpy.size(), 1);

	EXPECT_TRUE(tsm.signalExists(as1.appSignalID()));

	return;
}

TEST_F(TuningSignalManagerTests, signalHashesAndList)
{
	ILogFileStub logFile;
	ClientLib::TuningSignalManager tsm{s_softwareInfo.equipmentID(), &logFile};

	bool ok = tsm.load(protoSignalSet);

	EXPECT_TRUE(ok);
	EXPECT_EQ(tsm.signalsCount(), protoSignalSet.appsignal_size());

	// signalHashes

	std::vector<Hash> appSignals = tsm.signalHashes();

	for (const auto& ps : protoSignalSet.appsignal())
	{
		Hash hash = calcHash(QString::fromStdString(ps.appsignalid()));
		EXPECT_TRUE(std::find(appSignals.begin(), appSignals.end(), hash) != appSignals.end());
	}

	// signalList

	std::vector<AppSignalParam> aspList = tsm.signalList();

	EXPECT_EQ(aspList.size(), protoSignalSet.appsignal_size());

	QStringList appSignalIds;
	for (const auto& asp : aspList)
	{
		appSignalIds.push_back(asp.appSignalId());
	}

	for (int i = 0; i < protoSignalSet.appsignal_size(); i++)
	{
		QString setAppSignalId = QString::fromStdString(protoSignalSet.appsignal(i).appsignalid());
		EXPECT_TRUE(std::find(appSignalIds.begin(), appSignalIds.end(), setAppSignalId) != appSignalIds.end());
	}

	return;
}

TEST_F(TuningSignalManagerTests, signalHashesByLms)
{
	ILogFileStub logFile;
	ClientLib::TuningSignalManager tsm{s_softwareInfo.equipmentID(), &logFile};

	bool ok = tsm.load(protoSignalSet);

	EXPECT_TRUE(ok);
	EXPECT_EQ(tsm.signalsCount(), protoSignalSet.appsignal_size());

	std::vector lms = {::calcHash(QString{"LM1"}),
					   ::calcHash(QString{"NOLM"})};

	std::vector<Hash> appSignals = tsm.signalHashes(lms);

	ASSERT_EQ(appSignals.size(), 2);		// 2 for LM1

	Hash h1 = as1.hash();
	Hash h2 = as2.hash();

	EXPECT_TRUE((appSignals[0] == h1 && appSignals[1] == h2) || (appSignals[1] == h1 && appSignals[0] == h2));

	return;
}

TEST_F(TuningSignalManagerTests, appSignalParam)
{
	ILogFileStub logFile;
	ClientLib::TuningSignalManager tsm{s_softwareInfo.equipmentID(), &logFile};

	bool ok = tsm.load(protoSignalSet);

	EXPECT_TRUE(ok);

	bool found = false;

	AppSignalParam asp = tsm.signalParam(as1.appSignalID(), &found);
	EXPECT_EQ(found, true);
	EXPECT_EQ(asp.appSignalId(), as1.appSignalID());

	asp = tsm.signalParam(as2.appSignalID(), &found);
	EXPECT_EQ(found, true);
	EXPECT_EQ(asp.appSignalId(), as2.appSignalID());

	return;
}

TEST_F(TuningSignalManagerTests, setState)
{
	ILogFileStub logFile;
	ClientLib::TuningSignalManager tsm{s_softwareInfo.equipmentID(), &logFile};

	bool ok = tsm.load(protoSignalSet);

	EXPECT_TRUE(ok);
	EXPECT_EQ(tsm.signalsCount(), protoSignalSet.appsignal_size());

	TuningSignalState state1{as1.hash(), TuningSignalStateFlags{.valid = 1}, TuningValue{TuningValueType::Float, 101.0}};
	TuningSignalState state2{as2.hash(), TuningSignalStateFlags{.valid = 1}, TuningValue{TuningValueType::Float, 102.0}};

	tsm.setState(state1, ::calcHash(s_tuningServiceId1));
	tsm.setState(state2, ::calcHash(s_tuningServiceId1));

	bool f1 = false;
	bool f2 = false;
	bool f3 = false;

	TuningSignalState gotState1 = tsm.state(as1.appSignalID(), &f1);
	TuningSignalState gotState2 = tsm.state(as2.appSignalID(), &f2);
	[[maybe_unused]] TuningSignalState noState2 = tsm.state("#NOID", &f3);

	EXPECT_TRUE(f1);
	EXPECT_TRUE(f2);
	EXPECT_FALSE(f3);

	EXPECT_EQ(state1.valid(), gotState1.valid());
	EXPECT_EQ(state1.value(), gotState1.value());

	EXPECT_EQ(state2.valid(), gotState2.valid());
	EXPECT_EQ(state2.value(), gotState2.value());

	// --
	//
	state1.m_value.setFloatValue(1.0);
	state2.m_value.setFloatValue(2.0);

	std::vector<TuningSignalState> stateVector = {state1, state2};
	tsm.setStates(stateVector, ::calcHash(s_tuningServiceId1));

	gotState1 = tsm.state(as1.appSignalID(), &f1);
	gotState2 = tsm.state(as2.appSignalID(), &f2);

	EXPECT_TRUE(f1);
	EXPECT_TRUE(f2);

	EXPECT_EQ(state1.value(), TuningValue(TuningValueType::Float, 1.0));
	EXPECT_EQ(state2.value(), TuningValue(TuningValueType::Float, 2.0));

	return;
}

TEST_F(TuningSignalManagerTests, invalidateStates)
{
	ILogFileStub logFile;
	ClientLib::TuningSignalManager tsm{s_softwareInfo.equipmentID(), &logFile};

	bool ok = tsm.load(protoSignalSet);

	EXPECT_TRUE(ok);
	EXPECT_EQ(tsm.signalsCount(), protoSignalSet.appsignal_size());

	TuningSignalState state1{as1.hash(), TuningSignalStateFlags{.valid = 1}, TuningValue{TuningValueType::Double, 101.0}};
	TuningSignalState state2{as2.hash(), TuningSignalStateFlags{.valid = 1}, TuningValue{TuningValueType::Double, 102.0}};

	tsm.setState(state1, ::calcHash(s_tuningServiceId1));
	tsm.setState(state2, ::calcHash(s_tuningServiceId1));

	tsm.invalidateSignalStates(::calcHash(s_tuningServiceId1));

	// --
	//
	TuningSignalState gotState1 = tsm.state(as1.appSignalID(), nullptr);
	TuningSignalState gotState2 = tsm.state(as2.appSignalID(), nullptr);

	EXPECT_FALSE(gotState1.valid());
	EXPECT_FALSE(gotState2.valid());

	return;
}

TEST_F(TuningSignalManagerTests, setUnappliedValue)
{
	ILogFileStub logFile;
	ClientLib::TuningSignalManager tsm{s_softwareInfo.equipmentID(), &logFile};

	double oldValue_twoChannels = 101.0;
	double newValue_twoChannels = 256.0;

	double oldValue_singleChannel = 501.0;
	double newValue_singleChannel = 502.0;

	bool ok = tsm.load(protoSignalSet);

	EXPECT_TRUE(ok);
	EXPECT_EQ(tsm.signalsCount(), protoSignalSet.appsignal_size());

	// Check setting and resetting unapplied value for one service: s_tuningServiceId1

	// Set states for the signal from two channels: valid and non-valid
	//
	TuningSignalState state_service1{as1.hash(), TuningSignalStateFlags{.valid = 1}, TuningValue{TuningValueType::Float, oldValue_twoChannels}};
	TuningSignalState state_service2{as1.hash(), TuningSignalStateFlags{.valid = 0}, TuningValue{TuningValueType::Float, oldValue_twoChannels}};

	tsm.setState(state_service1, ::calcHash(s_tuningServiceId1));
	tsm.setState(state_service2, ::calcHash(s_tuningServiceId2));

	// Check states are set correctly
	//

	TuningSignalState gotStateService1 = tsm.state(as1.appSignalID(), ::calcHash(s_tuningServiceId1), &ok);
	EXPECT_TRUE(ok);
	EXPECT_EQ(gotStateService1.hash(), as1.hash());
	EXPECT_TRUE(gotStateService1.valid());

	TuningSignalState gotStateService2 = tsm.state(as1.appSignalID(), ::calcHash(s_tuningServiceId2), &ok);
	EXPECT_TRUE(ok);
	EXPECT_EQ(gotStateService2.hash(), as1.hash());
	EXPECT_FALSE(gotStateService2.valid());

	// Set unapplied value to the signal and check that unapplied flag has been set
	//
	tsm.setUnappliedValue(as1.hash(), TuningValue(TuningValueType::Float, newValue_twoChannels));

	EXPECT_TRUE(tsm.isUnapplied(as1.hash()));

	TuningValue nv = tsm.unappliedValue(as1.hash());

	EXPECT_FLOAT_EQ(nv.floatValue(), newValue_twoChannels);

	// Reset unapplied value to old value and and check that unapplied flag has been reset
	//
	tsm.setUnappliedValue(as1.hash(), TuningValue(TuningValueType::Float, oldValue_twoChannels));

	EXPECT_FALSE(tsm.isUnapplied(as1.hash()));

	// Check setting and resetting unapplied value for one service: s_tuningServiceId1

	// Set state for the signal from one channel
	//
	TuningSignalState state2{as2.hash(), TuningSignalStateFlags{.valid = 1}, TuningValue{TuningValueType::Float, oldValue_singleChannel}};

	tsm.setState(state2, ::calcHash(s_tuningServiceId1));

	// Set unapplied value to the signal and check that unapplied flag has been set
	//
	tsm.setUnappliedValue(as2.hash(), TuningValue(TuningValueType::Float, newValue_singleChannel));

	EXPECT_TRUE(tsm.isUnapplied(as2.hash()));

	// Reset unapplied value to old value and and check that unapplied flag has been reset
	//
	tsm.setUnappliedValue(as2.hash(), TuningValue(TuningValueType::Float, oldValue_singleChannel));

	EXPECT_FALSE(tsm.isUnapplied(as2.hash()));

	return;
}

TEST_F(TuningSignalManagerTests, setStateFromNetworkMessage)
{
	ILogFileStub logFile;
	ClientLib::TuningSignalManager tsm{s_softwareInfo.equipmentID(), &logFile};

	double oldValue = 101.0;
	double newValue = 256.0;

	bool ok = tsm.load(protoSignalSet);

	EXPECT_TRUE(ok);
	EXPECT_EQ(tsm.signalsCount(), protoSignalSet.appsignal_size());

	// Set signal value to oldValue
	//
	TuningSignalState state{as1.hash(), TuningSignalStateFlags{.valid = 1}, TuningValue{TuningValueType::Float, oldValue}};
	tsm.setState(state, ::calcHash(s_tuningServiceId1));

	// Set new unapplied value to newValue
	//
	tsm.setUnappliedValue(as1.hash(), TuningValue(TuningValueType::Float, newValue));
	EXPECT_TRUE(tsm.isUnapplied(as1.hash()));

	// Simulate a network packet from tuning service with expected value (newValue) and legal successful write time
	//
	::Network::TuningSignalState networkMessage;
	networkMessage.set_signalhash(as1.hash());
	networkMessage.set_valid(true);
	networkMessage.mutable_value()->set_doublevalue(newValue);
	networkMessage.mutable_value()->set_type(static_cast<int>(TuningValueType::Float));
	networkMessage.mutable_readlowbound()->set_doublevalue(0.0);
	networkMessage.mutable_readlowbound()->set_type(static_cast<int>(TuningValueType::Float));
	networkMessage.mutable_readhighbound()->set_doublevalue(500.0);
	networkMessage.mutable_readhighbound()->set_type(static_cast<int>(TuningValueType::Float));
	networkMessage.set_writeerrorcode(static_cast<int>(E::NetworkError::Success));
	networkMessage.set_writeclient(::calcHash(s_softwareInfo.equipmentID()));
	QDateTime tm = QDateTime::currentDateTime();
	networkMessage.set_successfulreadtime(tm.toMSecsSinceEpoch());
	networkMessage.set_writerequesttime(tm.toMSecsSinceEpoch());
	networkMessage.set_successfulwritetime(tm.toMSecsSinceEpoch());
	networkMessage.set_lmtime(tm.toMSecsSinceEpoch());

	// Set signal state to received from network
	//
	TuningSignalState state_written;
	state_written.setState(networkMessage);

	tsm.setState(state_written, ::calcHash(s_tuningServiceId1));

	// Check that unapplied state has been cleared and signal has new value
	//
	EXPECT_FALSE(tsm.isUnapplied(as1.hash()));

	bool found = false;
	TuningSignalState gotState = tsm.state(as1.appSignalID(), &found);

	EXPECT_TRUE(found);

	EXPECT_EQ(gotState.valid(), true);
	EXPECT_FLOAT_EQ(gotState.value().floatValue(), newValue);

	return;
}

TEST_F(TuningSignalManagerTests, signalIdsByTag)
{
	ILogFileStub logFile;
	ClientLib::TuningSignalManager tsm{s_softwareInfo.equipmentID(), &logFile};

	bool ok = tsm.load(protoSignalSet);

	EXPECT_TRUE(ok);
	EXPECT_EQ(tsm.signalsCount(), protoSignalSet.appsignal_size());

	QStringList sids =  tsm.signalIdsByTag("tag1");
	EXPECT_EQ(sids.size(), 1);
	if (sids.isEmpty() == false)
	{
		EXPECT_EQ(sids[0], as1.appSignalID());
	}

	sids =  tsm.signalIdsByTag("tag2");
	EXPECT_EQ(sids.size(), 1);
	if (sids.isEmpty() == false)
	{
		EXPECT_EQ(sids[0], as2.appSignalID());
	}

	return;
}
