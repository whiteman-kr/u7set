
#include "AdsInputController.h"

namespace TestSuite
{
	AdsInputController::AdsInputController(ClientLib::AppSignalManager& signalManager,
										   const SoftwareInfo& softwareInfo,
										   const std::vector<SoftwareEndpoint::AppDataService>& appDataServices,
										   ILogFile* logFile) :
		m_signalManager{signalManager},
		m_appLog{logFile, "AdsInputController"},
		m_connection{m_signalManager, &m_signalManager, logFile}
	{
		m_connection.updateConnections(softwareInfo, appDataServices);
		return;
	}

	bool AdsInputController::waitForConnection(qint64 timeoutMs) const
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
			std::vector<Tcp::ConnectionState> adsConnStates = m_connection.tcpSignalConnStates();
			if (std::all_of(adsConnStates.begin(), adsConnStates.end(), [](const auto& s) { return s.isConnected; }))
			{
				break;
			}
		}

		bool connected = true;
		std::vector<Tcp::ConnectionState> adsConnStates = m_connection.tcpSignalConnStates();

		for (const Tcp::ConnectionState& state : adsConnStates)
		{
			if (state.isConnected == false)
			{
				connected = false;
				m_appLog.writeError(QString{"Cannot establish connection to AppDataService %1"}
									.arg(state.serverEquipmentID));
			}
		}

		if (connected == false)
		{
			return false;
		}

		// Wait that AdsConnection loads all signal params.
		//
		m_appLog.writeMessage("Waiting for all AppSignalParams to load...");

		timer.restart();

		while (timer.hasExpired(30'000) == false && m_connection.signalParamsLoaded() == false)
		{
			if (QThread::currentThread()->isInterruptionRequested() == true)
			{
				return false;
			}

			QThread::msleep(200);
		}

		if (m_connection.signalParamsLoaded() == false)
		{
			m_appLog.writeError("Loading AppSignalParams timeout!");
			return false;
		}

		m_appLog.writeMessage("All AppSignalParams are loaded.");

		// Wait that AdsConnection requests all signal states at least once.
		//
		m_appLog.writeMessage("Waiting for all AppSignalStates to be requested...");

		timer.restart();

		while (timer.hasExpired(30'000) == false && m_connection.signalStatesLoaded() == false)
		{
			if (QThread::currentThread()->isInterruptionRequested() == true)
			{
				return false;
			}

			QThread::msleep(200);
		}

		if (m_connection.signalStatesLoaded() == false)
		{
			m_appLog.writeError("Loading AppSignalStates timeout!");
			return false;
		}

		m_appLog.writeMessage("All AppSignalStates are requested.");

		return true;
	}

	bool AdsInputController::signalExists(const QString& signalId) const
	{
		return m_signalManager.signalExists(signalId);
	}

	AppSignalParam AdsInputController::signalParam(const QString& appSignalId, bool* found) const
	{
		return m_signalManager.signalParam(appSignalId, found);
	}

	AppSignalState AdsInputController::signalState(const QString& appSignalId, bool* found) const
	{
		return m_signalManager.signalState(appSignalId, found);
	}

	bool AdsInputController::expectSignalValue(QString appSignalId, qint64 timeoutMs, double value, double tolerance) const
	{
		QElapsedTimer timer;
		timer.start();

		AppSignalState state;

		qint64 nsecs = static_cast<qint64>(timeoutMs) * 1'000'000;

		do
		{
			bool found = false;
			state = m_signalManager.signalState(appSignalId, &found);
			if (found == false || state.isStateAvailable() == false || state.isValid() == false)
			{
				return false;
			}

			if (std::isnan(value) == true && std::isnan(state.value()) == true)
			{
				return true;
			}

			if (std::isinf(value) == true && std::isinf(state.value()) == true && std::signbit(value) == std::signbit(state.value()))
			{
				return true;
			}

			if (std::abs(state.value() - value) <= tolerance)
			{
				return true;
			}

			qint64 timeLeftUs = std::min<qint64>((nsecs - timer.nsecsElapsed()) / 1'000, 10'000);
			if (timeLeftUs <= 0)
			{
				break;
			}

			QThread::usleep(static_cast<unsigned long>(timeLeftUs));

		} while(QThread::currentThread()->isInterruptionRequested() == false);

		return false;
	}
}

