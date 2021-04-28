#pragma once
#include <QQmlEngine>
#include "../CommonLib/Types.h"
#include "../AppSignalLib/AppSignalParam.h"
#include "SimScriptSignal.h"
#include "SimScriptDevUtils.h"
#include "SimScriptLogicModule.h"
#include "SimScriptLmDescription.h"
#include "SimScriptConnection.h"


namespace Sim
{
	class ScriptSimulator;
	class LogicModule;


	struct SimScriptItem
	{
		QString script;
		QString scriptCaption;
	};


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

		void setScripts(const std::vector<SimScriptItem>& scripts);

	private:
		ScriptSimulator* m_scriptSimulator = nullptr;
		ScopedLog m_log;

		std::vector<SimScriptItem> m_scripts;

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

		/// \brief Loaded project build directory, if empty then project is not loaded.
		Q_PROPERTY(QString buildPath READ buildPath)

		/// \brief Script execution timeout in milliseconds, if -1 then timeout is not applied.
		Q_PROPERTY(qint64 executionTimeout READ executionTimeout WRITE setExecutionTimeout)

		/// \brief Unlocks simulation timer binding to PC's time. This param can significantly increase simulation speed but it depends on underlying hardware and project size.
		Q_PROPERTY(bool unlockTimer READ unlockTimer WRITE setUnlockTimer)

		/// \brief Allows or disables LogicModules' LAN communications like Application Data transmittion to AppDataSrv, TuningService communications (note: Tuning Key and Arming Key must be set to 1). This is global flag for all simulated communications.
		Q_PROPERTY(bool enabledLanComm READ enabledLanComm WRITE setEnabledLanComm)

	public:
		explicit ScriptSimulator(Simulator* simulator, QObject* parent = nullptr);
		virtual ~ScriptSimulator();

		bool runScripts(const std::vector<SimScriptItem>& scripts);
		bool stopScript();

		bool isRunning() const;

		bool wait(unsigned long msecs = ULONG_MAX);		// Wait script to stop
		bool result() const;

		static void throwScriptException(const QObject* object, QString text);

		// Public slots which are part of Script API
		//
	public slots:
        void debugOutput(QString str);					// Debug output to qDebug

        /// \brief Run the simulation for \a msec milliseconds, if \a msec is -1 then simulation will last till the programm interrupted.
		/// <b>Note:</b> Simulation process can last longer than \a msec milliseconds, it depends on project size and simulation hardware.
		bool startForMs(int msecs);

		/// \brief Reset all simulations to initial state.
		/// <b>Note:</b> Function sets reset flag and actual reset will be performed on the next \c startForMs call.
		bool reset();

		/// \brief Get signal state, if signal is not found then exception is thrown.
		QJSValue signalState(QString appSignalId);

		/// \brief Get signal value, if signal is not found then exception is thrown.
		/// <b>Note:</b> This function does not return full signal state with validity and other flags.
		double signalValue(QString appSignalId);

		/// \brief Override signal value. Returns true if signal value is overriden.
		/// <b>Note:</b> At least one work cycle must be run [startForMs(5)] to apply override to signal.
		/// <b>Note:</b> Not all signals can be overriden. For example, some signals can be optimized to constant value, as they don not have location in RAM they connot be overriden.
		bool overrideSignalValue(QString appSignalId, double value);

		/// \brief Remove all overriden signals.
		/// <b>Note:</b> At least one work cycle must be run [startForMs(5)] to apply this function.
		void overridesReset();

		/// \brief Checks if a LogicModule exists.
		bool logicModuleExists(QString equipmentId) const;

		/// \brief Returns LogicModule (type ScriptLogicModule) or undefined if it is not exists.
		QJSValue logicModule(QString equipmentId);

		/// \brief Returns Connection by ID (type ScriptConnection) or undefined if it is not exists.
		QJSValue connection(QString connectionID);

		/// \brief Sets enable property to all connections.
		void connectionsSetEnabled(bool value);

		/// \brief Checks if a signal exists.
		bool signalExists(QString appSignalId) const;

		/// \brief Get signal description, if a signal is not found then exception is thrown.
		AppSignalParam signalParam(QString appSignalId);

		/// \brief Get full signal description, if a signal is not found then exception is thrown.
		ScriptSignal signalParamExt(QString appSignalId);

		/// \brief Returns ScriptLmDescription for LM  with specified equipmentId, if LM is not found then exception is thrown.
		ScriptLmDescription scriptLmDescription(QString equipmentId);

		ScriptDevUtils devUtils();

	public:
		[[nodiscard]] ScopedLog& log();

		QString buildPath() const;

		qint64 executionTimeout() const;
		void setExecutionTimeout(qint64 value);

		[[nodiscard]] Simulator* simulator();
		[[nodiscard]] const Simulator* simulator() const;

	private:
		[[nodiscard]] bool unlockTimer() const;
		void setUnlockTimer(bool value);

		[[nodiscard]] bool enabledLanComm() const;
		void setEnabledLanComm(bool value);

		// Data
		//
	private:
		Simulator* m_simulator = nullptr;
		mutable ScopedLog m_log;

		ScriptWorkerThread m_workerThread{this};

		qint64 m_executionTimeout = -1;		// Script execution timeout in milliseconds, negative means no timeout
	};

}

