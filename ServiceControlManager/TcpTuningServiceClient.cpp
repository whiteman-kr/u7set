#include "TcpTuningServiceClient.h"
#include "version.h"


TcpTuningServiceClient::TcpTuningServiceClient(const SoftwareInfo& softwareInfo,
											   const HostAddressPort& serverAddressPort) :
	Tcp::Client(softwareInfo, serverAddressPort)
{
}


TcpTuningServiceClient::TcpTuningServiceClient(const SoftwareInfo& softwareInfo,
											   const HostAddressPort& serverAddressPort1,
											   const HostAddressPort& serverAddressPort2) :
	Tcp::Client(softwareInfo, serverAddressPort1, serverAddressPort2)
{
}


TcpTuningServiceClient::~TcpTuningServiceClient()
{
}


void TcpTuningServiceClient::startStateUpdating()
{
	if (m_updateStatesTimer == nullptr)
	{
		m_updateStatesTimer = new QTimer(this);
		connect(m_updateStatesTimer, &QTimer::timeout, this, &TcpTuningServiceClient::updateStates);
	}

	m_updateStatesTimer->start(200);
}


void TcpTuningServiceClient::updateStates()
{
	if (isClearToSendRequest())
	{
		m_clientsIsReady = false;
		m_stateIsReady = false;

		m_tuningSourcesStateIsReady = false;

		sendRequest(TDS_GET_TUNING_SOURCES_STATES, m_getTuningSourcesStates);	// Check for tuning sources availability
	}
}


void TcpTuningServiceClient::onClientThreadStarted()
{

}


void TcpTuningServiceClient::onClientThreadFinished()
{

}


void TcpTuningServiceClient::onConnection()
{
	init();

	sendRequest(TDS_GET_TUNING_SOURCES_INFO, m_getTuningSourcesInfo);
}


void TcpTuningServiceClient::onDisconnection()
{
	if (m_updateStatesTimer != nullptr)
	{
		m_updateStatesTimer->stop();

		delete m_updateStatesTimer;

		m_updateStatesTimer = nullptr;
	}

	m_clientsIsReady = false;
	m_settingsIsReady = false;
	m_stateIsReady = false;

	m_tuningSourcesInfoIsReady = false;
	m_tuningSourcesStateIsReady = false;

	emit disconnected();
}


void TcpTuningServiceClient::onReplyTimeout()
{
	closeConnection();
}


void TcpTuningServiceClient::init()
{
	m_clientsIsReady = false;
	m_settingsIsReady = false;
	m_stateIsReady = false;

	m_tuningSourcesInfoIsReady = false;
	m_tuningSourcesStateIsReady = false;
}


void TcpTuningServiceClient::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
{
	restartWatchdogTimer();

	switch(requestID)
	{
	// Static data
	//
	case TDS_GET_TUNING_SOURCES_INFO:
		onGetTuningSourcesInfo(replyData, replyDataSize);
		sendRequest(TDS_GET_TUNING_SERVICE_SETTINGS);
		break;

	case TDS_GET_TUNING_SERVICE_SETTINGS:
		onGetServiceSettings(replyData, replyDataSize);
		startStateUpdating();
		updateStates();
		break;

	// Dynamic data
	//
	case TDS_GET_TUNING_SOURCES_STATES:
		onGetTuningSourcesStates(replyData, replyDataSize);
		sendRequest(RQID_GET_CLIENT_LIST);
		break;

	case RQID_GET_CLIENT_LIST:
		onGetClientList(replyData, replyDataSize);
		break;

	default:
		assert(false);
	}
}

void TcpTuningServiceClient::onGetClientList(const char *replyData, quint32 replyDataSize)
{
	bool result = m_serviceClientsMessage.ParseFromArray(replyData, replyDataSize);

	if (result == false)
	{
		assert(false);
		return;
	}

	m_clientsIsReady = true;
	emit clientsLoaded();
}

void TcpTuningServiceClient::onGetServiceSettings(const char* replyData, quint32 replyDataSize)
{
	bool result = m_getServiceSettings.ParseFromArray(replyData, replyDataSize);

	if (result == false)
	{
		assert(false);
		return;
	}

	m_equipmentID = QString::fromStdString(m_getServiceSettings.equipmentid());
	m_configIP1 = QString::fromStdString(m_getServiceSettings.configip1());
	m_configIP2 = QString::fromStdString(m_getServiceSettings.configip2());

	m_settingsIsReady = true;
	emit settingsLoaded();
}

void TcpTuningServiceClient::onGetTuningSourcesInfo(const char *replyData, quint32 replyDataSize)
{
	bool ok = m_tuningSourcesInfoReply.ParseFromArray(replyData, replyDataSize);

	if (ok == false)
	{
		assert(ok);
		closeConnection();
		return;
	}

	if (m_tuningSourcesInfoReply.error() != 0)
	{
		assert(false);
		closeConnection();
		return;
	}

	m_tuningSources.clear();

	for (int i = 0; i < m_tuningSourcesInfoReply.tuningsourceinfo_size(); i++)
	{
		const ::Network::DataSourceInfo& dsi = m_tuningSourcesInfoReply.tuningsourceinfo(i);

		bool isAlreadyExists = false;

		for (const TuningSource& ts : m_tuningSources)
		{
			if (ts.id() == dsi.id() && ts.equipmentId() == QString::fromStdString(dsi.equipmentid()))
			{
				isAlreadyExists = true;
				break;
			}
		}

		if (isAlreadyExists == true)
		{
			continue;
		}

		TuningSource ts;
		ts.info = dsi;

		m_tuningSources.push_back(ts);
	}

	m_tuningSourcesInfoIsReady = true;
	emit tuningSourcesInfoLoaded();
}

void TcpTuningServiceClient::onGetTuningSourcesStates(const char *replyData, quint32 replyDataSize)
{
	bool ok = m_tuningSourcesStatesReply.ParseFromArray(replyData, replyDataSize);

	if (ok == false)
	{
		assert(ok);
		closeConnection();
		return;
	}

	if (m_tuningSourcesStatesReply.error() != 0)
	{
		assert(false);
		closeConnection();
		return;
	}

	for (int i = 0; i < m_tuningSourcesStatesReply.tuningsourcesstate_size(); i++)
	{
		const ::Network::TuningSourceState& tss = m_tuningSourcesStatesReply.tuningsourcesstate(i);

		quint64 id = tss.sourceid();

		bool found = false;

		for (auto& ts : m_tuningSources)
		{
			if (ts.id() == id)
			{
				ts.setNewState(tss);

				found = true;
				break;
			}
		}

		assert(found == true);
	}

	m_tuningSourcesStateIsReady = true;
	emit tuningSoursesStateUpdated();
}

