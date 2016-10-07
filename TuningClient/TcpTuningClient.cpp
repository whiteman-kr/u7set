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

void TcpTuningClient::resetToGetTuningState()
{
	QThread::msleep(theSettings.m_requestInterval);

	requestTuningSourcesState();
	return;
}

void TcpTuningClient::requestTuningSourcesInfo()
{
	assert(isClearToSendRequest());
	sendRequest(TDS_GET_TUNING_SOURCES_INFO);
}

void TcpTuningClient::requestTuningSourcesState()
{
	assert(isClearToSendRequest());
	sendRequest(TDS_GET_TUNING_SOURCES_STATES);
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

			ts.m_id = dsi.id();
			ts.m_equipmentId = dsi.equipmentid().c_str();
			ts.m_caption = dsi.caption().c_str();
			ts.m_dataType = dsi.datatype();
			ts.m_ip = dsi.ip().c_str();
			ts.m_port = dsi.port();
			ts.m_channel = dsi.channel();
			ts.m_subsystemID = dsi.subsystemid();
			ts.m_subsystem = dsi.subsystem().c_str();

			ts.m_lmNumber = dsi.lmnumber();
			ts.m_lmModuleType = dsi.lmmoduletype();
			ts.m_lmAdapterID = dsi.lmadapterid().c_str();
			ts.m_lmDataEnable = dsi.lmdataenable();
			ts.m_lmDataID = dsi.lmdataid();

			quint64 id = dsi.id();

			if (m_tuningSources.find(id) != m_tuningSources.end())
			{
				// id is not unique
				assert(false);
				continue;
			}

			qDebug()<<"Id = "<<id;
			qDebug()<<"m_equipmentId = "<<ts.m_equipmentId;
			qDebug()<<"m_ip = "<<ts.m_ip;


			m_tuningSources[id] = ts;
		}
	}

	resetToGetTuningState();

	emit tuningSourcesArrived();

	return;
}

void TcpTuningClient::processTuningSourcesState(const QByteArray& data)
{

	bool ok = m_tuningDataSourcesStatesReply.ParseFromArray(data.constData(), data.size());

	if (ok == false)
	{
		assert(ok);
		resetToGetTuningSources();
		return;
	}

	if (m_tuningDataSourcesStatesReply.error() != 0)
	{
		theLogFile.writeError(tr("TcpTuningClient::processTuningSourcesState, error received: %1").arg(m_tuningDataSourcesStatesReply.error()));
		assert(m_tuningDataSourcesStatesReply.error() != 0);

		resetToGetTuningSources();
		return;
	}

	{
		QMutexLocker l(&m_mutex);

		for (int i = 0; i < m_tuningDataSourcesStatesReply.tuningdatasourcesstates_size(); i++)
		{
			const ::Network::TuningSourceState& tss = m_tuningDataSourcesStatesReply.tuningdatasourcesstates(i);

			quint64 id = tss.id();

			auto it = m_tuningSources.find(id);
			if (it == m_tuningSources.end())
			{
				// no id found
				assert(false);
				continue;
			}

			TuningSource& ts = it->second;

			ts.m_uptime = tss.uptime();
			ts.m_receivedDataSize = tss.receiveddatasize();
			ts.m_dataReceivingRate = tss.datareceivingrate();
			ts.m_respond = tss.respond();
		}
	}

	resetToGetTuningState();

	return;
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

TcpTuningClient* theTcpTuningClient = nullptr;

ObjectManager theObjects;

