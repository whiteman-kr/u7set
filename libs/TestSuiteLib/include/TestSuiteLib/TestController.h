#pragma once

#include "IInputController.h"
#include "IOutputController.h"

#include "../UtilsLib/ILogFile.h"

#include <QJSValue>

#include <atomic>
#include <set>

class ComparatorSet;

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
		// Q_PROPERTY(QString buildPath READ buildPath)

		/// \brief Loaded ProjectName.
		Q_PROPERTY(QString projectName READ projectName)

		/// \brief Loaded project build number, if 0 then project is not loaded.
		Q_PROPERTY(int buildNo READ buildNo)

		/// \brief Script execution timeout in milliseconds, if -1 then timeout is not applied.
		Q_PROPERTY(qint64 executionTimeout READ executionTimeout WRITE setExecutionTimeout)

		/// \brief Allows or disables debug log messages.
		Q_PROPERTY(bool debugMessagesEnabled READ debugMessagesEnabled WRITE setDebugMessagesEnabled)

	public:
		TestController(ILogFile* appLog,
					   ILogFile* testLog,
					   IInputController* inputController,
					   IOutputController* outputController,
					   const ComparatorSet* setpoints,
					   QObject* parent = nullptr);

		static void throwScriptException(const QObject* object, QString text);

		// Public slots which are part of Script API
		//
	public slots:
		void debugOutput(QString str); // Debug output to qDebug

		/// \brief Wait for specified numbers of milliseconds, same as waitForMs().
		bool startForMs(int msecs);

		/// \brief Wait for specified numbers of milliseconds, same as startForMs().
		bool waitForMs(int msecs);

		/// @brief Creates a new test observer object.
		/// @return A newly created empty ScriptTestObserver.
		QJSValue createObserver();

		/// \brief Get signal state, if signal is not found then exception is thrown.
		QJSValue signalState(QString appSignalId);

		/// \brief Get signal value, if signal is not found then exception is thrown.
		/// <b>Note:</b> This function does not return full signal state with validity and other flags.
		double signalValue(QString appSignalId);

		/// \brief Override signal value. Returns true if signal value is overriden.
		/// <b>Note:</b> After overriding all required signals, it is recommended to call <b>waitForSignalOverrides</b> function to wait for
		/// all signals to be written. After it returns, the test can continue and its further results can be analyzed.
		bool overrideSignalValue(QString appSignalId, QVariant value);

		/// \brief Waits while all overriden signal value is written to LM. Returns true if signal value is overriden, false on timeout.
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

		/// \brief Checks if a signal exists.
		bool signalExists(QString appSignalId) const;

		/// \brief Get signal description, if a signal is not found then exception is thrown.
		AppSignalParam signalParam(QString appSignalId);

		/// \brief Get activation status of tuning source (LM) in Single-LM control mode.
		///
		/// Returns true if all connections from Tuning Service(s) to LM are active. Otherwise, returns false.
		bool tuningSourceIsActive(QString lmEquipmentId);

		/// \brief Get activation status of tuning source (LM) in Single-LM control mode.
		///
		/// Returns true if all connections from Tuning Service(s) to LM are inactive. Otherwise, returns false.
		bool tuningSourceIsInactive(QString lmEquipmentId);

		/// \brief Activates or deactivates tuning source (LM) in  Single-LM control mode. Returns true on success, false on error.
		///
		/// \param lmEquipmentId specifies LM equipment id.
		/// \param activate specifies the operation type: true - activate, false - deactivate.
		bool activateTuningSource(QString lmEquipmentId, bool activate);

		/// \brief Loads an array of strings from the specified text file.
		///
		/// \param fileName specifies the name of the file. Can contain "%TEMP%" macro to specify a temp path.
		/// Returns an object with two fields: "ok" (boolean) and "strings" (array of strings).
		QVariantMap loadTextFile(QString fileName);

		/// \brief Returns list of Comparator (setpoint comparators) assigned to the signal specified by <b>signalId</b>.
		QJSValueList setpointsByInput(QString signalId) const;

		/// \brief Returns Comparator (setpoint comparator) assigned where <b>signalId</b> is an output signal.
		QJSValue setpointByOutput(QString signalId) const;

	public:
		QString projectName() const;
		void setProjectName(const QString& value);

		int buildNo() const;
		void setBuildNo(int value);

		qint64 executionTimeout() const;
		void setExecutionTimeout(qint64 value);

		bool debugMessagesEnabled() const;
		void setDebugMessagesEnabled(bool value);

		// Data
		//
	private:
		ILogFile* m_appLog = nullptr;
		ILogFile* m_testLog = nullptr;

		IInputController* m_inputController = nullptr;
		IOutputController* m_outputController = nullptr;
		
		const ComparatorSet* m_setpoints = nullptr;

		std::set<QString> m_overridenSignals; // Contains AppSignalIds of overriden signals

		std::atomic<qint64> m_executionTimeout{-1};
		bool m_debugMessagesEnabled = false;

		QString m_projectName;
		int m_buildNo = 0;
	};
} // namespace TestSuite
