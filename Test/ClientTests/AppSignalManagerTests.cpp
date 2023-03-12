#include "../../ClientLib/AppSignalManager.h"


using namespace testing;


TEST(AppSignalManagerTests, reset)
{
	ILogFileStub log;
	ClientLib::AppSignalManager sm{&log};

	// --
	//
	AppSignalParam sp1;
	AppSignalParam sp2;
	sp1.setAppSignalId("#SP1");
	sp2.setAppSignalId("#SP2");

	sm.addSignal(sp1, "ADS");
	sm.addSignal(sp2, "ADS");

	EXPECT_EQ(sm.signalsCount(), 2);

	// --
	//
	sm.reset();

	EXPECT_EQ(sm.signalsCount(), 0);

	return;
}

TEST(AppSignalManagerTests, notifySignalParamsUpdated)
{
	ILogFileStub log;
	ClientLib::AppSignalManager sm{&log};

	QSignalSpy spy{&sm, &ClientLib::AppSignalManager::signalParamsUpdated};

	sm.notifySignalParamsUpdated();
	EXPECT_EQ(spy.count(), 1);

	return;
}

TEST(AppSignalManagerTests, addSignal)
{
	ILogFileStub log;
	ClientLib::AppSignalManager sm{&log};

	QSignalSpy spy{&sm, &ClientLib::AppSignalManager::signalParamsUpdated};

	AppSignalParam sp1;
	AppSignalParam sp2;
	sp1.setAppSignalId("#SP1");
	sp2.setAppSignalId("#SP2");

	sm.addSignal(sp1, "ADS");
	sm.addSignal(sp2, "ADS");

	EXPECT_EQ(sm.signalsCount(), 2);

	return;
}

TEST(AppSignalManagerTests, addSignals)
{
	ILogFileStub log;
	ClientLib::AppSignalManager sm{&log};

	QSignalSpy spy{&sm, &ClientLib::AppSignalManager::signalParamsUpdated};

	AppSignalParam sp1;
	AppSignalParam sp2;
	sp1.setAppSignalId("#SP1");
	sp2.setAppSignalId("#SP2");

	std::vector<AppSignalParam> v;
	v.push_back(sp1);
	v.push_back(sp2);

	sm.addSignals(v, "ADS");

	EXPECT_EQ(sm.signalsCount(), 2);

	return;
}

TEST(AppSignalManagerTests, invalidateSignalStates)
{
	// Test of:
	//	void AppSignalManager::setState(const QString& appSignalId, const AppSignalState& state, Qt::HANDLE sourceThreadId);
	//
	ILogFileStub log;
	ClientLib::AppSignalManager sm{&log};

	QSignalSpy spy{&sm, &ClientLib::AppSignalManager::signalParamsUpdated};

	AppSignalParam sp1;
	AppSignalParam sp2;
	sp1.setAppSignalId("#SP1");
	sp2.setAppSignalId("#SP2");

	std::vector<AppSignalParam> v;
	v.push_back(sp1);
	v.push_back(sp2);

	sm.addSignals(v, "ADS1");
	sm.addSignals(v, "ADS2");

	EXPECT_EQ(sm.signalsCount(), 2);

	auto thread1 = std::bit_cast<Qt::HANDLE>(1ull);
	auto thread2 = std::bit_cast<Qt::HANDLE>(2ull);

	AppSignalState state1{sp2.hash(), {0, 0, 0}, 123.0, {.valid = 1, .stateAvailable = 1}};
	sm.setState("#SP2", state1, thread1);

	QThread::currentThread()->msleep(10);

	AppSignalState state2{sp2.hash(), {1, 1, 1}, 124.0, {.valid = 1, .stateAvailable = 1}};
	sm.setState("#SP2", state2, thread2);

	// --
	//
	auto state = sm.signalState(sp2.hash(), nullptr);
	EXPECT_TRUE(state.isValid());
	EXPECT_DOUBLE_EQ(state.value(), state2.m_value);

	sm.invalidateSignalStates(thread2);

	state = sm.signalState(sp2.hash(), nullptr);
	EXPECT_TRUE(state.isValid());
	EXPECT_DOUBLE_EQ(state.value(), state1.m_value);

	sm.invalidateSignalStates(thread1);

	state = sm.signalState(sp2.hash(), nullptr);
	EXPECT_FALSE(state.isValid());

	return;
}

TEST(AppSignalManagerTests, setState)
{
	// Test of:
	//	void AppSignalManager::setState(const QString& appSignalId, const AppSignalState& state, Qt::HANDLE sourceThreadId);
	//
	ILogFileStub log;
	ClientLib::AppSignalManager sm{&log};

	QSignalSpy spy{&sm, &ClientLib::AppSignalManager::signalParamsUpdated};

	AppSignalParam sp1;
	AppSignalParam sp2;
	sp1.setAppSignalId("#SP1");
	sp2.setAppSignalId("#SP2");

	std::vector<AppSignalParam> v;
	v.push_back(sp1);
	v.push_back(sp2);

	sm.addSignals(v, "ADS1");
	sm.addSignals(v, "ADS2");

	EXPECT_EQ(sm.signalsCount(), 2);

	AppSignalState state1{sp2.hash(), {}, 123.0, {.valid = 1, .stateAvailable = 0}};
	sm.setState("#SP2", state1, std::bit_cast<Qt::HANDLE>(1ull));

	QThread::currentThread()->msleep(10);

	AppSignalState state2{sp2.hash(), {}, 124.0, {.valid = 1, .stateAvailable = 0}};
	sm.setState("#SP2", state2, std::bit_cast<Qt::HANDLE>(2ull));

	// Check #SP1 is not valid
	//
	auto state = sm.signalState(sp1.hash(), nullptr);
	EXPECT_FALSE(state.isValid());
	EXPECT_EQ(state.hash(), sp1.hash());

	// Check #SP2 is valid
	//
	state = sm.signalState(sp2.hash(), nullptr);
	EXPECT_DOUBLE_EQ(state.value(), state2.m_value);	// .stateAvailable = 0, so the actual value will be from "thread" 2
	EXPECT_EQ(state.hash(), sp2.hash());

	return;
}

TEST(AppSignalManagerTests, setStateForZeroHash)
{
	// Set state for 0 hash, assert is expected
	//
#ifdef QT_DEBUG
	ASSERT_DEATH({
		ILogFileStub log;
		ClientLib::AppSignalManager sm{&log};
		sm.setState(Hash{0}, AppSignalState{}, std::bit_cast<Qt::HANDLE>(1ull));
	}, "");
#endif
}
