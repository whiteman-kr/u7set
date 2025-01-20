#pragma once

#include <TestSuiteLib/ControlParams.h>
#include <TestSuiteLib/ControlStatus.h>
#include <TestSuiteLib/IInputController.h>
#include <TestSuiteLib/IOutputController.h>
#include <TestSuiteLib/IScriptProvider.h>
#include <TestSuiteLib/TestLog.h>

#include "../OnlineLib/SoftwareInfo.h"

#include <QMutex>

#include <atomic>
#include <memory>


namespace TestSuite
{
	class ControlThread : public QThread
	{
		Q_OBJECT

	public:
		ControlThread(ILogFile* appLog, TestLog* testLog, const QString& runContext);
		virtual ~ControlThread();

	public:
		void addInputController(std::unique_ptr<IInputController> controller);
		void addOutputController(std::unique_ptr<IOutputController> controller);

		void setTestParams(const SoftwareInfo& softwareInfo, const IScriptProvider& scriptProvider, const ControlParams& controlParams);

		int result() const;

		::TestSuite::ControlStatus status() const;

	protected:
		virtual void run() = 0;

	protected:
		void checkAndInterruptTestExecution();

		virtual void init();
		virtual void cleanUp();

		virtual void taskInitInputController();
		virtual void taskInitOutputController();

		[[nodiscard]] virtual QString plant() const { return {}; }
		[[nodiscard]] virtual QString unit() const { return {}; }
		[[nodiscard]] virtual QString system() const { return {}; }

		[[nodiscard]] virtual QString projectName() const { return {}; }
		[[nodiscard]] virtual int buildNo() const { return {}; }

	protected:
		HasLogFile m_appLog;
		TestLog* m_testLog = nullptr;

		// --
		//
		SoftwareInfo m_softwareInfo;
		ControlParams m_controlParams;

		const IScriptProvider* m_scriptProvider = nullptr;

		// --
		//
		std::unique_ptr<IInputController> m_inputController;
		std::unique_ptr<IOutputController> m_outputController;

		// --
		//
		mutable QMutex m_statusMutex;
		ControlStatus m_status;

		std::atomic<int> m_result{0};
	};


	class Control : public QObject
	{
		Q_OBJECT

	public:
		Control(ILogFile* appLog, TestLog* testLog, ControlThread* controlThread);
		virtual ~Control();

	public:
		void addInputController(std::unique_ptr<IInputController> controller);
		void addOutputController(std::unique_ptr<IOutputController> controller);

	public:
		bool execute(const SoftwareInfo& softwareInfo, const IScriptProvider& scriptProvider, const ControlParams& controlParams);
		bool stop();
		bool isRunning() const;

		ControlStatus status() const;

	protected:
		ILogFile* m_appLog = nullptr;
		TestLog* m_testLog = nullptr;

		std::unique_ptr<ControlThread> m_controlThread;

	private:
		std::atomic<bool> m_stopRequested{false};
	};
} // namespace TestSuite
