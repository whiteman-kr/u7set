#pragma once

#include "../lib/Tuning/ITuningSignalManager.h"
#include "../lib/Tuning/ITuningTcpClient.h"
#include "VFrame30Lib_global.h"

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
		<b>Example:</b>

		\code
		// Request signal state by identifier "#APPSIGNALID"
		//
		var state = tuning.signalState("#APPSIGNALID");

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
		var newValue = state.Value;

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
	class VFRAME30LIBSHARED_EXPORT TuningController : public QObject
	{
		Q_OBJECT

	public:
		TuningController() = delete;
		TuningController(ITuningSignalManager* signalManager, ITuningTcpClient* tcpClient, QObject* parent = nullptr);

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

	protected:
		virtual bool writingEnabled() const;

	private:
		ITuningSignalManager* m_signalManager = nullptr;
		ITuningTcpClient* m_tcpClient = nullptr;
	};

}
