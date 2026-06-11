#include "TunsOutputController.h"

namespace
{
	bool AutoApply = true;
}

namespace TestSuite
{

	TunsOutputController::TunsOutputController(ILogFile* logFile) :
		m_signalManager{{}, logFile},
		m_appLog{logFile, "TunsOutputController"},
		m_authorization{},
		m_connection{m_signalManager, m_signalManager, m_signalManager, m_authorization, logFile, &m_tuningLogStub}
	{
		return;
	}

	bool TunsOutputController::init(qint64 timeoutMs)
	{
		m_connection.updateConnections(m_softwareInfo, m_tuningServices, AutoApply, m_lmStatusFlagMode);

		if (m_tuningServices.empty() == true)
		{
			return true;
		}

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
			QThread::msleep(200);

			// Wait for 30 replies, so all signals are loaded and some states are received.
			//
			std::vector<Tcp::ConnectionState> tunsConnStates = m_connection.tcpTuningConnStates();
			if (std::all_of(tunsConnStates.begin(),
							tunsConnStates.end(),
							[](const auto& s)
							{
								return s.isConnected;
							}))
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
				m_appLog.writeError(QString{"Cannot establish connection to TuningService %1"}.arg(state.serverEquipmentID));
			}
		}

		if (connected == false)
		{
			return false;
		}

		// Wait that TuningConnection loads all tuning sources info
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

		// Wait that TuningConnection loads all tuning sources info
		//
		m_appLog.writeMessage("Waiting for all TuningSignalStates to be requested...");

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
			m_appLog.writeError("Loading TuningSignalStates timeout!");
			return false;
		}

		m_appLog.writeMessage("All TuningSignalStates are requested.");

		return true;
	}

	bool TunsOutputController::shutdown()
	{
		m_connection.updateConnections(SoftwareInfo{}, {}, AutoApply, m_lmStatusFlagMode);
		return true;
	}

	bool TunsOutputController::writeSignalValue(const QString& appSignalId, const QVariant& value)
	{
		auto asp = m_signalManager.signalParam(appSignalId);
		if (asp.has_value() == false)
		{
			return false;
		}

		bool found = false;
		TuningSignalState state = m_signalManager.state(appSignalId, &found);
		if (found == false)
		{
			return false;
		}

		if (state.valid() == false || state.controlIsEnabled() == false || state.writingIsEnabled() == false)
		{
			return false;
		}

		m_signalManager.setUnappliedValue(::calcHash(appSignalId), TuningValue{asp->tuningType(), value.toDouble()});
		return m_connection.writeTuningSignal(appSignalId, value);
	}

	bool TunsOutputController::waitForAllSignalsWritten(qint64 timeoutMs, qint64& timeElapsedMs) const
	{
		using namespace std::chrono_literals;
		using namespace std::chrono;

		qint64 nsecs = static_cast<qint64>(timeoutMs) * 3'000'000;

		QElapsedTimer timer;
		timer.start();

		while (QThread::currentThread()->isInterruptionRequested() == false)
		{
			microseconds timeLeftUs{std::min<qint64>((nsecs - timer.nsecsElapsed()) / 1'000, 100'000)};
			if (timeLeftUs <= 0us)
			{
				timeLeftUs = 0us;
			}

			if (m_signalManager.waitForAllApplied(duration_cast<milliseconds>(timeLeftUs)) == true)
			{
				timeElapsedMs = timer.nsecsElapsed() / 1'000'000;
				assert(timeElapsedMs <= timeoutMs);
				return true;
			}

			if (timeLeftUs <= 0us)
			{
				break;
			}
		}

		timeElapsedMs = timeoutMs;
		return false;
	}

	bool TunsOutputController::tuningSourceIsActive(QString lmEquipmentId) const
	{
		Hash sourceHash = ::calcHash(lmEquipmentId);
		int sourceStatesCount = m_connection.tuningSourceStatesCount(sourceHash);
		int activeStatesCount = m_connection.activatedTuningSourceStatesCount(sourceHash);

		bool active = sourceStatesCount > 0 && sourceStatesCount == activeStatesCount;
		return active;
	}

	bool TunsOutputController::tuningSourceIsInactive(QString lmEquipmentId) const
	{
		Hash sourceHash = ::calcHash(lmEquipmentId);
		int sourceStatesCount = m_connection.tuningSourceStatesCount(sourceHash);
		int activeStatesCount = m_connection.activatedTuningSourceStatesCount(sourceHash);

		bool inactive = sourceStatesCount > 0 && activeStatesCount == 0;
		return inactive;
	}

	bool TunsOutputController::activateTuningSource(QString lmEquipmentId, bool activate)
	{
		Hash sourceHash = ::calcHash(lmEquipmentId);
		if (m_connection.activateTuningSource(sourceHash, activate) == false)
		{
			return false;
		}

		for (int i = 0; i < 50; i++)
		{
			if ((activate == true && tuningSourceIsActive(lmEquipmentId) == true) ||
				(activate == false && tuningSourceIsInactive(lmEquipmentId) == true))
			{
				return true;
			}
			QThread::msleep(100);
		}
		return false;
	}

	void TunsOutputController::updateConnections(const SoftwareInfo& softwareInfo,
												 const std::vector<SoftwareEndpoint::TuningService>& tuningServices,
												 const QByteArray& signalsFile,
												 TuningClientSettings::LmStatusFlagMode lmStatusFlagMode)
	{
		m_softwareInfo = softwareInfo;
		m_tuningServices = tuningServices;
		m_lmStatusFlagMode = lmStatusFlagMode;

		m_signalManager.setClientEquipmentId(softwareInfo.equipmentID());
		m_signalManager.load(signalsFile);

		return;
	}

	void TunsOutputController::setUserName(QString userName)
	{
		m_authorization.setUserName(userName);
	}
} // namespace TestSuite
