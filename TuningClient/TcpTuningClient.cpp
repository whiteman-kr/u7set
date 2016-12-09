#include "TcpTuningClient.h"
#include "Settings.h"
#include "MainWindow.h"

TcpTuningClient::TcpTuningClient(ConfigController* configController, const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2)
	:Tcp::Client(serverAddressPort1, serverAddressPort2),
	  m_cfgController(configController)
{
	assert(m_cfgController);

	qDebug() << "TcpTuningClient::TcpTuningClient(const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2)";

}


TcpTuningClient::~TcpTuningClient()
{
	qDebug() << "TcpTuningClient::~TcpTuningClient()";
}

void TcpTuningClient::onClientThreadStarted()
{
	qDebug() << "TcpTuningClient::onClientThreadStarted()";

	connect(m_cfgController, &ConfigController::configurationArrived,
			this, &TcpTuningClient::slot_configurationArrived,
			Qt::QueuedConnection);

    connect(theMainWindow, &MainWindow::signalsUpdated, this, &TcpTuningClient::slot_signalsUpdated);


	return;
}

void TcpTuningClient::onClientThreadFinished()
{
	qDebug() << "TcpTuningClient::onClientThreadFinished()";

	//theSignals.reset();
}

void TcpTuningClient::onConnection()
{
	theLogFile.writeMessage(tr("TcpTuningClient: connection established."));

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

void TcpTuningClient::onDisconnection()
{
	theLogFile.writeMessage(tr("TcpTuningClient: connection failed."));

	emit connectionFailed();
}

void TcpTuningClient::onReplyTimeout()
{
	theLogFile.writeMessage(tr("TcpTuningClient: reply timeout."));
}

void TcpTuningClient::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
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

    default:
		assert(false);
		theLogFile.writeError(tr("TcpTuningClient::processReply: Wrong requestID."));

		resetToGetTuningSources();
	}

	return;
}

void TcpTuningClient::resetToGetTuningSources()
{
	QThread::msleep(theSettings.m_requestInterval);

	requestTuningSourcesInfo();
	return;
}

void TcpTuningClient::resetToGetTuningSourcesState()
{
	QThread::msleep(theSettings.m_requestInterval);

	requestTuningSourcesState();
	return;
}

void TcpTuningClient::requestTuningSourcesInfo()
{
    assert(isClearToSendRequest());

    m_getTuningSourcesInfo.set_clientequipmentid(theSettings.instanceStrId().toUtf8());

    sendRequest(TDS_GET_TUNING_SOURCES_INFO, m_getTuningSourcesInfo);
}

void TcpTuningClient::requestTuningSourcesState()
{
	assert(isClearToSendRequest());

    m_getTuningSourcesStates.set_clientequipmentid(theSettings.instanceStrId().toUtf8());

    sendRequest(TDS_GET_TUNING_SOURCES_STATES, m_getTuningSourcesStates);
}

void TcpTuningClient::processTuningSignals()
{
    // if there is a queued data to write something, write it.
    //

    QMutexLocker lQueue(&m_mutex);

    bool writeQueueEmpty = m_writeQueue.empty();

    lQueue.unlock();

    if (writeQueueEmpty == false)
    {
        requestWriteTuningSignals();
        return;
    }

    // request states
    //

    requestReadTuningSignals();

}

void TcpTuningClient::requestReadTuningSignals()
{
    QMutexLocker lObjects(&theObjects.m_mutex);

    int objectCount = theObjects.objectCount();

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

    m_readTuningSignals.set_clientequipmentid(theSettings.instanceStrId().toUtf8());

    m_readTuningSignals.mutable_signalhash()->Reserve(READ_TUNING_SIGNALS_MAX);

    m_readTuningSignals.mutable_signalhash()->Clear();

    for (int i = 0; i < m_readTuningSignalCount; i++)
    {
        TuningObject* object = theObjects.objectPtr(m_readTuningSignalIndex + i);
        if (object == nullptr)
        {
            assert(object);
            continue;
        }

        m_readTuningSignals.mutable_signalhash()->AddAlreadyReserved(object->appSignalHash());
    }

    int s = m_readTuningSignals.mutable_signalhash()->size();
    int c = m_readTuningSignals.mutable_signalhash()->Capacity();

    qDebug()<<s;
    qDebug()<<c;

    sendRequest(TDS_TUNING_SIGNALS_READ, m_readTuningSignals);

}

void TcpTuningClient::requestWriteTuningSignals()
{
    QMutexLocker lQueue(&m_mutex);

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

    m_writeTuningSignals.set_clientequipmentid(theSettings.instanceStrId().toUtf8());
    m_writeTuningSignals.set_autoappay(true);

    m_writeTuningSignals.mutable_tuningsignalwrite()->Reserve(writeTuningSignalCount);

    m_writeTuningSignals.mutable_tuningsignalwrite()->Clear();

    for (int i = 0; i < writeTuningSignalCount; i++)
    {
        if (m_writeQueue.empty() == true)
        {
            assert(false);
            break;
        }

        const WriteCommand& cmd = m_writeQueue.front();

        ::Network::TuningSignalWrite wrCmd;
        wrCmd.set_value(cmd.m_value);
        wrCmd.set_signalhash(cmd.m_hash);

        m_writeQueue.pop();

        m_writeTuningSignals.mutable_tuningsignalwrite()->AddAllocated(&wrCmd);
    }

    int s = m_writeTuningSignals.mutable_tuningsignalwrite()->size();
    int c = m_writeTuningSignals.mutable_tuningsignalwrite()->Capacity();

    qDebug()<<s;
    qDebug()<<c;

    //sendRequest(TDS_GET_TUNING_SOURCES_INFO, m_writeTuningSignals);
}

void TcpTuningClient::processTuningSourcesInfo(const QByteArray& data)
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
		theLogFile.writeError(tr("TcpTuningClient::m_tuningDataSourcesInfoReply, error received: %1").arg(m_tuningDataSourcesInfoReply.error()));
		assert(m_tuningDataSourcesStatesReply.error() != 0);

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
	}

    requestTuningSourcesState();

	emit tuningSourcesArrived();

	return;
}

void TcpTuningClient::processTuningSourcesState(const QByteArray& data)
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
		theLogFile.writeError(tr("TcpTuningClient::processTuningSourcesState, error received: %1").arg(m_tuningDataSourcesStatesReply.error()));
		assert(m_tuningDataSourcesStatesReply.error() != 0);

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

    requestReadTuningSignals();

    //resetToGetTuningSourcesState();

	return;
}



void TcpTuningClient::processReadTuningSignals(const QByteArray& data)
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
        theLogFile.writeError(tr("TcpTuningClient::processReadTuningSignals, error received: %1").arg(m_readTuningSignalsReply.error()));
        assert(m_tuningDataSourcesStatesReply.error() != 0);

        resetToGetTuningSourcesState();
        return;
    }

    QMutexLocker l(&theObjects.m_mutex);

    for (int i = 0; i < m_readTuningSignalsReply.tuningsignalstate_size(); i++)
    {
        const ::Network::TuningSignalState& tss = m_readTuningSignalsReply.tuningsignalstate(i);

        if (tss.error() != 0)
        {
            theLogFile.writeError(tr("TcpTuningClient::processReadTuningSignals, TuningSignalState error received: %1").arg(tss.error()));
            assert(tss.error() != 0);

            continue;
        }

        TuningObject* object = theObjects.objectPtrByHash(tss.signalhash());

        if (object == nullptr)
        {
            // no such signal found
            continue;
        }

        object->setValid(tss.valid());
        object->setValue(tss.value());
    }

    // increase the requested signal index, wrap the request index if needed
    //

    int objectCount = theObjects.objectCount();

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


void TcpTuningClient::processWriteTuningSignals(const QByteArray& data)
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
        theLogFile.writeError(tr("TcpTuningClient::processWriteTuningSignals, error received: %1").arg(m_writeTuningSignalsReply.error()));
        assert(m_writeTuningSignalsReply.error() != 0);

        resetToGetTuningSourcesState();
        return;
    }

    processTuningSignals();
}

void TcpTuningClient::slot_configurationArrived(ConfigSettings configuration)
{
    theLogFile.writeError(tr("TcpTuningClient::slot_configurationArrived"));

	HostAddressPort h1 = configuration.tuns1.address();
	HostAddressPort h2 = configuration.tuns1.address();

	setServers(h1, h2, true);

	return;
}

void TcpTuningClient::slot_signalsUpdated()
{
    theLogFile.writeError(tr("TcpTuningClient::slot_signalsUpdated"));

    QMutexLocker l(&m_mutex);

    m_readTuningSignalIndex = 0;
    m_readTuningSignalCount = 0;

    while (m_writeQueue.empty() == false)
    {
        m_writeQueue.pop();
    }

}

std::vector<TuningSource> TcpTuningClient::tuningSourcesInfo()
{
	std::vector<TuningSource> result;

	QMutexLocker l(&m_mutex);

	for (auto ds : m_tuningSources)
	{
		result.push_back(ds.second);
	}

	return result;
}

bool TcpTuningClient::tuningSourceInfo(quint64 id, TuningSource& result)
{
    QMutexLocker l(&m_mutex);

    auto it = m_tuningSources.find(id);

    if (it == m_tuningSources.end())
    {
        return false;
    }

    result = it->second;

    return true;
}

void TcpTuningClient::writeTuningSignal(Hash hash, double value)
{
    QMutexLocker l(&m_mutex);

    WriteCommand cmd(hash, value);

    m_writeQueue.push(cmd);
}

TcpTuningClient* theTcpTuningClient = nullptr;


