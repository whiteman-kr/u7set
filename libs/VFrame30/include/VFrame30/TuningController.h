#pragma once

#include "../AppSignalLib/ITuningSignalManager.h"
#include "../lib/Tuning/ITuningAuthorization.h"
#include "../lib/Tuning/ITuningConnection.h"

#include <QJSValueList>
#include <QStringList>

class AppSignalParam;
class TuningSignalState;

namespace VFrame30
{

	/*! \class TuningController
		\ingroup controllers
		\brief This class is used to get tuning signals parameters and states, write tuning values and in Monitor and TuningClient applications

		This class is used to get tuning signals parameters and states, write tuning values and in Monitor and TuningClient applications.
		 It is accessed by global <b>tuning</b> object.

		Information about signal parameters and states is requested from TuningService.

		\warning TuningController is always available in TuningClient. In Monitor it is available only in non-safety projects when Tuning function is enabled.

		\n
		\warning It is highly recommended to check function return values, because errors can occur. For example,
		connection to TuningService can be down, or signal with specified identifier could not exist.<br>

		\n
		\warning It is highly recommended to check value types and signals types matching:
		- Boolean vaulues should be written only to discrete signals, and number values only to analog signals;
		- Fractional numbers should be written only to signals with floating-point type;
		- Numbers should be in the correct range. For example, if signal type is 32-bit integer, writing <b>23e+12</b> value will cause an error.
		<br>

		\n
		<b>Example:</b>

		\code{.js}
		// Request signal state by identifier "#APPSIGNALID"
		//
		let state = tuning.signalState("#APPSIGNALID");

		if (state == undefined)
		{
			// No state was received for this signal, print an error message
			//
			view.errorMessageBox("Signal does not exist!");
			return;
		}

		// Check signal validity
		//
		if (state.Valid == false)
		{
			view.errorMessageBox("Signal is not valid!");
			return;
		}

		//Increase signal value to 10
		//
		let newValue = state.Value;

		newValue =+ 10;

		// Write new value to logic module
		//
		if (tuning.writeValue("#APPSIGNALID", newValue) == false)
		{
			view.errorMessageBox("Value set error!");
			return;
		}
		\endcode

		\code{.js}
		// Getting signal list with specified tag
		//
		let ids = tuning.signalIdsByTag("sim");

		for (let i = 0; i < ids.length; i++)
		{
			// ids[i] contains AppSignalID of signal with tag "sim"
			//
		}
		\endcode
	*/
	class TuningController final : public QObject
	{
		Q_OBJECT

	public:
		TuningController() = delete;
		TuningController(ITuningSignalManager& signalManager,
						 ITuningConnection& tuningConnection,
						 ITuningAuthorization& tuningAuthorization,
						 QWidget* parent);

	public:
		AppSignalParam signalParam(const QString& appSignalId, bool* ok) const;
		TuningSignalState signalState(const QString& appSignalId, bool* ok) const;

	public slots:
		/// \brief Returns AppSignalParam structure or undefined if signal does not exist.
		QVariant signalParam(const QString& appSignalId) const;	// If no signal with specified appSignalID found, QVariant is undefined

		/// \brief Returns TuningSignalState structure or undefined if signal does not exist.
		QVariant signalState(const QString& appSignalId) const;	// If no signal with specified appSignalID found, QVariant is undefined

		/// \brief Returns an array of TuningSignalState structures of signals specified by <b>signalIds</b>. If signal is not found, then the <b>stateAvailable</b> is false.
		QJSValueList signalStates(QStringList appSignalIds) const;	// Returns TuningSignalState

		/// \brief Returns <b>true</b> if signal specified by <b>signalId</b> is exist.
		bool signalExists(QString signalId) const;

		/// \brief The function takes a list of signal IDs and returns true or false depending on whether all signals exist or not.
		bool signalsExist(QStringList signalIds);

		/// \brief Returns <b>true</b> for discrete signals.
		bool isDiscrete(QString signalId) const;

		/// \brief Returns <b>true</b> for analog signals.
		bool isAnalog(QString signalId) const;

		/// \brief Returns precision for analog signals. If signal is discrete or not found, returns 0.
		int precision(QString signalId) const;

		/// \brief Returns list of AppSignalIDs with specified <b>tag</b>.
		QStringList signalIdsByTag(QString tag) const;

		/// \brief Writes value of tuning signal. On success, returns true. Returns false if signal is not found, connection to TuningService is not established or value is out of range.
		bool writeValue(QString appSignalId, QVariant value);

		/// \brief Writes value of tuning signal of boolean value. On success, returns true. Returns false if signal is not found, connection to TuningService is not established.
		bool writeValueBool(QString appSignalId, bool value);

		/// \brief Writes value of tuning signal of integer value. On success, returns true. Returns false if signal is not found, connection to TuningService is not established.
		bool writeValueInt(QString appSignalId, int value);

		/// \brief Writes value of tuning signal of double value. On success, returns true. Returns false if signal is not found, connection to TuningService is not established.
		bool writeValueDouble(QString appSignalId, double value);

		/// \brief Copies written values from <b>Tuning Mode Tuning Values</b> area to <b>Run Mode Tuning Values</b> area. Should be used in TuningClient scripts only when <b>AutoApply</b> property is set to <b>false</b>. Monitor always applies values automatically. Returns false on authorization error.
		bool apply();

		/// \brief Returns true if tuning user authorization is enabled, otherwise returns false
		bool tuningLogin() const;

		/// \brief Returns state of tuning user authorization. If tuning authorization is disabled, also returns true.
		bool isLoggedIn() const;

		/// \brief Returns name of authorized tuning user. If tuning authorization is disabled or user is not logged in, returns empty string.
		QString userName() const;

		/// \brief Returns tags of authorized tuning user. If tuning authorization is disabled or user is not logged in, returns empty string.
		QStringList userTags() const;

	private:
		QWidget* m_parent{nullptr};
		ITuningSignalManager& m_signalManager;
		ITuningConnection& m_tuningConnection;
		ITuningAuthorization& m_tuningAuthorization;
	};

}
