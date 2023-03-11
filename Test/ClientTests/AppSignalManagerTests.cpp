#include "../../ClientLib/AppSignalManager.h"

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

using namespace testing;

TEST(AppSignalManagerTests, Reset)
{
	ILogFileStub log;
	ClientLib::AppSignalManager sm(&log);

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

