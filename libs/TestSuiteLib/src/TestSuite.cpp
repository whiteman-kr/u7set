#include <TestSuiteLib/TestControl.h>
#include <TestSuiteLib/TestSuite.h>

namespace TestSuite
{

	TestSuite::TestSuite(const SoftwareInfo& softwareInfo,
						 ILogFile* appLog,
						 TestLog& testLog,
						 std::unique_ptr<::TestSuite::TestControl> testControl) :
		m_appLog{appLog, "TestLibrary"},
		m_testLog{testLog},
		m_softwareInfo{softwareInfo},
		m_testControl{std::move(testControl)}
	{
		connect(m_testControl.get(), &TestControl::testStarted, this, &TestSuite::testStarted);
		connect(m_testControl.get(), &TestControl::testFinished, this, &TestSuite::testFinished);

		connect(m_testControl.get(),
				&TestControl::finished,
				[this](int result)
				{
					emit finished(result);
				});

		return;
	}

	TestSuite::TestSuite(const SoftwareInfo& softwareInfo, ILogFile* appLog, TestLog& testLog) :
		TestSuite{softwareInfo, appLog, testLog, std::make_unique<TestControl>(appLog, &m_testLog)}
	{
		return;
	}

	TestSuite::~TestSuite() = default;

	bool TestSuite::execute(const ::TestSuite::IScriptProvider& scriptProvider, const ::TestSuite::ControlParams& controlParams)
	{
		m_testLog.clear();
		return m_testControl->execute(m_softwareInfo, scriptProvider, controlParams);
	}

	void TestSuite::stop()
	{
		m_testControl->stop();
		return;
	}

	bool TestSuite::isRunning() const
	{
		return m_testControl->isRunning();
	}

	void TestSuite::addInputController(std::unique_ptr<::TestSuite::IInputController> controller) 
	{
		m_testControl->addInputController(std::move(controller));
	}

	void TestSuite::addOutputController(std::unique_ptr<::TestSuite::IOutputController> controller) 
	{
		m_testControl->addOutputController(std::move(controller));
	}

	TestLog& TestSuite::testLog()
	{
		return m_testLog;
	}

	::TestSuite::ControlStatus TestSuite::testStatus() const
	{
		return m_testControl->status();
	}

	const ::SoftwareInfo& TestSuite::softwareInfo() const
	{
		return m_softwareInfo;
	}
} // namespace TestSuite
