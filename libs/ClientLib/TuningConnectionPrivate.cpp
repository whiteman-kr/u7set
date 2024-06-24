#ifndef CLIENT_LIB_DOMAIN
#error Do not include this file in the project! Link ClientLib instead.
#endif

#include "TuningConnectionPrivate.h"
#include "TuningTcpClient.h"
#include <ClientLib/TuningSignalManager.h>
#include "../UtilsLib/SimpleThread.h"

namespace ClientLib
{

	TuningConnectionPrivate::Connection::Connection(const SoftwareInfo& softwareInfo,
													const SoftwareEndpoint::TuningService& tuns,
													bool autoApply,
													TuningClientSettings::LmStatusFlagMode lmStatusFlagMode,
													ITuningSignalUpdater& signalUpdater,
													IRecentAppSignals& recentTuningSignals,
													ITuningAuthorization& tuningAuthorization,
													ILogFile* logFile,
													ITuningLog* tuningLog)
	{
		tcpTuningClient = new TuningTcpClient{softwareInfo, tuns, signalUpdater, recentTuningSignals, tuningAuthorization, logFile, tuningLog};
		tcpTuningClient->setServers(tuns.clientRequestAddress, tuns.clientRequestAddress, true);
		tcpTuningClient->setAutoApply(autoApply);
		tcpTuningClient->setLmStatusFlagMode(lmStatusFlagMode);

		tcpClientThread = new ::SimpleThread{tcpTuningClient};
		tcpClientThread->start();
		return;
	}

	TuningConnectionPrivate::Connection::~Connection()
	{
		stopAndDestroy();
		return;
	}

	void TuningConnectionPrivate::Connection::stopAndDestroy()
	{
		if (tcpClientThread != nullptr)
		{
			tcpClientThread->quitAndWait(10000);
			delete tcpClientThread;
		}

		tcpTuningClient = nullptr;
		tcpClientThread = nullptr;

		return;
	}

	HostAddressPort TuningConnectionPrivate::Connection::address() const
	{
		Q_ASSERT(tcpTuningClient);
		return tcpTuningClient->serverAddressPort1();
	}

	bool TuningConnectionPrivate::Connection::signalStatesLoaded() const
	{
		Q_ASSERT(tcpTuningClient);
		return tcpTuningClient->signalStatesLoaded();
	}

	TuningConnectionPrivate::TuningConnectionPrivate(ITuningSignalManager& tuningSignalManager,
													 ITuningSignalUpdater& tuningSignalUpdater,
													 IRecentAppSignals& recentTuningSignals,
													 ITuningAuthorization& tuningAuthorization,
													 ILogFile* logFile,
													 ITuningLog* tuningLog) :
		m_tuningSignalManager{tuningSignalManager},
		m_tuningSignalUpdater{tuningSignalUpdater},
		m_recentTuningSignals(recentTuningSignals),
		m_tuningAuthorization(tuningAuthorization),
		m_logFile{logFile, "TuningConnection"},
		m_tuningLog(tuningLog)
	{
		return;
	}

	void TuningConnectionPrivate::updateConnections(const SoftwareInfo& softwareInfo,
													const std::vector<SoftwareEndpoint::TuningService>& tuningServices,
													bool autoApply,
													TuningClientSettings::LmStatusFlagMode lmStatusFlagMode)
	{
		m_logFile.writeMessage("updateConnections()");

		m_conns.clear();	// it will stop all connection threads and destroy them

		for (const SoftwareEndpoint::TuningService& tuns : tuningServices)
		{
			auto it = std::find_if(m_conns.begin(), m_conns.end(), [&tuns](const Connection& c)
								   {
									   return c.address() == tuns.clientRequestAddress;
								   });

			if (it != m_conns.end())
			{
				// Such connection already exists
				//
				continue;
			}

			m_conns.emplace_back(softwareInfo, tuns, autoApply, lmStatusFlagMode, m_tuningSignalUpdater, m_recentTuningSignals, m_tuningAuthorization,
								 m_logFile.logFile(), m_tuningLog);
		}

		return;
	}

	std::vector<Tcp::ConnectionState> TuningConnectionPrivate::tcpTuningConnStates() const
	{
		std::vector<Tcp::ConnectionState> states;
		states.reserve(m_conns.size());

		for (const Connection& c : m_conns)
		{
			states.emplace_back(c.tcpTuningClient->getConnectionState());
		}

		return states;
	}

	std::vector<TuningSource> TuningConnectionPrivate::tuningSourcesInfo() const
	{
		std::vector<TuningSource> result;
		result.reserve(m_conns.size() * 2);

		for (const Connection& c : m_conns)
		{
			std::vector<TuningSource> tsi = c.tcpTuningClient->tuningSourcesInfo();
			result.insert(result.end(), tsi.begin(), tsi.end());
		}

		return result;
	}

	std::vector<TuningSource> TuningConnectionPrivate::tuningSourceInfo(Hash sourceHash) const
	{
		std::vector<TuningSource> result;
		result.reserve(m_conns.size());

		ClientLib::TuningSource ts;

		for (const Connection& c : m_conns)
		{
			bool ok = c.tcpTuningClient->tuningSourceInfo(sourceHash, &ts);
			if (ok == true)
			{
				result.push_back(ts);
			}
		}
		return result;
	}

	int TuningConnectionPrivate::tuningSourceStatesCount(Hash sourceHash) const
	{
		int connectedCount = 0;

		for (const Connection& c : m_conns)
		{
			if (c.tcpTuningClient->isConnected() == true &&
				c.tcpTuningClient->singleLmControlMode() == true &&
				c.tcpTuningClient->hasTuningSource(sourceHash) == true)
			{
				connectedCount++;
			}
		}

		return connectedCount;
	}

	int TuningConnectionPrivate::activatedTuningSourceStatesCount(Hash sourceHash) const
	{
		int activeCount = 0;

		for (const Connection& c : m_conns)
		{
			if (c.tcpTuningClient->isConnected() == true &&
				c.tcpTuningClient->singleLmControlMode() == true &&
				c.tcpTuningClient->hasTuningSource(sourceHash) == true)
			{
				if (c.tcpTuningClient->activeTuningSource() == sourceHash)
				{
					activeCount++;
				}
			}
		}

		return activeCount;
	}

	bool TuningConnectionPrivate::activateTuningSource(Hash sourceHash, bool activate) const
	{
		bool result = true;

		for (const Connection& c : m_conns)
		{
			if (c.tcpTuningClient->isConnected() == true &&
				c.tcpTuningClient->singleLmControlMode() == true &&
				c.tcpTuningClient->hasTuningSource(sourceHash) == true)
			{
				bool sourceActive = c.tcpTuningClient->activeTuningSource() == sourceHash;

				if (sourceActive != activate)
				{
					bool forceTakeControl = c.tcpTuningClient->clientIsActive() == false;
					result &= c.tcpTuningClient->activateTuningSourceControl(sourceHash, activate, forceTakeControl);
				}
			}
		}

		return result;
	}

	QString TuningConnectionPrivate::clientControlInfo() const
	{
		QString str;

		for (const Connection& c : m_conns)
		{
			str += tr("%1: ").arg(c.tcpTuningClient->connectedSoftwareInfo().equipmentID());

			QString activeClientId = c.tcpTuningClient->activeClientId();
			QString activeClientIp = c.tcpTuningClient->activeClientIp();

			if (activeClientId.isEmpty() == false && activeClientIp.isEmpty() == false)
			{
				str += tr("active client is %1, %2").arg(activeClientId).arg(activeClientIp);

				if (c.tcpTuningClient->clientIsActive() == true)
				{
					str += tr(" (current)");
				}
			}
			else
			{
				str += tr("active");
			}

			str += "\n";
		}

		str = str.trimmed();

		return str;
	}

	bool TuningConnectionPrivate::takeClientControl(Hash sourceHash) const
	{
		bool result = true;

		for (const Connection& c : m_conns)
		{
			if (c.tcpTuningClient->isConnected() == true &&
				c.tcpTuningClient->singleLmControlMode() == true &&
				c.tcpTuningClient->clientIsActive() == false)
			{
				if (c.tcpTuningClient->hasTuningSource(sourceHash) == true)
				{
					result &= c.tcpTuningClient->activateTuningSourceControl(sourceHash, true, true);
				}
			}
		}

		return result;
	}

	bool TuningConnectionPrivate::writeTuningSignals(const std::vector<TuningWriteCommand>& writeCommands)
	{
		// Write values on all clients
		//
		for (const Connection& c : m_conns)
		{
			if (c.tcpTuningClient->isConnected() == false)
			{
				continue;
			}

			std::vector<TuningWriteCommand> commands;
			commands.reserve(writeCommands.size());

			for (const TuningWriteCommand& command : writeCommands)
			{
				if (c.tcpTuningClient->hasTuningSignal(command.appSignalHash) == false)
				{
					continue;
				}

				// Take state from client, NOT (!) from tuningSignalManager, to skip writing non-valid signals in multi-channel case.
				//
				bool found = false;
				AppSignalParam param = m_tuningSignalManager.signalParam(command.appSignalHash, &found);
				if (found == false)
				{
					Q_ASSERT(false);
					return false;
				}

				TuningSignalState state = m_tuningSignalManager.state(command.appSignalHash, c.tcpTuningClient->tuningServiceHash(), &found);
				if (found == false)
				{
					Q_ASSERT(false);
					return false;
				}

				if (state.limitsUnbalance(param) == true)
				{
					m_logFile.writeAlert(tr("writeTuningSignal(), There is limits mismatch in signal '%1'. Operation is disabled.").arg(param.customSignalId()));
					continue;
				}

				if (found == true &&
					state.valid() == true &&
					state.controlIsEnabled() == true &&
					state.writingIsEnabled() == true)
				{
					m_tuningLog->write(param, state.value(), command.value);

					commands.push_back({command.appSignalHash, command.value});
				}
			}

			if (commands.empty() == false)
			{
				c.tcpTuningClient->writeTuningSignal(commands);
			}
		}

		return true;
	}

	bool TuningConnectionPrivate::writeTuningSignal(const QString& appSignalId, const TuningValue& tuningValue)
	{
		std::vector<TuningWriteCommand> commands;
		commands.push_back({appSignalId, tuningValue});
		return writeTuningSignals(commands);
	}

	bool TuningConnectionPrivate::writeTuningSignal(const QString& appSignalId, QVariant value)
	{
		bool ok = false;
		AppSignalParam appSignal = m_tuningSignalManager.signalParam(appSignalId, &ok);
		if (ok == false)
		{
			return false;
		}

		// Adjust value type to match signal type
		//
		auto valueType = value.metaType().id();

		if (valueType != QMetaType::Bool &&
			valueType != QMetaType::Int &&
			valueType != QMetaType::Double)
		{
			m_logFile.writeError(tr("writeTuningSignal(%1, %2) - Unsupported value type (%3), type must be bool, integer or double.")
								 .arg(appSignalId)
								 .arg(value.toString())
								 .arg(value.metaType().name()));
			return false;
		}

		TuningValueType tuningType = appSignal.tuningType();

		switch (tuningType)
		{
		case TuningValueType::Discrete:
			{
				if (valueType == QMetaType::Bool)
				{
					break;
				}
				if (valueType == QMetaType::Int)
				{
					value = value.toInt() == 0 ? false : true;
					break;
				}
				if (valueType == QMetaType::Double)
				{
					value = value.toDouble() == 0 ? false : true;
					break;
				}
				Q_ASSERT(false);
			}
			break;
		case TuningValueType::SignedInt32:
			{
				if (valueType == QMetaType::Bool)
				{
					m_logFile.writeWarning(tr("writeTuningSignal(%1, %2) - type bool is implicitly converted to SignedInt32.")
										   .arg(appSignalId)
										   .arg(value.toString()));

					value = value.toBool() == false ? static_cast<int>(0) : static_cast<int>(1);
					break;
				}

				if (valueType == QMetaType::Int)
				{
					break;
				}

				if (valueType == QMetaType::Double)
				{
					if (double valueDouble = value.toDouble();
						valueDouble < std::numeric_limits<qint32>::min() || valueDouble > std::numeric_limits<qint32>::max())
					{
						m_logFile.writeError(tr("writeTuningSignal(%1, %2) - value is out of range of type SignedInt32.")
											 .arg(appSignalId)
											 .arg(value.toString()));
						return false;
					}

					value = value.toInt();
					break;
				}
			}
			break;
		case TuningValueType::Float:
			{
				if (valueType == QMetaType::Bool)
				{
					m_logFile.writeWarning(tr("writeTuningSignal(%1, %2) - type bool is implicitly converted to Float32.")
										   .arg(appSignalId)
										   .arg(value.toString()));

					value = value.toBool() == false ? static_cast<float>(0) : static_cast<float>(1);
					break;
				}

				if (valueType == QMetaType::Int)
				{
					value = value.toFloat();
					break;
				}

				if (valueType == QMetaType::Double)
				{
					if (double valueDouble = value.toDouble();
						valueDouble < static_cast<double>(std::numeric_limits<float>::lowest()) ||
						valueDouble > static_cast<double>(std::numeric_limits<float>::max()))
					{
						m_logFile.writeError(tr("writeTuningSignal(%1, %2) - value is out of range of type Float32.")
											 .arg(appSignalId)
											 .arg(value.toString()));
						return false;
					}

					value = value.toFloat();
					break;
				}
			}
			break;
		case TuningValueType::SignedInt64:
			{
				return false;
			}
		case TuningValueType::Double:
			{
				return false;
			}
		}

		TuningValue tuningValue{value};

		// Check range for analog signal
		//
		if (appSignal.tuningType() != TuningValueType::Discrete)
		{
			if (tuningValue < appSignal.tuningLowBound() || tuningValue > appSignal.tuningHighBound())
			{
				m_logFile.writeError(tr("writeTuningSignal(%1, %2) - value is out tuning of range [%3, %4].")
									 .arg(appSignalId)
									 .arg(value.toString())
									 .arg(appSignal.tuningLowBound().toString())
									 .arg(appSignal.tuningHighBound().toString()));
				return false;
			}
		}

		return writeTuningSignal(appSignalId, tuningValue);
	}

	bool TuningConnectionPrivate::signalStatesLoaded() const
	{
		return std::all_of(m_conns.begin(), m_conns.end(), [](const Connection& c) { return c.signalStatesLoaded(); });
	}

	void TuningConnectionPrivate::applyTuningSignals(const std::vector<Hash>& signalHashes)
	{
		for (const Connection& c : m_conns)
		{
			if (c.tcpTuningClient->hasTuningSignals(signalHashes) == true)
			{
				c.tcpTuningClient->applyTuningSignals();
			}
		}
	}

	void TuningConnectionPrivate::applyTuningSignals()
	{
		for (const Connection& c : m_conns)
		{
			c.tcpTuningClient->applyTuningSignals();
		}

	}

}	// namespace
