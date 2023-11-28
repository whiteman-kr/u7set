#include "TestSuite.h"

namespace TestSuite
{

	TestSuite::TestSuite(const SoftwareInfo& softwareInfo, const TestSuiteSettings& settings, ILogFile* appLog, ITestLogOutput* testOutput):
		m_appLog{appLog, "TestLibrary"},
		m_testLog{testOutput},
		m_softwareInfo{softwareInfo},
		m_settings{settings},
		m_testControl{appLog, &m_testLog},
		m_runControl{appLog, &m_testLog}
	{
		connect(&m_testControl, &TestControl::testStarted, this, &TestSuite::testStarted);
		connect(&m_testControl, &TestControl::testFinished, this, &TestSuite::testFinished);

		connect(&m_testControl, &TestControl::finished, this, &TestSuite::finished);

		connect(&m_runControl, &RunControl::scriptPermissionChanged, this, &TestSuite::scriptPermissionChanged);
		connect(&m_runControl, &RunControl::globalPermissionChanged, this, &TestSuite::globalPermissionChanged);
		connect(&m_runControl, &RunControl::noPermissionsExist, this, &TestSuite::noPermissionsExist);

		return;
	}

	TestSuite::~TestSuite()
	{
		stopRunControl();
	}

	bool TestSuite::executeRunControl(const ControlParams& controlParams)
	{
		return m_runControl.execute(m_softwareInfo, m_settings, controlParams);
	}

	bool TestSuite::hasRunControl()
	{
		return m_runControl.isRunning();
	}

	void TestSuite::resetRunControl()
	{
		m_runControl.reset();
	}

	void TestSuite::stopRunControl()
	{
		if (m_runControl.isRunning() == true)
		{
			m_runControl.stop();
		}
	}

	bool TestSuite::execute(const ControlParams& controlParams)
	{
		m_testLog.clear();
		return m_testControl.execute(m_softwareInfo, m_settings, controlParams);
	}

	void TestSuite::stop()
	{
		m_testControl.stop();
		return;
	}

	bool TestSuite::isRunning() const
	{
		return m_testControl.isRunning();
	}

	void TestSuite::updateSettings(const TestSuiteSettings& settings, const ControlParams& controlParams)
	{
		m_settings = settings;
		m_softwareInfo.setEquipmentID(settings.instanceStrId());

		// Restart run control with new parameters
		//
		if (m_runControl.isRunning() == true)
		{
			m_runControl.stop();
			m_runControl.execute(m_softwareInfo, m_settings, controlParams);
		}
	}

	TestLog& TestSuite::testLog()
	{
		return m_testLog;
	}

	ControlStatus TestSuite::testStatus() const
	{
		return m_testControl.status();
	}

	ControlStatus TestSuite::runStatus() const
	{
		return m_runControl.status();
	}

	bool TestSuite::scriptPermission(const QString& fileName) const
	{
		return m_runControl.scriptPermission(fileName);
	}

	bool TestSuite::globalPermission() const
	{
		return m_runControl.globalPermission();
	}
}
