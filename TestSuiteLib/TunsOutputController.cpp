#include "TunsOutputController.h"

namespace TestSuite
{
	TunsOutputController::TunsOutputController(const SoftwareInfo& softwareInfo,
											   const std::vector<SoftwareEndpoint::TuningService>& tuningServices,
											   TuningClientSettings::LmStatusFlagMode lmStatusFlagMode,
											   ILogFile* logFile):
		m_signalManager{softwareInfo.equipmentID(), logFile},
		m_appLog{logFile, "TunsOutputController"},
		m_connection{m_signalManager, m_signalManager, logFile, &m_tuningLogStub}
	{

		m_connection.updateConnections(softwareInfo,
							   tuningServices,
							   true/*autoApply*/,
							   lmStatusFlagMode);


	}


	bool TunsOutputController::waitForConnection(qint64 timeoutMs) const
	{
		// Wait for connection established
		//
		QElapsedTimer timer;
		timer.start();

		while (timer.hasExpired(timeoutMs) == false)
		{
			if (QThread::currentThread()->isInterruptionRequested() == true)
			{
				return false;
			}

			//QCoreApplication::instance()->processEvents();
			QThread::msleep(200);

			// Wait for 30 replies, so all signals are loaded and some states are received.
			//
			std::vector<Tcp::ConnectionState> tunsConnStates = m_connection.tcpTuningConnStates();
			if (std::all_of(tunsConnStates.begin(), tunsConnStates.end(), [](const auto& s) { return s.isConnected; }))
			{
				break;
			}
		}

		bool connected = true;
		std::vector<Tcp::ConnectionState> tunsConnStates = m_connection.tcpTuningConnStates();

		for (const Tcp::ConnectionState& state : tunsConnStates)
		{
			if (state.isConnected == false)
			{
				connected = false;
				m_appLog.writeError(QString{"Cannot establish connection to TuningService %1"}
									.arg(state.serverEquipmentID));
			}
		}

		if (connected == false)
		{
			return false;
		}

		// Wait that TuningConnection loads all signal params.
		//
		timer.restart();


		while (timer.hasExpired(30'000) == false && m_connection.tuningSourcesInfo().empty() == true)
		{
			if (QThread::currentThread()->isInterruptionRequested() == true)
			{
				return false;
			}

			QThread::msleep(200);
		}

		if (m_connection.tuningSourcesInfo().empty() == true)
		{
			m_appLog.writeError("Receiving TuningSources info timeout");
			return false;
		}

		m_appLog.writeMessage("TuningSources info arrived");
		return true;
	}
}
