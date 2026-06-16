#include <QSignalSpy>
#include <TestSuiteLib/ScriptRunner.h>
#include <TestSuiteLib/TestControl.h>
#include <TestSuiteLib/TestSuite.h>

#include "MockTestObserver.h"


using ::testing::_;
using ::testing::AtLeast;
using ::testing::An;
using ::testing::Return;


namespace
{
	const QString GlobalScriptText{
		R"('use strict'
function assert(condition, message)
{
    if (!condition)
    {
        message = message || "Assertion failed";
        throw new Error(message);
    }
})"};
	const TestSuite::TestScript GlobalScript{TestSuite::TestScript::GlobalScriptID, GlobalScriptText};


	class TestLogOutput : public TestSuite::ITestLogOutput
	{
		void logItemArrived(const TestSuite::TestLogItem& item) override { qDebug() << "TestLogOutput: " << item.toText(); }
	};


	class TestInputController : public TestSuite::IInputController
	{
	public:
		TestInputController(bool supportsTestObserver = false) :
			m_supportsTestObserver{supportsTestObserver}
		{
		}

		virtual bool init(qint64 /*timeoutMs*/) override { return true; }
		virtual bool shutdown() override { return true; }

		virtual bool signalExists(const QString& signalId) const override { return expectedStates.contains(signalId); }

		virtual std::optional<AppSignalParam> signalParam(const QString& appSignalId) const override
		{
			auto it = expectedStates.find(appSignalId);
			if (it == expectedStates.end())
			{
				return std::nullopt;
			}

			AppSignalParam result;
			result.setAppSignalId(appSignalId);
			return result;
		}

		virtual std::optional<AppSignalState> signalState(const QString& appSignalId) const override
		{
			auto it = expectedStates.find(appSignalId);
			if (it != expectedStates.end())
			{
				return it->second;
			}

			return std::nullopt;
		}

		virtual bool expectSignalValue(QString appSignalId, qint64 /*timeoutMs*/, double value, double tolerance = 0) const override
		{
			auto it = expectedStates.find(appSignalId);
			if (it == expectedStates.end())
			{
				return false;
			}

			return std::abs(it->second.value() - value) <= tolerance;
		}

		virtual tl::expected<std::unique_ptr<ITestObserver>, QString> createTestObserver() override
		{
			if (m_supportsTestObserver == true)
			{
				return std::make_unique<MockTestObserver>();
			}
			else
			{
				return tl::make_unexpected("Not supported");
			}
		}

	private:
		// clang-format off
		// Key is signals id
		const std::map<QString, AppSignalState> expectedStates = 
		{
			{"#EXPECTED10", AppSignalState{::calcHash(QString{"#EXPECTED10"}), Times{}, 10.0, AppSignalStateFlags{.all = 3}}},
			{"#SIGNAL1", AppSignalState{::calcHash(QString{"#SIGNAL1"}), Times{}, 101.0, AppSignalStateFlags{.all = 3}}},
			{"#SIGNAL2", AppSignalState{::calcHash(QString{"#SIGNAL2"}), Times{}, 102.0, AppSignalStateFlags{.all = 3}}}
		};
		// clang-format on

		bool m_supportsTestObserver = false;
	};


	class MockOutputController : public ::TestSuite::IOutputController
	{
	public:
		MOCK_METHOD(bool, init, (qint64 timeoutMs), (override));
		MOCK_METHOD(bool, shutdown, (), (override));
		MOCK_METHOD(bool, writeSignalValue, (const QString& appSignalId, const QVariant& value), (override));
		MOCK_METHOD(bool, waitForAllSignalsWritten, (qint64 timeoutMs, qint64& timeElapsedMs), (const, override));
		MOCK_METHOD(bool, tuningSourceIsActive, (QString lmEquipmentId), (const, override));
		MOCK_METHOD(bool, tuningSourceIsInactive, (QString lmEquipmentId), (const, override));
		MOCK_METHOD(bool, activateTuningSource, (QString lmEquipmentId, bool activate), (override));
	};

	// class TestOutputController : public TestSuite::IOutputController
	//{
	// public:
	//	bool init(qint64 /*timeoutMs*/) override { return true; }
	//	bool shutdown() override { return true; }

	//	bool writeSignalValue(const QString& appSignalId, const QVariant& /*value*/) override
	//	{
	//		if (appSignalId == "#SIGNAL")
	//		{
	//			return true;
	//		}

	//		return false;
	//	}
	//	bool waitForAllSignalsWritten(qint64 /*timeoutMs*/, qint64& /*timeElapsedMs*/) const override { return false; }

	//	bool tuningSourceIsActive(QString /*lmEquipmentId*/) const override { return true; }
	//	bool tuningSourceIsInactive(QString /*lmEquipmentId*/) const override { return true; }
	//	bool activateTuningSource(QString /*lmEquipmentId*/, bool /*activate*/) override { return true; }
	//};


	class MockInputController : public ::TestSuite::IInputController
	{
	public:
		MOCK_METHOD(bool, init, (qint64 timeoutMs), (override));
		MOCK_METHOD(bool, shutdown, (), (override));
		MOCK_METHOD(bool, signalExists, (const QString& signalId), (const, override));
		MOCK_METHOD(std::optional<AppSignalParam>, signalParam, (const QString& appSignalId), (const, override));
		MOCK_METHOD(std::optional<AppSignalState>, signalState, (const QString& appSignalId), (const, override));
		MOCK_METHOD(bool, expectSignalValue, (QString appSignalId, qint64 timeoutMs, double value, double tolerance), (const, override));
	};

	class TestableTestControlThread : public ::TestSuite::TestControlThread
	{
	public:
		TestableTestControlThread(ILogFile* appLog, ::TestSuite::TestLog* testLog) :
			::TestSuite::TestControlThread{appLog, testLog}
		{
		}

	public:
		[[nodiscard]] virtual QString plant() const override { return "TEST_PLANT"; }
		[[nodiscard]] virtual QString unit() const override { return "TEST_UNIT"; }
		[[nodiscard]] virtual QString system() const override { return "TEST_SYSTEM"; }

		[[nodiscard]] virtual QString projectName() const override { return "TEST_PROJECT"; }
		[[nodiscard]] virtual int buildNo() const override { return 111; }
	};

	class TestControlStub : public ::TestSuite::TestControl
	{
	public:
		explicit TestControlStub(ILogFile* appLog, ::TestSuite::TestLog* testLog) :
			::TestSuite::TestControl{appLog, testLog, new TestableTestControlThread{appLog, testLog}}
		{
		}
	};

	class MockILogFile : public ILogFile
	{
	public:
		MOCK_METHOD(bool, writeAlert, (const QString& text, const QString& tag), (override));
		MOCK_METHOD(bool, writeError, (const QString& text, const QString& tag), (override));
		MOCK_METHOD(bool, writeWarning, (const QString& text, const QString& tag), (override));
		MOCK_METHOD(bool, writeMessage, (const QString& text, const QString& tag), (override));
		MOCK_METHOD(bool, writeText, (const QString& text, const QString& tag), (override));
	};

	class MockTestLogOutput : public ::TestSuite::ITestLogOutput
	{
	public:
		MOCK_METHOD(void, logItemArrived, (const TestSuite::TestLogItem& item), (override));
	};

	class MockTestLog : public ::TestSuite::TestLog
	{
	public:
		explicit MockTestLog(MockTestLogOutput& output) :
			::TestSuite::TestLog{&output}
		{
		}

		MOCK_METHOD(bool, writeAlert, (const QString& text, const QString& tag), (override));
		MOCK_METHOD(bool, writeError, (const QString& text, const QString& tag), (override));
		MOCK_METHOD(bool, writeWarning, (const QString& text, const QString& tag), (override));
		MOCK_METHOD(bool, writeMessage, (const QString& text, const QString& tag), (override));
		MOCK_METHOD(bool, writeText, (const QString& text, const QString& tag), (override));
	};

	class MockScriptProvider : public ::TestSuite::IScriptProvider
	{
	public:
		MOCK_METHOD(QStringList, getScriptFileNames, (), (const, override));
		MOCK_METHOD(std::vector<TestSuite::TestScript>, getScripts, (), (const, override));

		MOCK_METHOD(std::optional<TestSuite::TestScript>, getGloablScript, (), (const, override));
		MOCK_METHOD(std::optional<TestSuite::TestScript>, getScriptByFileName, (const QString& fileName), (const, override));
	};
} // namespace


class TestSuiteUnitTest : public ::testing::Test
{
protected:
	virtual void SetUp() {}

	virtual void TearDown() {}

protected:
	SoftwareInfo m_softwareInfo{E::SoftwareType::TestSuite, "TESTSUITE_SOFTWARE_ID"};

	ILogFileConsole m_logStub;

	TestLogOutput m_testLogOutput;
	::TestSuite::TestLog m_testLog{&m_testLogOutput};

	::TestSuite::IScriptProviderStub m_scriptProviderStub;
};

//
// Test creation and stopping of TestSuite.
//
TEST_F(TestSuiteUnitTest, CreatedAndStop)
{
	auto testControl = std::make_unique<TestControlStub>(&m_logStub, &m_testLog);
	auto testControlRawPtr = testControl.get();

	TestSuite::TestSuite testSuite{m_softwareInfo, &m_logStub, m_testLog, std::move(testControl)};

	testSuite.addInputController(std::make_unique<::TestSuite::InputControllerStub>());

	QSignalSpy spyControlTestStarted(testControlRawPtr, &::TestSuite::TestControl::testStarted);
	QSignalSpy spyControlTestFinished(testControlRawPtr, &::TestSuite::TestControl::testFinished);
	QSignalSpy spyControlFinished(testControlRawPtr, &::TestSuite::TestControl::finished);

	QSignalSpy spyTestStarted(&testSuite, &::TestSuite::TestSuite::testStarted);
	QSignalSpy spyTestFinished(&testSuite, &::TestSuite::TestSuite::testFinished);
	QSignalSpy spyFinished(&testSuite, &::TestSuite::TestSuite::finished);

	testSuite.execute(m_scriptProviderStub, {}, nullptr);
	testSuite.stop();

	EXPECT_EQ(spyControlTestStarted.count(), 0);
	EXPECT_EQ(spyControlTestFinished.count(), 0);
	EXPECT_EQ(spyControlFinished.count(), 1);

	EXPECT_EQ(spyTestStarted.count(), 0);
	EXPECT_EQ(spyTestFinished.count(), 0);
	EXPECT_EQ(spyFinished.count(), 1);

	return;
}

//
// Run simple test script, control execution by catching log message.
//
TEST_F(TestSuiteUnitTest, CanRunSimpleEmptyTest)
{
	MockTestLogOutput testLogOutput;
	MockTestLog testLog{testLogOutput};
	EXPECT_CALL(testLog, writeMessage(_, _)).Times(AtLeast(0));
	EXPECT_CALL(testLog, writeMessage(QString("TEST_WAS_EXECUTED"), _)).Times(1);
	EXPECT_CALL(testLog, writeWarning(_, _)).Times(AtLeast(0));
	EXPECT_CALL(testLog, writeText(_, _)).Times(AtLeast(0));

	MockILogFile appLog;
	EXPECT_CALL(appLog, writeMessage(_, _)).Times(AtLeast(0));
	EXPECT_CALL(appLog, writeMessage(QString("<DEBUG> TEST_DEBUG_OUTPUT"), _)).Times(1);
	EXPECT_CALL(appLog, writeWarning(_, _)).Times(AtLeast(0));
	EXPECT_CALL(appLog, writeText(_, _)).Times(AtLeast(0));

	auto testControl = std::make_unique<TestControlStub>(&appLog, &testLog);

	TestSuite::TestSuite testSuite{m_softwareInfo, &appLog, m_testLog, std::move(testControl)};

	testSuite.addInputController(std::make_unique<::TestSuite::InputControllerStub>());
	testSuite.addOutputController(std::make_unique<::TestSuite::OutputControllerStub>());

	// clang-format off
	const QString simpleScript = R"(
function test_simple(ctrl)
{
	log.writeMessage("TEST_WAS_EXECUTED");
	ctrl.debugMessagesEnabled = true;
	if (ctrl.debugMessagesEnabled === false)
	{
		throw new Error("Debug messages are not enabled!");
	}
	ctrl.debugOutput("TEST_DEBUG_OUTPUT");	// Expected "<DEBUG> TEST_DEBUG_OUTPUT" to appLog.
})";
	// clang-format on

	MockScriptProvider scriptProvider;
	EXPECT_CALL(scriptProvider, getGloablScript()).Times(AtLeast(0)).WillRepeatedly(Return(GlobalScript));
	EXPECT_CALL(scriptProvider, getScriptFileNames()).Times(AtLeast(0));
	EXPECT_CALL(scriptProvider, getScripts())
		.Times(AtLeast(0))
		.WillRepeatedly(Return(std::vector<TestSuite::TestScript>{TestSuite::TestScript{"ScriptName", simpleScript}}));

	testSuite.execute(scriptProvider, {}, nullptr);

	QDeadlineTimer timer{5000};
	while (testSuite.isRunning() == true && timer.hasExpired() == false)
	{
		QThread::msleep(20);
	}

	EXPECT_EQ(testSuite.isRunning(), false);

	return;
}

//
// Test without input controller.
//
TEST_F(TestSuiteUnitTest, RunWithoutInputController)
{
	MockTestLogOutput testLogOutput;
	MockTestLog testLog{testLogOutput};
	EXPECT_CALL(testLog, writeMessage(_, _)).Times(AtLeast(0));
	EXPECT_CALL(testLog, writeWarning(_, _)).Times(AtLeast(0));
	EXPECT_CALL(testLog, writeText(_, _)).Times(AtLeast(0));
	EXPECT_CALL(testLog, writeError(_, _)).Times(AtLeast(0));

	auto testControl = std::make_unique<TestControlStub>(&m_logStub, &testLog);

	TestSuite::TestSuite testSuite{m_softwareInfo, &m_logStub, m_testLog, std::move(testControl)};

	// clang-format off
	std::array<std::pair<QString, QString>, 6> scripts = {
		std::make_pair("signalState",		R"(function test_signalState(ctrl)			{ ctrl.signalState("#SOMESIGNAL");})"),
		std::make_pair("signalValue",		R"(function test_signalValue(ctrl)			{ ctrl.signalValue("#SOMESIGNAL");})"),
		std::make_pair("expectSignalValue", R"(function test_expectSignalValue(ctrl)	{ ctrl.expectSignalValue("#SOMESIGNAL", 100, 0, 0);})"),
		std::make_pair("signalExists",		R"(function test_signalExists(ctrl)			{ ctrl.signalExists("#SOMESIGNAL");})"),
		std::make_pair("signalParam",		R"(function test_signalParam(ctrl)			{ ctrl.signalParam("#SOMESIGNAL");})"),
		std::make_pair("createObserver",	R"(function test_createObserver(ctrl)		{ ctrl.createObserver();})")};
	// clang-format on

	MockScriptProvider scriptProvider;
	EXPECT_CALL(scriptProvider, getGloablScript()).Times(AtLeast(0));
	EXPECT_CALL(scriptProvider, getScriptFileNames()).Times(AtLeast(0));
	EXPECT_CALL(scriptProvider, getScripts())
		.Times(AtLeast(0))
		.WillOnce(Return(std::vector<TestSuite::TestScript>{TestSuite::TestScript{scripts[0].first, scripts[0].second}}))
		.WillOnce(Return(std::vector<TestSuite::TestScript>{TestSuite::TestScript{scripts[1].first, scripts[1].second}}))
		.WillOnce(Return(std::vector<TestSuite::TestScript>{TestSuite::TestScript{scripts[2].first, scripts[2].second}}))
		.WillOnce(Return(std::vector<TestSuite::TestScript>{TestSuite::TestScript{scripts[3].first, scripts[3].second}}))
		.WillOnce(Return(std::vector<TestSuite::TestScript>{TestSuite::TestScript{scripts[4].first, scripts[4].second}}))
		.WillOnce(Return(std::vector<TestSuite::TestScript>{TestSuite::TestScript{scripts[5].first, scripts[5].second}}));

	// Expected signal `void finished(int result);`, where result == 1
	//
	QSignalSpy spyFinished(&testSuite, &::TestSuite::TestSuite::finished);

	// Repeat scripts.size() times, each time scriptProvider will return different script on getScripts().
	//
	for (size_t i = 0; i < scripts.size(); i++)
	{
		testSuite.execute(scriptProvider, {}, nullptr);

		QDeadlineTimer timer{5000};
		while (testSuite.isRunning() == true && timer.hasExpired() == false)
		{
			QThread::msleep(20);
		}

		EXPECT_EQ(testSuite.isRunning(), false);

		ASSERT_EQ(spyFinished.count(), 1);
		ASSERT_EQ(spyFinished.at(0).size(), 1);
		EXPECT_EQ(spyFinished.at(0).at(0).toInt(), 1);

		spyFinished.clear();
	}

	return;
}

//
// Test without output controller.
//
TEST_F(TestSuiteUnitTest, RunWithoutOutputController)
{
	MockTestLogOutput testLogOutput;
	MockTestLog testLog{testLogOutput};
	EXPECT_CALL(testLog, writeMessage(_, _)).Times(AtLeast(0));
	EXPECT_CALL(testLog, writeWarning(_, _)).Times(AtLeast(0));
	EXPECT_CALL(testLog, writeText(_, _)).Times(AtLeast(0));
	EXPECT_CALL(testLog, writeError(_, _)).Times(AtLeast(0));

	auto testControl = std::make_unique<TestControlStub>(&m_logStub, &testLog);

	TestSuite::TestSuite testSuite{m_softwareInfo, &m_logStub, m_testLog, std::move(testControl)};

	// clang-format off
	std::array<std::pair<QString, QString>, 6> scripts = {
		std::make_pair("overrideSignalValue",		R"(function test_overrideSignalValue(ctrl)		{ ctrl.overrideSignalValue("#SOMESIGNAL", 1);})"),
		std::make_pair("waitForSignalOverrides",	R"(function test_waitForSignalOverrides(ctrl)	{ ctrl.waitForSignalOverrides(1000);})"),
		std::make_pair("overridesReset",			R"(function test_overridesReset(ctrl)			{ ctrl.overridesReset(1000);})"),
		std::make_pair("tuningSourceIsActive",		R"(function test_tuningSourceIsActive(ctrl)		{ ctrl.tuningSourceIsActive("LMID");})"),
		std::make_pair("tuningSourceIsInactive",	R"(function test_tuningSourceIsInactive(ctrl)	{ ctrl.tuningSourceIsInactive("LMID");})"),
		std::make_pair("activateTuningSource",		R"(function test_activateTuningSource(ctrl)		{ ctrl.activateTuningSource("LMID", true);})")};
	// clang-format on

	MockScriptProvider scriptProvider;
	EXPECT_CALL(scriptProvider, getGloablScript()).Times(AtLeast(0));
	EXPECT_CALL(scriptProvider, getScriptFileNames()).Times(AtLeast(0));
	EXPECT_CALL(scriptProvider, getScripts())
		.Times(AtLeast(0))
		.WillOnce(Return(std::vector<TestSuite::TestScript>{TestSuite::TestScript{scripts[0].first, scripts[0].second}}))
		.WillOnce(Return(std::vector<TestSuite::TestScript>{TestSuite::TestScript{scripts[1].first, scripts[1].second}}))
		.WillOnce(Return(std::vector<TestSuite::TestScript>{TestSuite::TestScript{scripts[2].first, scripts[2].second}}))
		.WillOnce(Return(std::vector<TestSuite::TestScript>{TestSuite::TestScript{scripts[3].first, scripts[3].second}}))
		.WillOnce(Return(std::vector<TestSuite::TestScript>{TestSuite::TestScript{scripts[4].first, scripts[4].second}}))
		.WillOnce(Return(std::vector<TestSuite::TestScript>{TestSuite::TestScript{scripts[5].first, scripts[5].second}}));

	// Expected signal `void finished(int result);`, where result == 1
	//
	QSignalSpy spyFinished(&testSuite, &::TestSuite::TestSuite::finished);

	// Repeat scripts.size() times, each time scriptProvider will return different script on getScripts().
	//
	for (size_t i = 0; i < scripts.size(); i++)
	{
		testSuite.execute(scriptProvider, {}, nullptr);

		QDeadlineTimer timer{5000};
		while (testSuite.isRunning() == true && timer.hasExpired() == false)
		{
			QThread::msleep(20);
		}

		EXPECT_EQ(testSuite.isRunning(), false);

		ASSERT_EQ(spyFinished.count(), 1);
		ASSERT_EQ(spyFinished.at(0).size(), 1);
		EXPECT_EQ(spyFinished.at(0).at(0).toInt(), 1);

		spyFinished.clear();
	}

	return;
}

//
// Test without output controller.
//
TEST_F(TestSuiteUnitTest, ScriptFuncStartForMs)
{
	auto testControl = std::make_unique<TestControlStub>(&m_logStub, &m_testLog);

	TestSuite::TestSuite testSuite{m_softwareInfo, &m_logStub, m_testLog, std::move(testControl)};

	// clang-format off
	QString script = R"(function test_startForMs(ctrl)		
{ 
	let start = Date.now();

	ctrl.startForMs(500);

	let end = Date.now();
	let duration = end - start;

	if (duration < 500 || duration > 1000)
	{
		log.writeError("Duration is not in range 500-1000ms: " + duration);
		return false;
	}
})";
	// clang-format on

	MockScriptProvider scriptProvider;
	EXPECT_CALL(scriptProvider, getGloablScript()).Times(AtLeast(0));
	EXPECT_CALL(scriptProvider, getScriptFileNames()).Times(AtLeast(0));
	EXPECT_CALL(scriptProvider, getScripts())
		.Times(AtLeast(0))
		.WillOnce(Return(std::vector<TestSuite::TestScript>{TestSuite::TestScript{"test_startForMs", script}}));

	QSignalSpy spyFinished(&testSuite, &::TestSuite::TestSuite::finished);

	testSuite.execute(scriptProvider, {}, nullptr);

	QDeadlineTimer timer{5000};
	while (testSuite.isRunning() == true && timer.hasExpired() == false)
	{
		QThread::msleep(20);
	}

	EXPECT_EQ(testSuite.isRunning(), false);

	ASSERT_EQ(spyFinished.count(), 1);
	ASSERT_EQ(spyFinished.at(0).size(), 1);
	EXPECT_EQ(spyFinished.at(0).at(0).toInt(), 0); // 0 - suceess

	return;
}

//
// Test checks returned properties  projectName, buildNo.
//
TEST_F(TestSuiteUnitTest, ScriptCheckProperties)
{
	auto testControl = std::make_unique<TestControlStub>(&m_logStub, &m_testLog);

	TestSuite::TestSuite testSuite{m_softwareInfo, &m_logStub, m_testLog, std::move(testControl)};

	// clang-format off
	QString script = R"(function test_checkProperties(ctrl)		
{ 
	assert(ctrl.projectName === "TEST_PROJECT", "ProjectName is not 'TEST_PROJECT', but " + ctrl.projectName);
	assert(ctrl.buildNo === 111, "BuildNo is not 111, but " + ctrl.buildNo);
})";
	// clang-format on

	MockScriptProvider scriptProvider;
	EXPECT_CALL(scriptProvider, getGloablScript()).Times(AtLeast(0)).WillRepeatedly(Return(GlobalScript));
	EXPECT_CALL(scriptProvider, getScriptFileNames()).Times(AtLeast(0));
	EXPECT_CALL(scriptProvider, getScripts())
		.Times(AtLeast(0))
		.WillOnce(Return(std::vector<TestSuite::TestScript>{TestSuite::TestScript{"test_startForMs", script}}));

	QSignalSpy spyFinished(&testSuite, &::TestSuite::TestSuite::finished);

	testSuite.execute(scriptProvider, {}, nullptr);

	QDeadlineTimer timer{5000};
	while (testSuite.isRunning() == true && timer.hasExpired() == false)
	{
		QThread::msleep(20);
	}

	EXPECT_EQ(testSuite.isRunning(), false);

	ASSERT_EQ(spyFinished.count(), 1);
	ASSERT_EQ(spyFinished.at(0).size(), 1);
	EXPECT_EQ(spyFinished.at(0).at(0).toInt(), 0); // 0 - suceess

	return;
}

//
// Test checks that functions:
//		initTestCase() - will be called before the first test function is executed.
//		cleanupTestCase() - will be called after the last test function was executed.
//		init() - will be called before each test function is executed.
//		cleanup() - will be called after every test function.
//
TEST_F(TestSuiteUnitTest, InitCleanUpAreCalled)
{
	MockTestLogOutput testLogOutput;
	MockTestLog testLog{testLogOutput};
	EXPECT_CALL(testLog, writeMessage(_, _)).Times(AtLeast(0));
	EXPECT_CALL(testLog, writeWarning(_, _)).Times(AtLeast(0));
	EXPECT_CALL(testLog, writeText(_, _)).Times(AtLeast(0));

	EXPECT_CALL(testLog, writeWarning(QString("initTestCase"), _)).Times(1);
	EXPECT_CALL(testLog, writeWarning(QString("cleanupTestCase"), _)).Times(1);

	EXPECT_CALL(testLog, writeMessage(QString("init 2 times"), _)).Times(2);
	EXPECT_CALL(testLog, writeMessage(QString("cleanup 2 times"), _)).Times(2);

	EXPECT_CALL(testLog, writeText(QString("test_t1"), _)).Times(1);
	EXPECT_CALL(testLog, writeText(QString("test_t2"), _)).Times(1);

	auto testControl = std::make_unique<TestControlStub>(&m_logStub, &testLog);

	TestSuite::TestSuite testSuite{m_softwareInfo, &m_logStub, testLog, std::move(testControl)};

	QString script = R"(
// Before the first test function is executed.
function initTestCase(ctrl)
{
	log.writeWarning("initTestCase");
}

// After the last test function was executed.
function cleanupTestCase(ctrl)
{
	log.writeWarning("cleanupTestCase");
}

// Before EACH test function is executed.
//
function init(ctrl)
{
	log.writeMessage("init 2 times");
}

// After EACH test function.
//
function cleanup(ctrl)
{
	log.writeMessage("cleanup 2 times");
}

function test_t1(ctrl)
{
	log.writeText("test_t1");
}

function test_t2(ctrl)
{
	log.writeText("test_t2");
}
)";

	MockScriptProvider scriptProvider;
	EXPECT_CALL(scriptProvider, getGloablScript()).Times(AtLeast(0)).WillRepeatedly(Return(GlobalScript));
	EXPECT_CALL(scriptProvider, getScriptFileNames()).Times(AtLeast(0));
	EXPECT_CALL(scriptProvider, getScripts())
		.Times(AtLeast(0))
		.WillOnce(Return(std::vector<TestSuite::TestScript>{TestSuite::TestScript{"SomeTestFile", script}}));

	QSignalSpy spyFinished(&testSuite, &::TestSuite::TestSuite::finished);

	testSuite.execute(scriptProvider, {}, nullptr);

	QDeadlineTimer timer{5000};
	while (testSuite.isRunning() == true && timer.hasExpired() == false)
	{
		QThread::msleep(20);
	}

	EXPECT_EQ(testSuite.isRunning(), false);

	ASSERT_EQ(spyFinished.count(), 1);
	ASSERT_EQ(spyFinished.at(0).size(), 1);
	EXPECT_EQ(spyFinished.at(0).at(0).toInt(), 0); // 0 - suceess

	return;
}

//
// Test script functions ctrl.signalState(), ctrl.signalValue().
//
TEST_F(TestSuiteUnitTest, ScriptFuncSignalStateAndValue)
{
	auto testControl = std::make_unique<TestControlStub>(&m_logStub, &m_testLog);

	TestSuite::TestSuite testSuite{m_softwareInfo, &m_logStub, m_testLog, std::move(testControl)};

	testSuite.addInputController(std::make_unique<TestInputController>());

	// clang-format off
	// ScriptFileName, ScriptContent, ExpectedResult (0 success, 1 fail)
	//
	constexpr int Success = 0;
	constexpr int Fail = 1;

	std::array<std::tuple<QString, QString, int>, 6> scripts = {
		std::make_tuple(QString{"signalStateNoSignal"},		QString{R"(function test_signalStateNoSignal(ctrl)		{ ctrl.signalState("#NO_SIGNAL");})"},							Fail),
		std::make_tuple(QString{"signalStateExpected10"},	QString{R"(function test_signalStateExpected10(ctrl)	{ assert(ctrl.signalState("#EXPECTED10").value === 10);})"},	Success),
		std::make_tuple(QString{"signalStateExpected99"},	QString{R"(function test_signalStateExpected99(ctrl)	{ assert(ctrl.signalState("#EXPECTED99").value === 111);})"},	Fail),

		std::make_tuple(QString{"signalValueNoSignal"},		QString{R"(function test_signalValueNoSignal(ctrl)		{ ctrl.signalValue("#NO_SIGNAL");})"},							Fail),
		std::make_tuple(QString{"signalValueExpected10"},	QString{R"(function test_signalValueExpected10(ctrl)	{ assert(ctrl.signalValue("#EXPECTED10") === 10);})"},			Success),
		std::make_tuple(QString{"signalValueExpected99"},	QString{R"(function test_signalValueExpected99(ctrl)	{ assert(ctrl.signalValue("#EXPECTED99") === 111);})"},			Fail),
	};

	MockScriptProvider scriptProvider;
	EXPECT_CALL(scriptProvider, getGloablScript())
		.Times(AtLeast(0))
		.WillRepeatedly(Return(GlobalScript));
	EXPECT_CALL(scriptProvider, getScriptFileNames()).Times(AtLeast(0));
	EXPECT_CALL(scriptProvider, getScripts())
		.Times(AtLeast(0))
		.WillOnce(Return(std::vector<TestSuite::TestScript>{TestSuite::TestScript{std::get<0>(scripts[0]), std::get<1>(scripts[0])}}))
		.WillOnce(Return(std::vector<TestSuite::TestScript>{TestSuite::TestScript{std::get<0>(scripts[1]), std::get<1>(scripts[1])}}))
		.WillOnce(Return(std::vector<TestSuite::TestScript>{TestSuite::TestScript{std::get<0>(scripts[1]), std::get<1>(scripts[2])}}))
		.WillOnce(Return(std::vector<TestSuite::TestScript>{TestSuite::TestScript{std::get<0>(scripts[3]), std::get<1>(scripts[3])}}))
		.WillOnce(Return(std::vector<TestSuite::TestScript>{TestSuite::TestScript{std::get<0>(scripts[4]), std::get<1>(scripts[4])}}))
		.WillOnce(Return(std::vector<TestSuite::TestScript>{TestSuite::TestScript{std::get<0>(scripts[5]), std::get<1>(scripts[5])}}));

	// clang-format on

	QSignalSpy spyFinished(&testSuite, &::TestSuite::TestSuite::finished);

	// Repeat scripts.size() times, each time scriptProvider will return different script on getScripts().
	//
	for (const auto& [fileName, script, expectedResult] : scripts)
	{
		testSuite.execute(scriptProvider, {}, nullptr);

		QDeadlineTimer timer{5000};
		while (testSuite.isRunning() == true && timer.hasExpired() == false)
		{
			QThread::msleep(20);
		}

		EXPECT_EQ(testSuite.isRunning(), false);

		ASSERT_EQ(spyFinished.count(), 1);
		ASSERT_EQ(spyFinished.at(0).size(), 1);
		EXPECT_EQ(spyFinished.at(0).at(0).toInt(), expectedResult);

		spyFinished.clear();
	}

	return;
}

//
// Test script function ctrl.signalExists().
//
TEST_F(TestSuiteUnitTest, ScriptFuncSignalExists)
{
	auto testControl = std::make_unique<TestControlStub>(&m_logStub, &m_testLog);

	TestSuite::TestSuite testSuite{m_softwareInfo, &m_logStub, m_testLog, std::move(testControl)};

	testSuite.addInputController(std::make_unique<TestInputController>());

	// clang-format off
	// ScriptFileName, ScriptContent, ExpectedResult (0 success, 1 fail)
	//
	[[maybe_unused]] constexpr int Success = 0;
	[[maybe_unused]] constexpr int Fail = 1;

	std::array<std::tuple<QString, QString, int>, 2> scripts = {
		std::make_tuple(QString{"signalStateExists1"},	QString{R"(function test_signalStateExists1(ctrl)	{ assert(!ctrl.signalExists("#NO_SIGNAL"));})"},	Success),
		std::make_tuple(QString{"signalStateExists2"},	QString{R"(function test_signalStateExists2(ctrl)	{ assert(ctrl.signalExists("#EXPECTED10"));})"},	Success)
	};

	MockScriptProvider scriptProvider;
	EXPECT_CALL(scriptProvider, getGloablScript())
		.Times(AtLeast(0))
		.WillRepeatedly(Return(GlobalScript));
	EXPECT_CALL(scriptProvider, getScriptFileNames()).Times(AtLeast(0));
	EXPECT_CALL(scriptProvider, getScripts())
		.Times(AtLeast(0))
		.WillOnce(Return(std::vector<TestSuite::TestScript>{TestSuite::TestScript{std::get<0>(scripts[0]), std::get<1>(scripts[0])}}))
		.WillOnce(Return(std::vector<TestSuite::TestScript>{TestSuite::TestScript{std::get<0>(scripts[1]), std::get<1>(scripts[1])}}));

	// clang-format on

	QSignalSpy spyFinished(&testSuite, &::TestSuite::TestSuite::finished);

	// Repeat scripts.size() times, each time scriptProvider will return different script on getScripts().
	//
	for (const auto& [fileName, script, expectedResult] : scripts)
	{
		testSuite.execute(scriptProvider, {}, nullptr);

		QDeadlineTimer timer{5000};
		while (testSuite.isRunning() == true && timer.hasExpired() == false)
		{
			QThread::msleep(20);
		}

		EXPECT_EQ(testSuite.isRunning(), false);

		ASSERT_EQ(spyFinished.count(), 1);
		ASSERT_EQ(spyFinished.at(0).size(), 1);
		EXPECT_EQ(spyFinished.at(0).at(0).toInt(), expectedResult);

		spyFinished.clear();
	}

	return;
}

//
// Test script function ctrl.signalParam().
//
TEST_F(TestSuiteUnitTest, ScriptFuncSignalParam)
{
	auto testControl = std::make_unique<TestControlStub>(&m_logStub, &m_testLog);

	TestSuite::TestSuite testSuite{m_softwareInfo, &m_logStub, m_testLog, std::move(testControl)};

	testSuite.addInputController(std::make_unique<TestInputController>());

	// clang-format off
	// ScriptFileName, ScriptContent, ExpectedResult (0 success, 1 fail)
	//
	constexpr int Success = 0;
	constexpr int Fail = 1;

	std::array<std::tuple<QString, QString, int>, 2> scripts = {
		std::make_tuple(QString{"signalStateParam1"},	QString{R"(function test_signalStateParam1(ctrl)	{ ctrl.signalParam("#NO_SIGNAL");})"},	Fail),
		std::make_tuple(QString{"signalStateParam2"},	QString{R"(function test_signalStateParam2(ctrl)	{ assert(ctrl.signalParam("#EXPECTED10").appSignalID === "#EXPECTED10");})"},	Success)
	};

	MockScriptProvider scriptProvider;
	EXPECT_CALL(scriptProvider, getGloablScript())
		.Times(AtLeast(0))
		.WillRepeatedly(Return(GlobalScript));
	EXPECT_CALL(scriptProvider, getScriptFileNames()).Times(AtLeast(0));
	EXPECT_CALL(scriptProvider, getScripts())
		.Times(AtLeast(0))
		.WillOnce(Return(std::vector<TestSuite::TestScript>{TestSuite::TestScript{std::get<0>(scripts[0]), std::get<1>(scripts[0])}}))
		.WillOnce(Return(std::vector<TestSuite::TestScript>{TestSuite::TestScript{std::get<0>(scripts[1]), std::get<1>(scripts[1])}}));

	// clang-format on

	QSignalSpy spyFinished(&testSuite, &::TestSuite::TestSuite::finished);

	// Repeat scripts.size() times, each time scriptProvider will return different script on getScripts().
	//
	for (const auto& [fileName, script, expectedResult] : scripts)
	{
		testSuite.execute(scriptProvider, {}, nullptr);

		QDeadlineTimer timer{5000};
		while (testSuite.isRunning() == true && timer.hasExpired() == false)
		{
			QThread::msleep(20);
		}

		EXPECT_EQ(testSuite.isRunning(), false);

		ASSERT_EQ(spyFinished.count(), 1);
		ASSERT_EQ(spyFinished.at(0).size(), 1);
		EXPECT_EQ(spyFinished.at(0).at(0).toInt(), expectedResult);

		spyFinished.clear();
	}

	return;
}

//
// Test execution timeout.
//
TEST_F(TestSuiteUnitTest, ExecutionTimeOut)
{
	MockTestLogOutput testLogOutput;
	MockTestLog testLog{testLogOutput};
	EXPECT_CALL(testLog, writeMessage(_, _)).Times(AtLeast(0));
	EXPECT_CALL(testLog, writeMessage(QString("THIS_LINE_MUST_NOT_BE_RUN"), _)).Times(0);
	EXPECT_CALL(testLog, writeWarning(_, _)).Times(AtLeast(0));
	EXPECT_CALL(testLog, writeText(_, _)).Times(AtLeast(0));
	EXPECT_CALL(testLog, writeError(_, _)).Times(AtLeast(1)); // Expected error message about timeout.

	auto testControl = std::make_unique<TestControlStub>(&m_logStub, &testLog);

	TestSuite::TestSuite testSuite{m_softwareInfo, &m_logStub, testLog, std::move(testControl)};

	// clang-format off
	const QString simpleScript = R"(
function test_simple(ctrl)
{
	ctrl.executionTimeout = 500;
	
	// ctrl.waitForMs(1000);  Timeout somehow not triggered when control in this function.

	let start = Date.now();
	let elapsed = 0;
	while (elapsed < 1000) { // Run the loop for 1 second
		elapsed = Date.now() - start;
	}

	// Expect that timeout will be triggered and test will be terminated.
	//
	log.writeMessage("THIS_LINE_MUST_NOT_BE_RUN");
})";
	// clang-format on

	MockScriptProvider scriptProvider;
	EXPECT_CALL(scriptProvider, getGloablScript()).Times(AtLeast(0));
	EXPECT_CALL(scriptProvider, getScriptFileNames()).Times(AtLeast(0));
	EXPECT_CALL(scriptProvider, getScripts())
		.Times(AtLeast(0))
		.WillRepeatedly(Return(std::vector<TestSuite::TestScript>{TestSuite::TestScript{"ScriptName", simpleScript}}));

	testSuite.execute(scriptProvider, {}, nullptr);

	QDeadlineTimer timer{5000};
	while (testSuite.isRunning() == true && timer.hasExpired() == false)
	{
		QThread::msleep(20);
	}

	EXPECT_EQ(testSuite.isRunning(), false);

	return;
}

//
// Try to create test observer, but observer is not supported by input controller.
//
TEST_F(TestSuiteUnitTest, TestObserverNotSupported)
{
	auto testControl = std::make_unique<TestControlStub>(&m_logStub, &m_testLog);

	TestSuite::TestSuite testSuite{m_softwareInfo, &m_logStub, m_testLog, std::move(testControl)};
	testSuite.addInputController(std::make_unique<TestInputController>());

	QSignalSpy spyFinished(&testSuite, &::TestSuite::TestSuite::finished);

	// clang-format off
	const QString simpleScript = R"(
function test_createTestObserver(ctrl)
{
	observer = ctrl.createObserver(); // Triggers exception as observer is not supported by input controller.
})";
	// clang-format on

	MockScriptProvider scriptProvider;
	EXPECT_CALL(scriptProvider, getGloablScript()).Times(AtLeast(0));
	EXPECT_CALL(scriptProvider, getScriptFileNames()).Times(AtLeast(0));
	EXPECT_CALL(scriptProvider, getScripts())
		.Times(AtLeast(0))
		.WillRepeatedly(Return(std::vector<TestSuite::TestScript>{TestSuite::TestScript{"ScriptName", simpleScript}}));

	testSuite.execute(scriptProvider, {}, nullptr);

	QDeadlineTimer timer{5000};
	while (testSuite.isRunning() == true && timer.hasExpired() == false)
	{
		QThread::msleep(20);
	}

	EXPECT_EQ(testSuite.isRunning(), false);

	ASSERT_EQ(spyFinished.count(), 1);
	ASSERT_EQ(spyFinished.at(0).size(), 1);
	EXPECT_EQ(spyFinished.at(0).at(0).toInt(), 1); // 1 - fail

	return;
}

//
// Try to create test observer, but observer is not supported by input controller.
//
TEST_F(TestSuiteUnitTest, CreateTestObserver)
{
	auto testControl = std::make_unique<TestControlStub>(&m_logStub, &m_testLog);

	TestSuite::TestSuite testSuite{m_softwareInfo, &m_logStub, m_testLog, std::move(testControl)};
	testSuite.addInputController(std::make_unique<TestInputController>(true));

	QSignalSpy spyFinished(&testSuite, &::TestSuite::TestSuite::finished);

	// clang-format off
	const QString simpleScript = R"(
function test_createTestObserver(ctrl)
{
	observer = ctrl.createObserver();
})";
	// clang-format on

	MockScriptProvider scriptProvider;
	EXPECT_CALL(scriptProvider, getGloablScript()).Times(AtLeast(0));
	EXPECT_CALL(scriptProvider, getScriptFileNames()).Times(AtLeast(0));
	EXPECT_CALL(scriptProvider, getScripts())
		.Times(AtLeast(0))
		.WillRepeatedly(Return(std::vector<TestSuite::TestScript>{TestSuite::TestScript{"ScriptName", simpleScript}}));

	testSuite.execute(scriptProvider, {}, nullptr);

	QDeadlineTimer timer{5000};
	while (testSuite.isRunning() == true && timer.hasExpired() == false)
	{
		QThread::msleep(20);
	}

	EXPECT_EQ(testSuite.isRunning(), false);

	ASSERT_EQ(spyFinished.count(), 1);
	ASSERT_EQ(spyFinished.at(0).size(), 1);
	EXPECT_EQ(spyFinished.at(0).at(0).toInt(), 0); // 0 - success

	return;
}

//
// Try ctrl.overrideSignalValue(QString appSignalId, QVariant value)
//
TEST_F(TestSuiteUnitTest, OverrideSignalValue)
{
	auto testControl = std::make_unique<TestControlStub>(&m_logStub, &m_testLog);

	TestSuite::TestSuite testSuite{m_softwareInfo, &m_logStub, m_testLog, std::move(testControl)};

	testSuite.addInputController(std::make_unique<TestInputController>(true));

	auto outputController = std::make_unique<MockOutputController>();
	EXPECT_CALL(*outputController, init(_)).Times(1).WillOnce(Return(true));
	EXPECT_CALL(*outputController, shutdown()).Times(1).WillOnce(Return(true));
	EXPECT_CALL(*outputController, writeSignalValue(QString{"#SIGNAL1"}, QVariant{101})).Times(1).WillOnce(Return(true));
	EXPECT_CALL(*outputController, writeSignalValue(QString{"#SIGNAL1"}, QVariant{0}))
		.Times(1)
		.WillOnce(Return(true));       // called in overridesReset
	EXPECT_CALL(*outputController, writeSignalValue(QString{"#SIGNAL2"}, QVariant{102})).Times(1).WillOnce(Return(true));
	EXPECT_CALL(*outputController, writeSignalValue(QString{"#SIGNAL2"}, QVariant{0}))
		.Times(1)
		.WillOnce(Return(true));       // called in overridesReset

	EXPECT_CALL(*outputController, waitForAllSignalsWritten(_, _))
		.Times(AtLeast(1))
		.WillRepeatedly(Return(true)); // 1 - directly for script, 1 for overridesReset

	testSuite.addOutputController(std::move(outputController));

	QSignalSpy spyFinished(&testSuite, &::TestSuite::TestSuite::finished);

	// clang-format off
	const QString simpleScript = R"(
function test_overrideSignalValue(ctrl)
{
	assert(ctrl.overrideSignalValue("#SIGNAL1", 101) === true);
	assert(ctrl.overrideSignalValue("#SIGNAL2", 102) === true);
	assert(ctrl.waitForSignalOverrides(1000) === true);
	assert(ctrl.getOverridenSignals().length === 2);
	ctrl.overridesReset(1000);
	assert(ctrl.getOverridenSignals().length === 0);
})";
	// clang-format on

	MockScriptProvider scriptProvider;
	EXPECT_CALL(scriptProvider, getGloablScript()).Times(AtLeast(0)).WillRepeatedly(Return(GlobalScript));
	EXPECT_CALL(scriptProvider, getScriptFileNames()).Times(AtLeast(0));
	EXPECT_CALL(scriptProvider, getScripts())
		.Times(AtLeast(0))
		.WillRepeatedly(Return(std::vector<TestSuite::TestScript>{TestSuite::TestScript{"ScriptName", simpleScript}}));

	testSuite.execute(scriptProvider, {}, nullptr);

	QDeadlineTimer timer{5000};
	while (testSuite.isRunning() == true && timer.hasExpired() == false)
	{
		QThread::msleep(20);
	}

	EXPECT_EQ(testSuite.isRunning(), false);

	ASSERT_EQ(spyFinished.count(), 1);
	ASSERT_EQ(spyFinished.at(0).size(), 1);
	EXPECT_EQ(spyFinished.at(0).at(0).toInt(), 0); // 0 - success

	return;
}

//
// Try ctrl.expectSignalValue(QString appSignalId, qint64 timeoutMs, double value, double tolerance /*= 0*/)
//
TEST_F(TestSuiteUnitTest, expectSignalValue)
{
	auto testControl = std::make_unique<TestControlStub>(&m_logStub, &m_testLog);

	TestSuite::TestSuite testSuite{m_softwareInfo, &m_logStub, m_testLog, std::move(testControl)};

	auto inputController = std::make_unique<TestInputController>(true);
	testSuite.addInputController(std::move(inputController));

	QSignalSpy spyFinished(&testSuite, &::TestSuite::TestSuite::finished);

	// clang-format off
	const QString simpleScript = R"(
function test_expectSignalValue(ctrl)
{
	assert(ctrl.expectSignalValue("#SIGNAL1", 1000, 101, 0) === true);
})";
	// clang-format on

	MockScriptProvider scriptProvider;
	EXPECT_CALL(scriptProvider, getGloablScript()).Times(AtLeast(0)).WillRepeatedly(Return(GlobalScript));
	EXPECT_CALL(scriptProvider, getScriptFileNames()).Times(AtLeast(0));
	EXPECT_CALL(scriptProvider, getScripts())
		.Times(AtLeast(0))
		.WillRepeatedly(Return(std::vector<TestSuite::TestScript>{TestSuite::TestScript{"ScriptName", simpleScript}}));

	testSuite.execute(scriptProvider, {}, nullptr);

	QDeadlineTimer timer{5000};
	while (testSuite.isRunning() == true && timer.hasExpired() == false)
	{
		QThread::msleep(20);
	}

	EXPECT_EQ(testSuite.isRunning(), false);

	ASSERT_EQ(spyFinished.count(), 1);
	ASSERT_EQ(spyFinished.at(0).size(), 1);
	EXPECT_EQ(spyFinished.at(0).at(0).toInt(), 0); // 0 - success

	return;
}


//
// Try ScriptRunner should provide evaluate scrips and provide them.
//
TEST_F(TestSuiteUnitTest, ScriptRunnerShouldEvaluateScripts)
{
	// clang-format off
	const QString scriptFileName = "ScriptFileName";

	const QString simpleScript = R"(

var captionScriptFileName = "Script Sample Name";

var ScriptTags = ["tag1", "tag2"];

function test_test1(ctrl)
{
	assert(ctrl.expectSignalValue("#SIGNAL1", 1000, 101, 0) === true);
}

var caption_test2 = "Test2 Caption";
function test_test2(ctrl)
{
	assert(false);
}

function test_test3(ctrl)
{
	assert(true);
}
)";
	// clang-format on

	TestSuite::OutputControllerStub outputControllerStub;
	TestSuite::InputControllerStub inputControllerStub;

	TestSuite::TestController testController{nullptr, nullptr, &inputControllerStub, &outputControllerStub, nullptr};

	TestSuite::ControlStatus fakeStatus;
	QMutex fakeStatusMutex;

	TestSuite::ScriptRunner sr{"TESTSUITE_SOFTWARE_ID",
							   TestSuite::TestScript{scriptFileName, simpleScript},
							   GlobalScript,
							   testController,
							   m_testLog,
							   fakeStatus,
							   fakeStatusMutex};

	const TestSuite::ScriptInfo& scriptInfo = sr.scriptInfo();

	ASSERT_EQ(scriptInfo.testsCount(), 3);
	EXPECT_FALSE(scriptInfo.empty());

	EXPECT_TRUE(scriptInfo.testsList.contains("test_test1"));
	EXPECT_TRUE(scriptInfo.testsList.contains("test_test2"));
	EXPECT_TRUE(scriptInfo.testsList.contains("test_test3"));

	EXPECT_EQ(scriptInfo.fileName, scriptFileName);
	EXPECT_EQ(scriptInfo.scriptCaption, "Script Sample Name");

	// Check captions
	//
	bool found = false;
	QString c1 = scriptInfo.testCaption("test_test1", &found);
	EXPECT_FALSE(found);

	QString c2 = scriptInfo.testCaption("test_test2", &found);
	EXPECT_TRUE(found);
	EXPECT_EQ(c2, "Test2 Caption");

	QString c3 = scriptInfo.testCaption("test_test3", &found);
	EXPECT_FALSE(found);

	// Check tags
	//
	bool tag12 = scriptInfo.checkScriptTags({"tag1", "tag2"});
	bool tag2 = scriptInfo.checkScriptTags({"tag2"});
	bool tag3 = scriptInfo.checkScriptTags({"ttt", "t"});
	bool tagTa = scriptInfo.checkScriptTags({"ta"});
	bool tagE = scriptInfo.checkScriptTags({});

	EXPECT_TRUE(tag12);
	EXPECT_TRUE(tag2);
	EXPECT_FALSE(tag3);
	EXPECT_FALSE(tagTa);
	EXPECT_FALSE(tagE);

	TestSuite::ScriptInfo scriptInfo2{"asd"}; // If tags are empty, then all tags are accepted.

	bool tag12_2 = scriptInfo2.checkScriptTags({"tag1", "tag2"});
	bool tag2_2 = scriptInfo2.checkScriptTags({"tag2"});
	EXPECT_TRUE(tag12_2);
	EXPECT_TRUE(tag2_2);

	return;
}