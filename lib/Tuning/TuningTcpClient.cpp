
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
TuningTcpClient::TuningTcpClient(const SoftwareInfo& softwareInfo, const QString& tuningServiceId, bool singleLmControlMode, TuningSignalManager* signalManager) :
	Tcp::Client(softwareInfo, HostAddressPort("0.0.0.0", 0), "TuningTcpClient"),
	m_tuningServiceId(tuningServiceId),
	m_singleLmControlMode(singleLmControlMode),
	m_instanceId(softwareInfo.equipmentID()),
	m_instanceIdHash(::calcHash(softwareInfo.equipmentID())),
	m_signals(signalManager)
{
	assert(m_signals);

	qRegisterMetaType<LmStatusFlagMode>("LmStatusFlagMode");

	return;
}

TuningTcpClient::~TuningTcpClient()
{
}

std::vector<Hash> TuningTcpClient::tuningSourcesEquipmentHashes() const
{
	QReadLocker l(&m_tuningSourcesLock);

	std::vector<Hash> result;

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

bool TuningTcpClient::activateTuningSourceControl(const QString& equipmentId, bool enableControl, bool forceTakeControl)
{
	{
		QReadLocker l(&m_tuningSourcesLock);

		if (m_tuningSources.find(::calcHash(equipmentId)) == m_tuningSources.end())
		{
			assert(false);
			return false;
		}
	}

	if (forceTakeControl == true && clientIsActive() == true)
	{
		qDebug() << "Do not allow forceTakeControl command if current client is already active";
		writeLogError(QString("activateTuningSourceControl(%1, enableControl=%2, forceTakeControl=%3), Do not allow forceTakeControl command if current client is already active").arg(equipmentId).arg(enableControl).arg(forceTakeControl));

		assert(false);
		return false;
	}

	writeLogMessage(tr("Tuning Source %1 is %2.").arg(equipmentId).arg(enableControl ? tr("activated") : tr("deactivated")));

	QMutexLocker l(&m_writeQueueMutex);

	m_writeQueue.emplace(TuningWriteCommand(equipmentId, enableControl, forceTakeControl));

	return true;
}

bool TuningTcpClient::hasTuningSignals(const std::vector<Hash> appSignalHashes) const
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

bool TuningTcpClient::hasTuningSignal(QString appSignalId) const
{
	return hasTuningSignal(::calcHash(appSignalId));
}

void TuningTcpClient::writeTuningSignal(const TuningWriteCommand& data)
{
	std::vector<TuningWriteCommand> toVector;
	toVector.push_back(data);

	return writeTuningSignal(toVector);
}

void TuningTcpClient::writeTuningSignal(const std::vector<TuningWriteCommand>& data)
{
	QMutexLocker l(&m_writeQueueMutex);

	bool found = false;

	for (const TuningWriteCommand& command : data)
	{
		// Write command to log
		//
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
			writeLogAlert(tr("writeTuningSignal(), There is limits mismatch in signal '%1'. Operation is disabled.").arg(param.customSignalId()));
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
	if (isConnected() == false)
	{
		return false;
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

	writeLogSignalChange(tr("applyTuningSignals(), 'Apply' command is sent."));

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
	writeLogMessage(tr("onClientThreadFinished(), connection established."));

	assert(isClearToSendRequest() == true);

	{
		QMutexLocker l(&m_writeQueueMutex);

		decltype(m_writeQueue) clearQueue;
		std::swap(m_writeQueue, clearQueue);
	}

	{
		QWriteLocker l(&m_tuningSourcesLock);
		m_tuningSources.clear();
	}

	resetToGetTuningSources();

	return;
}

void TuningTcpClient::onDisconnection()
{
	writeLogMessage(tr("onDisconnection(), connection closed."));

	m_signals->invalidateStates();

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
		writeLogWarning(tr("onReplyTimeout(), reply timeout."));
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
		writeLogError(tr("processReply(): Wrong requestId, %1").arg(requestID));

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
	QMutexLocker locker(&m_writeQueueMutex);

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

				locker.unlock();

				requestApplyTuningSignals();

				break;
			}
		case TuningWriteCommand::TuningWriteCommandType::ActivateLm:
			{
				m_writeQueue.pop();

				// Activate LM
				//
				locker.unlock();

				requestActivateTuningSource(cmd.m_equipmentId, cmd.m_enableControl, cmd.m_forceTakeControl);

				return;
			}

		case TuningWriteCommand::TuningWriteCommandType::WriteValue:
			{
				// Write request
				//
				locker.unlock();

				requestWriteTuningSignals();

				return;
			}

		default:
			assert(false);

			locker.unlock();

			requestReadTuningSignals();

			return;
		}
	}
	else
	{
		locker.unlock();

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
		writeLogMessage(tr("requestTuningSourcesInfo(), isConnected() == false."));
		return;
	}

	if (isClearToSendRequest() == false)
	{
		writeLogMessage(tr("requestTuningSourcesInfo(), isClearToSendRequest() == false, reconnecting."));
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
		writeLogError(tr("m_tuningDataSourcesInfoReply(), error received: %1")
					  .arg(networkErrorStr(static_cast<NetworkError>(m_tuningSourcesInfoReply.error()))));

		resetToProcessTuningSignals();
		return;
	}

	{
		QWriteLocker l(&m_tuningSourcesLock);

		m_tuningSources.clear();

		for (int i = 0; i < m_tuningSourcesInfoReply.tuningsourceinfo_size(); i++)
		{
			const ::Network::DataSourceInfo& dsi = m_tuningSourcesInfoReply.tuningsourceinfo(i);

			TuningSource ts(dsi);

			Hash hash = ::calcHash(QString::fromStdString(ts.info().moduleequipmentid()));

			assert(m_tuningSources.count(hash) == 0);

			m_tuningSources[hash] = ts;
		}
	}

	requestTuningSourcesState();

	// Initialize list of signal hashes processed by this client
	//
	{
		std::vector<Hash> equipmentHashes = tuningSourcesEquipmentHashes();

		QWriteLocker l(&m_signalHashesLock);

		m_signalHashes = m_signals->signalHashes(equipmentHashes);

		m_signalHashesSet.reserve(m_signalHashes.size());
		for (const Hash& hash : m_signalHashes)
		{
			m_signalHashesSet.insert(hash);
		}

	}

	emit tuningSourcesInfoArrived();

	return;
}

void TuningTcpClient::requestTuningSourcesState()
{
	if (isConnected() == false)
	{
		writeLogMessage(tr("requestTuningSourcesState(), isConnected() == false."));
		return;
	}

	if (isClearToSendRequest() == false)
	{
		writeLogMessage(tr("requestTuningSourcesState(), isClearToSendRequest() == false, reconnecting."));
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
		writeLogError(tr("processTuningSourcesState(), error received: %1")
					  .arg(networkErrorStr(static_cast<NetworkError>(m_tuningSourcesStatesReply.error()))));

		resetToProcessTuningSignals();
		return;
	}

	{
		QWriteLocker l(&m_tuningSourcesLock);

		for (int i = 0; i < m_tuningSourcesStatesReply.tuningsourcesstate_size(); i++)
		{
			const ::Network::TuningSourceState& tss = m_tuningSourcesStatesReply.tuningsourcesstate(i);

			quint64 id = tss.sourceid();

			//bool found = false;

			for (auto& it : m_tuningSources)
			{
				TuningSource& ts = it.second;

				if (ts.id() == id)
				{
					// --------------------------------------------------------------------

					/*
					int ask_whiteman_about_this = 1;

					//qDebug() << "service id = " << tuningServiceId() << " states count = " << ts.statesCount();

					//qDebug() << "state = " << tuningServiceId() << " lan id = " << QString::fromStdString(state.lanequipmentid());

					bool wrongService = false;

					for (int q = 0; q < ts.controllersCount(); q++)
					{
						if (ts.controllerEquipmentId(q) == QString::fromStdString(tss.lanequipmentid()))
						{
							QString serviceId = QString::fromStdString(ts.info().lancontrollerinfo(q).tuningserviceid());

							if (serviceId.startsWith(tuningServiceId()) == false)
							{
								qDebug() << "LM Adapter " <<  QString::fromStdString(tss.lanequipmentid()) << " is not processed by service " << tuningServiceId();
								wrongService = true;
							}
						}
					}

					if (wrongService == true)
					{
						continue;
					}*/

					// --------------------------------------------------------------------

					// Write SOR change to tuning log

					for (int s = 0; s < ts.statesCount(); s++)
					{
						const ::Network::TuningSourceState& state = ts.state(s);

						if (state.isreply() == true && m_lmStatusFlagMode != LmStatusFlagMode::None)
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
					} // Write SOR

					// Set new source state

					/*
					::Network::TuningSourceState tss1 = tss;

					static int x = 0;
					static int c = 0;

					if ((c++) & 1)
					{
						if (x < 200)
						{
							tss1.set_errfotipoperationcode(x++);
						}
						else
						{
							tss1.set_errfotipoperationcode(x);
						}

					}*/

					//tss. tuningServiceId()

					ts.setNewState(tss);

					//

					//found = true;

					break;

				}	//ts.id() == id
			}

			//assert(found == true);
		}
	}

	{
		QMutexLocker l(&m_activeClientMutex);
		m_activeClientId = m_tuningSourcesStatesReply.activeclientid().c_str();
		m_activeClientIp = m_tuningSourcesStatesReply.activeclientip().c_str();

		QString localAddress = localAddressPort().addressStr();
		m_currentClientIsActive = (singleLmControlMode() == false) || (m_activeClientId == m_instanceId && m_activeClientIp == localAddress);
	}

	//

	resetToProcessTuningSignals();

	return;
}


void TuningTcpClient::requestActivateTuningSource(const QString& equipmentId, bool enableControl, bool forceTakeControl)
{
	if (isConnected() == false)
	{
		writeLogMessage(tr("requestActivateTuningSource(), isConnected() == false."));
		return;
	}

	if (isClearToSendRequest() == false)
	{
		writeLogMessage(tr("requestActivateTuningSource(), isClearToSendRequest() == false, reconnecting."));
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
		writeLogError(tr("processActivateTuningSource(), error received: %1")
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
		writeLogMessage(tr("requestReadTuningSignals(), isConnected() == false."));
		return;
	}

	if (isClearToSendRequest() == false)
	{
		writeLogMessage(tr("isClearToSendRequest(), isClearToSendRequest() == false, reconnecting."));
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
		writeLogError(tr("processReadTuningSignals(), error received: %1")
					  .arg(networkErrorStr(static_cast<NetworkError>(m_readTuningSignalsReply.error()))));

		resetToGetTuningSourcesState();
		return;
	}

	int stateCount = m_readTuningSignalsReply.tuningsignalstate_size();

	std::vector<TuningSignalState> arrivedStates;
	arrivedStates.reserve(stateCount);

	for (int i = 0; i < stateCount; i++)
	{

		const ::Network::TuningSignalState& stateMessage = m_readTuningSignalsReply.tuningsignalstate(i);

		NetworkError error = static_cast<NetworkError>(stateMessage.error());

		if (error != NetworkError::Success && error != NetworkError::LmControlIsNotActive)
		{
			writeLogError(tr("processReadTuningSignals(), TuningSignalState error received: %1")
						  .arg(networkErrorStr(error)));

			continue;
		}

		TuningSignalState arrivedState(stateMessage);

		// When updating states, we have to set some properties locally
		//
		arrivedState.m_flags.controlIsEnabled = (error == NetworkError::LmControlIsNotActive) ? false : true;

		if (lmStatusFlagMode() != LmStatusFlagMode::AccessKey)
		{
			// Set Access key flag to Validity flag & Control flag if Access Key function is inactive
			//
			arrivedState.m_flags.writingIsEnabled = arrivedState.valid() & arrivedState.controlIsEnabled();
		}
		else
		{
			arrivedState.m_flags.writingIsEnabled = arrivedState.valid() & arrivedState.writingIsEnabled();
		}

		bool found = false;

		TuningSignalState currentState = m_signals->state(stateMessage.signalhash(), &found);

		if (found == true)
		{
			int todo_synchronization = 1;

			// Compare time, if it is less than existing time - do not process state
			//
			if (arrivedState.lmTime() <= currentState.lmTime())
			{
				continue;
			}

			// Process write error only if writing was performed by current client
			//
			Hash writeClientHash = stateMessage.writeclient();

			if (m_instanceIdHash == writeClientHash)
			{
				if (static_cast<NetworkError>(stateMessage.writeerrorcode()) == NetworkError::Success)
				{
					if (arrivedState.successfulWriteTime() > currentState.successfulWriteTime())
					{
						m_signals->setNewValueAsApplied(arrivedState.hash());
					}
				}
				else
				{
					if (arrivedState.unsuccessfulWriteTime() > currentState.unsuccessfulWriteTime())
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

						writeLogAlert(tr("processReadTuningSignals(), Error writing value '%1' to signal '%2' (%3), logic module '%4': %5")
									  .arg(m_signals->newValue(arrivedState.hash()).toString())
									  .arg(param.customSignalId())
									  .arg(param.caption())
									  .arg(param.lmEquipmentId())
									  .arg(networkErrorStr(static_cast<NetworkError>(stateMessage.writeerrorcode())))
									  );
					}
				}
			}

			//
		}

		// --
		//
		arrivedStates.push_back(arrivedState);
	}

	m_signals->setState(arrivedStates);

	// Increase the requested signal index, wrap the request index if needed
	//

	QReadLocker l(&m_signalHashesLock);

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
		writeLogMessage(tr("requestWriteTuningSignals(), isConnected() == false."));
		return;
	}

	if (isClearToSendRequest() == false)
	{
		writeLogMessage(tr("requestWriteTuningSignals(), isClearToSendRequest() == false, reconnecting."));
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
		writeLogError(tr("processWriteTuningSignals(), error received: %1")
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
			writeLogError(tr("processWriteTuningSignals(), TuningSignalWriteResult error received: %1, hash = %2")
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
		writeLogMessage(tr("requestApplyTuningSignals(), isConnected() == false."));
		return;
	}

	if (isClearToSendRequest() == false)
	{
		writeLogMessage(tr("requestApplyTuningSignals(), isClearToSendRequest() == false, reconnecting."));
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
		writeLogError(tr("processApplyTuningSignals(), error received: %1")
					  .arg(networkErrorStr(static_cast<NetworkError>(m_applyTuningSignalsReply.error()))));

		resetToGetTuningSourcesState();
		return;
	}

	resetToProcessTuningSignals();

	return;
}

void TuningTcpClient::slot_configurationArrived(HostAddressPort address, bool autoApply, LmStatusFlagMode lmStatusFlagMode)
{
	writeLogMessage(tr("slot_configurationArrived()"));

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
	writeLogMessage(tr("slot_signalsUpdated()"));

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
		QWriteLocker l(&m_signalHashesLock);

		m_signalHashes.clear();
		m_signalHashesSet.clear();
	}

	// --
	//
	if (isConnected() == true)
	{
		resetToGetTuningSources();
	}

	return;
}

QString TuningTcpClient::networkErrorStr(NetworkError error)
{
	return getNetworkErrorStr(error);
}

void TuningTcpClient::writeLogAlert(const QString& message)
{
	qDebug() << "TuningTcpClient: " << message;
}

void TuningTcpClient::writeLogError(const QString& message)
{
	qDebug() << "TuningTcpClient: " << message;
}

void TuningTcpClient::writeLogWarning(const QString& message)
{
	qDebug() << "TuningTcpClient: " << message;
}

void TuningTcpClient::writeLogMessage(const QString& message)
{
	qDebug() << "TuningTcpClient: " << message;
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

QString TuningTcpClient::tuningServiceId() const
{
	return m_tuningServiceId;
}

void TuningTcpClient::setTuningServiceId(const QString& tuningServiceId)
{
	m_tuningServiceId = tuningServiceId;
}

bool TuningTcpClient::singleLmControlMode() const
{
	return m_singleLmControlMode;
}

bool TuningTcpClient::clientIsActive() const
{
	if (singleLmControlMode() == false)
	{
		return true;
	}

	return m_currentClientIsActive;
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

	QReadLocker l(&m_tuningSourcesLock);

	for (const auto& it : m_tuningSources)
	{
		const TuningSource& ts = it.second;

		for (int i = 0; i < ts.statesCount(); i++)
		{
			if (ts.state(i).controlisactive() == true)
			{
				result++;
				break;	// Break if found control in the source, do not count twice
			}
		}
	}

	return result;
}


QString TuningTcpClient::activeTuningSource() const
{
	if (singleLmControlMode() == false)
	{
		assert(false);
		return QString();
	}

	QReadLocker l(&m_tuningSourcesLock);

	for (const auto& it : m_tuningSources)
	{
		const TuningSource& ts = it.second;

		for (int i = 0; i < ts.statesCount(); i++)
		{
			if (ts.state(i).controlisactive() == true)
			{
				return ts.equipmentId();
			}
		}
	}

	return QString();
}

LmStatusFlagMode TuningTcpClient::lmStatusFlagMode() const
{
	return m_lmStatusFlagMode;
}

void TuningTcpClient::setLmStatusFlagMode(const LmStatusFlagMode& mode)
{
	m_lmStatusFlagMode = mode;
}
