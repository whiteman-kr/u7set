#include "../AppSignalLib/TuningSignalManager.h"
#include "../Proto/serialization.pb.h"

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

			::Proto::AppSignal* protoAppSignal = protoSignalSet.add_appsignal();
			as1.saveToProto(protoAppSignal);
		}

		{
			as2.setAppSignalID("#SIGNAL2");
			as2.setHash(::calcHash(as2.appSignalID()));
			as2.setEnableTuning(true);
			as2.setLmEquipmentID("LM1");

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
};


TEST_F(TuningSignalManagerTests, loadFromBinary)
{
	TuningSignalManager tsm{};

	QSignalSpy loadSpy{&tsm, &TuningSignalManager::signalsLoaded};

	std::string buffer;
	protoSignalSet.SerializeToString(&buffer);
	QByteArray ba = QByteArray::fromRawData(buffer.data(), buffer.size());

	bool ok = tsm.load(ba);

	EXPECT_TRUE(ok);
	EXPECT_EQ(tsm.signalsCount(), protoSignalSet.appsignal_size());

	EXPECT_EQ(loadSpy.size(), 1);

	return;
}

TEST_F(TuningSignalManagerTests, loadFromProto)
{
	TuningSignalManager tsm{};

	QSignalSpy loadSpy{&tsm, &TuningSignalManager::signalsLoaded};

	bool ok = tsm.load(protoSignalSet);

	EXPECT_TRUE(ok);
	EXPECT_EQ(tsm.signalsCount(), protoSignalSet.appsignal_size());

	EXPECT_EQ(loadSpy.size(), 1);

	return;
}

TEST_F(TuningSignalManagerTests, signalHashes)
{
	TuningSignalManager tsm{};

	bool ok = tsm.load(protoSignalSet);

	EXPECT_TRUE(ok);
	EXPECT_EQ(tsm.signalsCount(), protoSignalSet.appsignal_size());

	std::vector<Hash> appSignals = tsm.signalHashes();

	for (const auto& ps : protoSignalSet.appsignal())
	{
		Hash hash = calcHash(QString::fromStdString(ps.appsignalid()));
		EXPECT_TRUE(std::find(appSignals.begin(), appSignals.end(), hash) != appSignals.end());
	}

	return;
}

TEST_F(TuningSignalManagerTests, signalHashesByLms)
{
	TuningSignalManager tsm{};

	bool ok = tsm.load(protoSignalSet);

	EXPECT_TRUE(ok);
	EXPECT_EQ(tsm.signalsCount(), protoSignalSet.appsignal_size());

	std::vector lms = {::calcHash(QString{"LM1"}),
					   ::calcHash(QString{"NOLM"})};

	std::vector<Hash> appSignals = tsm.signalHashes(lms);

	ASSERT_EQ(appSignals.size(), 2);		// 2 for LM1

	Hash h1 = as1.hash();
	Hash h2 = as2.hash();

	EXPECT_TRUE((appSignals[0] == h1 && appSignals[1] == h2) || (appSignals[1] == h1 && appSignals[2] == h2));

	return;
}

TEST_F(TuningSignalManagerTests, setState)
{
	TuningSignalManager tsm{};

	bool ok = tsm.load(protoSignalSet);

	EXPECT_TRUE(ok);
	EXPECT_EQ(tsm.signalsCount(), protoSignalSet.appsignal_size());

	TuningSignalState state1{as1.hash(), TuningSignalStateFlags{.valid = 1}, TuningValue{TuningValueType::Float, 101.0}};
	TuningSignalState state2{as2.hash(), TuningSignalStateFlags{.valid = 1}, TuningValue{TuningValueType::Float, 102.0}};

	tsm.setState(as1.appSignalID(), state1);
	tsm.setState(as2.appSignalID(), state2);

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
	tsm.setState(stateVector);

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
	TuningSignalManager tsm{};

	bool ok = tsm.load(protoSignalSet);

	EXPECT_TRUE(ok);
	EXPECT_EQ(tsm.signalsCount(), protoSignalSet.appsignal_size());

	TuningSignalState state1{as1.hash(), TuningSignalStateFlags{.valid = 1}, TuningValue{TuningValueType::Double, 101.0}};
	TuningSignalState state2{as2.hash(), TuningSignalStateFlags{.valid = 1}, TuningValue{TuningValueType::Double, 102.0}};

	tsm.setState(as1.appSignalID(), state1);
	tsm.setState(as2.appSignalID(), state2);

	tsm.invalidateStates();

	// --
	//
	TuningSignalState gotState1 = tsm.state(as1.appSignalID(), nullptr);
	TuningSignalState gotState2 = tsm.state(as2.appSignalID(), nullptr);

	EXPECT_FALSE(gotState1.valid());
	EXPECT_FALSE(gotState2.valid());

	return;
}
