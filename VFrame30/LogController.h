#pragma once

#include "../lib/ILogFile.h"

namespace VFrame30
{
	/*! \class LogController
		\ingroup controllers
		\brief This class is used to write information to application's log file.

		This class is used to write information to application's log file.
		It is accessed by global <b>log</b> object.

		Each log record contain following columns:
		- Session identifier (e.g. "5314966085873379440");
		- Date and time (e.g. "23.09.2020 14:57:33.607");
		- Record type (e.g. "MSG");
		- Record text (e.g. "Application started").

		Log file records can be one of the following types: <b>Alert, Error, Warning, Message and Text</b>.

		When <b>Alert</b> record is written to the log file, <b>Monitor</b> and <b>TuningClient</b> applications also display
		a pop-up window with its text. This type of record should be used to additionally inform operator
		about emergency events, for example, when tunable signal failed to write.

		When <b>Error</b> and <b>Warning</b> record is written to the log file, <b>Monitor</b> and <b>TuningClient</b> applications
		highlight an item on status bar. Thereby operator can see that some errors or warnings have been occured.

		<b>Message</b> record is written to the log file without interface alerting.

		<b>Text</b> record is written to the log file, but it does not contain date and time. It can be used for some
		log appearance improvements. For example, it can add a divider ("---") text before application session messages.

		\n
		<b>Example:</b>

		\code

		if (result === false)
		{
			log.writeAlert("Operation failed!");
		}
		else
		{
			log.writeMessage("Operation successful.");
		}
		\endcode

		\n
		<b>Log file example:</b>
		\code
5314966085873379440  	---
5314966085873379440  	23.09.2020 14:57:33.607		MSG	Application started
5314966085873379440  	23.09.2020 14:57:35.884		MSG	New configuration arrived
5314966085873379440  	23.09.2020 14:57:35.884		MSG	TUNS1 (id, ip, port): SYSTEMID_RACKID_WS00_TUNS, 127.0.0.1, 13333
5314966085873379440  	23.09.2020 14:57:35.889		MSG	TuningTcpClient::slot_signalsUpdated
5314966085873379440  	23.09.2020 14:57:35.892		MSG	New configuration: file '/Schemas.tvs/TS001.tvs' updated
5314966085873379440  	23.09.2020 14:57:35.892		MSG	New configuration: file '/Schemas.tvs/TS002.tvs' updated
5314966085873379440  	23.09.2020 14:57:35.892		MSG	New configuration: file '/SYSTEMID_RACKID_WS00_TUN/TuningSignals.dat' updated
5314966085873379440  	23.09.2020 14:57:35.892		MSG	New configuration: file '/SYSTEMID_RACKID_WS00_TUN/ObjectFilters.xml' updated
5314966085873379440  	23.09.2020 14:57:35.892		MSG	New configuration: file '/SYSTEMID_RACKID_WS00_TUN/GlobalScript.js' updated
5314966085873379440  	23.09.2020 14:57:35.892		MSG	New configuration: file '/SYSTEMID_RACKID_WS00_TUN/OnConfigurationArrivedScript.js' updated
5314966085873379440  	23.09.2020 14:57:35.892		MSG	New configuration: file '/SYSTEMID_RACKID_WS00_TUN/TuningClientBehavior.xml' updated
5314966085873379440  	23.09.2020 14:57:35.892		MSG	New configuration: appearance updated
5314966085873379440  	23.09.2020 14:57:35.892		MSG	New configuration: servers updated
5314966085873379440  	23.09.2020 14:57:35.892		MSG	TuningTcpClient::slot_configurationArrived
5314966085873379440  	23.09.2020 14:57:46.719		MSG	Application terminated
		\endcode

	*/
	class LogController : public QObject
	{
		Q_OBJECT

	public:
		LogController() = delete;
		LogController(ILogFile* logFile, QObject* parent = nullptr);

	public slots:
		/// \brief Writes Alert record to the log file.
		bool writeAlert(QString text);

		/// \brief Writes Error record to the log file.
		bool writeError(QString text);

		/// \brief Writes Warning record to the log file.
		bool writeWarning(QString text);

		/// \brief Writes Message record to the log file.
		bool writeMessage(QString text);

		/// \brief Writes Text record to the log file.
		bool writeText(QString text);

	private:
		ILogFile* m_logFile = nullptr;

	};
}
