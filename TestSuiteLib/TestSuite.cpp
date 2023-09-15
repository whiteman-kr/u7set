#include "TestSuite.h"

namespace TestSuite
{

	TestSuite::TestSuite(const SoftwareInfo& softwareInfo, const TestSuiteSettings& settings, ILogFile* appLog, ITestLogOutput* testOutput):
		m_appLog{appLog, "TestLibrary"},
		m_testLog{testOutput},
		m_softwareInfo{softwareInfo},
		m_settings{settings},
		m_control{appLog, &m_testLog}
	{
		connect(&m_control, &Control::testFinished, [this](QString scriptFileName, QString testFunction, bool result){
			emit testFinished(scriptFileName, testFunction, result);
		});

		connect(&m_control, &Control::finished, this, &TestSuite::finished);
		return;
	}

	bool TestSuite::execute(const QStringList& scriptsFiles,		// List of script files for execution, if empty then exec all.
							const QString& scriptsPath,				// Load scripts from disk, path to dir for *.js files.
							const TestScriptSelection& testsFilter,				// Tests filter
							const QString& userName,
							const QString& password)
	{
		m_testLog.clear();

		return m_control.execute(m_softwareInfo, m_settings, scriptsFiles, scriptsPath, testsFilter, userName, password);
	}

	void TestSuite::updateSettings(const TestSuiteSettings& settings)
	{
		m_settings = settings;
		m_softwareInfo.setEquipmentID(settings.instanceStrId());
	}

	void TestSuite::stop()
	{
		m_control.stop();
		return;
	}

	bool TestSuite::isRunning() const
	{
		return m_control.isRunning();
	}

	TestLog& TestSuite::testLog()
	{
		return m_testLog;
	}

	ControlStatus TestSuite::status() const
	{
		return m_control.status();
	}

	ReportLib::ReportTemplateStorage TestSuite::reportTemplates() const
	{
		return m_control.reportTemplates();
	}
}
