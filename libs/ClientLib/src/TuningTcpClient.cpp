#ifndef CLIENT_LIB_DOMAIN
	#error Do not include this file in the project! Link ClientLib instead.
#endif

#include "TuningTcpClient.h"

namespace
{
	thread_local ::Network::GetTuningSourcesStates tl_getTuningSourcesStates;
	thread_local ::Network::GetTuningSourcesStatesReply tl_tuningSourcesStatesReply;

	thread_local ::Network::GetTuningSourcesInfo tl_getTuningSourcesInfo;
	thread_local ::Network::GetTuningSourcesInfoReply tl_tuningSourcesInfoReply;

	thread_local ::Network::ChangeConrolledTuningSourceRequest tl_activateTuningSource;
	thread_local ::Network::ChangeConrolledTuningSourceReply tl_activateTuningSourceReply;

	thread_local ::Network::TuningSignalsRead tl_readTuningSignals;
	thread_local ::Network::TuningSignalsReadReply tl_readTuningSignalsReply;

	thread_local ::Network::GetTuningSignalsStateChangesRequest tl_readChangedTuningSignals;
	thread_local ::Network::GetTuningSignalsStateChangesReply tl_readChangedTuningSignalsReply;

	thread_local ::Network::TuningSignalsWrite tl_writeTuningSignals;
	thread_local ::Network::TuningSignalsWriteReply tl_writeTuningSignalsReply;

	thread_local ::Network::TuningSignalsApply tl_applyTuningSignals;
	thread_local ::Network::TuningSignalsApplyReply tl_applyTuningSignalsReply;
} // namespace

namespace ClientLib
{
	//
	// TuningTcpClient
	//
	TuningTcpClient::TuningTcpClient(const SoftwareInfo& softwareInfo,
									 const SoftwareEndpoint::TuningService& tunsInfo,
									 ITuningSignalUpdater& signalUpdater,
									 IRecentAppSignals& recentTuningSignals,
									 ITuningAuthorization& tuningAuthorization,
									 ILogFile* log,
									 ITuningLog* tuningLog) :
		Tcp::Client(softwareInfo, tunsInfo.clientRequestAddress, "TuningTcpClient", tunsInfo.equipmentId),
		TcpClientStatistics(this),
		m_logFile(log, "TuningTcpClient"),
		m_tuningLog(tuningLog),
		m_serverSettings(tunsInfo),
		m_tuningServiceHash(::calcHash(tunsInfo.equipmentId)),
		m_signalUpdater(signalUpdater),
		m_recentTuningSignals(recentTuningSignals),
		m_tuningAuthorization(tuningAuthorization)
	{
		setObjectName("TuningTcpClient " + tunsInfo.shortenId);

		qRegisterMetaType<TuningClientSettings::LmStatusFlagMode>("LmStatusFlagMode");

		connect(this,
				&Tcp::Client::signal_wrongServerID,
				[this](const QString& errorMessage)
				{
					m_logFile.writeError(errorMessage);
				});

		return;
	}

	TuningTcpClient::~TuningTcpClient() {}

	const SoftwareEndpoint::TuningService& TuningTcpClient::server() const
	{
		return m_serverSettings;
	}

	Hash TuningTcpClient::tuningServiceHash() const
	{
		return m_tuningServiceHash;
	}

	std::vector<Hash> TuningTcpClient::tuningSourcesHashes() const
	{
		QReadLocker l(&m_tuningSourcesLock);

		std::vector<Hash> result;
		result.reserve(m_tuningSources.size());

		for (const auto& p : m_tuningSources)
		{
			result.push_back(p.first);
		}

		return result;
	}

	std::vector<TuningSource> TuningTcpClient::tuningSourcesInfo() const
	{
		QReadLocker l(&m_tuningSourcesLock);

		std::vector<TuningSource> result;
		result.reserve(m_tuningSources.size());

		for (const auto& ds : m_tuningSources)
		{
			result.push_back(ds.second);
		}

		return result;
	}

	bool TuningTcpClient::tuningSourceInfo(Hash equipmentHash, TuningSource* result) const
	{
		if (result == nullptr)
		{
			assert(result);
			return false;
		}

		QReadLocker l(&m_tuningSourcesLock);

		auto it = m_tuningSources.find(equipmentHash);

		if (it == m_tuningSources.end())
		{
			return false;
		}

		*result = it->second;

		return true;
	}

	bool TuningTcpClient::hasTuningSource(Hash equipmentHash) const
	{
		QReadLocker l(&m_tuningSourcesLock);

		return m_tuningSources.contains(equipmentHash);
	}

	bool TuningTcpClient::activateTuningSourceControl(Hash equipmentHash, bool enableControl, bool forceTakeControl)
	{
		QString equipmentId;

		{
			QReadLocker l(&m_tuningSourcesLock);

			auto it = m_tuningSources.find(equipmentHash);
			if (it == m_tuningSources.end())
			{
				assert(false);
				return false;
			}

			equipmentId = it->second.equipmentId();
		}

		if (forceTakeControl == true && clientIsActive() == true)
		{
			m_logFile.writeError(QString("activateTuningSourceControl([%1], enableControl=%2, forceTakeControl=%3), Do not allow "
										 "forceTakeControl command if current client is already active")
									 .arg(equipmentId)
									 .arg(enableControl)
									 .arg(forceTakeControl));

			assert(false);
			return false;
		}

		m_logFile.writeMessage(tr("Tuning Source [%1] is %2.").arg(equipmentId).arg(enableControl ? tr("activated") : tr("deactivated")));

		{
			std::lock_guard locker(m_writeQueueMutex);
			m_writeQueue.push(TuningWriteCommand(equipmentHash, enableControl, forceTakeControl));
			m_writeQueueCondition.notify_one();
		}

		return true;
	}

	bool TuningTcpClient::hasTuningSignals(const std::vector<Hash>& appSignalHashes) const
	{
		QReadLocker l(&m_signalHashesLock);

		for (const Hash hash : appSignalHashes)
		{
			if (m_signalHashesSet.find(hash) != m_signalHashesSet.end())
			{
				return true;
			}
		}

		return false;
	}

	bool TuningTcpClient::hasTuningSignal(Hash appSignalHash) const
	{
		QReadLocker l(&m_signalHashesLock);
		return m_signalHashesSet.find(appSignalHash) != m_signalHashesSet.end();
	}

	void TuningTcpClient::writeTuningSignal(const std::vector<TuningWriteCommand>& data)
	{
		if (isConnected() == false)
		{
			return;
		}

		{
			std::lock_guard locker(m_writeQueueMutex);
			for (const TuningWriteCommand& command : data)
			{
				// Push command to the queue
				//
				m_writeQueue.push(command);
			}
			m_writeQueueCondition.notify_one();
		}

		return;
	}

	// Apply states
	//
	void TuningTcpClient::applyTuningSignals()
	{
		if (isConnected() == false)
		{
			return;
		}

		{
			std::lock_guard locker(m_writeQueueMutex);
			m_writeQueue.emplace(true);
			m_writeQueueCondition.notify_one();
		}

		m_tuningLog->write(tr("'Apply' command is sent."));

		return;
	}

	bool TuningTcpClient::signalStatesLoaded() const
	{
		return m_signalStatesLoaded.load();
	}

	void TuningTcpClient::onClientThreadStarted()
	{
		return;
	}

	void TuningTcpClient::onClientThreadFinished() {}

	void TuningTcpClient::onConnection()
	{
		m_logFile.writeMessage(tr("onConnection(), connection established."));

		assert(isClearToSendRequest() == true);

		{
			std::lock_guard locker(m_writeQueueMutex);

			// Clearing m_writeQueue - std::queue has no clear method
			//
			decltype(m_writeQueue) clearQueue;
			std::swap(m_writeQueue, clearQueue);
		}

		{
			QWriteLocker l(&m_tuningSourcesLock);
			m_tuningSources.clear();
		}

		m_lastReadRequestType = ReadRequestType::Generic;

		requestTuningSourcesInfo();

		return;
	}

	void TuningTcpClient::onDisconnection()
	{
		m_logFile.writeMessage(tr("onDisconnection(), connection closed."));

		m_signalUpdater.invalidateSignalStates(m_tuningServiceHash);

		{
			QWriteLocker l(&m_tuningSourcesLock);

			for (auto& it : m_tuningSources)
			{
				TuningSource& ts = it.second;

				ts.invalidate();
			}
		}

		return;
	}

	void TuningTcpClient::onReplyTimeout()
	{
		if (isConnected() == true)
		{
			m_logFile.writeWarning(tr("onReplyTimeout(), reply timeout."));
			closeConnection();
		}

		return;
	}

	void TuningTcpClient::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
	{
		if (replyData == nullptr)
		{
			assert(replyData);
			return;
		}

		QByteArray data = QByteArray::fromRawData(replyData, replyDataSize);

		switch (requestID)
		{
		case TDS_GET_TUNING_SOURCES_INFO:
			processTuningSourcesInfo(data);
			break;

		case TDS_GET_TUNING_SOURCES_STATES:
			Q_ASSERT(m_lastReadRequestType == ReadRequestType::SourceState);
			processTuningSourcesState(data);
			break;

		case TDS_GET_SIGNALS_STATE_CHANGES:
			Q_ASSERT(m_lastReadRequestType == ReadRequestType::Changed);
			processReadChangedTuningSignals(data);
			break;

		case TDS_TUNING_SIGNALS_READ:
			if (m_lastReadRequestType == ReadRequestType::Recent)
			{
				processReadRecentTuningSignals(data);
			}
			else
			{
				Q_ASSERT(m_lastReadRequestType == ReadRequestType::Generic);
				processReadTuningSignals(data);
			}
			break;

		case TDS_TUNING_SIGNALS_WRITE:
			processWriteTuningSignals(data);
			break;

		case TDS_TUNING_SIGNALS_APPLY:
			processApplyTuningSignals(data);
			break;

		case TDS_CHANGE_CONTROLLED_TUNING_SOURCE:
			processActivateTuningSource(data);
			break;

		default:
			assert(false);
			m_logFile.writeError(tr("processReply(): Wrong requestId, %1").arg(requestID));

			requestTuningSourcesInfo();
		}

		return;
	}

	void TuningTcpClient::continueRequestLoop()
	{
		// Choose which read request to send based on previous request
		//
		switch (m_lastReadRequestType)
		{
		case ReadRequestType::Generic:
			{
				if (sendWriteRequest(m_requestInterval) == false)
				{
					m_lastReadRequestType = ReadRequestType::SourceState;
					requestTuningSourcesState();
				}
				break;
			}
		case ReadRequestType::SourceState:
			if (sendWriteRequest(0) == false)
			{
				m_lastReadRequestType = ReadRequestType::Changed;
				requestReadChangedTuningSignals();
			}
			break;
		case ReadRequestType::Changed:
			{
				if (sendWriteRequest(0) == false)
				{
					m_lastReadRequestType = ReadRequestType::Recent;
					requestReadRecentTuningSignals();
				}
				break;
			}
		case ReadRequestType::Recent:
			{
				if (sendWriteRequest(0) == false)
				{
					m_lastReadRequestType = ReadRequestType::Generic;
					requestReadTuningSignals();
				}
				break;
			}
		default:
			Q_ASSERT(false);
		}
		return;
	}

	bool TuningTcpClient::sendWriteRequest(int waitTimeMs)
	{
		std::unique_lock locker(m_writeQueueMutex);

		if (waitTimeMs > 0)
		{
			// Wait interval of time before requesting pack of signal states.
			// If write queue is not empty - do not wait, write them immediately
			//
			if (m_writeQueueCondition.wait_for(locker,
											   std::chrono::milliseconds{waitTimeMs},
											   [this]()
											   {
												   return m_writeQueue.empty() == false;
											   }) == false)
			{
				return false;
			}
		}
		else
		{
			// If write queue is empty - do not wait, return immediately
			//
			if (m_writeQueue.empty() == true)
			{
				return false;
			}
		}

		// If there is a queued data to write something, write it or apply.
		//
		const TuningWriteCommand cmd = m_writeQueue.front();

		switch (cmd.type)
		{
		case TuningWriteCommand::TuningWriteCommandType::Apply:
			{
				// Apply request
				//
				m_writeQueue.pop();

				locker.unlock();

				requestApplyTuningSignals();

				break;
			}
		case TuningWriteCommand::TuningWriteCommandType::ActivateLm:
			{
				// Activate LM request
				//
				m_writeQueue.pop();

				locker.unlock();

				requestActivateTuningSource(cmd.equipmentHash, cmd.enableControl, cmd.forceTakeControl);

				break;
			}

		case TuningWriteCommand::TuningWriteCommandType::WriteValue:
			{
				// Write request
				//
				decltype(m_writeQueue) writeQueue;

				for (int i = 0; i < MaxStateWriteCount && m_writeQueue.empty() == false; i++)
				{
					auto& frontCmd = m_writeQueue.front();

					if (frontCmd.type != TuningWriteCommand::TuningWriteCommandType::WriteValue)
					{
						// Queue potentially can have commands of different type, so stop if we meet a command of non-wrirteValue type
						//
						break;
					}

					writeQueue.push(frontCmd);
					m_writeQueue.pop();
				}

				locker.unlock();

				requestWriteTuningSignals(std::move(writeQueue));

				break;
			}

		default:
			// Unknown command - just pop it and skip
			//
			assert(false);
			m_writeQueue.pop();
			locker.unlock();
			return false;
		}

		return true;
	}

	void TuningTcpClient::requestTuningSourcesInfo()
	{
		if (isConnected() == false)
		{
			m_logFile.writeMessage(tr("requestTuningSourcesInfo(), isConnected() == false."));
			return;
		}

		if (isClearToSendRequest() == false)
		{
			m_logFile.writeMessage(tr("requestTuningSourcesInfo(), isClearToSendRequest() == false, reconnecting."));
			closeConnection();
			return;
		}

		tl_getTuningSourcesInfo.Clear();

		sendRequest(TDS_GET_TUNING_SOURCES_INFO, tl_getTuningSourcesInfo);

		return;
	}

	void TuningTcpClient::processTuningSourcesInfo(const QByteArray& data)
	{
		bool ok = tl_tuningSourcesInfoReply.ParseFromArray(data.constData(), static_cast<int>(data.size()));

		if (ok == false)
		{
			assert(ok);
			continueRequestLoop();
			return;
		}

		if (tl_tuningSourcesInfoReply.error() != static_cast<int>(E::NetworkError::Success))
		{
			m_logFile.writeError(tr("processTuningSourcesInfo(), in tl_tuningSourcesInfoReply error received: %1")
									 .arg(E::valueToString(static_cast<E::NetworkError>(tl_tuningSourcesInfoReply.error()))));

			continueRequestLoop();
			return;
		}

		{
			QWriteLocker l(&m_tuningSourcesLock);

			m_tuningSources.clear();

			for (int i = 0; i < tl_tuningSourcesInfoReply.tuningsourceinfo_size(); i++)
			{
				const ::Network::DataSourceInfo& dsi = tl_tuningSourcesInfoReply.tuningsourceinfo(i);

				TuningSource ts(dsi);

				Hash hash = ::calcHash(QString::fromStdString(ts.info().moduleequipmentid()));

				assert(m_tuningSources.count(hash) == 0);

				m_tuningSources[hash] = ts;
			}
		}

		// Initialize list of signal hashes processed by this client
		//
		{
			std::vector<Hash> equipmentHashes = tuningSourcesHashes();

			QWriteLocker l(&m_signalHashesLock);

			m_signalHashes = m_signalUpdater.signalHashes(equipmentHashes);

			m_signalHashesSet.reserve(m_signalHashes.size());
			for (const Hash& hash : m_signalHashes)
			{
				m_signalHashesSet.insert(hash);
			}

			m_signalStatesSet.clear();
		}

		emit tuningSourcesInfoArrived();

		continueRequestLoop();

		return;
	}

	void TuningTcpClient::requestTuningSourcesState()
	{
		if (isConnected() == false)
		{
			m_logFile.writeMessage(tr("requestTuningSourcesState(), isConnected() == false."));
			return;
		}

		if (isClearToSendRequest() == false)
		{
			m_logFile.writeMessage(tr("requestTuningSourcesState(), isClearToSendRequest() == false, reconnecting."));
			closeConnection();
			return;
		}

		tl_getTuningSourcesStates.Clear();

		sendRequest(TDS_GET_TUNING_SOURCES_STATES, tl_getTuningSourcesStates);

		return;
	}

	void TuningTcpClient::processTuningSourcesState(const QByteArray& data)
	{
		bool ok = tl_tuningSourcesStatesReply.ParseFromArray(data.constData(), static_cast<int>(data.size()));

		if (ok == false)
		{
			assert(ok);
			continueRequestLoop();
			return;
		}

		if (tl_tuningSourcesStatesReply.error() != static_cast<int>(E::NetworkError::Success))
		{
			m_logFile.writeError(tr("processTuningSourcesState(), error received: %1")
									 .arg(E::valueToString(static_cast<E::NetworkError>(tl_tuningSourcesStatesReply.error()))));

			continueRequestLoop();
			return;
		}

		{
			QWriteLocker l(&m_tuningSourcesLock);

			for (int i = 0; i < tl_tuningSourcesStatesReply.tuningsourcesstate_size(); i++)
			{
				const ::Network::TuningSourceState& tss = tl_tuningSourcesStatesReply.tuningsourcesstate(i);

				quint64 id = tss.sourceid();

				// bool found = false;

				for (auto& it : m_tuningSources)
				{
					TuningSource& ts = it.second;

					if (ts.id() == id)
					{
						// --------------------------------------------------------------------

						// Write SOR change to tuning log

						for (int s = 0; s < ts.statesCount(); s++)
						{
							const ::Network::TuningSourceState& state = ts.state(s);

							if (state.isreply() == true && m_lmStatusFlagMode != TuningClientSettings::LmStatusFlagMode::None)
							{
								TuningValue oldSor;
								oldSor.setType(TuningValueType::Discrete);
								oldSor.setDiscreteValue(state.setsor() ? 1 : 0);

								TuningValue newSor;
								newSor.setType(TuningValueType::Discrete);
								newSor.setDiscreteValue(tss.setsor() ? 1 : 0);

								if (oldSor != newSor)
								{
									AppSignalParam param;
									param.setEquipmentId(QString::fromStdString(ts.info().moduleequipmentid()));

									switch (m_lmStatusFlagMode)
									{
									case TuningClientSettings::LmStatusFlagMode::AccessKey:
										{
											param.setCustomSignalId(tr("Access Key"));
											break;
										}
									case TuningClientSettings::LmStatusFlagMode::SOR:
										{
											param.setCustomSignalId(tr("SOR is set"));
											break;
										}
									default:
										Q_ASSERT(false);
										param.setCustomSignalId(tr("LM Status Flag"));
										break;
									}

									param.setPrecision(0);

									m_tuningLog->write(param, oldSor, newSor);
								}
							}
						} // Write SOR

						// Set new source state

						ts.setNewState(tss);

						break;

					} // ts.id() == id
				}
			}
		}

		{
			QWriteLocker l(&m_activeClientMutex);
			m_activeClientId = tl_tuningSourcesStatesReply.activeclientid().c_str();
			m_activeClientIp = tl_tuningSourcesStatesReply.activeclientip().c_str();
			m_singleLmControlMode = tl_tuningSourcesStatesReply.singlelmcontrolmode();

			QString localAddress = localAddressPort().addressStr();
			m_currentClientIsActive = (m_singleLmControlMode == false) ||
									  (m_activeClientId == m_localSoftwareInfo.equipmentID() && m_activeClientIp == localAddress);
		}

		//

		continueRequestLoop();

		return;
	}

	void TuningTcpClient::requestActivateTuningSource(Hash equipmentHash, bool enableControl, bool forceTakeControl)
	{
		if (isConnected() == false)
		{
			m_logFile.writeMessage(tr("requestActivateTuningSource(), isConnected() == false."));
			return;
		}

		if (isClearToSendRequest() == false)
		{
			m_logFile.writeMessage(tr("requestActivateTuningSource(), isClearToSendRequest() == false, reconnecting."));
			closeConnection();
			return;
		}

		// Create the request
		//
		QString equipmentId;

		{
			QReadLocker l(&m_tuningSourcesLock);

			auto it = m_tuningSources.find(equipmentHash);
			if (it == m_tuningSources.end())
			{
				assert(false);
				return;
			}

			equipmentId = it->second.equipmentId();
		}

		tl_activateTuningSource.set_tuningsourceequipmentid(equipmentId.toUtf8());
		tl_activateTuningSource.set_activatecontrol(enableControl);
		tl_activateTuningSource.set_takecontrol(forceTakeControl);

		sendRequest(TDS_CHANGE_CONTROLLED_TUNING_SOURCE, tl_activateTuningSource);

		return;
	}

	void TuningTcpClient::processActivateTuningSource(const QByteArray& data)
	{
		bool ok = tl_activateTuningSourceReply.ParseFromArray(data.constData(), static_cast<int>(data.size()));

		if (ok == false)
		{
			assert(ok);
			continueRequestLoop();
			return;
		}

		if (tl_activateTuningSourceReply.error() != static_cast<int>(E::NetworkError::Success))
		{
			m_logFile.writeError(tr("processActivateTuningSource(), error received: %1")
									 .arg(E::valueToString(static_cast<E::NetworkError>(tl_activateTuningSourceReply.error()))));

			return;
		}

		continueRequestLoop();

		return;
	}

	void TuningTcpClient::requestReadRecentTuningSignals()
	{
		if (isConnected() == false)
		{
			m_logFile.writeMessage(tr("requestReadRecentTuningSignals(), isConnected() == false."));
			return;
		}

		if (isClearToSendRequest() == false)
		{
			m_logFile.writeMessage(tr("requestReadRecentTuningSignals(), isClearToSendRequest() == false, reconnecting."));
			closeConnection();
			return;
		}

		std::vector<Hash> recentSignals = m_recentTuningSignals.recentlyUsedAppSignals(connectedSoftwareInfo().equipmentID());

		int recentCount = static_cast<int>(recentSignals.size());
		if (recentCount > MaxStateRequestCount)
		{
			Q_ASSERT(recentCount <= MaxStateRequestCount);
			recentCount = MaxStateRequestCount;
		}

		// Create the request
		//
		tl_readTuningSignals.Clear();
		tl_readTuningSignals.mutable_signalhash()->Reserve(recentCount);

		for (int i = 0; i < recentCount; i++)
		{
			tl_readTuningSignals.mutable_signalhash()->Add(recentSignals[i]);
		}

		sendRequest(TDS_TUNING_SIGNALS_READ, tl_readTuningSignals);

		return;
	}

	void TuningTcpClient::processReadRecentTuningSignals(const QByteArray& data)
	{
		bool ok = processTuningSignalsReadReply(data);
		if (ok == false)
		{
			return;
		}

		// Continue the current loop
		//
		continueRequestLoop();

		return;
	}

	void TuningTcpClient::requestReadTuningSignals()
	{
		if (isConnected() == false)
		{
			m_logFile.writeMessage(tr("requestReadTuningSignals(), isConnected() == false."));
			return;
		}

		if (isClearToSendRequest() == false)
		{
			m_logFile.writeMessage(tr("isClearToSendRequest(), isClearToSendRequest() == false, reconnecting."));
			closeConnection();
			return;
		}

		QReadLocker l(&m_signalHashesLock);

		int totalSignalCount = static_cast<int>(m_signalHashes.size());

		// If no signals in the database, start the new request loop
		//
		if (totalSignalCount == 0)
		{
			l.unlock();

			continueRequestLoop();

			return;
		}

		// Determine the amount of signals needed to be requested
		//
		m_readTuningSignalCount = MaxStateRequestCount;

		if (m_readTuningSignalIndex >= totalSignalCount)
		{
			// Possibly, the database was updated and last requested index is larger than current database size
			//
			m_readTuningSignalIndex = 0;
		}

		if (m_readTuningSignalIndex + m_readTuningSignalCount >= totalSignalCount)
		{
			m_readTuningSignalCount = totalSignalCount - m_readTuningSignalIndex;
		}

		// Create the request
		//
		tl_readTuningSignals.Clear();
		tl_readTuningSignals.mutable_signalhash()->Reserve(m_readTuningSignalCount);

		for (int i = 0; i < m_readTuningSignalCount; i++)
		{
			Hash hash = m_signalHashes[m_readTuningSignalIndex + i];

			tl_readTuningSignals.mutable_signalhash()->Add(hash);
		}

		l.unlock();

		sendRequest(TDS_TUNING_SIGNALS_READ, tl_readTuningSignals);

		return;
	}

	void TuningTcpClient::processReadTuningSignals(const QByteArray& data)
	{
		bool ok = processTuningSignalsReadReply(data);
		if (ok == false)
		{
			return;
		}

		// Increase the requested signal index, wrap the request index if needed
		//

		QReadLocker l(&m_signalHashesLock);

		int totalSignalCount = static_cast<int>(m_signalHashes.size());

		l.unlock();

		m_readTuningSignalIndex += m_readTuningSignalCount;

		if (m_readTuningSignalIndex >= totalSignalCount)
		{
			m_readTuningSignalIndex = 0;
		}

		// Start the new loop
		//
		continueRequestLoop();

		return;
	}

	void TuningTcpClient::requestReadChangedTuningSignals()
	{
		if (isConnected() == false)
		{
			m_logFile.writeMessage(tr("requestReadChangedTuningSignals(), isConnected() == false."));
			return;
		}

		if (isClearToSendRequest() == false)
		{
			m_logFile.writeMessage(tr("requestReadChangedTuningSignals(), isClearToSendRequest() == false, reconnecting."));
			closeConnection();
			return;
		}

		sendRequest(TDS_GET_SIGNALS_STATE_CHANGES);
	}

	void TuningTcpClient::processReadChangedTuningSignals(const QByteArray& data)
	{
		bool ok = tl_readChangedTuningSignalsReply.ParseFromArray(data.constData(), static_cast<int>(data.size()));

		if (ok == false)
		{
			assert(ok);
			continueRequestLoop();
			return;
		}

		if (tl_readChangedTuningSignalsReply.error() != static_cast<int>(E::NetworkError::Success))
		{
			m_logFile.writeError(tr("processReadChangedTuningSignals(), error received: %1")
									 .arg(E::valueToString(static_cast<E::NetworkError>(tl_readChangedTuningSignalsReply.error()))));

			continueRequestLoop();
			return;
		}

		int stateCount = tl_readChangedTuningSignalsReply.tuningsignalstate_size();

		if (stateCount > 0)
		{
			std::vector<TuningSignalState> arrivedStates;
			arrivedStates.reserve(stateCount);

			for (int i = 0; i < stateCount; i++)
			{
				const ::Network::TuningSignalState& stateMessage = tl_readChangedTuningSignalsReply.tuningsignalstate(i);
				if (processTuningSignalStateMessage(stateMessage, arrivedStates) == false)
				{
					continue;
				}

				if (m_signalStatesSet.contains(arrivedStates.back().hash()) == false)
				{
					m_signalStatesSet.insert(arrivedStates.back().hash()); // Mark signal as received at least once

					QReadLocker l(&m_signalHashesLock);
					if (m_signalStatesSet.size() == m_signalHashes.size())
					{
						m_signalStatesLoaded.store(true);                  // Notify that states of all signals are received
					}
				}
			}

			m_signalUpdater.setStates(arrivedStates, m_tuningServiceHash);
		}

		if (tl_readChangedTuningSignalsReply.pendingsignalsstatechanges() > MaxStateRequestCount / 2)
		{
			// Request other pending changes immediately
			//
			requestReadChangedTuningSignals();
		}
		else
		{
			// Continue the current loop
			//
			continueRequestLoop();
		}
	}

	void TuningTcpClient::requestWriteTuningSignals(std::queue<TuningWriteCommand> writeQueue)
	{
		if (isConnected() == false)
		{
			m_logFile.writeMessage(tr("requestWriteTuningSignals(), isConnected() == false."));
			return;
		}

		if (isClearToSendRequest() == false)
		{
			m_logFile.writeMessage(tr("requestWriteTuningSignals(), isClearToSendRequest() == false, reconnecting."));
			closeConnection();
			return;
		}

		// Create the request
		//
		tl_writeTuningSignals.Clear();

		tl_writeTuningSignals.set_matsuser(m_tuningAuthorization.userName().toStdString());
		tl_writeTuningSignals.set_autoapply(m_autoApply);
		tl_writeTuningSignals.mutable_commands()->Reserve(static_cast<int>(writeQueue.size()));

		while (writeQueue.empty() == false)
		{
			const TuningWriteCommand& cmd = writeQueue.front();

			::Network::TuningWriteCommand* protoCommand = tl_writeTuningSignals.mutable_commands()->Add();
			cmd.toProtoWriteCommand(protoCommand);

			writeQueue.pop();
		}


		sendRequest(TDS_TUNING_SIGNALS_WRITE, tl_writeTuningSignals);

		return;
	}

	void TuningTcpClient::processWriteTuningSignals(const QByteArray& data)
	{
		bool ok = tl_writeTuningSignalsReply.ParseFromArray(data.constData(), static_cast<int>(data.size()));

		if (ok == false)
		{
			assert(ok);
			continueRequestLoop();
			return;
		}

		if (tl_writeTuningSignalsReply.error() != static_cast<int>(E::NetworkError::Success))
		{
			m_logFile.writeError(tr("processWriteTuningSignals(), error received: %1")
									 .arg(E::valueToString(static_cast<E::NetworkError>(tl_writeTuningSignalsReply.error()))));

			continueRequestLoop();
			return;
		}

		int writeResultCount = tl_writeTuningSignalsReply.writeresult_size();

		for (int i = 0; i < writeResultCount; i++)
		{
			const ::Network::TuningSignalWriteResult& twr = tl_writeTuningSignalsReply.writeresult(i);

			if (twr.error() != static_cast<int>(E::NetworkError::Success))
			{
				m_logFile.writeError(tr("processWriteTuningSignals(), TuningSignalWriteResult error received: %1, hash = %2")
										 .arg(E::valueToString(static_cast<E::NetworkError>(twr.error())))
										 .arg(twr.signalhash()));

				continue;
			}
		}

		continueRequestLoop();

		return;
	}

	void TuningTcpClient::requestApplyTuningSignals()
	{
		if (isConnected() == false)
		{
			m_logFile.writeMessage(tr("requestApplyTuningSignals(), isConnected() == false."));
			return;
		}

		if (isClearToSendRequest() == false)
		{
			m_logFile.writeMessage(tr("requestApplyTuningSignals(), isClearToSendRequest() == false, reconnecting."));
			closeConnection();
			return;
		}

		sendRequest(TDS_TUNING_SIGNALS_APPLY, tl_applyTuningSignals);

		return;
	}

	void TuningTcpClient::processApplyTuningSignals(const QByteArray& data)
	{
		bool ok = tl_applyTuningSignalsReply.ParseFromArray(data.constData(), static_cast<int>(data.size()));

		if (ok == false)
		{
			assert(ok);
			continueRequestLoop();
			return;
		}

		if (tl_applyTuningSignalsReply.error() != static_cast<int>(E::NetworkError::Success))
		{
			m_logFile.writeError(tr("processApplyTuningSignals(), error received: %1")
									 .arg(E::valueToString(static_cast<E::NetworkError>(tl_applyTuningSignalsReply.error()))));

			continueRequestLoop();
			return;
		}

		continueRequestLoop();

		return;
	}

	bool TuningTcpClient::processTuningSignalsReadReply(const QByteArray& data)
	{
		bool ok = tl_readTuningSignalsReply.ParseFromArray(data.constData(), static_cast<int>(data.size()));

		if (ok == false)
		{
			assert(ok);
			continueRequestLoop();
			return false;
		}

		if (tl_readTuningSignalsReply.error() != static_cast<int>(E::NetworkError::Success))
		{
			m_logFile.writeError(tr("processTuningSignalsReadReply(), error received: %1")
									 .arg(E::valueToString(static_cast<E::NetworkError>(tl_readTuningSignalsReply.error()))));

			continueRequestLoop();
			return false;
		}

		int stateCount = tl_readTuningSignalsReply.tuningsignalstate_size();

		if (stateCount > 0)
		{
			std::vector<TuningSignalState> arrivedStates;
			arrivedStates.reserve(stateCount);

			for (int i = 0; i < stateCount; i++)
			{
				const ::Network::TuningSignalState& stateMessage = tl_readTuningSignalsReply.tuningsignalstate(i);

				if (processTuningSignalStateMessage(stateMessage, arrivedStates) == false)
				{
					continue;
				}

				if (m_signalStatesSet.contains(arrivedStates.back().hash()) == false)
				{
					m_signalStatesSet.insert(arrivedStates.back().hash()); // Mark signal as received at least once

					QReadLocker l(&m_signalHashesLock);
					if (m_signalStatesSet.size() == m_signalHashes.size())
					{
						m_signalStatesLoaded.store(true);                  // Notify that states of all signals are received
					}
				}
			}

			m_signalUpdater.setStates(arrivedStates, m_tuningServiceHash);
		}
		else
		{
			bool noTuningSignalsExist = false;
			{
				QReadLocker l(&m_signalHashesLock);
				noTuningSignalsExist = m_signalHashes.empty() == true;
			}
			if (noTuningSignalsExist == true)
			{
				// No signals exist at all, set flag that all signals states are received
				//
				if (m_signalStatesLoaded.load() == false)
				{
					m_signalStatesLoaded.store(true);
				}
			}
		}

		return true;
	}

	bool TuningTcpClient::processTuningSignalStateMessage(const ::Network::TuningSignalState& stateMessage,
														  std::vector<TuningSignalState>& arrivedStates)
	{
		E::NetworkError error = static_cast<E::NetworkError>(stateMessage.error());

		if (error != E::NetworkError::Success && error != E::NetworkError::LmControlIsNotActive)
		{
			m_logFile.writeError(
				tr("processTuningSignalStateMessage(), TuningSignalState error received: %1").arg(E::valueToString(error)));

			return false;
		}

		TuningSignalState arrivedState(stateMessage);

		// When updating states, we have to set some properties locally
		//
		arrivedState.m_flags.controlIsEnabled = (error == E::NetworkError::LmControlIsNotActive) ? false : true;

		if (lmStatusFlagMode() == TuningClientSettings::LmStatusFlagMode::AccessKey) 
		{
			arrivedState.m_flags.writingIsEnabled = arrivedState.valid() && arrivedState.writingIsEnabled();
		}
		else
		{
			// Set Access key flag to Validity flag & Control flag if Access Key function is inactive
			//
			arrivedState.m_flags.writingIsEnabled = arrivedState.valid() && arrivedState.controlIsEnabled();
		}

		arrivedStates.push_back(arrivedState);

		return true;
	}

	bool TuningTcpClient::autoApply() const
	{
		return m_autoApply;
	}

	void TuningTcpClient::setAutoApply(bool value)
	{
		m_autoApply = value;
	}

	bool TuningTcpClient::singleLmControlMode() const
	{
		QReadLocker l(&m_activeClientMutex);
		return m_singleLmControlMode;
	}

	bool TuningTcpClient::clientIsActive() const
	{
		QReadLocker l(&m_activeClientMutex);
		if (m_singleLmControlMode == false)
		{
			return true;
		}
		return m_currentClientIsActive;
	}

	QString TuningTcpClient::activeClientId() const
	{
		QReadLocker l(&m_activeClientMutex);
		return m_activeClientId;
	}

	QString TuningTcpClient::activeClientIp() const
	{
		QReadLocker l(&m_activeClientMutex);
		return m_activeClientIp;
	}

	Hash TuningTcpClient::activeTuningSource() const
	{
		if (singleLmControlMode() == false)
		{
			assert(false);
			return UNDEFINED_HASH;
		}

		QReadLocker l(&m_tuningSourcesLock);

		for (const auto& it : m_tuningSources)
		{
			const TuningSource& ts = it.second;

			for (int i = 0; i < ts.statesCount(); i++)
			{
				if (ts.state(i).controlisactive() == true)
				{
					return ::calcHash(ts.equipmentId());
				}
			}
		}

		return UNDEFINED_HASH;
	}

	TuningClientSettings::LmStatusFlagMode TuningTcpClient::lmStatusFlagMode() const
	{
		return m_lmStatusFlagMode;
	}

	void TuningTcpClient::setLmStatusFlagMode(const TuningClientSettings::LmStatusFlagMode& mode)
	{
		m_lmStatusFlagMode = mode;
	}
} // namespace ClientLib
