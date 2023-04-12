#ifndef CLIENT_LIB_DOMAIN
#error Don't include this file in the project! Link ClientLib instead.
#endif

#include "../AppSignalLib/TuningSignalManager.h"
#include "../UtilsLib/SimpleThread.h"
#include "TuningConnection.h"

namespace ClientLib
{

TuningConnection::Connection::Connection(const SoftwareInfo& softwareInfo,
										 const SoftwareEndpoint::TuningService& tuns,
										 bool autoApply,
										 TuningClientSettings::LmStatusFlagMode lmStatusFlagMode,
										 ITuningSignalUpdater& signalUpdater,
										 ILogFile* logFile,
										 ITuningLog* tuningLog)
	{
		tcpTuningClient = new TuningTcpClient{softwareInfo, tuns, signalUpdater, logFile, tuningLog};
		tcpTuningClient->setServers(tuns.clientRequestAddress, tuns.clientRequestAddress, true);
		tcpTuningClient->setAutoApply(autoApply);
		tcpTuningClient->setLmStatusFlagMode(lmStatusFlagMode);

		tcpClientThread = new ::SimpleThread{tcpTuningClient};
		tcpClientThread->start();
		return;
	}

	TuningConnection::Connection::~Connection()
	{
		stopAndDestroy();
		return;
	}

	void TuningConnection::Connection::stopAndDestroy()
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

	HostAddressPort TuningConnection::Connection::address() const
	{
		Q_ASSERT(tcpTuningClient);
		return tcpTuningClient->serverAddressPort1();
	}

	TuningConnection::TuningConnection(ITuningSignalManager& tuningSignalManager,
									   ITuningSignalUpdater& tuningSignalUpdater,
									   ILogFile* logFile,
									   ITuningLog* tuningLog) :
		m_tuningSignalManager{tuningSignalManager},
		m_tuningSignalUpdater{tuningSignalUpdater},
		m_logFile{logFile, "TuningConnection"},
		m_tuningLog(tuningLog)
	{
		return;
	}

	void TuningConnection::updateConnections(const SoftwareInfo& softwareInfo,
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

			m_conns.emplace_back(softwareInfo, tuns, autoApply, lmStatusFlagMode, m_tuningSignalUpdater, m_logFile.logFile(), m_tuningLog);
		}

		return;
	}

	std::vector<Tcp::ConnectionState> TuningConnection::tcpTuningConnStates() const
	{
		std::vector<Tcp::ConnectionState> states;
		states.reserve(m_conns.size());

		for (const Connection& c : m_conns)
		{
			states.emplace_back(c.tcpTuningClient->getConnectionState());
		}

		return states;
	}

	std::vector<TuningSource> TuningConnection::tuningSourcesInfo() const
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

	std::vector<TuningSource> TuningConnection::tuningSourceInfo(Hash sourceHash) const
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

	int TuningConnection::tuningSourceStatesCount(Hash sourceHash) const
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

	int TuningConnection::activatedTuningSourceStatesCount(Hash sourceHash) const
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

	bool TuningConnection::activateTuningSource(Hash sourceHash, bool activate) const
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

	QString TuningConnection::clientControlInfo() const
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

	bool TuningConnection::takeClientControl(Hash sourceHash) const
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

	bool TuningConnection::writeTuningSignals(const std::vector<TuningWriteCommand>& writeCommands)
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

	bool TuningConnection::writeTuningSignal(QString appSignalId, TuningValue tuningValue)
	{
		std::vector<TuningWriteCommand> commands;
		commands.push_back({appSignalId, tuningValue});
		return writeTuningSignals(commands);
	}

	void TuningConnection::applyTuningSignals(const std::vector<Hash>& signalHashes)
	{
		for (const Connection& c : m_conns)
		{
			if (c.tcpTuningClient->hasTuningSignals(signalHashes) == true)
			{
				c.tcpTuningClient->applyTuningSignals();
			}
		}
	}

	void TuningConnection::applyTuningSignals()
	{
		for (const Connection& c : m_conns)
		{
			c.tcpTuningClient->applyTuningSignals();
		}

	}

}	// namespace
