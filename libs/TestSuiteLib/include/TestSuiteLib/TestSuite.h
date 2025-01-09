#pragma once
#include "ControlParams.h"
#include "ControlStatus.h"
#include "IInputController.h"
#include "IOutputController.h"
#include "IScriptProvider.h"
#include "TestLog.h"

#include "../OnlineLib/SoftwareInfo.h"
#include "../UtilsLib/ILogFile.h"

#include <memory>

namespace TestSuite
{
	class TestControl;
}


namespace TestSuite
{
	class TestSuite : public QObject
	{
		Q_OBJECT

	public:
		TestSuite(const SoftwareInfo& softwareInfo,
				  ILogFile* appLog,
				  ::TestSuite::TestLog& testLog,
				  std::unique_ptr<::TestSuite::TestControl> testControl);
		TestSuite(const SoftwareInfo& softwareInfo, ILogFile* appLog, ::TestSuite::TestLog& testLog);

		TestSuite() = delete;
		TestSuite(const TestSuite&) = delete;
		TestSuite(TestSuite&&) = delete;
		TestSuite& operator=(const TestSuite&) = delete;
		TestSuite& operator=(TestSuite&&) = delete;
		virtual ~TestSuite();

	private:
		static std::unique_ptr<::TestSuite::TestControl> emptyTestControlPtr(); // Hide TestControl unique ptr construction

	public:
		bool execute(const ::TestSuite::IScriptProvider& scriptProvider, const ::TestSuite::ControlParams& controlParams);
		void stop();
		[[nodiscard]] bool isRunning() const;

		void addInputController(std::unique_ptr<::TestSuite::IInputController> controller);
		void addOutputController(std::unique_ptr<::TestSuite::IOutputController> controller);

		::TestSuite::TestLog& testLog();
		[[nodiscard]] ::TestSuite::ControlStatus testStatus() const;

	signals:
		void testStarted(QString scriptFileName, QString testFunction);
		void testFinished(QString scriptFileName, QString testFunction, bool result);
		void finished(int result);

	public:
		const ::SoftwareInfo& softwareInfo() const;

	private:
		HasLogFile m_appLog;
		TestLog& m_testLog;

		SoftwareInfo m_softwareInfo;

		// Test runtime
		//
		std::unique_ptr<TestControl> m_testControl;
	};
} // namespace TestSuite
