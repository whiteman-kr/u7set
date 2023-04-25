#pragma once

#include <QObject>
#include <chrono>
#include "TestLog.h"
#include "TestSuiteSettings.h"
#include "TestSuiteConfigController.h"
#include "TestScriptsStorage.h"
#include "IInputController.h"
#include "IOutputController.h"
#include "../ClientLib/AppSignalManager.h"

namespace TestSuite
{
	using namespace std::literals::chrono_literals;

	enum class ControlState
	{
		Stop,
		Run,
		Pause
	};

	struct ControlData
	{
		// Keep this struct simple, it should copy fast enough
		//
		//std::vector<SimControlRunStruct> m_lms;			// LMs added to simulation
		ControlState m_state = ControlState::Stop;

		std::chrono::microseconds m_startTime = 0us;		// When simulation was started, computer time
		//std::chrono::microseconds m_sliceStartTime = 0us;	// When simulation was started for current 'slice' (duration)
		std::chrono::microseconds m_currentTime = 0us;		// Current time in simulation

		std::chrono::microseconds m_duration{0};		// Simulation is started for this time
														// if time < 0 then no time limit
														// if time == 0 then run one cycle (NO, IT WILL RESET IF ON PAUSE MODE)
														// if time > 0 then run this time

		QDateTime currentTime() const
		{
			auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(m_currentTime);
			return QDateTime::fromMSecsSinceEpoch(ms.count(), Qt::UTC);
		}
	};

	struct ControlStatus
	{
		ControlStatus() = default;

		ControlStatus(const ControlData& cd) :
			m_startTime(cd.m_startTime),
			m_currentTime(cd.m_currentTime),
			m_duration(cd.m_currentTime - cd.m_startTime),
			m_state(cd.m_state)
		{
//			m_lmDeviceModes.reserve(cd.m_lms.size());

//			for (const SimControlRunStruct& lm : cd.m_lms)
//			{
//				m_lmDeviceModes.push_back(Sim::ControlStatus::LmMode{lm.equipmentId(), lm.m_lm->deviceState()});
//			}
		}

		std::chrono::microseconds m_startTime = 0us;	// When testing was started, it's computer time
		std::chrono::microseconds m_currentTime = 0us;	// Current time in testing

		std::chrono::microseconds m_duration{0};
		ControlState m_state = ControlState::Stop;

//		struct LmMode
//		{
//			QString lmEquipmentId;
//			Sim::DeviceState deviceState;
//		};

//		std::vector<Sim::ControlStatus::LmMode> m_lmDeviceModes;
	};

	class ControlThread : public QThread
	{
		Q_OBJECT

	public:
		ControlThread(ILogFile* appLog, ITestLog* testLog);

	public:
		void setTestParams(const SoftwareInfo& softwareInfo,
						   const TestSuiteSettings& settings,
						   const QStringList& executionTests,	// List of tests for execution, if empty then exec all.
						   const QString& scriptsPath);			// Load scripts from disk, path to dir for *.js files.

		int result() const;

	protected:
		virtual void run() override;

	private:
		void cleanUp();
		void checkAndInterruptTestExecution();

		void taskCfgServiceConnection();
		void taskInitInputController();
		void taskInitOutputController();

		void taskRunTests();

	private:
		HasLogFile m_appLog;
		ITestLog* m_testLog = nullptr;

		SoftwareInfo m_softwareInfo;
		TestSuiteSettings m_settings;

		QStringList m_executionTests;	// List of tests for execution, if empty then exec all.
		QString m_scriptsPath;			// Load scripts from disk, path to dir for *.js files.

		// --
		//
		std::atomic<int> m_result{0};

		ConfigSettings m_configuration;
		std::vector<TestScript> m_scripts;

		ClientLib::AppSignalManager m_signals;
		std::unique_ptr<IInputController> m_inputController;
		std::unique_ptr<IOutputController> m_outputController;
	};


	class Control : public QObject
	{
		Q_OBJECT

	public:
		explicit Control(ILogFile* appLog, ITestLog* testLog);

	public:
		bool execute(const SoftwareInfo& softwareInfo,
					 const TestSuiteSettings& settings,
					 const QStringList& executionTests,
					 const QString& scriptsPath);
		bool stop();
		bool isRunning() const;

	signals:
		void finished(int result);

	private:
		ILogFile* m_appLog = nullptr;
		ITestLog* m_testLog = nullptr;

		ControlThread m_controlThread;
	};
}
