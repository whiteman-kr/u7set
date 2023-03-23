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

	TuningSignalManager tsm2{};
	ba.fill(0x55, 1024);
	ok = tsm2.load(ba);
	EXPECT_FALSE(ok);

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

	EXPECT_TRUE(tsm.signalExists(as1.appSignalID()));

	return;
}

TEST_F(TuningSignalManagerTests, signalHashesAndList)
{
	TuningSignalManager tsm{};

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

	EXPECT_TRUE((appSignals[0] == h1 && appSignals[1] == h2) || (appSignals[1] == h1 && appSignals[0] == h2));

	return;
}

TEST_F(TuningSignalManagerTests, appSignalParam)
{
	TuningSignalManager tsm{};

	bool ok = tsm.load(protoSignalSet);

	EXPECT_TRUE(ok);

	bool found = false;

	AppSignalParam asp = tsm.signalParam(as1.appSignalID(), &found);
	EXPECT_EQ(found, true);
	EXPECT_EQ(asp.appSignalId(), as1.appSignalID());

	found = tsm.signalParam(calcHash(as2.appSignalID()), &asp);
	EXPECT_EQ(found, true);
	EXPECT_EQ(asp.appSignalId(), as2.appSignalID());

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

TEST_F(TuningSignalManagerTests, setNewValue)
{
	TuningSignalManager tsm{};

	bool ok = tsm.load(protoSignalSet);

	EXPECT_TRUE(ok);
	EXPECT_EQ(tsm.signalsCount(), protoSignalSet.appsignal_size());

	// set and apply state

	TuningSignalState state1{as1.hash(), TuningSignalStateFlags{.valid = 1}, TuningValue{TuningValueType::Float, 101.0}};

	tsm.setState(as1.appSignalID(), state1);

	tsm.setNewValue(as1.hash(), TuningValue(TuningValueType::Float, 256.0));

	EXPECT_TRUE(tsm.newValueIsUnapplied(as1.hash()));

	TuningValue nv = tsm.newValue(as1.hash());

	EXPECT_TRUE(fabs(nv.floatValue() - 256.0) < std::numeric_limits<float>::epsilon());

	tsm.setNewValueAsApplied(as1.hash());

	EXPECT_FALSE(tsm.newValueIsUnapplied(as1.hash()));

	// set and unset state by old value

	TuningSignalState state2{as2.hash(), TuningSignalStateFlags{.valid = 1}, TuningValue{TuningValueType::Float, 501.0}};

	tsm.setState(as2.appSignalID(), state2);

	tsm.setNewValue(as2.hash(), TuningValue(TuningValueType::Float, 502.0));

	EXPECT_TRUE(tsm.newValueIsUnapplied(as2.hash()));

	tsm.setNewValue(as2.hash(), TuningValue(TuningValueType::Float, 501.0));

	EXPECT_FALSE(tsm.newValueIsUnapplied(as2.hash()));

	return;
}

TEST_F(TuningSignalManagerTests, signalIdsByTag)
{
	TuningSignalManager tsm{};

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
