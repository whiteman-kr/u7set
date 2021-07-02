#pragma once

#include "../lib/Tuning/ITuningSignalManager.h"
#include "../lib/Tuning/ITuningTcpClient.h"

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

		\code
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
	*/
	class TuningController : public QObject
	{
		Q_OBJECT

	public:
		TuningController() = delete;
		TuningController(ITuningSignalManager* signalManager, ITuningTcpClient* tcpClient, QWidget* parent = nullptr);

		void setTcpClient(ITuningTcpClient* tcpClient);
		void resetTcpClient();

	public slots:
		AppSignalParam signalParam(const QString& appSignalId, bool* ok);
		TuningSignalState signalState(const QString& appSignalId, bool* ok);

		/// \brief Returns AppSignalParam structure or undefined if signal does not exist.
		QVariant signalParam(const QString& appSignalId);	// If no signal with specified appSignalID found, QVariant is undefined

		/// \brief Returns TuningSignalState structure or undefined if signal does not exist.
		QVariant signalState(const QString& appSignalId);	// If no signal with specified appSignalID found, QVariant is undefined

		/// \brief Writes value of tuning signal. On success, returns true. Returns false if signal is not found, connection to TuningService is not established or value is out of range.
		bool writeValue(QString appSignalId, QVariant value);

		/// \brief Copies written values from <b>Tuning Mode Tuning Values</b> area to <b>Run Mode Tuning Values</b> area. Should be used in TuningClient scripts only when <b>AutoApply</b> property is set to <b>false</b>. Monitor always applies values automatically.
		void apply();

	protected:
		virtual bool checkTuningAccess() const;

	private:
		ITuningSignalManager* m_signalManager = nullptr;
		ITuningTcpClient* m_tcpClient = nullptr;
	};

}
