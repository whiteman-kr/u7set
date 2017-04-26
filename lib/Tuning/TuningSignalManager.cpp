#include "../lib/Tuning/TuningSignalManager.h"



//
//TuningSignalManager
//


TuningSignalManager::TuningSignalManager()
	:Tcp::Client(HostAddressPort (QLatin1String("0.0.0.0"), 0))
{
}

TuningSignalManager::~TuningSignalManager()
{
}

void TuningSignalManager::setInstanceId(const QString& instanceId)
{
	m_instanceId = instanceId;
}

void TuningSignalManager::setRequestInterval(int requestInterval)
{
	m_requestInterval = requestInterval;
}

bool TuningSignalManager::loadDatabase(const QByteArray& data, QString* errorCode)
{
	if (errorCode == nullptr)
	{
		assert(errorCode);
		return false;
	}

	bool result = true;

	std::map<Hash, int> statesMap;

	{
		QMutexLocker l(&m_signalsMutex);

		result &= m_signals.loadSignals(data, errorCode);

		// Create states map
		//
		for (int i = 0; i < m_signals.signalsCount(); i++)
		{
			Hash hash = m_signals.signalPtrByIndex(i)->hash();
			statesMap[hash] = i;
		}

	}

	{
		QMutexLocker sl(&m_statesMutex);

		m_states.resize(statesMap.size());
		m_statesMap = statesMap;
	}

	// Create Tuning sources
	//
	{
		QMutexLocker sl(&m_tuningSourcesMutex);

		m_tuningSourcesList.clear();

		int count = m_signals.signalsCount();
		for (int i = 0; i < count; i++)
		{
			const AppSignalParam* o = m_signals.signalPtrByIndex(i);
			if (o == nullptr)
			{
				assert(o);
				continue;
			}

			if (m_tuningSourcesList.indexOf(o->equipmentId()) == -1)
			{
				m_tuningSourcesList.append(o->equipmentId());
			}
		}
	}


	return result;
}

TuningSignalStorage TuningSignalManager::signalsStorage()
{
	QMutexLocker l(&m_signalsMutex);
	return m_signals;
}

// WARNING!!! Lock the m_signalsMutex before calling this function!!!
//
bool TuningSignalManager::signalExists(Hash hash) const
{
	return m_signals.signalExists(hash);
}

// WARNING!!! Lock the m_statesMutex before calling this function!!!
//
TuningSignalState TuningSignalManager::stateByHash(Hash hash) const
{
	auto it = m_statesMap.find(hash);
	if (it == m_statesMap.end())
	{
		assert(false);
		return TuningSignalState();
	}

	int index = it->second;
	if (index < 0 || index >= m_states.size())
	{
		assert(false);
		return TuningSignalState();
	}

	return m_states[index];
}

// WARNING!!! Lock the m_statesMutex before calling this function!!!
//
TuningSignalState* TuningSignalManager::statePtrByHash(Hash hash)
{
	auto it = m_statesMap.find(hash);
	if (it == m_statesMap.end())
	{
		assert(false);
		return nullptr;
	}

	int index = it->second;
	if (index < 0 || index >= m_states.size())
	{
		assert(false);
		return nullptr;
	}

	return& m_states[index];
}

QStringList TuningSignalManager::tuningSourcesEquipmentIds()
{
	QMutexLocker l(&m_tuningSourcesMutex);
	return m_tuningSourcesList;
}

void TuningSignalManager::onClientThreadStarted()
{

	return;
}

void TuningSignalManager::onClientThreadFinished()
{
}

void TuningSignalManager::onConnection()
{
	writeLogMessage(tr("TuningSignalManager: connection established."));

	assert(isClearToSendRequest() == true);

	QMutexLocker l(&m_statesMutex);
	while (m_writeQueue.empty() == false)
	{
		m_writeQueue.pop();
	}
	l.unlock();

	resetToGetTuningSources();

	return;
}

void TuningSignalManager::onDisconnection()
{
	writeLogMessage(tr("TuningSignalManager: connection failed."));

	invalidateSignals();

	emit connectionFailed();
}

void TuningSignalManager::onReplyTimeout()
{
	writeLogMessage(tr("TuningSignalManager: reply timeout."));
}

void TuningSignalManager::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
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

	default:
		assert(false);
		writeLogError(tr("TcpTuningClient::processReply: Wrong requestID."));

		resetToGetTuningSources();
	}

	return;
}

void TuningSignalManager::invalidateSignals()
{
	QMutexLocker l(&m_statesMutex);

	for (TuningSignalState& state : m_states)
	{
		state.invalidate();
	}
}


void TuningSignalManager::resetToGetTuningSources()
{
	QThread::msleep(m_requestInterval);

	requestTuningSourcesInfo();
	return;
}

void TuningSignalManager::resetToGetTuningSourcesState()
{
	QThread::msleep(m_requestInterval);

	requestTuningSourcesState();
	return;
}

void TuningSignalManager::requestTuningSourcesInfo()
{
	assert(isClearToSendRequest());

	m_getTuningSourcesInfo.set_clientequipmentid(m_instanceId.toUtf8());

	sendRequest(TDS_GET_TUNING_SOURCES_INFO, m_getTuningSourcesInfo);
}

void TuningSignalManager::requestTuningSourcesState()
{
	assert(isClearToSendRequest());

	m_getTuningSourcesStates.set_clientequipmentid(m_instanceId.toUtf8());

	sendRequest(TDS_GET_TUNING_SOURCES_STATES, m_getTuningSourcesStates);
}

void TuningSignalManager::processTuningSignals()
{
	// if there is a queued data to write something, write it.
	//

	QMutexLocker l(&m_statesMutex);

	bool writeQueueEmpty = m_writeQueue.empty();

	l.unlock();

	if (writeQueueEmpty == false)
	{
		requestWriteTuningSignals();
		return;
	}

	// request states
	//

	requestReadTuningSignals();

}

void TuningSignalManager::requestReadTuningSignals()
{
	QMutexLocker l(&m_signalsMutex);

	int objectCount = m_signals.signalsCount();

	// if no signals in the database, start the new request loop
	//

	if (objectCount == 0)
	{
		resetToGetTuningSourcesState();
		return;
	}

	// determine the amount of signals needed to be requested
	//

	const int READ_TUNING_SIGNALS_MAX = 100;

	m_readTuningSignalCount = READ_TUNING_SIGNALS_MAX;

	if (m_readTuningSignalIndex >= objectCount - 1)
	{
		// possibly, the database was updated and last requested index is larger than current database size
		//

		m_readTuningSignalIndex = 0;
	}

	if (m_readTuningSignalIndex + m_readTuningSignalCount >= objectCount)
	{
		m_readTuningSignalCount = objectCount - m_readTuningSignalIndex;
	}

	// create the request
	//

	assert(isClearToSendRequest());

	m_readTuningSignals.set_clientequipmentid(m_instanceId.toUtf8());

	m_readTuningSignals.mutable_signalhash()->Reserve(READ_TUNING_SIGNALS_MAX);

	m_readTuningSignals.mutable_signalhash()->Clear();

	for (int i = 0; i < m_readTuningSignalCount; i++)
	{
		const AppSignalParam* object = m_signals.signalPtrByIndex(m_readTuningSignalIndex + i);
		if (object == nullptr)
		{
			assert(object);
			continue;
		}

		Hash hash = object->hash();

		m_readTuningSignals.mutable_signalhash()->AddAlreadyReserved(hash);
	}

	l.unlock();

	//int s = m_readTuningSignals.mutable_signalhash()->size();
	//int c = m_readTuningSignals.mutable_signalhash()->Capacity();

	//qDebug()<<s;
	//qDebug()<<c;

	sendRequest(TDS_TUNING_SIGNALS_READ, m_readTuningSignals);

}

void TuningSignalManager::requestWriteTuningSignals()
{
	QMutexLocker l(&m_statesMutex);

	// determine the amount of signals needed to be written
	//

	const int WRITE_TUNING_SIGNALS_MAX = 100;

	int writeTuningSignalCount = WRITE_TUNING_SIGNALS_MAX;

	if (writeTuningSignalCount >= m_writeQueue.size())
	{
		writeTuningSignalCount = static_cast<int>(m_writeQueue.size());
	}

	// create the request
	//

	assert(isClearToSendRequest());

	m_writeTuningSignals.set_clientequipmentid(m_instanceId.toUtf8());
	m_writeTuningSignals.set_autoapply(true);

	m_writeTuningSignals.mutable_tuningsignalwrite()->Reserve(WRITE_TUNING_SIGNALS_MAX);

	m_writeTuningSignals.mutable_tuningsignalwrite()->Clear();

	for (int i = 0; i < writeTuningSignalCount; i++)
	{
		if (m_writeQueue.empty() == true)
		{
			assert(false);
			break;
		}

		const WriteCommand& cmd = m_writeQueue.front();

		::Network::TuningSignalWrite* wrCmd = new ::Network::TuningSignalWrite();

		wrCmd->set_value(cmd.m_value);
		wrCmd->set_signalhash(cmd.m_hash);

		m_writeTuningSignals.mutable_tuningsignalwrite()->AddAllocated(wrCmd);

		m_writeQueue.pop();
	}

	l.unlock();

	//int s = m_writeTuningSignals.mutable_tuningsignalwrite()->size();
	//int c = m_writeTuningSignals.mutable_tuningsignalwrite()->Capacity();

	//qDebug()<<s;
	//qDebug()<<c;

	sendRequest(TDS_TUNING_SIGNALS_WRITE, m_writeTuningSignals);
}

void TuningSignalManager::processTuningSourcesInfo(const QByteArray& data)
{

	bool ok = m_tuningDataSourcesInfoReply.ParseFromArray(data.constData(), data.size());

	if (ok == false)
	{
		assert(ok);
		resetToGetTuningSources();
		return;
	}

	if (m_tuningDataSourcesInfoReply.error() != 0)
	{
		writeLogError(tr("TcpTuningClient::m_tuningDataSourcesInfoReply, error received: %1")
					  .arg(networkErrorStr(static_cast<NetworkError>(m_tuningDataSourcesInfoReply.error()))));

		resetToGetTuningSources();
		return;
	}

	{
		QMutexLocker l(&m_tuningSourcesMutex);
		m_tuningSources.clear();

		for (int i = 0; i < m_tuningDataSourcesInfoReply.datasourceinfo_size(); i++)
		{
			TuningSource ts;

			const ::Network::DataSourceInfo& dsi = m_tuningDataSourcesInfoReply.datasourceinfo(i);

			ts.m_info = dsi;

			quint64 id = dsi.id();

			if (m_tuningSources.find(id) != m_tuningSources.end())
			{
				// id is not unique
				assert(false);
				continue;
			}

			/*qDebug()<<"Id = "<<id;
			qDebug()<<"m_equipmentId = "<<dsi.equipmentid().c_str();
			qDebug()<<"m_ip = "<<dsi.ip().c_str();*/

			m_tuningSources[id] = ts;
		}

		l.unlock();
	}

	requestTuningSourcesState();

	emit tuningSourcesArrived();

	return;
}

void TuningSignalManager::processTuningSourcesState(const QByteArray& data)
{

	bool ok = m_tuningDataSourcesStatesReply.ParseFromArray(data.constData(), data.size());

	if (ok == false)
	{
		assert(ok);
		resetToGetTuningSourcesState();
		return;
	}

	if (m_tuningDataSourcesStatesReply.error() != 0)
	{
		writeLogError(tr("TcpTuningClient::processTuningSourcesState, error received: %1")
					  .arg(networkErrorStr(static_cast<NetworkError>(m_tuningDataSourcesStatesReply.error()))));

		resetToGetTuningSourcesState();
		return;
	}

	QMutexLocker l(&m_tuningSourcesMutex);

	for (int i = 0; i < m_tuningDataSourcesStatesReply.tuningsourcesstate_size(); i++)
	{
		const ::Network::TuningSourceState& tss = m_tuningDataSourcesStatesReply.tuningsourcesstate(i);

		quint64 id = tss.sourceid();

		auto it = m_tuningSources.find(id);
		if (it == m_tuningSources.end())
		{
			// no id found
			assert(false);
			continue;
		}

		TuningSource& ts = it->second;

		ts.m_state = tss;
	}

	l.unlock();

	processTuningSignals();

	return;
}



void TuningSignalManager::processReadTuningSignals(const QByteArray& data)
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
		writeLogError(tr("TcpTuningClient::processReadTuningSignals, error received: %1")
					  .arg(networkErrorStr(static_cast<NetworkError>(m_readTuningSignalsReply.error()))));

		resetToGetTuningSourcesState();
		return;
	}

	QMutexLocker l(&m_statesMutex);

	int readReplyCount = m_readTuningSignalsReply.tuningsignalstate_size();

	for (int i = 0; i < readReplyCount; i++)
	{
		const ::Network::TuningSignalState& tss = m_readTuningSignalsReply.tuningsignalstate(i);

		if (tss.error() != 0)
		{
			writeLogError(tr("TcpTuningClient::processReadTuningSignals, TuningSignalState error received: %1")
						  .arg(networkErrorStr(static_cast<NetworkError>(tss.error()))));

			continue;
		}

		TuningSignalState* object = statePtrByHash(tss.signalhash());
		if (object == nullptr)
		{
			writeLogError(tr("TcpTuningClient::processReadTuningSignals, object not found by hash: %1")
						  .arg(tss.signalhash()));

			// no such signal found
			continue;
		}

		bool writingFailed = false;

		object->onReceiveValue(tss.readlowbound(), tss.readhighbound(), tss.valid(), tss.value(), &writingFailed);

		if (writingFailed == true)
		{
			writeLogError(tr("Error writing wignal with hash = %1, value = %2")
						  .arg(tss.signalhash())
						  .arg(tss.value())
						  );
		}

	}

	l.unlock();

	QMutexLocker sl(&m_signalsMutex);

	int objectCount = m_signals.signalsCount();

	// increase the requested signal index, wrap the request index if needed
	//

	m_readTuningSignalIndex += m_readTuningSignalCount;

	if (m_readTuningSignalIndex >= objectCount)
	{
		m_readTuningSignalIndex  = 0;

		// start the new loop

		resetToGetTuningSourcesState();
	}
	else
	{
		// continue the current loop

		processTuningSignals();
	}
}


void TuningSignalManager::processWriteTuningSignals(const QByteArray& data)
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
			writeLogError(tr("TcpTuningClient::processWriteTuningSignals, TuningSignalWriteResult error received: %1, hash = %2")
						  .arg(networkErrorStr(static_cast<NetworkError>(twr.error())))
						  .arg(twr.signalhash()));

			continue;
		}
	}

	processTuningSignals();
}

void TuningSignalManager::slot_serversArrived(HostAddressPort address1, HostAddressPort address2)
{
	writeLogMessage(tr("TcpTuningClient::slot_configurationArrived"));

	setServers(address1, address2, true);

	return;
}

void TuningSignalManager::slot_signalsUpdated(QByteArray data)
{
	QString errorStr;
	if (loadDatabase(data, &errorStr) == false)
	{
		QString completeErrorMessage = tr("TuningSignals.xml file loading error: %1").arg(errorStr);
		writeLogMessage(completeErrorMessage);

		return;
	}

	writeLogMessage(tr("TcpTuningClient::slot_signalsUpdated"));

	QMutexLocker l(&m_statesMutex);

	m_readTuningSignalIndex = 0;
	m_readTuningSignalCount = 0;

	while (m_writeQueue.empty() == false)
	{
		m_writeQueue.pop();
	}

	l.unlock();

}
/*
void TuningSignalManager::slot_exists(QString appSignalID, bool* result, bool* ok)
{
	if (result == nullptr || ok == nullptr)
	{
		assert(result);
		assert(ok);
		return;
	}

	Hash hash = ::calcHash(appSignalID);

	QMutexLocker l(&m_signalsMutex);

	*result = signalExists(hash);
	*ok = true;
}

void TuningSignalManager::slot_analog(QString appSignalID, bool* result, bool* ok)
{
	if (result == nullptr || ok == nullptr)
	{
		assert(result);
		assert(ok);
		return;
	}

	Hash hash = ::calcHash(appSignalID);

	QMutexLocker l(&m_signalsMutex);

	AppSignalParam* signal = m_signals.signalPtrByHash(hash);
	if (signal == nullptr)
	{
		*ok = false;
		return;
	}

	*result = signal->isAnalog();

}

void TuningSignalManager::slot_valid(QString appSignalID, bool* result, bool* ok)
{
	if (result == nullptr || ok == nullptr)
	{
		assert(result);
		assert(ok);
		return;
	}

	Hash hash = ::calcHash(appSignalID);

	QMutexLocker l(&m_statesMutex);

	TuningSignalState& state = stateByHash(hash);

	*result = state.valid();

}


void TuningSignalManager::slot_value(QString appSignalID, float* result, bool* ok)
{
	if (result == nullptr || ok == nullptr)
	{
		assert(result);
		assert(ok);
		return;
	}

	Hash hash = ::calcHash(appSignalID);

	QMutexLocker l(&m_statesMutex);

	TuningSignalState& state = stateByHash(hash);

	*result = state.value();
}*/

void TuningSignalManager::slot_writeValue(QString appSignalID, float value, bool* ok)
{
	if (ok == nullptr)
	{
		assert(ok);
		return;
	}

	// Check ranges data

	Hash hash = ::calcHash(appSignalID);

	QMutexLocker l(&m_signalsMutex);

	AppSignalParam* signal = m_signals.signalPtrByHash(hash);
	if (signal == nullptr)
	{
		*ok = false;
		return;
	}

	if (value < signal->lowEngineeringUnits() || value > signal->highEngineeringUnits())
	{
		*ok = false;
		return;
	}

	l.unlock();

	// Write data

	std::pair<Hash, float> val;
	val.first = hash;
	val.second = value;

	std::vector<std::pair<Hash, float>> data;
	data.push_back(val);

	writeTuningSignals(data);

	*ok = true;
	return;
}

void TuningSignalManager::slot_signalParam(QString appSignalID, AppSignalParam* result, bool* ok)
{
	if (result == nullptr || ok == nullptr)
	{
		assert(result);
		assert(ok);
		return;
	}

	Hash hash = ::calcHash(appSignalID);

	QMutexLocker l(&m_signalsMutex);

	AppSignalParam* object = m_signals.signalPtrByHash(hash);
	if (object == nullptr)
	{
		*ok = false;
		return;
	}

	*result =* object;
}

void TuningSignalManager::slot_signalState(QString appSignalID, TuningSignalState* result, bool* ok)
{
	if (result == nullptr || ok == nullptr)
	{
		assert(result);
		assert(ok);
		return;
	}

	Hash hash = ::calcHash(appSignalID);

	QMutexLocker l(&m_statesMutex);

	TuningSignalState* state = statePtrByHash(hash);
	if (state == nullptr)
	{
		*ok = false;
		return;
	}

	*result =* state;
}

std::vector<TuningSource> TuningSignalManager::tuningSourcesInfo()
{
	std::vector<TuningSource> result;

	QMutexLocker l(&m_tuningSourcesMutex);

	for (auto ds : m_tuningSources)
	{
		result.push_back(ds.second);
	}

	l.unlock();

	return result;
}

bool TuningSignalManager::tuningSourceInfo(quint64 id, TuningSource* result)
{
	if (result == nullptr)
	{
		assert(result);
		return false;
	}

	QMutexLocker l(&m_tuningSourcesMutex);

	auto it = m_tuningSources.find(id);

	if (it == m_tuningSources.end())
	{
		return false;
	}

	*result = it->second;

	l.unlock();

	return true;
}

void TuningSignalManager::writeTuningSignals(std::vector<std::pair<Hash, float>>& data)
{

	QMutexLocker l(&m_statesMutex);

	for (std::pair<Hash, float>& pair: data)
	{
		Hash& hash = pair.first;
		float& value = pair.second;

		TuningSignalState* state = statePtrByHash(hash);
		if (state == nullptr)
		{
			assert(state);
			return;
		}

		// set edit value and writing flags to states
		//

		state->onSendValue(value);

		// push command to the queue
		//
		WriteCommand cmd(hash, value);
		m_writeQueue.push(cmd);
	}

	l.unlock();
}

QString TuningSignalManager::getStateToolTip()
{
	Tcp::ConnectionState connectionState = getConnectionState();

	QString result = tr("Tuning Service connection\r\n\r\n");
	result += tr("IP address (primary): %1\r\n").arg(serverAddressPort(0).addressPortStr());
	result += tr("IP address (secondary): %1\r\n").arg(serverAddressPort(1).addressPortStr());
	result += tr("Connection: ") + (connectionState.isConnected ? tr("established\r\n") : tr("no connection\r\n"));

	return result;
}

void TuningSignalManager::connectTuningController(TuningController* controller)
{
	if (controller == nullptr)
	{
		assert(controller);
		return;
	}

	if (m_tuningControllersMap.find(controller) != m_tuningControllersMap.end())
	{
		assert(false);
		return; // This controller is already attached
	}

	qDebug()<<"TuningSignalManager::connectTuningController : connected";

	m_tuningControllersMap[controller] = true;

	connect(controller, &TuningController::signal_writeValue, this, &TuningSignalManager::slot_writeValue, Qt::DirectConnection);

	connect(controller, &TuningController::signal_getParam, this, &TuningSignalManager::slot_signalParam, Qt::DirectConnection);
	connect(controller, &TuningController::signal_getState, this, &TuningSignalManager::slot_signalState, Qt::DirectConnection);
}

QString TuningSignalManager::networkErrorStr(NetworkError error)
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


void TuningSignalManager::writeLogError(const QString& message)
{
	Q_UNUSED(message);
}

void TuningSignalManager::writeLogWarning(const QString& message)
{
	Q_UNUSED(message);
}

void TuningSignalManager::writeLogMessage(const QString& message)
{
	Q_UNUSED(message);
}
