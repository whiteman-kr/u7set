#include "../lib/Tuning/TuningObjectManager.h"


TuningObjectManager::TuningObjectManager()
	:Tcp::Client(HostAddressPort (QLatin1String("0.0.0.0"), 0))
{
}

TuningObjectManager::~TuningObjectManager()
{
}

void TuningObjectManager::setInstanceId(const QString& instanceId)
{
	m_instanceId = instanceId;
}

void TuningObjectManager::setRequestInterval(int requestInterval)
{
	m_requestInterval = requestInterval;
}

bool TuningObjectManager::loadDatabase(const QByteArray& data, QString *errorCode)
{
    if (errorCode == nullptr)
    {
        assert(errorCode);
        return false;
    }

    bool result = true;

    QMutexLocker l(&m_mutex);

    result &= m_objects.loadSignals(data, errorCode);

    // Create Tuning sources
    //
    m_tuningSourcesList.clear();

    int count = m_objects.objectCount();
    for (int i = 0; i < count; i++)
    {
        const TuningObject* o = m_objects.objectPtr(i);
        if (o == nullptr)
        {
            assert(o);
            continue;
        }

        if (m_tuningSourcesList.indexOf(o->equipmentID()) == -1)
        {
            m_tuningSourcesList.append(o->equipmentID());
        }
    }

    return result;
}

TuningObjectStorage TuningObjectManager::objectStorage()
{
    QMutexLocker l(&m_mutex);
    return m_objects;
}

// WARNING!!! Lock the mutex before calling this function!!!
//
bool TuningObjectManager::objectExists(Hash hash) const
{
    return m_objects.objectExists(hash);
}

// WARNING!!! Lock the mutex before calling this function!!!
//
TuningObject* TuningObjectManager::objectPtrByHash(Hash hash) const
{
    return m_objects.objectPtrByHash(hash);
}

QStringList TuningObjectManager::tuningSourcesEquipmentIds()
{
    QMutexLocker l(&m_mutex);
    return m_tuningSourcesList;
}

void TuningObjectManager::onClientThreadStarted()
{
	//qDebug() << "TuningObjectManager::onClientThreadStarted()";

    return;
}

void TuningObjectManager::onClientThreadFinished()
{
	//qDebug() << "TuningObjectManager::onClientThreadFinished()";

    //theSignals.reset();
}

void TuningObjectManager::onConnection()
{
	writeLogMessage(tr("TuningObjectManager: connection established."));

    assert(isClearToSendRequest() == true);

    QMutexLocker l(&m_mutex);
    while (m_writeQueue.empty() == false)
    {
        m_writeQueue.pop();
    }
    l.unlock();

    resetToGetTuningSources();

    return;
}

void TuningObjectManager::onDisconnection()
{
	writeLogMessage(tr("TuningObjectManager: connection failed."));

    QMutexLocker l(&m_mutex);

    m_objects.invalidateSignals();

    emit connectionFailed();
}

void TuningObjectManager::onReplyTimeout()
{
	writeLogMessage(tr("TuningObjectManager: reply timeout."));
}

void TuningObjectManager::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
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

void TuningObjectManager::resetToGetTuningSources()
{
	QThread::msleep(m_requestInterval);

    requestTuningSourcesInfo();
    return;
}

void TuningObjectManager::resetToGetTuningSourcesState()
{
	QThread::msleep(m_requestInterval);

    requestTuningSourcesState();
    return;
}

void TuningObjectManager::requestTuningSourcesInfo()
{
    assert(isClearToSendRequest());

	m_getTuningSourcesInfo.set_clientequipmentid(m_instanceId.toUtf8());

    sendRequest(TDS_GET_TUNING_SOURCES_INFO, m_getTuningSourcesInfo);
}

void TuningObjectManager::requestTuningSourcesState()
{
    assert(isClearToSendRequest());

	m_getTuningSourcesStates.set_clientequipmentid(m_instanceId.toUtf8());

    sendRequest(TDS_GET_TUNING_SOURCES_STATES, m_getTuningSourcesStates);
}

void TuningObjectManager::processTuningSignals()
{
    // if there is a queued data to write something, write it.
    //

    QMutexLocker l(&m_mutex);

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

void TuningObjectManager::requestReadTuningSignals()
{
    QMutexLocker l(&m_mutex);

    int objectCount = m_objects.objectCount();

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
        const TuningObject* object = m_objects.objectPtr(m_readTuningSignalIndex + i);
        if (object == nullptr)
        {
            assert(object);
            continue;
        }

        Hash hash = object->appSignalHash();

        m_readTuningSignals.mutable_signalhash()->AddAlreadyReserved(hash);
    }

    l.unlock();

    //int s = m_readTuningSignals.mutable_signalhash()->size();
    //int c = m_readTuningSignals.mutable_signalhash()->Capacity();

    //qDebug()<<s;
    //qDebug()<<c;

    sendRequest(TDS_TUNING_SIGNALS_READ, m_readTuningSignals);

}

void TuningObjectManager::requestWriteTuningSignals()
{
    QMutexLocker l(&m_mutex);

    // determine the amount of signals needed to be written
    //

    const int WRITE_TUNING_SIGNALS_MAX = 100;

    int writeTuningSignalCount = WRITE_TUNING_SIGNALS_MAX;

    if (writeTuningSignalCount >= m_writeQueue.size())
    {
        writeTuningSignalCount = (int)m_writeQueue.size();
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

void TuningObjectManager::processTuningSourcesInfo(const QByteArray& data)
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
        QMutexLocker l(&m_mutex);
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

            qDebug()<<"Id = "<<id;
            qDebug()<<"m_equipmentId = "<<dsi.equipmentid().c_str();
            qDebug()<<"m_ip = "<<dsi.ip().c_str();

            m_tuningSources[id] = ts;
        }

        l.unlock();
    }

    requestTuningSourcesState();

    emit tuningSourcesArrived();

    return;
}

void TuningObjectManager::processTuningSourcesState(const QByteArray& data)
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

    QMutexLocker l(&m_mutex);

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



void TuningObjectManager::processReadTuningSignals(const QByteArray& data)
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

    QMutexLocker l(&m_mutex);

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

        TuningObject* object = m_objects.objectPtrByHash(tss.signalhash());
        if (object == nullptr)
        {
			writeLogError(tr("TcpTuningClient::processReadTuningSignals, object not found by hash: %1")
                                  .arg(tss.signalhash()));

            // no such signal found
            continue;
        }


        object->setReadLowLimit(tss.readlowbound());
        object->setReadHighLimit(tss.readhighbound());
        object->setValid(tss.valid());

        bool writingFailed = false;
        object->onReceiveValue(tss.value(), writingFailed);

        if (writingFailed == true)
        {
			writeLogError(tr("Error writing wignal %1 (%2), value = %3, expected to write %4 ")
                                  .arg(object->appSignalID())
                                  .arg(object->caption())
                                  .arg(object->value())
                                  .arg(object->editValue())
                                  );
        }

    }

    int objectCount = m_objects.objectCount();

    l.unlock();

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


void TuningObjectManager::processWriteTuningSignals(const QByteArray& data)
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

void TuningObjectManager::slot_serversArrived(HostAddressPort address1, HostAddressPort address2)
{
	writeLogMessage(tr("TcpTuningClient::slot_configurationArrived"));

	setServers(address1, address2, true);

    return;
}

void TuningObjectManager::slot_signalsUpdated(QByteArray data)
{
	QString errorStr;
	if (loadDatabase(data, &errorStr) == false)
	{
		QString completeErrorMessage = tr("TuningSignals.xml file loading error: %1").arg(errorStr);
		writeLogMessage(completeErrorMessage);

		return;
	}

	writeLogMessage(tr("TcpTuningClient::slot_signalsUpdated"));

    QMutexLocker l(&m_mutex);

    m_readTuningSignalIndex = 0;
    m_readTuningSignalCount = 0;

    while (m_writeQueue.empty() == false)
    {
        m_writeQueue.pop();
    }

    l.unlock();

}

void TuningObjectManager::slot_exists(QString appSignalID, bool* result, bool* ok)
{
	if (result == nullptr || ok == nullptr)
	{
		assert(result);
		assert(ok);
		return;
	}

	Hash hash = ::calcHash(appSignalID);

	QMutexLocker l(&m_mutex);

	*result = objectExists(hash);
	*ok = true;
}

void TuningObjectManager::slot_valid(QString appSignalID, bool* result, bool* ok)
{
	if (result == nullptr || ok == nullptr)
	{
		assert(result);
		assert(ok);
		return;
	}

	Hash hash = ::calcHash(appSignalID);

	QMutexLocker l(&m_mutex);

	TuningObject* object = objectPtrByHash(hash);
	if (object == nullptr)
	{
		*ok = false;
		return;
	}

	*result = object->valid();

}

void TuningObjectManager::slot_value(QString appSignalID, float* result, bool* ok)
{
	if (result == nullptr || ok == nullptr)
	{
		assert(result);
		assert(ok);
		return;
	}

	Hash hash = ::calcHash(appSignalID);

	QMutexLocker l(&m_mutex);

	TuningObject* object = objectPtrByHash(hash);
	if (object == nullptr)
	{
		*ok = false;
		return;
	}

	//qDebug()<<Q_FUNC_INFO<<appSignalID<<object->value();

	*result = object->value();
}

void TuningObjectManager::slot_setValue(QString appSignalID, float value, bool* ok)
{
	if (ok == nullptr)
	{
		assert(ok);
		return;
	}

	//qDebug()<<Q_FUNC_INFO<<appSignalID<<value;

	Hash hash = ::calcHash(appSignalID);

	writeTuningSignal(hash, value);

	QMutexLocker l(&m_mutex);

	TuningObject* baseObject = m_objects.objectPtrByHash(hash);
	if (baseObject == nullptr)
	{
		*ok = false;
		return;
	}

	if (value < baseObject->lowLimit() || value > baseObject->highLimit())
	{
		*ok = false;
		return;
	}

	baseObject->onSendValue(value);
	baseObject->setWriting(true);

}

void TuningObjectManager::slot_highLimit(QString appSignalID, float *result, bool* ok)
{
	if (result == nullptr || ok == nullptr)
	{
		assert(result);
		assert(ok);
		return;
	}

	Hash hash = ::calcHash(appSignalID);

	QMutexLocker l(&m_mutex);

	TuningObject* object = objectPtrByHash(hash);
	if (object == nullptr)
	{
		*ok = false;
		return;
	}

	*result = object->highLimit();
}

void TuningObjectManager::slot_lowLimit(QString appSignalID, float *result, bool* ok)
{
	if (result == nullptr || ok == nullptr)
	{
		assert(result);
		assert(ok);
		return;
	}

	Hash hash = ::calcHash(appSignalID);

	QMutexLocker l(&m_mutex);

	TuningObject* object = objectPtrByHash(hash);
	if (object == nullptr)
	{
		*ok = false;
		return;
	}

	*result = object->lowLimit();
}


std::vector<TuningSource> TuningObjectManager::tuningSourcesInfo()
{
    std::vector<TuningSource> result;

    QMutexLocker l(&m_mutex);

    for (auto ds : m_tuningSources)
    {
        result.push_back(ds.second);
    }

    l.unlock();

    return result;
}

bool TuningObjectManager::tuningSourceInfo(quint64 id, TuningSource& result)
{
    QMutexLocker l(&m_mutex);

    auto it = m_tuningSources.find(id);

    if (it == m_tuningSources.end())
    {
        return false;
    }

    result = it->second;

    l.unlock();

    return true;
}

void TuningObjectManager::writeTuningSignal(Hash hash, float value)
{
    QMutexLocker l(&m_mutex);

    WriteCommand cmd(hash, value);

    m_writeQueue.push(cmd);

    l.unlock();
}

void TuningObjectManager::writeModifiedTuningObjects(std::vector<TuningObject>& objects)
{
    QMutexLocker l(&m_mutex);

    for (TuningObject& editObject : objects)
    {
        if (editObject.userModified() == false)
        {
            continue;
		}

		// push command to the queue
		//

		WriteCommand cmd(editObject.appSignalHash(), editObject.editValue());
		m_writeQueue.push(cmd);

		// set edit value and writing flags to edit object
		//

		editObject.clearUserModified();
		editObject.setWriting(true);

		// set edit value and writing flags to base object
		//

		TuningObject* baseObject = m_objects.objectPtrByHash(cmd.m_hash);
		if (baseObject == nullptr)
		{
			assert(baseObject);
			continue;
		}

		baseObject->onSendValue(cmd.m_value);
		baseObject->setWriting(true);

	}

    l.unlock();
}

QString TuningObjectManager::getStateToolTip()
{
	Tcp::ConnectionState connectionState = getConnectionState();

	QString result = tr("Tuning Service connection\r\n\r\n");
	result += tr("IP address (primary): %1\r\n").arg(serverAddressPort(0).addressPortStr());
	result += tr("IP address (secondary): %1\r\n").arg(serverAddressPort(1).addressPortStr());
	result += tr("Connection: ") + (connectionState.isConnected ? tr("established\r\n") : tr("no connection\r\n"));

	return result;
}

void TuningObjectManager::connectTuningController(TuningController* controller)
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

	qDebug()<<"TuningObjectManager::connectTuningController : connected";

	m_tuningControllersMap[controller] = true;

	connect(controller, &TuningController::signal_exists, this, &TuningObjectManager::slot_exists, Qt::DirectConnection);
	connect(controller, &TuningController::signal_valid, this, &TuningObjectManager::slot_valid, Qt::DirectConnection);

	connect(controller, &TuningController::signal_value, this, &TuningObjectManager::slot_value, Qt::DirectConnection);
	connect(controller, &TuningController::signal_setValue, this, &TuningObjectManager::slot_setValue, Qt::DirectConnection);

	connect(controller, &TuningController::signal_highLimit, this, &TuningObjectManager::slot_highLimit, Qt::DirectConnection);
	connect(controller, &TuningController::signal_lowLimit, this, &TuningObjectManager::slot_lowLimit, Qt::DirectConnection);

}

QString TuningObjectManager::networkErrorStr(NetworkError error)
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


void TuningObjectManager::writeLogError(const QString& message)
{
	Q_UNUSED(message);
}

void TuningObjectManager::writeLogWarning(const QString& message)
{
	Q_UNUSED(message);
}

void TuningObjectManager::writeLogMessage(const QString& message)
{
	Q_UNUSED(message);
}