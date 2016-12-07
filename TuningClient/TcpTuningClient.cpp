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

	{
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
	}

    //if (false)
    //{
        // if there is a queued data to write something, write it.
        //
    //}
    //else
    {
        // read tuning signals state
        //
        //requestReadTuningSignals();
        resetToGetTuningSourcesState();
    }

	return;
}

void TcpTuningClient::requestReadTuningSignals()
{
    const int READ_TUNING_SIGNALS_MAX = 100;

    // if no signals in the database, start the new request loop
    //
    QMutexLocker l(&theObjects.m_mutex);

    int objectCount = theObjects.objectCount();

    if (objectCount == 0)
    {
        resetToGetTuningSourcesState();
        return;
    }

    // wrap the request index if needed
    //

    if (m_readTuningSignalIndex >= objectCount)
    {
        m_readTuningSignalIndex  = 0;
    }

    // determine the amount of signals needed to be requested
    //

    int readTuningSignalCount = READ_TUNING_SIGNALS_MAX;

    if (m_readTuningSignalIndex + readTuningSignalCount >= objectCount)
    {
        readTuningSignalCount = objectCount - m_readTuningSignalIndex;
    }

    // create the request
    //

    assert(isClearToSendRequest());

    m_readTuningSignals.set_clientequipmentid(theSettings.instanceStrId().toUtf8());

    m_readTuningSignals.mutable_signalhash()->Reserve(READ_TUNING_SIGNALS_MAX);

    m_readTuningSignals.mutable_signalhash()->Clear();

    for (int i = 0; i < readTuningSignalCount; i++)
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

    //sendRequest(TDS_GET_TUNING_SOURCES_INFO, m_readTuningSignals);

    // increase the requested signal index
    //

    m_readTuningSignalIndex += readTuningSignalCount;

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

    {
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
                assert(object);
                continue;
            }

            object->setValid(tss.valid());
            object->setValue(tss.value());
        }

    }

    //if (false)
    //{
        // if there is a queued data to write something, write it.
        //
    //}
    //else
    {
        // read tuning signals state
        //
        requestReadTuningSignals();
    }
}


void TcpTuningClient::slot_configurationArrived(ConfigSettings configuration)
{
	HostAddressPort h1 = configuration.tuns1.address();
	HostAddressPort h2 = configuration.tuns1.address();

	setServers(h1, h2, true);

	return;
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

TcpTuningClient* theTcpTuningClient = nullptr;


