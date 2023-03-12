#include "../../ClientLib/AppSignalManager.h"
#include "gtest/gtest.h"

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

TEST(AppSignalManagerTests, setStateAsVector)
{
	// Test of:
	//	void AppSignalManager::setState(onst std::vector<AppSignalState>& states, Qt::HANDLE sourceThreadId);
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

	AppSignalState state1{sp1.hash(), {}, 1.0, {.valid = 1, .stateAvailable = 1}};
	AppSignalState state2{sp2.hash(), {}, 2.0, {.valid = 1, .stateAvailable = 1}};

	sm.setState({state1, state2}, std::bit_cast<Qt::HANDLE>(1ull));

	auto state = sm.signalState(sp1.hash(), nullptr);
	EXPECT_TRUE(state.isValid());
	EXPECT_EQ(state.hash(), sp1.hash());
	EXPECT_EQ(state.value(), 1.0);

	state = sm.signalState(sp2.hash(), nullptr);
	EXPECT_TRUE(state.isValid());
	EXPECT_EQ(state.hash(), sp2.hash());
	EXPECT_EQ(state.value(), 2.0);

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

TEST(AppSignalManagerTests, recentUsedAdd)
{
	// 1. Outdated is a signal which was not fetched for 3 seconds or more.
	// 2. Outdated signals are removed from the list when hashes() is called
	//

	ClientLib::RecentUsed recentUsed{5};	// Keep up to 5 signals

	recentUsed.add(1);
	recentUsed.add(2);
	recentUsed.add(3);
	recentUsed.add(4);
	recentUsed.add(5);
	QThread::currentThread()->msleep(10);
	recentUsed.add(6);

	std::vector<Hash> keptSignals = recentUsed.hashes();

	bool eq = keptSignals == std::vector<Hash>{2, 3, 4, 5, 6};
	EXPECT_TRUE(eq);

	return;
}

TEST(AppSignalManagerTests, recentUsedRemove)
{
	// 1. Outdated is a signal which was not fetched for 3 seconds or more.
	// 2. Outdated signals are removed from the list when hashes() is called
	//
	ClientLib::RecentUsed recentUsed{10};	// Keep up to 5 signals

	recentUsed.add({1, 2, 3, 4, 5, 6, 7, 8, 9, 10});

	recentUsed.remove(2);
	recentUsed.remove({4, 6, 8});

	std::vector<Hash> keptSignals = recentUsed.hashes();

	bool eq = keptSignals == std::vector<Hash>{1, 3, 5, 7, 9, 10};
	EXPECT_TRUE(eq);

	return;
}

TEST(AppSignalManagerTests, recentUsedRemoveOutdated)
{
	// 1. Outdated is a signal which was not fetched for 3 seconds or more.
	// 2. Outdated signals are removed from the list when hashes() is called
	//
	ClientLib::RecentUsed recentUsed{10};

	for (Hash h = 1; h <= 10; h++)
	{
		recentUsed.add(h);
		QThread::currentThread()->msleep(500);

		[[maybe_unused]] auto s = recentUsed.hashes();	// This keep cache alive, as it has expiration time 3 secs (RecentUsed::ExpiredTimeMs).
	}

	recentUsed.removeOutdated();

	std::vector<Hash> keptSignals = recentUsed.hashes();

	bool eq = keptSignals == std::vector<Hash>{6, 7, 8, 9, 10};
	EXPECT_TRUE(eq);

	return;
}

TEST(AppSignalManagerTests, recentUsedFetchOutdate)
{
	// 1. Outdated is a signal which was not fetched for 3 seconds or more.
	// 2. Outdated signals are removed from the list when hashes() is called
	//
	ClientLib::RecentUsed recentUsed{10};

	for (Hash h = 1; h <= 10; h++)
	{
		recentUsed.add(h);
		QThread::currentThread()->msleep(500);
	}

	recentUsed.add({11, 12, 13});		// Now ftech timer already ellapsed, so no actual add should happen.

	// There were no fetches during 3 secconds, so cache is completely invalidated and
	// not used till any new fetch (no item can be added till call hashes()).
	//
	auto s = recentUsed.hashes();
	EXPECT_TRUE(s.empty());

	recentUsed.removeOutdated();

	return;
}

TEST(AppSignalManagerTests, addRecentAppSignal)
{
	ILogFileStub log;
	ClientLib::AppSignalManager sm{&log};

	AppSignalParam sp1;
	AppSignalParam sp2;
	AppSignalParam sp3;
	sp1.setAppSignalId("#SP1");
	sp2.setAppSignalId("#SP2");
	sp2.setAppSignalId("#SP3");

	std::vector<AppSignalParam> v1;
	v1.push_back(sp1);
	v1.push_back(sp2);
	sm.addSignals(v1, "ADS1");

	std::vector<AppSignalParam> v2;
	v2.push_back(sp1);
	v2.push_back(sp2);
	v2.push_back(sp3);	// ADS2 has one more signal
	sm.addSignals(v2, "ADS2");

	sm.addRecentAppSignals({sp1.hash(), sp2.hash(), sp3.hash()});

	{
		std::vector<Hash> adsSignals = sm.recentlyUsedAppSignals("ADS1");
		EXPECT_EQ(adsSignals.size(), 2);

		bool f1 = std::find(adsSignals.begin(), adsSignals.end(), sp1.hash()) != adsSignals.end();
		bool f2 = std::find(adsSignals.begin(), adsSignals.end(), sp2.hash()) != adsSignals.end();

		EXPECT_TRUE(f1);
		EXPECT_TRUE(f2);
	}

	{
		std::vector<Hash> adsSignals = sm.recentlyUsedAppSignals("ADS2");
		EXPECT_EQ(adsSignals.size(), 3);

		bool f1 = std::find(adsSignals.begin(), adsSignals.end(), sp1.hash()) != adsSignals.end();
		bool f2 = std::find(adsSignals.begin(), adsSignals.end(), sp2.hash()) != adsSignals.end();
		bool f3 = std::find(adsSignals.begin(), adsSignals.end(), sp3.hash()) != adsSignals.end();

		EXPECT_TRUE(f1);
		EXPECT_TRUE(f2);
		EXPECT_TRUE(f3);
	}

	return;
}

TEST(AppSignalManagerTests, signalHashes)
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

	auto hashes = sm.signalHashes();

	EXPECT_EQ(hashes.size(), 2);

	bool f1 = std::find(hashes.begin(), hashes.end(), sp1.hash()) != hashes.end();
	bool f2 = std::find(hashes.begin(), hashes.end(), sp2.hash()) != hashes.end();

	EXPECT_TRUE(f1);
	EXPECT_TRUE(f2);

	return;
}

TEST(AppSignalManagerTests, signalList)
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

	auto allSignals = sm.signalList();

	EXPECT_EQ(allSignals.size(), 2);

	bool f1 = std::find_if(allSignals.begin(), allSignals.end(), [&sp1](const auto& s){ return s.hash() == sp1.hash(); }) != allSignals.end();
	bool f2 = std::find_if(allSignals.begin(), allSignals.end(), [&sp2](const auto& s){ return s.hash() == sp2.hash(); }) != allSignals.end();

	EXPECT_TRUE(f1);
	EXPECT_TRUE(f2);

	return;
}

TEST(AppSignalManagerTests, signalExists)
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
	EXPECT_TRUE(sm.signalExists(sp1.hash()));
	EXPECT_TRUE(sm.signalExists(sp2.hash()));
	EXPECT_TRUE(sm.signalExists(sp1.appSignalId()));
	EXPECT_TRUE(sm.signalExists(sp2.appSignalId()));

	EXPECT_FALSE(sm.signalExists(123ull));
	EXPECT_FALSE(sm.signalExists("#FALSEID"));

	return;
}

TEST(AppSignalManagerTests, signalParam)
{
	ILogFileStub log;
	ClientLib::AppSignalManager sm{&log};

	AppSignalParam sp1;
	AppSignalParam sp2;
	sp1.setAppSignalId("#SP1");
	sp2.setAppSignalId("#SP2");

	sm.addSignal(sp1, "ADS");
	sm.addSignal(sp2, "ADS");

	bool found = false;

	{
		auto hsp = sm.signalParam(sp1.hash(), &found);
		EXPECT_TRUE(found);
		EXPECT_EQ(hsp.hash(), sp1.hash());
		EXPECT_EQ(hsp.appSignalId(), sp1.appSignalId());
	}

	{
		auto hsp = sm.signalParam(sp1.appSignalId(), &found);
		EXPECT_TRUE(found);
		EXPECT_EQ(hsp.hash(), sp1.hash());
		EXPECT_EQ(hsp.appSignalId(), sp1.appSignalId());
	}

	{
		auto hsp = sm.signalParam(sp2.hash(), &found);
		EXPECT_TRUE(found);
		EXPECT_EQ(hsp.hash(), sp2.hash());
		EXPECT_EQ(hsp.appSignalId(), sp2.appSignalId());
	}

	{
		auto hsp = sm.signalParam(sp2.appSignalId(), &found);
		EXPECT_TRUE(found);
		EXPECT_EQ(hsp.hash(), sp2.hash());
		EXPECT_EQ(hsp.appSignalId(), sp2.appSignalId());
	}

	{
		auto hsp = sm.signalParam(123ull, &found);
		EXPECT_FALSE(found);
		EXPECT_EQ(hsp.hash(), 0);
	}

	{
		auto hsp = sm.signalParam("#FALSEID", &found);
		EXPECT_FALSE(found);
		EXPECT_EQ(hsp.hash(), 0);
	}

	return;
}

TEST(AppSignalManagerTests, signalState)
{
	// Test of:
	//	void AppSignalManager::signalState(...);
	//
	ILogFileStub log;
	ClientLib::AppSignalManager sm{&log};

	AppSignalParam sp1;
	AppSignalParam sp2;
	sp1.setAppSignalId("#SP1");
	sp2.setAppSignalId("#SP2");

	std::vector<AppSignalParam> v;
	v.push_back(sp1);
	v.push_back(sp2);

	sm.addSignals(v, "ADS1");
	sm.addSignals(v, "ADS2");

	AppSignalState state1{sp1.hash(), {}, 1.0, {.valid = 1, .stateAvailable = 1}};
	sm.setState("#SP1", state1, std::bit_cast<Qt::HANDLE>(1ull));

	AppSignalState state2{sp2.hash(), {}, 2.0, {.valid = 1, .stateAvailable = 1}};
	sm.setState("#SP2", state2, std::bit_cast<Qt::HANDLE>(1ull));

	// --
	//
	bool found = false;

	auto state = sm.signalState(sp1.appSignalId(), &found);
	EXPECT_TRUE(found);
	EXPECT_TRUE(state.isValid());
	EXPECT_EQ(state.hash(), sp1.hash());
	EXPECT_EQ(state.value(), 1.0);

	state = sm.signalState(sp2.appSignalId(), &found);
	EXPECT_TRUE(found);
	EXPECT_TRUE(state.isValid());
	EXPECT_EQ(state.hash(), sp2.hash());
	EXPECT_EQ(state.value(), 2.0);

	state = sm.signalState("#FALSEID", &found);
	EXPECT_FALSE(found);
	EXPECT_FALSE(state.isValid());
	EXPECT_EQ(state.hash(), ::calcHash(QLatin1String("#FALSEID")));

	std::vector<AppSignalState> recStates;
	int foundCount = 0;
	sm.signalState({sp1.appSignalId(), sp2.appSignalId(), QLatin1String("#FALSEID")}, &recStates, &foundCount);

	ASSERT_EQ(recStates.size(), 3);
	EXPECT_EQ(foundCount, 2);

	EXPECT_TRUE(recStates[0].isValid());
	EXPECT_EQ(recStates[0].hash(), sp1.hash());
	EXPECT_EQ(recStates[0].value(), 1.0);

	EXPECT_TRUE(recStates[1].isValid());
	EXPECT_EQ(recStates[1].hash(), sp2.hash());
	EXPECT_EQ(recStates[1].value(), 2.0);

	EXPECT_FALSE(recStates[2].isValid());
	EXPECT_EQ(recStates[2].hash(), ::calcHash(QLatin1String("#FALSEID")));

	return;
}
