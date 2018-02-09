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
TuningTcpClient::TuningTcpClient(const SoftwareInfo& softwareInfo,
								 TuningSignalManager* signalManager) :
	Tcp::Client(softwareInfo, HostAddressPort("0.0.0.0", 0)),
	m_signals(signalManager)
{
	assert(m_signals);
}

TuningTcpClient::~TuningTcpClient()
{
}

#ifdef Q_DEBUG
void TuningTcpClient::setSimulationMode(bool value)
{
	m_simulationMode = value;
	if (m_simulationMode == true)
	{
		m_signals->validateStates();
	}
}
#endif

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

bool TuningTcpClient::tuningSourceInfoById(quint64 id, TuningSource* result) const
{
	Hash hash = 0;

	{
		QMutexLocker l(&m_tuningSourcesMutex);
		auto it = m_tuningSourcesIdToHashMap.find(id);
		if (it == m_tuningSourcesIdToHashMap.end())
		{
			return false;
		}

		hash = it->second;
	}

	return tuningSourceInfoByHash(hash, result);
}

bool TuningTcpClient::tuningSourceInfoByHash(Hash equipmentHash, TuningSource* result) const
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

bool TuningTcpClient::activateTuningSourceControl(const QString& equipmentId, bool enableControl)
{
	if (m_tuningSources.find(::calcHash(equipmentId)) == m_tuningSources.end())
	{
		assert(false);
		return false;
	}

	QMutexLocker l(&m_writeQueueMutex);

	m_writeQueue.emplace(TuningWriteCommand(equipmentId, enableControl));

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
#ifdef Q_DEBUG
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
			}


		}

		return;
	}
#endif

	QMutexLocker l(&m_writeQueueMutex);

	bool found = false;

	for (const TuningWriteCommand& command : data)
	{
		// Write command to log

		AppSignalParam param =m_signals->signalParam(command.m_hash, &found);
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

		writeLogSignalChange(param, state.value(), command.m_value);

		// Push command to the queue
		//
		m_writeQueue.emplace(command);
	}

	return;
}

// Apply states
//
void TuningTcpClient::applyTuningSignals()
{
	QMutexLocker l(&m_writeQueueMutex);

	m_writeQueue.emplace(TuningWriteCommand());

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

	resetToGetTuningSources();

	return;
}

void TuningTcpClient::onDisconnection()
{
	writeLogMessage(tr("TuningTcpClient: connection failed."));

	m_signals->invalidateStates();

	return;
}

void TuningTcpClient::onReplyTimeout()
{
	writeLogMessage(tr("TuningTcpClient: reply timeout."));
	closeConnection();
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
		writeLogError(tr("TcpTuningClient::processReply: Wrong requestID."));

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
		const TuningWriteCommand& cmd = m_writeQueue.front();

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

				requestActivateTuningSource(cmd.m_equipmentId, cmd.m_enableControl);

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
	assert(isClearToSendRequest());

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

	if (m_tuningSourcesInfoReply.error() != 0)
	{
		assert(false);
		writeLogError(tr("TcpTuningClient::m_tuningDataSourcesInfoReply, error received: %1")
					  .arg(networkErrorStr(static_cast<NetworkError>(m_tuningSourcesInfoReply.error()))));

		resetToProcessTuningSignals();
		return;
	}

	{
		QMutexLocker l(&m_tuningSourcesMutex);

		m_tuningSources.clear();
		m_tuningSourcesIdToHashMap.clear();

		for (int i = 0; i < m_tuningSourcesInfoReply.tuningsourceinfo_size(); i++)
		{
			const ::Network::DataSourceInfo& dsi = m_tuningSourcesInfoReply.tuningsourceinfo(i);

			TuningSource ts;
			ts.info = dsi;

			Hash hash = ::calcHash(QString(ts.info.equipmentid().c_str()));

			assert(m_tuningSources.count(hash) == 0);

			m_tuningSources[hash] = ts;
			m_tuningSourcesIdToHashMap[ts.id()] = hash;
		}
	}

	m_singleLmControlMode = m_tuningSourcesInfoReply.singlelmcontrolmode();

	requestTuningSourcesState();

	emit tuningSourcesArrived();

	return;
}

void TuningTcpClient::requestTuningSourcesState()
{
	assert(isClearToSendRequest());

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

	if (m_tuningSourcesStatesReply.error() != 0)
	{
		assert(false);
		writeLogError(tr("TcpTuningClient::processTuningSourcesState, error received: %1")
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

			auto hashIt = m_tuningSourcesIdToHashMap.find(id);
			if (hashIt == m_tuningSourcesIdToHashMap.end())
			{
				assert(false);
				continue;
			}

			Hash hash = hashIt->second;

			auto it = m_tuningSources.find(hash);
			if (it == m_tuningSources.end())
			{
				assert(false);
				continue;
			}

			TuningSource& ts = it->second;

			// Write SOR to tuning log

			TuningValue oldSor;
			oldSor.setType(TuningValueType::Discrete);
			oldSor.setDiscreteValue(ts.state.setsor() ? 1 : 0);

			TuningValue newSor;
			newSor.setType(TuningValueType::Discrete);
			newSor.setDiscreteValue(tss.setsor() ? 1 : 0);

			AppSignalParam param;
			param.setEquipmentId(ts.info.equipmentid().c_str());
			param.setCaption(tr("SOR is set"));
			param.setPrecision(0);

			writeLogSignalChange(param, oldSor, newSor);

			//

			ts.state = tss;
		}

		// Set signals' flags that belong to tuning sources

		for (auto sourceIt : m_tuningSources)
		{
			Hash tsHash = sourceIt.first;

			const TuningSource& ts = sourceIt.second;

			std::pair <std::multimap<Hash, Hash>::iterator, std::multimap<Hash,Hash>::iterator> ret;

			ret = m_equipmentToSignalMap.equal_range(tsHash);

			for (std::multimap<Hash, Hash>::iterator signalIt = ret.first; signalIt != ret.second; ++signalIt)
			{
				Hash signalHash = signalIt->second;

				bool found = false;

				TuningSignalState state = m_signals->state(signalHash, &found);
				if (found == false)
				{
					continue;
				}

				state.m_flags.controlIsEnabled = true;//ts.state.controlisactive();

				m_signals->setState(signalHash, state);
			}
		}
	}

	//

	resetToProcessTuningSignals();

	return;
}


void TuningTcpClient::requestActivateTuningSource(const QString& equipmentId, bool enableControl)
{
	assert(isClearToSendRequest());

	// Create the request
	//

	m_activateTuningSource.set_tuningsourceequipmentid(equipmentId.toUtf8());
	m_activateTuningSource.set_activatecontrol(enableControl);

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

	if (m_activateTuningSourceReply.error() != 0)
	{
		assert(false);

		writeLogError(tr("TcpTuningClient::processActivateTuningSource, error received: %1")
					  .arg(networkErrorStr(static_cast<NetworkError>(m_activateTuningSourceReply.error()))));

		return;
	}

	resetToProcessTuningSignals();

	return;

}

void TuningTcpClient::requestReadTuningSignals()
{
	assert(isClearToSendRequest());

	int signalCount = static_cast<int>(m_signalHashes.size());

	// If no signals in the database, start the new request loop
	//
	if (signalCount == 0)
	{
		resetToGetTuningSourcesState();
		return;
	}

	// Determine the amount of signals needed to be requested
	//
	m_readTuningSignalCount = TDS_TUNING_MAX_READ_STATES;

	if (m_readTuningSignalIndex >= signalCount - 1)
	{
		// Possibly, the database was updated and last requested index is larger than current database size
		//
		m_readTuningSignalIndex = 0;
	}

	if (m_readTuningSignalIndex + m_readTuningSignalCount >= signalCount)
	{
		m_readTuningSignalCount = signalCount - m_readTuningSignalIndex;
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

	if (m_readTuningSignalsReply.error() != 0)
	{
		assert(false);
		writeLogError(tr("TcpTuningClient::processReadTuningSignals, error received: %1")
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

		if (stateMessage.error() != 0)
		{
			assert(false);
			writeLogError(tr("TcpTuningClient::processReadTuningSignals, TuningSignalState error received: %1")
						  .arg(networkErrorStr(static_cast<NetworkError>(stateMessage.error()))));

			continue;
		}

		arrivedStates.emplace_back(stateMessage);
	}

	m_signals->setState(arrivedStates);

	// Increase the requested signal index, wrap the request index if needed
	//
	int totalSignalCount = static_cast<int>(m_signalHashes.size());

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
	assert(isClearToSendRequest());

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

	if (m_writeTuningSignalsReply.error() != 0)
	{
		assert(false);
		writeLogError(tr("TcpTuningClient::processWriteTuningSignals, error received: %1")
					  .arg(networkErrorStr(static_cast<NetworkError>(m_writeTuningSignalsReply.error()))));

		resetToGetTuningSourcesState();
		return;
	}

	int writeResultCount = m_writeTuningSignalsReply.writeresult_size();

	for (int i = 0; i < writeResultCount; i++)
	{
		const ::Network::TuningSignalWriteResult& twr = m_writeTuningSignalsReply.writeresult(i);

		if (twr.error() != 0)
		{
			assert(false);
			writeLogError(tr("TcpTuningClient::processWriteTuningSignals, TuningSignalWriteResult error received: %1, hash = %2")
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
	assert(isClearToSendRequest());

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

	if (m_applyTuningSignalsReply.error() != 0)
	{
		assert(false);
		writeLogError(tr("TcpTuningClient::processApplyTuningSignals, error received: %1")
					  .arg(networkErrorStr(static_cast<NetworkError>(m_applyTuningSignalsReply.error()))));

		resetToGetTuningSourcesState();
		return;
	}

	resetToProcessTuningSignals();

	return;
}

void TuningTcpClient::slot_configurationArrived(HostAddressPort address1, HostAddressPort address2, bool autoApply)
{
	writeLogMessage(tr("TuningTcpClient::slot_configurationArrived"));

	setServers(address1, address2, true);

	setAutoApply(autoApply);

	return;
}

void TuningTcpClient::slot_signalsUpdated()
{
	writeLogMessage(tr("TcpTuningClient::slot_signalsUpdated"));

	m_readTuningSignalIndex = 0;
	m_readTuningSignalCount = 0;

	{
		QMutexLocker l(&m_writeQueueMutex);

		while (m_writeQueue.empty() == false)
		{
			m_writeQueue.pop();
		}
	}

	m_signalHashes = m_signals->signalHashes();

	// Build m_equipmentToSignalMap
	{
		QMutexLocker sl(&m_tuningSourcesMutex);

		m_equipmentToSignalMap.clear();

		AppSignalParam asp;

		for (Hash hash : m_signalHashes)
		{
			bool found = m_signals->signalParam(hash, &asp);
			if (found == false)
			{
				assert(false);
				continue;
			}

			Hash equipmentHash = ::calcHash(asp.equipmentId());
			m_equipmentToSignalMap.insert(std::pair<Hash,Hash>(equipmentHash, hash));
		}
	}

	//

	if (isConnected() == true)
	{
		resetToGetTuningSources();
	}

#ifdef Q_DEBUG
	if (m_simulationMode == true)
	{
		m_signals->validateStates();
	}
#endif


	return;
}

//void TuningTcpClient::slot_writeValue(QString appSignalID, float value, bool* ok)
//{
//	if (ok == nullptr)
//	{
//		assert(ok);
//		return;
//	}

//	// Check ranges data

//	Hash hash = ::calcHash(appSignalID);

//	QMutexLocker l(&m_signalsMutex);

//	AppSignalParam* signal = m_signals.signalPtrByHash(hash);
//	if (signal == nullptr)
//	{
//		*ok = false;
//		return;
//	}

//	if (value < signal->lowEngineeringUnits() || value > signal->highEngineeringUnits())
//	{
//		*ok = false;
//		return;
//	}

//	l.unlock();

//	// Write data

//	std::pair<Hash, float> val;
//	val.first = hash;
//	val.second = value;

//	std::vector<std::pair<Hash, float>> data;
//	data.push_back(val);

//	writeTuningSignals(data);

//	*ok = true;
//	return;
//}

//void TuningTcpClient::slot_signalParam(QString appSignalID, AppSignalParam* result, bool* ok)
//{
//	if (result == nullptr || ok == nullptr)
//	{
//		assert(result);
//		assert(ok);
//		return;
//	}

//	Hash hash = ::calcHash(appSignalID);

//	QMutexLocker l(&m_signalsMutex);

//	AppSignalParam* object = m_signals.signalPtrByHash(hash);
//	if (object == nullptr)
//	{
//		*ok = false;
//		return;
//	}

//	*result =* object;
//}

//void TuningTcpClient::slot_signalState(QString appSignalID, TuningSignalState* result, bool* ok)
//{
//	if (result == nullptr || ok == nullptr)
//	{
//		assert(result);
//		assert(ok);
//		return;
//	}

//	Hash hash = ::calcHash(appSignalID);

//	QMutexLocker l(&m_statesMutex);

//	TuningSignalState* state = statePtrByHash(hash);
//	if (state == nullptr)
//	{
//		*ok = false;
//		return;
//	}

//	*result =* state;
//}

//QString TuningTcpClient::getStateToolTip()
//{
//	Tcp::ConnectionState connectionState = getConnectionState();

//	QString result = tr("Tuning Service connection\r\n\r\n");
//	result += tr("IP address (primary): %1\r\n").arg(serverAddressPort(0).addressPortStr());
//	result += tr("IP address (secondary): %1\r\n").arg(serverAddressPort(1).addressPortStr());
//	result += tr("Connection: ") + (connectionState.isConnected ? tr("established\r\n") : tr("no connection\r\n"));

//	return result;
//}

QString TuningTcpClient::networkErrorStr(NetworkError error)
{
	switch (error)
	{
	case NetworkError::Success:                         return "NetworkError::Success"; break;
	case NetworkError::WrongPartNo:                     return "NetworkError::WrongPartNo"; break;
	case NetworkError::RequestParamExceed:              return "NetworkError::RequestParamExceed"; break;
	case NetworkError::RequestStateExceed:              return "NetworkError::RequestStateExceed"; break;
	case NetworkError::ParseRequestError:               return "NetworkError::ParseRequestError"; break;
	case NetworkError::RequestDataSourcesStatesExceed:  return "NetworkError::RequestDataSourcesStatesExceed"; break;
	case NetworkError::UnitsExceed:                     return "NetworkError::UnitsExceed"; break;
	case NetworkError::UnknownTuningClientID:           return "NetworkError::UnknownTuningClientID"; break;
	case NetworkError::UnknownSignalHash:               return "NetworkError::UnknownSignalHash"; break;
	case NetworkError::InternalError:                   return "NetworkError::InternalError"; break;
	default:
		assert(false);
	}

	return "?";
}


void TuningTcpClient::writeLogError(const QString& message)
{
	Q_UNUSED(message);
}

void TuningTcpClient::writeLogWarning(const QString& message)
{
	Q_UNUSED(message);
}

void TuningTcpClient::writeLogMessage(const QString& message)
{
	Q_UNUSED(message);
}

void TuningTcpClient::writeLogSignalChange(const AppSignalParam& param, const TuningValue& oldValue, const TuningValue& newValue)
{
	Q_UNUSED(param);
	Q_UNUSED(oldValue);
	Q_UNUSED(newValue);
}

QString TuningTcpClient::instanceId() const
{
	return m_instanceId;
}

void TuningTcpClient::setInstanceId(const QString& instanceId)
{
	m_instanceId = instanceId;
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

bool TuningTcpClient::singleLmControlMode() const
{
	return m_singleLmControlMode;
}
