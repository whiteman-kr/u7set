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
		explicit TestController(IInputController& inputController, IOutputController& outputController, QObject* parent = nullptr);
		virtual ~TestController();

		static void throwScriptException(const QObject* object, QString text);

		// Public slots which are part of Script API
		//
	public slots:
		void debugOutput(QString str);					// Debug output to qDebug

		/// \brief Wait for specified numbers of milliseconds
		bool startForMs(int msecs);
		bool waitForMs(int msecs);

//		/// \brief Reset all simulations to initial state.
//		/// <b>Note:</b> Function sets reset flag and actual reset will be performed on the next \c startForMs call.
//		bool reset();

		/// \brief Get signal state, if signal is not found then exception is thrown.
		QJSValue signalState(QString appSignalId);

		/// \brief Get signal value, if signal is not found then exception is thrown.
		/// <b>Note:</b> This function does not return full signal state with validity and other flags.
		double signalValue(QString appSignalId);

		/// \brief Override signal value. Returns true if signal value is overriden.
		bool overrideSignalValue(QString appSignalId, QVariant value);

		/// \brief Waits while all overrided signal value is written to LM. Returns true if signal value is overriden, false on timeout.
		bool waitForSignalOverrides(qint64 timeoutMs);

		/// \brief Waits while signal value is set to specified value. Returns true if value is correct, false on timeout.
		bool expectSignalValue(QString appSignalId, double value, qint64 timeoutMs);

		/// \brief Remove all overriden signals.
		/// <b>Note:</b> At least one work cycle must be run [startForMs(5)] to apply this function.
		void overridesReset();

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

		// Data
		//
	private:
		IInputController& m_inputController;
		IOutputController& m_outputController;

		//std::atomic<qint64> m_executionTimeout = -1;		// Script execution timeout in milliseconds, negative means no timeout
	};
}
