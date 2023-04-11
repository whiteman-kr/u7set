#pragma once

#include "IOutputController.h"
#include "IInputController.h"
#include "../AppSignalLib/AppSignalParam.h"

namespace TestSuite
{
	// Proxy class for using in scripts
	//
	/*! \class TestController
	\ingroup testsuite
	\brief Represents class that runs all hardware test functions.
	*/
	class TestController : public QObject
	{
		Q_OBJECT

		/// \brief Loaded project build directory, if empty then project is not loaded.
		//Q_PROPERTY(QString buildPath READ buildPath)

		/// \brief Script execution timeout in milliseconds, if -1 then timeout is not applied.
		//Q_PROPERTY(qint64 executionTimeout READ executionTimeout WRITE setExecutionTimeout)

		/// \brief Allows or disables debug log messages.
		//Q_PROPERTY(bool debugMessagesEnabled READ (m_log.debugMessagesEnabled) WRITE (m_log.setDebugMessagesEnabled))

	public:
		explicit TestController(QObject* parent = nullptr);
		virtual ~TestController();

		static void throwScriptException(const QObject* object, QString text);

		// Public slots which are part of Script API
		//
	public slots:
		void debugOutput(QString str);					// Debug output to qDebug

		/// \brief Run the simulation for \a msec milliseconds, if \a msec is -1 then simulation will last till the programm interrupted.
		/// <b>Note:</b> Simulation process can last longer than \a msec milliseconds, it depends on project size and simulation hardware.
		//bool startForMs(int msecs);

		/// \brief Reset all simulations to initial state.
		/// <b>Note:</b> Function sets reset flag and actual reset will be performed on the next \c startForMs call.
		//bool reset();

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
		//void overridesReset();

		/// \brief Checks if a LogicModule exists.
		//bool logicModuleExists(QString equipmentId) const;

		/// \brief Returns LogicModule (type ScriptLogicModule) or undefined if it is not exists.
		//QJSValue logicModule(QString equipmentId);

		/// \brief Returns Connection by ID (type ScriptConnection) or undefined if it is not exists.
		//QJSValue connection(QString connectionID);

		/// \brief Sets enable property to all connections.
		//void connectionsSetEnabled(bool value);

		/// \brief Checks if a signal exists.
		bool signalExists(QString appSignalId) const;

		/// \brief Get signal description, if a signal is not found then exception is thrown.
		AppSignalParam signalParam(QString appSignalId);

		/// \brief Get full signal description, if a signal is not found then exception is thrown.
		//ScriptSignal signalParamExt(QString appSignalId);

		/// \brief Returns ScriptLmDescription for LM  with specified equipmentId, if LM is not found then exception is thrown.
		//ScriptLmDescription scriptLmDescription(QString equipmentId);

		//ScriptDevUtils devUtils();

		/// \brief Returns uninitialized RamAddress object
		//RamAddress createRamAddress();

		/// \brief Returns initialized RamAddress object
		//RamAddress createRamAddress(int offset, int bit);

	public:
		//[[nodiscard]] ScopedLog& log();

		//QString buildPath() const;

		//qint64 executionTimeout() const;
		//void setExecutionTimeout(qint64 value);

		//bool checkSkipOnBuildConst() const;
		//void setCheckSkipOnBuildConst(bool value);

		//[[nodiscard]] Simulator* simulator();
		//[[nodiscard]] const Simulator* simulator() const;

	private:
		//[[nodiscard]] bool unlockTimer() const;
		//void setUnlockTimer(bool value);

		//[[nodiscard]] bool enabledLanComm() const;
		//void setEnabledLanComm(bool value);

		// Data
		//
	private:
		//Simulator* m_simulator = nullptr;
		//mutable ScopedLog m_log;
		//OutputController* m_outputController = nullptr;
		//InputController* m_inputController = nullptr;
		//ScriptTestLog* m_scriptTestLog = nullptr;

		//std::atomic<qint64> m_executionTimeout = -1;		// Script execution timeout in milliseconds, negative means no timeout
		//std::atomic<bool> m_checkSkipOnBuildConst = false;	// If true then check script global variable SkipOnBuild, and both re true the SKIP this file
	};
}
