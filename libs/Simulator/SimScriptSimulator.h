#pragma once

#include <Simulator/SimScriptItem.h>

#include "SimScriptConnection.h"
#include "SimScriptDevUtils.h"
#include "SimScriptLmDescription.h"
#include "SimScriptSignal.h"


namespace Sim
{
	class ScriptSimulator;
	class LogicModuleImpl;


	class ScriptWorkerThread : public QThread
	{
		Q_OBJECT

	public:
		ScriptWorkerThread(ScriptSimulator* scriptSimulator);

	private:
		virtual void run() override;

		bool runScriptFunction(const QString& functionName);

	public:
		void start(QThread::Priority priority = InheritPriority);
		bool interruptScript();

		bool result() const;

		void setScripts(const std::vector<SimScriptItem>& scripts, const SimScriptItem& globalScript);

	private:
		ScriptSimulator* m_scriptSimulator = nullptr;
		ScopedLog m_log;

		SimScriptItem m_globalScript;
		std::vector<SimScriptItem> m_scripts;

		std::mutex m_jsMutex; // Mutex only on changing m_jsEngine value as pointer.
		std::unique_ptr<QJSEngine> m_jsEngine;

		QJSValue m_jsThis;
		QJSValue m_jsLog;

		std::atomic_bool m_result{true};
	};


	// Proxy class for using in scripts
	//
	/*! \class ScriptSimulator
		\ingroup simulator
		\brief Represents class that runs all simulations on compiled project.
	*/
	class ScriptSimulator : public QObject
	{
		Q_OBJECT

		/// \brief Loaded project build directory, if empty then project is not loaded. Simulator only.
		Q_PROPERTY(QString buildPath READ buildPath)

		/// \brief Loaded ProjectName.
		Q_PROPERTY(QString projectName READ projectName)

		/// \brief Loaded project build number, if 0 then project is not loaded.
		Q_PROPERTY(int buildNo READ buildNo)

		/// \brief Script execution timeout in milliseconds, if -1 then timeout is not applied.
		Q_PROPERTY(qint64 executionTimeout READ executionTimeout WRITE setExecutionTimeout)

		/// \brief Unlocks simulation timer binding to PC's time. This param can significantly increase simulation speed but it depends on underlying hardware and project size. Also see speedFactor.
		Q_PROPERTY(bool unlockTimer READ unlockTimer WRITE setUnlockTimer)

		/// \brief This param can increase or decrease simulation speed but it depends on underlying hardware and project size. Accepts values [0.1 - 256.0].
		Q_PROPERTY(double speedFactor READ speedFactor WRITE setSpeedFactor)

		/// \brief Allows or disables LogicModules' LAN communications like Application Data transmission to AppDataSrv, TuningService communications (note: Tuning Key and Arming Key must be set to 1). This is global flag for all simulated communications.
		Q_PROPERTY(bool enabledLanComm READ enabledLanComm WRITE setEnabledLanComm)

		/// \brief Allows or disables debug log messages.
		Q_PROPERTY(bool debugMessagesEnabled READ(m_log.debugMessagesEnabled) WRITE(m_log.setDebugMessagesEnabled))

	public:
		explicit ScriptSimulator(SimulatorPrivate* simulator, QObject* parent = nullptr);
		virtual ~ScriptSimulator();

		bool runScripts(const std::vector<SimScriptItem>& scripts, const SimScriptItem& globalScript);
		bool stopScript();

		bool isRunning() const;

		bool wait(unsigned long msecs = ULONG_MAX); // Wait script to stop
		bool result() const;

		static void throwScriptException(const QObject* object, QString text);

		// Public slots which are part of Script API
		//
	public slots:
		void debugOutput(QString str); // Debug output to qDebug

		/// \brief Run the simulation for \a msecs milliseconds, if \a msecs is -1 then simulation will last till the program interrupted.
		/// <b>Note:</b> Simulation process can last longer than \a msecs milliseconds, it depends on project size and simulation hardware.
		/// <b>Note:</b> This function is the same as waitForMs.
		bool startForMs(int msecs);

		/// \brief Run the simulation for \a msecs milliseconds, if \a msecs is -1 then simulation will last till the program interrupted.
		/// <b>Note:</b> Simulation process can last longer than \a msecs milliseconds, it depends on project size and simulation hardware.
		/// <b>Note:</b> This function is the same as startForMs.
		bool waitForMs(int msecs);

		/// \brief Reset all simulations to initial state.
		/// <b>Note:</b> Function sets reset flag and actual reset will be performed on the next \c startForMs call.
		bool reset();

		/// @brief Creates a new test observer object.
		/// @return A newly created empty ScriptTestObserver.
		QJSValue createObserver();

		/// \brief Get signal state, if signal is not found then exception is thrown.
		QJSValue signalState(QString appSignalId);

		/// \brief Get signal value, if signal is not found then exception is thrown.
		/// <b>Note:</b> This function does not return full signal state with validity and other flags.
		double signalValue(QString appSignalId);

		/// \brief Override signal value. Returns true if signal value is overriden.
		/// <b>Note:</b> At least one work cycle must be run [startForMs(5)] to apply override to signal.
		/// <b>Note:</b> Not all signals can be overriden. For example, some signals can be optimized to constant value, as they don not have location in RAM they cannot be overriden.
		bool overrideSignalValue(QString appSignalId, double value);

		/// \brief Waits while all overriden signal value is written to LM. 
		/// <b>Note:</b> For the simulator, \a timeoutMs is ignored and function runs one work cycle to apply overrides.
		bool waitForSignalOverrides(qint64 timeoutMs);

		/// \brief Waits while signal value is set to specified value. Returns true if value is correct, false on timeout. 
		/// Optional tolerance parameter specifies accuracy of comparison, recommended for use with analog signals.
		bool expectSignalValue(QString appSignalId, qint64 timeoutMs, double value, double tolerance = 0);

		/// \brief Resets all overridden signals.
		/// 
		/// This function resets all overridden signals to their default values. 
		/// It automatically calls \c waitForSignalOverrides with a specified timeout (default value: 5000 ms) 
		/// to wait for all signals to be reset. If the function times out, an exception is thrown.
		/// \param timeoutMs The timeout value in milliseconds for waiting on signal overrides.
		/// \param excludeAppSignals Optionally specifies the list of signals that should not be reset.
		void overridesReset(qint64 timeoutMs = 5000, QStringList excludeAppSignals = {});

		/// \brief Get overriden signals list
		/// 
		/// This function returns currently overridden signals identifiers. 
		QStringList getOverridenSignals() const;

		/// \brief Checks if a LogicModule exists. Simulator only.

		bool logicModuleExists(QString equipmentId) const;

		/// \brief Returns LogicModule (type ScriptLogicModule) or undefined if it is not exists. Simulator only.
		QJSValue logicModule(QString equipmentId);

		/// \brief Returns Connection by ID (type ScriptConnection) or undefined if it is not exists. Simulator only.
		QJSValue connection(QString connectionID);

		/// \brief Sets enable property to all connections. Simulator only.
		void connectionsSetEnabled(bool value);

		/// \brief Checks if a signal exists.
		bool signalExists(QString appSignalId) const;

		/// \brief Get signal description, if a signal is not found then exception is thrown.
		AppSignalParam signalParam(QString appSignalId);

		/// \brief Get full signal description, if a signal is not found then exception is thrown. Simulator only.
		ScriptSignal signalParamExt(QString appSignalId);

		/// \brief Returns ScriptLmDescription for LM  with specified equipmentId, if LM is not found then exception is thrown. Simulator only.
		ScriptLmDescription scriptLmDescription(QString equipmentId);

		ScriptDevUtils devUtils();

		/// \brief Returns uninitialized RamAddress object. Simulator only.
		RamAddress createRamAddress();

		/// \brief Returns initialized RamAddress object. Simulator only.
		RamAddress createRamAddress(int offset, int bit);

	public:
		[[nodiscard]] ScopedLog& log();

		QString buildPath() const;
		QString projectName() const;
		int buildNo() const;

		qint64 executionTimeout() const;
		void setExecutionTimeout(qint64 value);

		bool checkSkipOnBuildConst() const;
		void setCheckSkipOnBuildConst(bool value);

		[[nodiscard]] SimulatorPrivate* simulator();
		[[nodiscard]] const SimulatorPrivate* simulator() const;

	private:
		[[nodiscard]] bool unlockTimer() const;
		void setUnlockTimer(bool value);

		[[nodiscard]] double speedFactor() const;
		void setSpeedFactor(double value);

		[[nodiscard]] bool enabledLanComm() const;
		void setEnabledLanComm(bool value);

		// Data
		//
	private:
		SimulatorPrivate* m_simulator = nullptr;
		mutable ScopedLog m_log;

		ScriptWorkerThread m_workerThread{this};

		std::atomic<qint64> m_executionTimeout = -1;       // Script execution timeout in milliseconds, negative means no timeout
		std::atomic<bool> m_checkSkipOnBuildConst = false; // If true then check script global variable SkipOnBuild, and both re true the SKIP this file
	};

} // namespace Sim
