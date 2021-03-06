
#include "../lib/Tuning/TuningTcpClient.h"

//
// TuningWriteCommand
//
bool TuningWriteCommand::save(Network::TuningWriteCommand* message) const
{
	message->set_signalhash(m_hash);
	m_value.save(message->mutable_value());
	return true;
}

bool TuningWriteCommand::load(const Network::TuningWriteCommand& message)
{
	m_hash = message.signalhash();
	m_value.load(message.value());
	return true;
}

//
// TuningTcpClient
//
TuningTcpClient::TuningTcpClient(const SoftwareInfo& softwareInfo, TuningSignalManager* signalManager) :
	Tcp::Client(softwareInfo, HostAddressPort("0.0.0.0", 0), "TuningTcpClient"),
	m_instanceId(softwareInfo.equipmentID()),
	m_instanceIdHash(::calcHash(softwareInfo.equipmentID())),
	m_signals(signalManager)
{
	assert(m_signals);

	QMutexLocker l(&m_signalHashesMutex);
    m_signalHashes = m_signals->signalHashes();

	qRegisterMetaType<LmStatusFlagMode>("LmStatusFlagMode");

	return;
}

TuningTcpClient::~TuningTcpClient()
{
}

void TuningTcpClient::setSimulationMode(bool value)
{
	m_simulationMode = value;

	if (m_simulationMode == true)
	{
		m_signals->validateStates();
	}
}

std::vector<Hash> TuningTcpClient::tuningSourcesEquipmentHashes() const
{
	QMutexLocker l(&m_tuningSourcesMutex);

	std::vector<Hash> result;

	for (auto p : m_tuningSources)
	{
		result.push_back(p.first);
	}

	return result;
}

std::vector<TuningSource> TuningTcpClient::TuningTcpClient::tuningSourcesInfo() const
{
	QMutexLocker l(&m_tuningSourcesMutex);

	std::vector<TuningSource> result;
	result.reserve(m_tuningSources.size());

	for (auto ds : m_tuningSources)
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

	QMutexLocker l(&m_tuningSourcesMutex);

	auto it = m_tuningSources.find(equipmentHash);

	if (it == m_tuningSources.end())
	{
		return false;
	}

	*result = it->second;

	return true;
}



bool TuningTcpClient::activateTuningSourceControl(const QString& equipmentId, bool enableControl, bool forceTakeControl)
{
	if (m_tuningSources.find(::calcHash(equipmentId)) == m_tuningSources.end())
	{
		assert(false);
		return false;
	}

	if (forceTakeControl == true && clientIsActive() == true)
	{
		qDebug() << "Do not allow forceTakeControl command if current client is already active";
		assert(false);
		return false;
	}

	writeLogMessage(tr("Tuning Source %1 is %2.").arg(equipmentId).arg(enableControl ? tr("activated") : tr("deactivated")));

	QMutexLocker l(&m_writeQueueMutex);

	m_writeQueue.emplace(TuningWriteCommand(equipmentId, enableControl, forceTakeControl));

	return true;
}

void TuningTcpClient::writeTuningSignal(const TuningWriteCommand& data)
{
	std::vector<TuningWriteCommand> toVector;
	toVector.push_back(data);

	return writeTuningSignal(toVector);
}

void TuningTcpClient::writeTuningSignal(const std::vector<TuningWriteCommand>& data)
{
	if (m_simulationMode == true)
	{
		bool ok = false;

		for (const TuningWriteCommand& cmd : data)
		{
			AppSignalParam param =m_signals->signalParam(cmd.m_hash, &ok);
			if (ok == false)
			{
				assert(false);
				return;
			}

			TuningSignalState state = m_signals->state(cmd.m_hash, &ok);
			if (ok == true)
			{
				writeLogSignalChange(param, state.value(), cmd.m_value);

				state.m_value = cmd.m_value;
				m_signals->setState(cmd.m_hash, state);
				m_signals->setNewValue(cmd.m_hash, cmd.m_value);
			}


		}

		return;
	}

	QMutexLocker l(&m_writeQueueMutex);

	bool found = false;

	for (const TuningWriteCommand& command : data)
	{
		// Write command to log

		AppSignalParam param = m_signals->signalParam(command.m_hash, &found);
		if (found == false)
		{
			assert(false);
			return;
		}

		TuningSignalState state = m_signals->state(command.m_hash, &found);
		if (found == false)
		{
			assert(false);
			return;
		}

		if (state.limitsUnbalance(param) == true)
		{
			writeLogAlert(tr("There is limits mismatch in signal '%1'. Operation is disabled.").arg(param.customSignalId()));
			continue;
		}

		writeLogSignalChange(param, state.value(), command.m_value);

		// Push command to the queue
		//
		m_writeQueue.emplace(command);
	}

	return;
}

bool TuningTcpClient::writeTuningSignal(QString appSignalId, TuningValue value)
{
	if (m_simulationMode == false)
	{
		if (isConnected() == false)
		{
			return false;
		}
	}

	TuningWriteCommand command(appSignalId, value);
	writeTuningSignal(command);

	return true;
}

// Apply states
//
void TuningTcpClient::applyTuningSignals()
{
	QMutexLocker l(&m_writeQueueMutex);

	m_writeQueue.emplace(TuningWriteCommand(true));

	writeLogSignalChange(tr("'Apply' command is sent."));

	return;
}

void TuningTcpClient::onClientThreadStarted()
{
	connect(m_signals, &TuningSignalManager::signalsLoaded, this, &TuningTcpClient::slot_signalsUpdated);

	return;
}

void TuningTcpClient::onClientThreadFinished()
{
}

void TuningTcpClient::onConnection()
{
	writeLogMessage(tr("TuningTcpClient: connection established."));

	assert(isClearToSendRequest() == true);

	{
		QMutexLocker l(&m_writeQueueMutex);

		decltype(m_writeQueue) clearQueue;
		std::swap(m_writeQueue, clearQueue);
	}

	{
		QMutexLocker l(&m_tuningSourcesMutex);
		m_tuningSources.clear();
	}

	resetToGetTuningSources();

	return;
}

void TuningTcpClient::onDisconnection()
{
	writeLogMessage(tr("TuningTcpClient: connection closed."));

	m_signals->invalidateStates();

	{
		QMutexLocker l(&m_tuningSourcesMutex);

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
		writeLogError(tr("TuningTcpClient: reply timeout."));
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
		processTuningSourcesState(data);
		break;

	case TDS_TUNING_SIGNALS_READ:
		processReadTuningSignals(data);
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
		writeLogError(tr("TuningTcpClient::processReply: Wrong requestID."));

		resetToGetTuningSources();
	}

	return;
}

void TuningTcpClient::resetToGetTuningSources()
{
	QThread::msleep(m_requestInterval);

	requestTuningSourcesInfo();
	return;
}

void TuningTcpClient::resetToGetTuningSourcesState()
{
	QThread::msleep(m_requestInterval);

	requestTuningSourcesState();
	return;
}

void TuningTcpClient::resetToProcessTuningSignals()
{
	// If there is a queued data to write something, write it or apply.
	//
	QMutexLocker l(&m_writeQueueMutex);

	bool writeQueueEmpty = m_writeQueue.empty();

	if (writeQueueEmpty == false)
	{
		const TuningWriteCommand cmd = m_writeQueue.front();

		switch (cmd.m_type)
		{
		case TuningWriteCommand::TuningWriteCommandType::Apply:
			{
				// Apply
				//

				m_writeQueue.pop();

				l.unlock();

				requestApplyTuningSignals();

				break;
			}
		case TuningWriteCommand::TuningWriteCommandType::ActivateLm:
			{
				m_writeQueue.pop();

				// Activate LM
				//

				l.unlock();

				requestActivateTuningSource(cmd.m_equipmentId, cmd.m_enableControl, cmd.m_forceTakeControl);

				return;
			}

		case TuningWriteCommand::TuningWriteCommandType::WriteValue:
			{
				// Write request
				//

				l.unlock();

				requestWriteTuningSignals();

				return;
			}

		default:
			assert(false);

			l.unlock();

			requestReadTuningSignals();

			return;
		}
	}
	else
	{
		l.unlock();

		// Request states
		//
		requestReadTuningSignals();

		return;
	}

	return;
}

void TuningTcpClient::requestTuningSourcesInfo()
{
	if (isConnected() == false)
	{
		writeLogMessage(tr("TuningTcpClient::requestTuningSourcesInfo, isConnected() == false."));
		return;
	}

	if (isClearToSendRequest() == false)
	{
		writeLogMessage(tr("TuningTcpClient::requestTuningSourcesInfo, isClearToSendRequest() == false, reconnecting."));
		closeConnection();
		return;
	}

	m_getTuningSourcesInfo.Clear();

	sendRequest(TDS_GET_TUNING_SOURCES_INFO, m_getTuningSourcesInfo);

	return;
}

void TuningTcpClient::processTuningSourcesInfo(const QByteArray& data)
{
	bool ok = m_tuningSourcesInfoReply.ParseFromArray(data.constData(), data.size());

	if (ok == false)
	{
		assert(ok);
		resetToProcessTuningSignals();
		return;
	}

	if (m_tuningSourcesInfoReply.error() != static_cast<int>(NetworkError::Success))
	{
		writeLogError(tr("TuningTcpClient::m_tuningDataSourcesInfoReply, error received: %1")
					  .arg(networkErrorStr(static_cast<NetworkError>(m_tuningSourcesInfoReply.error()))));

		resetToProcessTuningSignals();
		return;
	}

	QMutexLocker l(&m_tuningSourcesMutex);

	m_tuningSources.clear();

	for (int i = 0; i < m_tuningSourcesInfoReply.tuningsourceinfo_size(); i++)
	{
		const ::Network::DataSourceInfo& dsi = m_tuningSourcesInfoReply.tuningsourceinfo(i);

		TuningSource ts;
		ts.info = dsi;

		Hash hash = ::calcHash(QString::fromStdString(ts.info.lmequipmentid()));

		assert(m_tuningSources.count(hash) == 0);

		m_tuningSources[hash] = ts;
	}

	m_singleLmControlMode = m_tuningSourcesInfoReply.singlelmcontrolmode();

	requestTuningSourcesState();

	emit tuningSourcesArrived();

	return;
}

void TuningTcpClient::requestTuningSourcesState()
{
	if (isConnected() == false)
	{
		writeLogMessage(tr("TuningTcpClient::requestTuningSourcesState, isConnected() == false."));
		return;
	}

	if (isClearToSendRequest() == false)
	{
		writeLogMessage(tr("TuningTcpClient::requestTuningSourcesState, isClearToSendRequest() == false, reconnecting."));
		closeConnection();
		return;
	}

	m_getTuningSourcesStates.Clear();

	sendRequest(TDS_GET_TUNING_SOURCES_STATES, m_getTuningSourcesStates);

	return;
}

void TuningTcpClient::processTuningSourcesState(const QByteArray& data)
{
	bool ok = m_tuningSourcesStatesReply.ParseFromArray(data.constData(), data.size());

	if (ok == false)
	{
		assert(ok);
		resetToProcessTuningSignals();
		return;
	}

	if (m_tuningSourcesStatesReply.error() != static_cast<int>(NetworkError::Success))
	{
		writeLogError(tr("TuningTcpClient::processTuningSourcesState, error received: %1")
					  .arg(networkErrorStr(static_cast<NetworkError>(m_tuningSourcesStatesReply.error()))));

		resetToProcessTuningSignals();
		return;
	}

	{
		QMutexLocker l(&m_tuningSourcesMutex);

		for (int i = 0; i < m_tuningSourcesStatesReply.tuningsourcesstate_size(); i++)
		{
			const ::Network::TuningSourceState& tss = m_tuningSourcesStatesReply.tuningsourcesstate(i);

			quint64 id = tss.sourceid();

			bool found = false;

			for (auto& it : m_tuningSources)
			{
				TuningSource& ts = it.second;

				if (ts.id() == id)
				{
					// Write SOR change to tuning log

					if (ts.state.isreply() == true && m_lmStatusFlagMode != LmStatusFlagMode::None)
					{
						TuningValue oldSor;
						oldSor.setType(TuningValueType::Discrete);
						oldSor.setDiscreteValue(ts.state.setsor() ? 1 : 0);

						TuningValue newSor;
						newSor.setType(TuningValueType::Discrete);
						newSor.setDiscreteValue(tss.setsor() ? 1 : 0);

						if (oldSor != newSor)
						{
							AppSignalParam param;
							param.setEquipmentId(QString::fromStdString(ts.info.lmequipmentid()));

							switch (m_lmStatusFlagMode)
							{
							case LmStatusFlagMode::AccessKey:
							{
								param.setCustomSignalId(tr("Access Key"));
								break;
							}
							case LmStatusFlagMode::SOR:
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

							writeLogSignalChange(param, oldSor, newSor);
						}
					}

					// Set new source state

                    ts.setNewState(tss);

					//

					found = true;

					break;

				}	//ts.id() == id
			}

			assert(found == true);
		}
	}

	{
		QMutexLocker l(&m_activeClientMutex);
		m_activeClientId = m_tuningSourcesStatesReply.activeclientid().c_str();
		m_activeClientIp = m_tuningSourcesStatesReply.activeclientip().c_str();

		QString localAddress = localAddressPort().addressStr();
		m_currentClientIsActive = (m_singleLmControlMode == false) || (m_activeClientId == m_instanceId && m_activeClientIp == localAddress);
	}

	//

	resetToProcessTuningSignals();

	return;
}


void TuningTcpClient::requestActivateTuningSource(const QString& equipmentId, bool enableControl, bool forceTakeControl)
{
	if (isConnected() == false)
	{
		writeLogMessage(tr("TuningTcpClient::requestActivateTuningSource, isConnected() == false."));
		return;
	}

	if (isClearToSendRequest() == false)
	{
		writeLogMessage(tr("TuningTcpClient::requestActivateTuningSource, isClearToSendRequest() == false, reconnecting."));
		closeConnection();
		return;
	}

	// Create the request
	//

	m_activateTuningSource.set_tuningsourceequipmentid(equipmentId.toUtf8());
	m_activateTuningSource.set_activatecontrol(enableControl);
	m_activateTuningSource.set_takecontrol(forceTakeControl);

	sendRequest(TDS_CHANGE_CONTROLLED_TUNING_SOURCE, m_activateTuningSource);

	return;
}

void TuningTcpClient::processActivateTuningSource(const QByteArray& data)
{
	bool ok = m_activateTuningSourceReply.ParseFromArray(data.constData(), data.size());

	if (ok == false)
	{
		assert(ok);
		resetToProcessTuningSignals();
		return;
	}

	if (m_activateTuningSourceReply.error() != static_cast<int>(NetworkError::Success))
	{
		writeLogError(tr("TuningTcpClient::processActivateTuningSource, error received: %1")
					  .arg(networkErrorStr(static_cast<NetworkError>(m_activateTuningSourceReply.error()))));

		return;
	}

	resetToProcessTuningSignals();

	return;

}

void TuningTcpClient::requestReadTuningSignals()
{
	if (isConnected() == false)
	{
		writeLogMessage(tr("TuningTcpClient::requestReadTuningSignals, isConnected() == false."));
		return;
	}

	if (isClearToSendRequest() == false)
	{
		writeLogMessage(tr("TuningTcpClient::isClearToSendRequest, isClearToSendRequest() == false, reconnecting."));
		closeConnection();
		return;
	}

	QMutexLocker l(&m_signalHashesMutex);

	int totalSignalCount = static_cast<int>(m_signalHashes.size());

	// If no signals in the database, start the new request loop
	//
	if (totalSignalCount == 0)
	{
		l.unlock();

		resetToGetTuningSourcesState();

		return;
	}

	// Determine the amount of signals needed to be requested
	//
	m_readTuningSignalCount = TDS_TUNING_MAX_READ_STATES;

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
	m_readTuningSignals.Clear();
	m_readTuningSignals.mutable_signalhash()->Reserve(m_readTuningSignalCount);

	for (int i = 0; i < m_readTuningSignalCount; i++)
	{
		Hash hash = m_signalHashes[m_readTuningSignalIndex + i];

		m_readTuningSignals.mutable_signalhash()->Add(hash);
	}

	l.unlock();

	sendRequest(TDS_TUNING_SIGNALS_READ, m_readTuningSignals);

	return;
}

void TuningTcpClient::processReadTuningSignals(const QByteArray& data)
{
	bool ok = m_readTuningSignalsReply.ParseFromArray(data.constData(), data.size());

	if (ok == false)
	{
		assert(ok);
		resetToGetTuningSourcesState();
		return;
	}

	if (m_readTuningSignalsReply.error() != static_cast<int>(NetworkError::Success))
	{
		writeLogError(tr("TuningTcpClient::processReadTuningSignals, error received: %1")
					  .arg(networkErrorStr(static_cast<NetworkError>(m_readTuningSignalsReply.error()))));

		resetToGetTuningSourcesState();
		return;
	}

	int stateCount = m_readTuningSignalsReply.tuningsignalstate_size();

	std::vector<TuningSignalState> arrivedStates;
	arrivedStates.reserve(stateCount);

	bool found = false;

	for (int i = 0; i < stateCount; i++)
	{
		const ::Network::TuningSignalState& stateMessage = m_readTuningSignalsReply.tuningsignalstate(i);

		NetworkError error = static_cast<NetworkError>(stateMessage.error());

		if (error != NetworkError::Success && error != NetworkError::LmControlIsNotActive)
		{
			writeLogError(tr("TuningTcpClient::processReadTuningSignals, TuningSignalState error received: %1")
						  .arg(networkErrorStr(error)));

			continue;
		}


		TuningSignalState arrivedState(stateMessage);

		TuningSignalState previousState = m_signals->state(stateMessage.signalhash(), &found);

		if (found == true)
		{
			// Process write error only if writing was performed by current client
			//
			Hash writeClientHash = stateMessage.writeclient();

			if (m_instanceIdHash == writeClientHash)
			{
				if (static_cast<NetworkError>(stateMessage.writeerrorcode()) == NetworkError::Success)
				{
					if (arrivedState.successfulWriteTime() > previousState.successfulWriteTime())
					{
						m_signals->setNewValueAsApplied(arrivedState.hash());
					}
				}
				else
				{
					if (arrivedState.unsuccessfulWriteTime() > previousState.unsuccessfulWriteTime())
					{
//						qDebug() << "arrivedState.unsuccessfulWriteTime() " << arrivedState.unsuccessfulWriteTime().toMSecsSinceEpoch();
//						qDebug() << "previousState.unsuccessfulWriteTime() " << previousState.unsuccessfulWriteTime().toMSecsSinceEpoch();
//						qDebug() << "stateMessage.writeerrorcode() " << stateMessage.writeerrorcode();

						m_signals->setNewValueAsApplied(arrivedState.hash());

						AppSignalParam param = m_signals->signalParam(stateMessage.signalhash(), &found);
						if (found == false)
						{
							assert(false);
							continue;
						}

						writeLogAlert(tr("Error writing value '%1' to signal '%2' (%3), logic module '%4': %5")
									  .arg(m_signals->newValue(arrivedState.hash()).toString())
									  .arg(param.customSignalId())
									  .arg(param.caption())
									  .arg(param.lmEquipmentId())
									  .arg(networkErrorStr(static_cast<NetworkError>(stateMessage.writeerrorcode())))
									  );
					}
				}
			}
		}

		// When updating states, we have to set some properties locally
		//
        arrivedState.m_flags.controlIsEnabled = (error == NetworkError::LmControlIsNotActive) ? false : true;

		// --
		//
		arrivedStates.push_back(arrivedState);
	}

	if (m_simulationMode == false)
	{
		m_signals->setState(arrivedStates);
	}

	// Increase the requested signal index, wrap the request index if needed
	//

	QMutexLocker l(&m_signalHashesMutex);

	int totalSignalCount = static_cast<int>(m_signalHashes.size());

	l.unlock();

	m_readTuningSignalIndex += m_readTuningSignalCount;

	if (m_readTuningSignalIndex >= totalSignalCount)
	{
		m_readTuningSignalIndex = 0;

		// Start the new loop
		//
		resetToGetTuningSourcesState();
	}
	else
	{
		// Continue the current loop
		//
		resetToProcessTuningSignals();
	}

	return;
}

void TuningTcpClient::requestWriteTuningSignals()
{
	if (isConnected() == false)
	{
		writeLogMessage(tr("TuningTcpClient::requestWriteTuningSignals, isConnected() == false."));
		return;
	}

	if (isClearToSendRequest() == false)
	{
		writeLogMessage(tr("TuningTcpClient::requestWriteTuningSignals, isClearToSendRequest() == false, reconnecting."));
		closeConnection();
		return;
	}

	{
		QMutexLocker l(&m_writeQueueMutex);

		// Determine the amount of signals required to be written
		//
		int writeTuningSignalCount = TDS_TUNING_MAX_WRITE_RECORDS;

		if (writeTuningSignalCount >= m_writeQueue.size())
		{
			writeTuningSignalCount = static_cast<int>(m_writeQueue.size());
		}

		// Create the request
		//
		m_writeTuningSignals.Clear();

		m_writeTuningSignals.set_autoapply(m_autoApply);
		m_writeTuningSignals.mutable_commands()->Reserve(writeTuningSignalCount);

		for (int i = 0; i < writeTuningSignalCount; i++)
		{
			if (m_writeQueue.empty() == true)
			{
				assert(false);
				break;
			}

			const TuningWriteCommand& cmd = m_writeQueue.front();

			::Network::TuningWriteCommand* protoCommand = m_writeTuningSignals.mutable_commands()->Add();
			cmd.save(protoCommand);

			m_writeQueue.pop();
		}
	}

	sendRequest(TDS_TUNING_SIGNALS_WRITE, m_writeTuningSignals);

	return;
}

void TuningTcpClient::processWriteTuningSignals(const QByteArray& data)
{
	bool ok = m_writeTuningSignalsReply.ParseFromArray(data.constData(), data.size());

	if (ok == false)
	{
		assert(ok);
		resetToGetTuningSourcesState();
		return;
	}

	if (m_writeTuningSignalsReply.error() != static_cast<int>(NetworkError::Success))
	{
		writeLogError(tr("TuningTcpClient::processWriteTuningSignals, error received: %1")
					  .arg(networkErrorStr(static_cast<NetworkError>(m_writeTuningSignalsReply.error()))));

		resetToGetTuningSourcesState();
		return;
	}

	int writeResultCount = m_writeTuningSignalsReply.writeresult_size();

	for (int i = 0; i < writeResultCount; i++)
	{
		const ::Network::TuningSignalWriteResult& twr = m_writeTuningSignalsReply.writeresult(i);

		if (twr.error() != static_cast<int>(NetworkError::Success))
		{
			writeLogError(tr("TuningTcpClient::processWriteTuningSignals, TuningSignalWriteResult error received: %1, hash = %2")
						  .arg(networkErrorStr(static_cast<NetworkError>(twr.error())))
						  .arg(twr.signalhash()));

			continue;
		}
	}

	resetToProcessTuningSignals();

	return;
}

void TuningTcpClient::requestApplyTuningSignals()
{
	if (isConnected() == false)
	{
		writeLogMessage(tr("TuningTcpClient::requestApplyTuningSignals, isConnected() == false."));
		return;
	}

	if (isClearToSendRequest() == false)
	{
		writeLogMessage(tr("TuningTcpClient::requestApplyTuningSignals, isClearToSendRequest() == false, reconnecting."));
		closeConnection();
		return;
	}

	sendRequest(TDS_TUNING_SIGNALS_APPLY, m_applyTuningSignals);

	return;
}

void TuningTcpClient::processApplyTuningSignals(const QByteArray& data)
{
	bool ok = m_applyTuningSignalsReply.ParseFromArray(data.constData(), data.size());

	if (ok == false)
	{
		assert(ok);
		resetToGetTuningSourcesState();
		return;
	}

	if (m_applyTuningSignalsReply.error() != static_cast<int>(NetworkError::Success))
	{
		writeLogError(tr("TuningTcpClient::processApplyTuningSignals, error received: %1")
					  .arg(networkErrorStr(static_cast<NetworkError>(m_applyTuningSignalsReply.error()))));

		resetToGetTuningSourcesState();
		return;
	}

	resetToProcessTuningSignals();

	return;
}

void TuningTcpClient::slot_configurationArrived(HostAddressPort address, bool autoApply, LmStatusFlagMode lmStatusFlagMode)
{
	writeLogMessage(tr("TuningTcpClient::slot_configurationArrived"));

	if (serverAddressPort1() != address || serverAddressPort2() != address)
	{
		setServers(address, address, true);
	}

	setAutoApply(autoApply);

	m_lmStatusFlagMode = lmStatusFlagMode;

	return;
}

void TuningTcpClient::slot_signalsUpdated()
{
	writeLogMessage(tr("TuningTcpClient::slot_signalsUpdated"));

	m_readTuningSignalIndex = 0;
	m_readTuningSignalCount = 0;

	{
		QMutexLocker l(&m_writeQueueMutex);

		while (m_writeQueue.empty() == false)
		{
			m_writeQueue.pop();
		}
	}

	{
		QMutexLocker l(&m_signalHashesMutex);
		m_signalHashes = m_signals->signalHashes();
	}

	// --
	//
	if (isConnected() == true)
	{
		resetToGetTuningSources();
	}

	if (m_simulationMode == true)
	{
		m_signals->validateStates();
	}

	return;
}

QString TuningTcpClient::networkErrorStr(NetworkError error)
{
	return getNetworkErrorStr(error);
}

void TuningTcpClient::writeLogAlert(const QString& message)
{
	qDebug() << message;
}

void TuningTcpClient::writeLogError(const QString& message)
{
	qDebug() << message;
}

void TuningTcpClient::writeLogWarning(const QString& message)
{
	qDebug() << message;
}

void TuningTcpClient::writeLogMessage(const QString& message)
{
	qDebug() << message;
}

void TuningTcpClient::writeLogSignalChange(const AppSignalParam& param, const TuningValue& oldValue, const TuningValue& newValue)
{
	Q_UNUSED(param);
	Q_UNUSED(oldValue);
	Q_UNUSED(newValue);
}

void TuningTcpClient::writeLogSignalChange(const QString& message)
{
	qDebug() << message;
}

QString TuningTcpClient::instanceId() const
{
	return m_instanceId;
}

void TuningTcpClient::setInstanceId(const QString& instanceId)
{
	m_instanceId = instanceId;
	m_instanceIdHash = ::calcHash(m_instanceId);
}

Hash TuningTcpClient::instanceIdHash() const
{
	return m_instanceIdHash;
}

int TuningTcpClient::requestInterval() const
{
	return m_requestInterval;
}

void TuningTcpClient::setRequestInterval(int requestInterval)
{
	m_requestInterval = requestInterval;
}

bool TuningTcpClient::autoApply() const
{
	return m_autoApply;
}

void TuningTcpClient::setAutoApply(bool value)
{
	m_autoApply = value;
}

bool TuningTcpClient::clientIsActive() const
{
	if (m_singleLmControlMode == false)
	{
		return true;
	}

	return m_currentClientIsActive;
}

bool TuningTcpClient::singleLmControlMode() const
{
	return m_singleLmControlMode;
}

QString TuningTcpClient::activeClientId() const
{
	QMutexLocker l(&m_activeClientMutex);
	return m_activeClientId;
}

QString TuningTcpClient::activeClientIp() const
{
	QMutexLocker l(&m_activeClientMutex);
	return m_activeClientIp;
}

int TuningTcpClient::activeTuningSourceCount() const
{
	int result = 0;

	QMutexLocker l(&m_tuningSourcesMutex);

	for (const auto& it : m_tuningSources)
	{
		const TuningSource& ts = it.second;

		if (ts.state.controlisactive() == true)
		{
			result++;
		}
	}

	return result;
}


QString TuningTcpClient::singleActiveTuningSource() const
{
	if (m_singleLmControlMode == false)
	{
		assert(false);
		return QString();
	}

	QMutexLocker l(&m_tuningSourcesMutex);

	for (const auto& it : m_tuningSources)
	{
		const TuningSource& ts = it.second;

		if (ts.state.controlisactive() == true)
		{
			return ts.equipmentId();
		}
	}

	return QString();
}

LmStatusFlagMode TuningTcpClient::lmStatusFlagMode() const
{
	return m_lmStatusFlagMode;
}
