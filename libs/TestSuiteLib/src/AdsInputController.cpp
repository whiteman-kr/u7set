
#include "AdsInputController.h"
#include "TestObserver.h"

namespace TestSuite
{
	AdsInputController::AdsInputController(ClientLib::AppSignalManager& signalManager, ILogFile* logFile) :
		m_signalManager{signalManager},
		m_appLog{logFile, "AdsInputController"},
		m_connection{m_signalManager, &m_signalManager, logFile}
	{
		return;
	}

	bool AdsInputController::init(qint64 timeoutMs)
	{
		m_connection.updateConnections(m_softwareInfo, m_appDataServices);

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

			// QCoreApplication::instance()->processEvents();
			QThread::msleep(1200);

			// Wait for 30 replies, so all signals are loaded and some states are received.
			//
			std::vector<Tcp::ConnectionState> adsConnStates = m_connection.tcpSignalConnStates();
			if (std::all_of(adsConnStates.begin(),
							adsConnStates.end(),
							[](const auto& s)
							{
								return s.isConnected;
							}))
			{
				break;
			}
		}

		std::vector<Tcp::ConnectionState> adsConnStates = m_connection.tcpSignalConnStates();

		m_appLog.writeMessage("AppDataService connections, Count: " + QString::number(adsConnStates.size()));

		for (const Tcp::ConnectionState& state : adsConnStates)
		{
			if (state.isConnected == true)
			{
				m_appLog.writeMessage(
					QString{"AppDataService %1: %2"}.arg(state.serverEquipmentID).arg(state.isConnected ? "Connected" : "Not connected"));
			}
			else
			{
				m_appLog.writeError(
					QString{"AppDataService %1: %2"}.arg(state.serverEquipmentID).arg(state.isConnected ? "Connected" : "Not connected"));
			}
		}

		bool allAreConnected = std::all_of(adsConnStates.begin(),
										   adsConnStates.end(),
										   [](const auto& s)
										   {
											   return s.isConnected;
										   });
		if (allAreConnected == false)
		{
			m_appLog.writeError("Not all AppDataService connections are established!");
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

		m_appLog.writeMessage("All AppSignalParams are loaded, TotalSignalCount: " + QString::number(m_signalManager.signalsCount()));

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

		m_appLog.writeMessage("All AppSignalStates are requested, TotalSignalCount: " + QString::number(m_signalManager.signalsCount()));

		return true;
	}

	bool AdsInputController::shutdown()
	{
		m_connection.updateConnections(m_softwareInfo, {});
		return true;
	}

	bool AdsInputController::signalExists(const QString& signalId) const
	{
		return m_signalManager.signalExists(signalId);
	}

	std::optional<AppSignalParam> AdsInputController::signalParam(const QString& appSignalId) const
	{
		return m_signalManager.signalParam(appSignalId);
	}

	std::optional<AppSignalState> AdsInputController::signalState(const QString& appSignalId) const
	{
		return m_signalManager.signalState(appSignalId);
	}

	bool AdsInputController::expectSignalValue(QString appSignalId, qint64 timeoutMs, double value, double tolerance) const
	{
		QElapsedTimer timer;
		timer.start();

		qint64 nsecs = static_cast<qint64>(timeoutMs) * 1'000'000;

		do
		{
			std::optional<AppSignalState> state = m_signalManager.signalState(appSignalId);

			if (state.has_value() == false || state->isStateAvailable() == false || state->isValid() == false)
			{
				return false;
			}

			if (std::isnan(value) == true && std::isnan(state->value()) == true)
			{
				return true;
			}

			if (std::isinf(value) == true && std::isinf(state->value()) == true && std::signbit(value) == std::signbit(state->value()))
			{
				return true;
			}

			if (std::abs(state->value() - value) <= tolerance)
			{
				return true;
			}

			qint64 timeLeftUs = std::min<qint64>((nsecs - timer.nsecsElapsed()) / 1'000, 10'000);
			if (timeLeftUs <= 0)
			{
				break;
			}

			QThread::usleep(static_cast<unsigned long>(timeLeftUs));

		} while (QThread::currentThread()->isInterruptionRequested() == false);

		return false;
	}

	tl::expected<std::unique_ptr<ITestObserver>, QString> AdsInputController::createTestObserver()
	{
		if (m_appDataServices.empty() == true)
		{
			return tl::make_unexpected("No configured AppDataService(s).");
		}

		ClientLib::ISignalDataServer& signalDataServer = m_signalManager;

		auto testObserver = std::make_unique<TestSuite::TestObserver>(signalDataServer, m_softwareInfo, m_appDataServices, m_appLog);
		return testObserver;
	}

	void AdsInputController::updateConnections(const SoftwareInfo& softwareInfo,
											   const std::vector<SoftwareEndpoint::AppDataService>& appDataServices)
	{
		m_softwareInfo = softwareInfo;
		m_appDataServices = appDataServices;
		return;
	}
} // namespace TestSuite
