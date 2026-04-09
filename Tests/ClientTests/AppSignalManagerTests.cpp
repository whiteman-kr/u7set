#include "../../AppSignalLib/ComparatorSet.h"
#include <ClientLib/AppSignalManager.h>
#include <gtest/gtest.h>
#include <span>

using namespace testing;
using SourceIdType = ClientLib::IAppSignalUpdater::SourceIdType;

TEST(AppSignalManagerTests, reset)
{
	ILogFileStub log;
	ClientLib::AppSignalManager sm{&log};

	Proto::AppSignal psp1;
	Proto::AppSignal psp2;
	psp1.set_appsignalid("#SP1");
	psp2.set_appsignalid("#SP2");

	std::array<Proto::AppSignal, 2> arr = {psp1, psp2};
	sm.addSignals(arr, "ADS");

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

	Proto::AppSignal sp1;
	Proto::AppSignal sp2;
	sp1.set_appsignalid("#SP1");
	sp2.set_appsignalid("#SP2");

	std::array<Proto::AppSignal, 2> arr = {sp1, sp2};
	sm.addSignals(arr, "ADS");

	EXPECT_EQ(sm.signalsCount(), 2);

	return;
}

TEST(AppSignalManagerTests, addSignals)
{
	ILogFileStub log;
	ClientLib::AppSignalManager sm{&log};

	QSignalSpy spy{&sm, &ClientLib::AppSignalManager::signalParamsUpdated};

	Proto::AppSignal sp1;
	Proto::AppSignal sp2;
	sp1.set_appsignalid("#SP1");
	sp2.set_appsignalid("#SP2");

	std::vector<Proto::AppSignal> v;
	v.push_back(sp1);
	v.push_back(sp2);

	sm.addSignals(v, "ADS");

	EXPECT_EQ(sm.signalsCount(), 2);

	return;
}

TEST(AppSignalManagerTests, invalidateSignalStates)
{
	// Test of:
	//	void AppSignalManager::setState(const QString& appSignalId, const AppSignalState& state, SourceIdType sourceThreadId);
	//
	ILogFileStub log;
	ClientLib::AppSignalManager sm{&log};

	QString dataServerId{"DATA_SERVERID"};
	Hash dataServerHash = ::calcHash(dataServerId);

	QSignalSpy spy{&sm, &ClientLib::AppSignalManager::signalParamsUpdated};

	Proto::AppSignal sp1;
	Proto::AppSignal sp2;
	sp1.set_appsignalid("#SP1");
	sp2.set_appsignalid("#SP2");

	std::vector<Proto::AppSignal> v;
	v.push_back(sp1);
	v.push_back(sp2);

	sm.addSignals(v, "ADS1");
	sm.addSignals(v, "ADS2");

	EXPECT_EQ(sm.signalsCount(), 2);

	auto thread1 = SourceIdType(1ull);
	auto thread2 = SourceIdType(2ull);

	std::array<Proto::AppSignalState, 1> state1 = {
		AppSignalState{::calcHash(sp2.appsignalid()), {0, 0, 0}, 123.0, {.valid = 1, .stateAvailable = 1}}.save()};
	sm.setStates(state1, dataServerHash, thread1);

	QThread::currentThread()->msleep(10);

	std::array<Proto::AppSignalState, 1> state2 = {
		AppSignalState{::calcHash(sp2.appsignalid()), {1, 1, 1}, 124.0, {.valid = 1, .stateAvailable = 1}}.save()};
	sm.setStates(state2, dataServerHash, thread2);

	// --
	//
	auto state = sm.signalState(::calcHash(sp2.appsignalid()));
	ASSERT_TRUE(state.has_value());
	EXPECT_TRUE(state->isValid());
	EXPECT_DOUBLE_EQ(state->value(), state2[0].value());

	sm.invalidateSignalStates(thread2);

	state = sm.signalState(::calcHash(sp2.appsignalid()));
	ASSERT_TRUE(state.has_value());
	EXPECT_TRUE(state->isValid());
	EXPECT_DOUBLE_EQ(state->value(), state1[0].value());

	sm.invalidateSignalStates(thread1);

	state = sm.signalState(::calcHash(sp2.appsignalid()));
	ASSERT_TRUE(state.has_value());
	EXPECT_FALSE(state->isValid());

	return;
}

TEST(AppSignalManagerTests, setState)
{
	// Test of:
	//	void AppSignalManager::setState(const QString& appSignalId, const AppSignalState& state, SourceIdType sourceThreadId);
	//
	ILogFileStub log;
	ClientLib::AppSignalManager sm{&log};

	QString dataServerId{"DATA_SERVERID"};
	Hash dataServerHash = ::calcHash(dataServerId);

	QSignalSpy spy{&sm, &ClientLib::AppSignalManager::signalParamsUpdated};

	Proto::AppSignal sp1;
	Proto::AppSignal sp2;
	sp1.set_appsignalid("#SP1");
	sp2.set_appsignalid("#SP2");

	Hash sp1h = calcHash(sp1.appsignalid());
	Hash sp2h = calcHash(sp2.appsignalid());

	std::vector<Proto::AppSignal> v;
	v.push_back(sp1);
	v.push_back(sp2);

	sm.addSignals(v, "ADS1");
	sm.addSignals(v, "ADS2");

	EXPECT_EQ(sm.signalsCount(), 2);

	std::array<Proto::AppSignalState, 1> state1 = {AppSignalState{sp2h, {}, 123.0, {.valid = 1, .stateAvailable = 0}}.save()};
	sm.setStates(state1, dataServerHash, 1ull);

	QThread::currentThread()->msleep(10);

	std::array<Proto::AppSignalState, 1> state2 = {AppSignalState{sp2h, {}, 124.0, {.valid = 1, .stateAvailable = 0}}.save()};
	sm.setStates(state2, dataServerHash, 2ull);

	// Check #SP1 is not valid
	//
	auto state = sm.signalState(sp1h);
	ASSERT_TRUE(state.has_value());
	EXPECT_FALSE(state->isValid());
	EXPECT_EQ(state->hash(), sp1h);

	// Check #SP2 is valid
	//
	state = sm.signalState(sp2h);
	ASSERT_TRUE(state.has_value());
	EXPECT_DOUBLE_EQ(state->value(), state2[0].value()); // .stateAvailable = 0, so the actual value will be from "thread" 2
	EXPECT_EQ(state->hash(), sp2h);

	return;
}

TEST(AppSignalManagerTests, setStateAsVector)
{
	// Test of:
	//	void AppSignalManager::setState(const std::vector<AppSignalState>& states, SourceIdType sourceThreadId);
	//
	ILogFileStub log;
	ClientLib::AppSignalManager sm{&log};

	QString dataServerId{"DATA_SERVERID"};
	Hash dataServerHash = ::calcHash(dataServerId);

	QSignalSpy spy{&sm, &ClientLib::AppSignalManager::signalParamsUpdated};

	Proto::AppSignal sp1;
	Proto::AppSignal sp2;
	sp1.set_appsignalid("#SP1");
	sp2.set_appsignalid("#SP2");

	Hash sp1h = calcHash(sp1.appsignalid());
	Hash sp2h = calcHash(sp2.appsignalid());

	std::vector<Proto::AppSignal> v;
	v.push_back(sp1);
	v.push_back(sp2);

	sm.addSignals(v, "ADS1");
	sm.addSignals(v, "ADS2");

	EXPECT_EQ(sm.signalsCount(), 2);

	AppSignalState state1{sp1h, {}, 1.0, {.valid = 1, .stateAvailable = 1}};
	AppSignalState state2{sp2h, {}, 2.0, {.valid = 1, .stateAvailable = 1}};

	sm.setStates(std::vector{state1.save(), state2.save()}, dataServerHash, 1ull);

	// --
	//
	auto state = sm.signalState(sp1h);
	ASSERT_TRUE(state.has_value());

	EXPECT_TRUE(state->isValid());
	EXPECT_EQ(state->hash(), sp1h);
	EXPECT_EQ(state->value(), 1.0);

	// --
	//
	state = sm.signalState(sp2h);
	ASSERT_TRUE(state.has_value());

	EXPECT_TRUE(state->isValid());
	EXPECT_EQ(state->hash(), sp2h);
	EXPECT_EQ(state->value(), 2.0);

	return;
}

#if 0
TEST(AppSignalManagerTests, setStateForZeroHash)
{
	// Set state for 0 hash, assert is expected
	//
	#ifdef QT_DEBUG
	ASSERT_DEATH(
		{
			ILogFileStub log;
			ClientLib::AppSignalManager sm{&log};

			AppSignalState state(Hash{UNDEFINED_HASH}, Times{0, 0, 0}, 123.0, AppSignalStateFlags{.valid = 1, .stateAvailable = 1});
			std::span<const AppSignalState> spanState(&state, 1);

			sm.setStates(spanState, UNDEFINED_HASH, 1ull);
		},
		"");
	#endif
}
#endif

TEST(AppSignalManagerTests, recentUsedAdd)
{
	// 1. Outdated is a signal which was not fetched for 3 seconds or more.
	// 2. Outdated signals are removed from the list when hashes() is called
	//

	AppSignalLib::RecentUsed recentUsed{5}; // Keep up to 5 signals

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
	AppSignalLib::RecentUsed recentUsed{10}; // Keep up to 5 signals

	std::array<Hash, 10> testAddSet = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	recentUsed.add(testAddSet);

	recentUsed.remove(2);

	std::array<Hash, 3> testRemoveSet = {4, 6, 8};
	recentUsed.remove(testRemoveSet);

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
	AppSignalLib::RecentUsed recentUsed{10};

	for (Hash h = 1; h <= 10; h++)
	{
		recentUsed.add(h);
		QThread::currentThread()->msleep(500);

		[[maybe_unused]] auto s =
			recentUsed.hashes(); // This keep cache alive, as it has expiration time 3 secs (RecentUsed::ExpiredTimeMs).
	}

	QThread::currentThread()->msleep(50);

	recentUsed.removeOutdated();

	std::vector<Hash> keptSignals = recentUsed.hashes();
	std::vector<Hash> expSignals{6, 7, 8, 9, 10};

	EXPECT_EQ(keptSignals, expSignals);

	return;
}

TEST(AppSignalManagerTests, recentUsedFetchOutdate)
{
	// 1. Outdated is a signal which was not fetched for 3 seconds or more.
	// 2. Outdated signals are removed from the list when hashes() is called
	//
	AppSignalLib::RecentUsed recentUsed{10};

	for (Hash h = 1; h <= 10; h++)
	{
		recentUsed.add(h);
		QThread::currentThread()->msleep(500);
	}

	QThread::currentThread()->msleep(50);

	recentUsed.add(std::vector<Hash>{11, 12, 13}); // Now fetch timer already elapsed, so no actual add should happen.

	// There were no fetches during 3 seconds, so cache is completely invalidated and
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

	Proto::AppSignal sp1;
	Proto::AppSignal sp2;
	Proto::AppSignal sp3;
	sp1.set_appsignalid("#SP1");
	sp2.set_appsignalid("#SP2");
	sp3.set_appsignalid("#SP3");

	auto sp1h = calcHash(sp1.appsignalid());
	auto sp2h = calcHash(sp2.appsignalid());
	auto sp3h = calcHash(sp3.appsignalid());

	std::vector<Proto::AppSignal> v1;
	v1.push_back(sp1);
	v1.push_back(sp2);
	sm.addSignals(v1, "ADS1");

	std::vector<Proto::AppSignal> v2;
	v2.push_back(sp1);
	v2.push_back(sp2);
	v2.push_back(sp3); // ADS2 has one more signal
	sm.addSignals(v2, "ADS2");

	sm.addRecentAppSignals(std::vector{sp1h, sp2h, sp3h});

	{
		std::vector<Hash> adsSignals = sm.recentlyUsedAppSignals("ADS1");
		EXPECT_EQ(adsSignals.size(), 2);

		bool f1 = std::find(adsSignals.begin(), adsSignals.end(), sp1h) != adsSignals.end();
		bool f2 = std::find(adsSignals.begin(), adsSignals.end(), sp2h) != adsSignals.end();

		EXPECT_TRUE(f1);
		EXPECT_TRUE(f2);
	}

	{
		std::vector<Hash> adsSignals = sm.recentlyUsedAppSignals("ADS2");
		EXPECT_EQ(adsSignals.size(), 3);

		bool f1 = std::find(adsSignals.begin(), adsSignals.end(), sp1h) != adsSignals.end();
		bool f2 = std::find(adsSignals.begin(), adsSignals.end(), sp2h) != adsSignals.end();
		bool f3 = std::find(adsSignals.begin(), adsSignals.end(), sp3h) != adsSignals.end();

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
	Proto::AppSignal sp1;
	Proto::AppSignal sp2;
	sp1.set_appsignalid("#SP1");
	sp2.set_appsignalid("#SP2");

	auto sp1h = calcHash(sp1.appsignalid());
	auto sp2h = calcHash(sp2.appsignalid());

	std::array<Proto::AppSignal, 2> arr = {sp1, sp2};
	sm.addSignals(arr, "ADS");

	EXPECT_EQ(sm.signalsCount(), 2);

	auto hashes = sm.signalHashes();

	EXPECT_EQ(hashes.size(), 2);

	bool f1 = std::find(hashes.begin(), hashes.end(), sp1h) != hashes.end();
	bool f2 = std::find(hashes.begin(), hashes.end(), sp2h) != hashes.end();

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
	Proto::AppSignal sp1;
	Proto::AppSignal sp2;
	sp1.set_appsignalid("#SP1");
	sp2.set_appsignalid("#SP2");

	auto sp1h = calcHash(sp1.appsignalid());
	auto sp2h = calcHash(sp2.appsignalid());

	std::array<Proto::AppSignal, 2> arr = {sp1, sp2};
	sm.addSignals(arr, "ADS");

	EXPECT_EQ(sm.signalsCount(), 2);

	auto allSignals = sm.signalList();

	EXPECT_EQ(allSignals.size(), 2);

	bool f1 = std::find_if(allSignals.begin(),
						   allSignals.end(),
						   [sp1h](const auto& s)
						   {
							   return s.hash() == sp1h;
						   }) != allSignals.end();

	bool f2 = std::find_if(allSignals.begin(),
						   allSignals.end(),
						   [sp2h](const auto& s)
						   {
							   return s.hash() == sp2h;
						   }) != allSignals.end();

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
	Proto::AppSignal sp1;
	Proto::AppSignal sp2;
	sp1.set_appsignalid("#SP1");
	sp2.set_appsignalid("#SP2");

	auto sp1h = calcHash(sp1.appsignalid());
	auto sp2h = calcHash(sp2.appsignalid());

	std::array<Proto::AppSignal, 2> arr = {sp1, sp2};
	sm.addSignals(arr, "ADS");

	EXPECT_EQ(sm.signalsCount(), 2);
	EXPECT_TRUE(sm.signalExists(sp1h));
	EXPECT_TRUE(sm.signalExists(sp2h));
	EXPECT_TRUE(sm.signalExists(QString::fromStdString(sp1.appsignalid())));
	EXPECT_TRUE(sm.signalExists(QString::fromStdString(sp2.appsignalid())));

	EXPECT_FALSE(sm.signalExists(123ull));
	EXPECT_FALSE(sm.signalExists("#FALSEID"));

	return;
}

TEST(AppSignalManagerTests, signalParam)
{
	ILogFileStub log;
	ClientLib::AppSignalManager sm{&log};

	Proto::AppSignal sp1;
	Proto::AppSignal sp2;
	sp1.set_appsignalid("#SP1");
	sp2.set_appsignalid("#SP2");

	auto sp1h = calcHash(sp1.appsignalid());
	auto sp2h = calcHash(sp2.appsignalid());

	std::array<Proto::AppSignal, 2> arr = {sp1, sp2};
	sm.addSignals(arr, "ADS");

	{
		auto hsp = sm.signalParam(sp1h);
		ASSERT_TRUE(hsp.has_value());

		EXPECT_EQ(hsp->hash(), sp1h);
		EXPECT_EQ(hsp->appSignalId().toStdString(), sp1.appsignalid());
	}

	{
		auto hsp = sm.signalParam(QString::fromStdString(sp1.appsignalid()));
		ASSERT_TRUE(hsp.has_value());

		EXPECT_EQ(hsp->hash(), sp1h);
		EXPECT_EQ(hsp->appSignalId().toStdString(), sp1.appsignalid());
	}

	{
		auto hsp = sm.signalParam(sp2h);
		ASSERT_TRUE(hsp.has_value());

		EXPECT_EQ(hsp->hash(), sp2h);
		EXPECT_EQ(hsp->appSignalId().toStdString(), sp2.appsignalid());
	}

	{
		auto hsp = sm.signalParam(QString::fromStdString(sp2.appsignalid()));
		ASSERT_TRUE(hsp.has_value());

		EXPECT_EQ(hsp->hash(), sp2h);
		EXPECT_EQ(hsp->appSignalId().toStdString(), sp2.appsignalid());
	}

	{
		auto hsp = sm.signalParam(123ull);
		ASSERT_FALSE(hsp.has_value());
	}

	{
		auto hsp = sm.signalParam("#FALSEID");
		ASSERT_FALSE(hsp.has_value());
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

	QString dataServerId{"DATA_SERVERID"};
	Hash dataServerHash = ::calcHash(dataServerId);

	Proto::AppSignal sp1;
	Proto::AppSignal sp2;
	sp1.set_appsignalid("#SP1");
	sp2.set_appsignalid("#SP2");

	auto sp1h = calcHash(sp1.appsignalid());
	auto sp2h = calcHash(sp2.appsignalid());

	std::vector<Proto::AppSignal> v;
	v.push_back(sp1);
	v.push_back(sp2);

	sm.addSignals(v, "ADS1");
	sm.addSignals(v, "ADS2");

	std::array<::Proto::AppSignalState, 1> state1 = {AppSignalState{sp1h, {}, 1.0, {.valid = 1, .stateAvailable = 1}}.save()};
	sm.setStates(state1, dataServerHash, 1ull);

	std::array<::Proto::AppSignalState, 1> state2 = {AppSignalState{sp2h, {}, 2.0, {.valid = 1, .stateAvailable = 1}}.save()};
	sm.setStates(state2, dataServerHash, 1ull);

	// --
	//
	auto state = sm.signalState(QString::fromStdString(sp1.appsignalid()));
	ASSERT_TRUE(state.has_value());

	EXPECT_TRUE(state->isValid());
	EXPECT_EQ(state->hash(), sp1h);
	EXPECT_EQ(state->value(), 1.0);

	state = sm.signalState(QString::fromStdString(sp2.appsignalid()));
	ASSERT_TRUE(state.has_value());

	EXPECT_TRUE(state->isValid());
	EXPECT_EQ(state->hash(), sp2h);
	EXPECT_EQ(state->value(), 2.0);

	state = sm.signalState("#FALSEID");
	ASSERT_FALSE(state.has_value());

	std::vector<std::optional<AppSignalState>> recStates;

	std::vector<QString> ids = {QString::fromStdString(sp1.appsignalid()),
								QString::fromStdString(sp2.appsignalid()),
								QLatin1String("#FALSEID")};
	sm.signalState(ids, &recStates);

	ASSERT_EQ(recStates.size(), ids.size());

	ASSERT_TRUE(recStates[0].has_value());
	ASSERT_TRUE(recStates[1].has_value());
	ASSERT_FALSE(recStates[2].has_value());

	EXPECT_TRUE(recStates[0]->isValid());
	EXPECT_EQ(recStates[0]->hash(), sp1h);
	EXPECT_EQ(recStates[0]->value(), 1.0);

	EXPECT_TRUE(recStates[1]->isValid());
	EXPECT_EQ(recStates[1]->hash(), sp2h);
	EXPECT_EQ(recStates[1]->value(), 2.0);

	return;
}

TEST(AppSignalManagerTests, signalTags)
{
	ILogFileStub log;
	ClientLib::AppSignalManager sm{&log};

	Proto::AppSignal sp1;
	sp1.set_appsignalid("#SP1");
	sp1.add_tags("sp1");
	sp1.add_tags("ads");
	sp1.add_tags("test");

	Proto::AppSignal sp2;
	sp2.set_appsignalid("#SP2");
	sp2.add_tags("sp2");
	sp2.add_tags("ads");
	sp2.add_tags("test");
	sp2.add_tags("tags");

	std::array<Proto::AppSignal, 2> arr = {sp1, sp2};
	sm.addSignals(arr, "ADS");

	QStringList tags1 = sm.signalTags(QString::fromStdString(sp1.appsignalid()));
	QStringList tags2 = sm.signalTags(QString::fromStdString(sp2.appsignalid()));

	EXPECT_EQ(tags1.size(), 3);
	EXPECT_EQ(tags2.size(), 4);

	EXPECT_EQ(tags1, QStringList{} << "ads" << "sp1" << "test");
	EXPECT_EQ(tags2, QStringList{} << "ads" << "sp2" << "tags" << "test");

	return;
}

TEST(AppSignalManagerTests, signalHasTag)
{
	ILogFileStub log;
	ClientLib::AppSignalManager sm{&log};

	Proto::AppSignal sp1;
	sp1.set_appsignalid("#SP1");
	sp1.add_tags("sp1");
	sp1.add_tags("ads");
	sp1.add_tags("test");

	Proto::AppSignal sp2;
	sp2.set_appsignalid("#SP2");
	sp2.add_tags("sp2");
	sp2.add_tags("ads");
	sp2.add_tags("test");
	sp2.add_tags("tags");

	std::array<Proto::AppSignal, 2> arr = {sp1, sp2};
	sm.addSignals(arr, "ADS");

	EXPECT_TRUE(sm.signalHasTag(QString::fromStdString(sp1.appsignalid()), "sp1"));
	EXPECT_TRUE(sm.signalHasTag(QString::fromStdString(sp1.appsignalid()), "ads"));
	EXPECT_TRUE(sm.signalHasTag(QString::fromStdString(sp1.appsignalid()), "test"));
	EXPECT_FALSE(sm.signalHasTag(QString::fromStdString(sp1.appsignalid()), "fail"));
	EXPECT_FALSE(sm.signalHasTag(QString::fromStdString(sp1.appsignalid()), ""));

	EXPECT_TRUE(sm.signalHasTag(QString::fromStdString(sp2.appsignalid()), "sp2"));
	EXPECT_TRUE(sm.signalHasTag(QString::fromStdString(sp2.appsignalid()), "ads"));
	EXPECT_TRUE(sm.signalHasTag(QString::fromStdString(sp2.appsignalid()), "test"));
	EXPECT_TRUE(sm.signalHasTag(QString::fromStdString(sp2.appsignalid()), "tags"));
	EXPECT_FALSE(sm.signalHasTag(QString::fromStdString(sp2.appsignalid()), "fails"));

	return;
}

TEST(AppSignalManagerTests, signalIdsByTag)
{
	ILogFileStub log;
	ClientLib::AppSignalManager sm{&log};

	Proto::AppSignal sp1;
	sp1.set_appsignalid("#SP1");
	sp1.add_tags("sp1");
	sp1.add_tags("ads");
	sp1.add_tags("test");

	Proto::AppSignal sp2;
	sp2.set_appsignalid("#SP2");
	sp2.add_tags("sp2");
	sp2.add_tags("ads");
	sp2.add_tags("test");
	sp2.add_tags("tags");

	sm.addSignals(std::span<const Proto::AppSignal>{&sp1, 1}, "ADS1");
	sm.addSignals(std::span<const Proto::AppSignal>{&sp2, 1}, "ADS2");

	QStringList signals_sp1 = sm.signalIdsByTag("sp1");
	QStringList signals_ads = sm.signalIdsByTag("ads");
	QStringList signals_tags = sm.signalIdsByTag("tags");
	QStringList signals_empty = sm.signalIdsByTag("");

	EXPECT_EQ(signals_sp1, QStringList{} << "#SP1");
	EXPECT_EQ(signals_ads, QStringList{} << "#SP1" << "#SP2");
	EXPECT_EQ(signals_tags, QStringList{} << "#SP2");
	EXPECT_EQ(signals_empty, QStringList{});

	return;
}

TEST(AppSignalManagerTests, signalType)
{
	ILogFileStub log;
	ClientLib::AppSignalManager sm{&log};

	Proto::AppSignal sp1;
	sp1.set_appsignalid("#SP1");
	sp1.set_signaltype(static_cast<::int32_t>(E::SignalType::Analog));

	Proto::AppSignal sp2;
	sp2.set_appsignalid("#SP2");
	sp2.set_signaltype(static_cast<::int32_t>(E::SignalType::Discrete));

	sm.addSignals(std::span<const Proto::AppSignal>{&sp1, 1}, "ADS1");
	sm.addSignals(std::span<const Proto::AppSignal>{&sp2, 1}, "ADS2");

	bool f1 = false;
	bool f2 = false;
	bool f3 = false;

	auto t1 = sm.signalType(::calcHash(sp1.appsignalid()), &f1);
	auto t2 = sm.signalType(QString::fromStdString(sp2.appsignalid()), &f2);
	[[maybe_unused]] auto t3 = sm.signalType("#FAIL", &f3);

	EXPECT_TRUE(f1);
	EXPECT_TRUE(f2);
	EXPECT_FALSE(f3);

	EXPECT_EQ(t1, E::SignalType::Analog);
	EXPECT_EQ(t2, E::SignalType::Discrete);

	return;
}

TEST(AppSignalManagerTests, equipmentToAppSignalId)
{
	ILogFileStub log;
	ClientLib::AppSignalManager sm{&log};

	Proto::AppSignal sp1;
	sp1.set_appsignalid("#SP1");
	sp1.set_equipmentid("USB_LM1_IN1");

	Proto::AppSignal sp2;
	sp2.set_appsignalid("#SP2");
	sp2.set_equipmentid("USB_LM1_IN2");

	Proto::AppSignal sp3;
	sp3.set_appsignalid("#SP3");
	sp3.set_equipmentid("USB_LM1_IN3");

	auto v = std::array<Proto::AppSignal, 3>{sp1, sp2, sp3};
	sm.addSignals(v, "ADS");

	EXPECT_EQ(sm.equipmentToAppSignalId("@USB_LM1_IN1"), "#SP1"); // Symbol @ must be at the beginning
	EXPECT_EQ(sm.equipmentToAppSignalId("@USB_LM1_IN2"), "#SP2");
	EXPECT_EQ(sm.equipmentToAppSignalId("@USB_LM1_IN3"), "#SP3");
	EXPECT_EQ(sm.equipmentToAppSignalId("@FAIL"), "");

	return;
}

TEST(AppSignalManagerTests, setpointsByInput)
{
	ILogFileStub log;
	ClientLib::AppSignalManager sm{&log};

	Proto::AppSignal sp1;
	sp1.set_appsignalid("#SP1");
	sp1.set_equipmentid("LM1");

	Proto::AppSignal sp2;
	sp2.set_appsignalid("#SP2");
	sp2.set_equipmentid("LM2");

	std::array<Proto::AppSignal, 2> arr = {sp1, sp2};
	sm.addSignals(arr, "ADS");

	// Create ComparatorSet
	//
	auto cmp_sp1_1 = std::make_shared<Comparator>();
	cmp_sp1_1->input().setSignalParams(QString::fromStdString(sp1.appsignalid()), true, true, 1.0);
	cmp_sp1_1->output().setSignalParams("#OUT_CMP1_1", true, false, 1.0);
	cmp_sp1_1->setLabel("cmp_sp1_1");

	auto cmp_sp1_2 = std::make_shared<Comparator>();
	cmp_sp1_2->input().setSignalParams(QString::fromStdString(sp1.appsignalid()), true, true, 2.0);
	cmp_sp1_2->output().setSignalParams("#OUT_CMP1_2", true, false, 2.0);
	cmp_sp1_2->setLabel("cmp_sp1_2");

	auto cmp_sp2_1 = std::make_shared<Comparator>();
	cmp_sp2_1->input().setSignalParams(QString::fromStdString(sp2.appsignalid()), true, true, 3.0);
	cmp_sp2_1->output().setSignalParams("#OUT_CMP2_1", true, false, 2.0);
	cmp_sp2_1->setLabel("cmp_sp2_1");

	ComparatorSet cs;
	cs.insert("LM1", cmp_sp1_1);
	cs.insert("LM1", cmp_sp1_2);
	cs.insert("LM2", cmp_sp2_1);

	sm.setSetpoints(cs);

	auto sp1_comparators = sm.setpointsByInput(QString::fromStdString(sp1.appsignalid()));
	auto sp2_comparators = sm.setpointsByInput(QString::fromStdString(sp2.appsignalid()));

	ASSERT_EQ(sp1_comparators.size(), 2);
	ASSERT_TRUE((sp1_comparators[0]->label() == "cmp_sp1_1" && sp1_comparators[1]->label() == "cmp_sp1_2") ||
				(sp1_comparators[1]->label() == "cmp_sp1_1" && sp1_comparators[0]->label() == "cmp_sp1_2"));

	ASSERT_EQ(sp2_comparators.size(), 1);
	ASSERT_EQ(sp2_comparators[0]->label(), "cmp_sp2_1");

	return;
}

TEST(AppSignalManagerTests, dataServiceIds)
{
	ILogFileStub log;
	ClientLib::AppSignalManager sm{&log};

	::Proto::AppSignal sp1;
	::Proto::AppSignal sp2;
	::Proto::AppSignal sp3;

	sp1.set_appsignalid("#SP1");
	sp2.set_appsignalid("#SP2");

	std::array<Proto::AppSignal, 1> ads1v = {sp1};
	std::array<Proto::AppSignal, 2> ads2v = {sp1, sp2};

	sm.addSignals(ads1v, "ADS1");
	sm.addSignals(ads2v, "ADS2");

	auto s1adses = sm.dataServiceIds(sp1.appsignalid());
	auto s2adses = sm.dataServiceIds(sp2.appsignalid());
	auto s3adses = sm.dataServiceIds("#FALSEID");

	ASSERT_EQ(s1adses.size(), 2);
	EXPECT_TRUE((s1adses[0] == "ADS1" && s1adses[1] == "ADS2") || (s1adses[1] == "ADS1" && s1adses[0] == "ADS2"));

	ASSERT_EQ(s2adses.size(), 1);
	EXPECT_EQ(s2adses[0], "ADS2");

	EXPECT_EQ(s3adses.size(), 0);

	return;
}

TEST(AppSignalManagerTests, dataServiceHasSignal)
{
	ILogFileStub log;
	ClientLib::AppSignalManager sm{&log};

	Proto::AppSignal sp1;
	Proto::AppSignal sp2;
	Proto::AppSignal sp3;

	sp1.set_appsignalid("#SP1");
	sp2.set_appsignalid("#SP2");

	std::array<Proto::AppSignal, 1> ads1v = {sp1};
	std::array<Proto::AppSignal, 2> ads2v = {sp1, sp2};

	sm.addSignals(ads1v, "ADS1");
	sm.addSignals(ads2v, "ADS2");

	EXPECT_TRUE(sm.dataServiceHasSignal("ADS1", sp1.appsignalid()));
	EXPECT_TRUE(sm.dataServiceHasSignal("ADS2", sp1.appsignalid()));

	EXPECT_FALSE(sm.dataServiceHasSignal("ADS1", sp2.appsignalid()));
	EXPECT_TRUE(sm.dataServiceHasSignal("ADS2", sp2.appsignalid()));

	EXPECT_FALSE(sm.dataServiceHasSignal("ADS2", "#FALSEID"));
	EXPECT_FALSE(sm.dataServiceHasSignal("FALSE_ADS", sp1.appsignalid()));

	return;
}

TEST(AppSignalManagerTests, tags)
{
	ILogFileStub log;
	ClientLib::AppSignalManager sm{&log};

	Proto::AppSignal sp1;
	sp1.set_appsignalid("#SP1");
	sp1.add_tags("sp1");
	sp1.add_tags("ads");
	sp1.add_tags("test");

	Proto::AppSignal sp2;
	sp2.set_appsignalid("#SP2");
	sp2.add_tags("sp2");
	sp2.add_tags("ads");
	sp2.add_tags("test");
	sp2.add_tags("tags");

	std::array<Proto::AppSignal, 2> arr = {sp1, sp2};
	sm.addSignals(arr, "ADS");

	QStringList tags = sm.tags();

	EXPECT_EQ(tags.size(), 5);
	EXPECT_TRUE(tags.contains("sp1"));
	EXPECT_TRUE(tags.contains("ads"));
	EXPECT_TRUE(tags.contains("test"));
	EXPECT_TRUE(tags.contains("sp2"));
	EXPECT_TRUE(tags.contains("tags"));

	return;
}

TEST(AppSignalManagerTests, signalParamByEquipemntId)
{
	ILogFileStub log;
	ClientLib::AppSignalManager sm{&log};

	Proto::AppSignal sp1;
	sp1.set_appsignalid("#SP1");
	sp1.set_equipmentid("USB_LM1_IN1");

	Proto::AppSignal sp2;
	sp2.set_appsignalid("#SP2");
	sp2.set_equipmentid("USB_LM1_IN2");

	std::array<Proto::AppSignal, 2> arr = {sp1, sp2};
	sm.addSignals(arr, "ADS");

	auto signalParam1 = sm.signalParamByEquipmentId("@USB_LM1_IN1");
	auto signalParam2 = sm.signalParamByEquipmentId("@USB_LM1_IN2");
	auto signalParam3 = sm.signalParamByEquipmentId("@FLASE_EQ_ID");

	ASSERT_TRUE(signalParam1.has_value());
	ASSERT_TRUE(signalParam2.has_value());
	ASSERT_FALSE(signalParam3.has_value());

	EXPECT_EQ(signalParam1->appSignalId().toStdString(), sp1.appsignalid());
	EXPECT_EQ(signalParam2->appSignalId().toStdString(), sp2.appsignalid());

	return;
}
