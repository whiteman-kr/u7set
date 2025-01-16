#pragma once

#include "ControlParams.h"
#include "ControlStatus.h"
#include "TestSuiteConfigController.h"

#include "../UtilsLib/ILogFile.h"

#include <QObject>

#include <memory>


namespace ClientLib
{
	class AppSignalManager;
} // namespace ClientLib


namespace TestSuite
{
	class ITestLogOutput;
	class RunControl;
	class TestLog;
	class TestSuite;
	class MatsScriptProvider;


	class MatsTestSuite : public QObject
	{
		Q_OBJECT

	public:
		MatsTestSuite(::TestSuite::TestSuiteConfigController& configController, ILogFile* appLog, ITestLogOutput* testOutput);
		~MatsTestSuite() override;

	public:
		bool executeRunControl(const ::TestSuite::IScriptProvider& scriptProvider);
		[[nodiscard]] bool hasRunControl();
		void resetRunControl();
		void stopRunControl();

	public:
		// Run scripts from CfService
		//
		bool execute(const ::TestSuite::ControlParams& controlParams);

		// Run scripts from disk or any other source
		//
		bool execute(const ::TestSuite::IScriptProvider& scriptProvider, const ::TestSuite::ControlParams& controlParams);

		void stop();
		[[nodiscard]] bool isRunning() const;

		TestLog& testLog();
		[[nodiscard]] ::TestSuite::ControlStatus testStatus() const;
		[[nodiscard]] ::TestSuite::ControlStatus runStatus() const;

		[[nodiscard]] bool scriptPermission(const QString& fileName) const;
		[[nodiscard]] bool globalPermission() const;

	signals:
		void testStarted(QString scriptFileName, QString testFunction);
		void testFinished(QString scriptFileName, QString testFunction, bool result);
		void finished(int result);

		void scriptPermissionChanged(QString scriptFileName, bool result);
		void globalPermissionChanged(bool result);
		void noPermissionsExist();

	private:
		HasLogFile m_appLog;
		std::unique_ptr<::ClientLib::AppSignalManager> m_runControlSignals; // Signals for m_runControl
		std::unique_ptr<::ClientLib::AppSignalManager> m_appSignals;        // Signals for m_testSuite

		::TestSuite::TestSuiteConfigController& m_configController;
		std::unique_ptr<::TestSuite::TestLog> m_testLog;
		std::unique_ptr<::TestSuite::RunControl> m_runControl;
		std::unique_ptr<::TestSuite::TestSuite> m_testSuite;
	};
} // namespace TestSuite