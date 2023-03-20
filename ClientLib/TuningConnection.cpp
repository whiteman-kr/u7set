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
										 TuningSignalManager& tuningSignalManager,
										 ILogFile* logFile,
										 TuningLog::TuningLog* tuningLog)
	{
		tcpTuningClient = new TuningTcpClient{softwareInfo, tuns, tuningSignalManager, logFile, tuningLog};
		tcpTuningClient->setInstanceId(softwareInfo.equipmentID());
		const HostAddressPort addrPort = HostAddressPort(tuns.clientRequestIP, tuns.clientRequestPort);
		tcpTuningClient->setServers(addrPort, addrPort, true);
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

	TuningConnection::Connection::Connection(Connection&& src) noexcept
	{
		operator=(std::move(src));
		return;
	}

	TuningConnection::Connection& TuningConnection::Connection::operator=(Connection&& src) noexcept
	{
		if (this == &src)
		{
			Q_ASSERT(this != &src);
			return *this;
		}

		tcpTuningClient = src.tcpTuningClient;
		tcpClientThread = src.tcpClientThread;

		src.tcpTuningClient = nullptr;
		src.tcpClientThread = nullptr;

		return *this;
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

	TuningConnection::TuningConnection(TuningSignalManager& tuningSignalManager,
									   ILogFile* logFile,
									   TuningLog::TuningLog* tuningLog) :
		m_tuningSignalManager{tuningSignalManager},
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
				return c.address() == HostAddressPort(tuns.clientRequestIP, tuns.clientRequestPort);
			});

			if (it != m_conns.end())
			{
				// Such connection already exists
				//
				continue;
			}

			m_conns.emplace_back(softwareInfo, tuns, autoApply, lmStatusFlagMode, m_tuningSignalManager, m_logFile.logFile(), m_tuningLog);
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
			str += tr("%1: ").arg(c.tcpTuningClient->tuningServiceId());

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

	bool TuningConnection::takeClientControl(const std::set<Hash>& sourceHashes) const
	{
		bool result = true;

		for (const Connection& c : m_conns)
		{
			if (c.tcpTuningClient->isConnected() == true &&
					c.tcpTuningClient->singleLmControlMode() == true &&
					c.tcpTuningClient->clientIsActive() == false)
			{
				for (Hash sourceHash : sourceHashes)
				{
					if (c.tcpTuningClient->hasTuningSource(sourceHash) == true)
					{
						result &= c.tcpTuningClient->activateTuningSourceControl(sourceHash, true, true);
					}
				}
			}
		}

		return result;
	}

	std::vector<std::pair<QString, TuningSignalState>> TuningConnection::states(Hash appSignalHash) const
	{
		std::vector<std::pair<QString, TuningSignalState>> result;
		result.reserve(m_conns.size());

		bool found = false;

		for (const Connection& c : m_conns)
		{
			TuningSignalState state = c.tcpTuningClient->state(appSignalHash, &found);
			if (found == true)
			{
				result.push_back(std::make_pair(c.tcpTuningClient->tuningServiceId(), state));
			}
		}

		return result;
	}

	void TuningConnection::writeTuningSignals(const std::vector<TuningWriteCommand>& writeCommands)
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

			for (const TuningWriteCommand& command : writeCommands)
			{
				// Take state from client, NOT (!) from tuningSignalManager, to skip writing non-valid signals in multi-channel case.
				//
				bool found = false;

				TuningSignalState state = c.tcpTuningClient->state(command.m_appSignalHash, &found);

				if (found == false || state.valid() == false || state.controlIsEnabled() == false || state.writingIsEnabled() == false)
				{
					continue;
				}

				commands.push_back({command.m_appSignalHash, m_tuningSignalManager.newValue(command.m_appSignalHash)});
			}

			if (commands.empty() == false)
			{
				c.tcpTuningClient->writeTuningSignal(commands);
			}
		}
	}

	bool TuningConnection::hasTuningSignal(QString appSignalId) const
	{
		for (const Connection& c : m_conns)
		{
			if (c.tcpTuningClient->hasTuningSignal(appSignalId) == true)
			{
				return true;
			}
		}
		return false;
	}

	bool TuningConnection::writeTuningSignal(QString appSignalId, TuningValue tuningValue)
	{
		bool result = true;

		for (const Connection& c : m_conns)
		{
			if (c.tcpTuningClient->hasTuningSignal(appSignalId) == true)
			{
				result &= c.tcpTuningClient->writeTuningSignal(appSignalId, tuningValue);
			}
		}

		return result;
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
