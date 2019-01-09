#include "TcpAppDataClient.h"


TcpAppDataClient::TcpAppDataClient(const SoftwareInfo& softwareInfo,
								   const HostAddressPort& serverAddressPort) :
	Tcp::Client(softwareInfo, serverAddressPort)
{
}


TcpAppDataClient::TcpAppDataClient(const SoftwareInfo& softwareInfo,
								   const HostAddressPort& serverAddressPort1,
								   const HostAddressPort& serverAddressPort2) :
	Tcp::Client(softwareInfo, serverAddressPort1, serverAddressPort2)
{
}


TcpAppDataClient::~TcpAppDataClient()
{
	clearDataSources();
}


void TcpAppDataClient::clearDataSources()
{
	for(DataSource* source : m_appDataSources)
	{
		delete source;
	}

	m_appDataSources.clear();
}

void TcpAppDataClient::startStateUpdating()
{
	if (m_updateStatesTimer == nullptr)
	{
		m_updateStatesTimer = new QTimer(this);
		connect(m_updateStatesTimer, &QTimer::timeout, this, &TcpAppDataClient::updateStates);
	}

	m_updateStatesTimer->start(200);
}


void TcpAppDataClient::updateStates()
{
	if (isClearToSendRequest())
	{
		sendRequest(ADS_GET_STATE);
	}
}


void TcpAppDataClient::onClientThreadStarted()
{

}


void TcpAppDataClient::onClientThreadFinished()
{

}


void TcpAppDataClient::onConnection()
{
	init();

	sendRequest(ADS_GET_APP_DATA_SOURCES_INFO);
}


void TcpAppDataClient::onDisconnection()
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

	m_signalHashes.clear();
	m_signalParams.clear();
	m_states.clear();

	emit disconnected();
}


void TcpAppDataClient::onReplyTimeout()
{
	closeConnection();
}


void TcpAppDataClient::init()
{
	m_signalHashes.clear();

	m_totalItemsCount = 0;
	m_partCount = 0;
	m_itemsPerPart = 0;
	m_currentPart = 0;

	m_getParamsCurrentPart = 0;
	m_getStatesCurrentPart = 0;
}


void TcpAppDataClient::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
{
	restartWatchdogTimer();

	switch(requestID)
	{
	// Static data
	//
	case ADS_GET_APP_DATA_SOURCES_INFO:
		onGetAppDataSourcesInfoReply(replyData, replyDataSize);
		sendNextRequest(ADS_GET_APP_DATA_SOURCES_INFO);
		break;

	case ADS_GET_UNITS:
		onGetUnitsReply(replyData, replyDataSize);
		sendNextRequest(ADS_GET_UNITS);
		break;

	case ADS_GET_APP_SIGNAL_LIST_START:
		onGetAppSignalListStartReply(replyData, replyDataSize);
		sendNextRequest(ADS_GET_APP_SIGNAL_LIST_START);
		break;

	case ADS_GET_APP_SIGNAL_LIST_NEXT:
		onGetAppSignalListNextReply(replyData, replyDataSize);
		sendNextRequest(ADS_GET_APP_SIGNAL_LIST_NEXT);
		break;

	case ADS_GET_APP_SIGNAL:
	case ADS_GET_APP_SIGNAL_PARAM:
		onGetAppSignalReply(replyData, replyDataSize);
		sendNextRequest(ADS_GET_APP_SIGNAL);
		break;

	case ADS_GET_SETTINGS:
		onGetServiceSettings(replyData, replyDataSize);
		sendNextRequest(ADS_GET_SETTINGS);
		break;

	// Dynamic data
	//
	case ADS_GET_STATE:
		onGetServiceState(replyData, replyDataSize);
		sendNextRequest(ADS_GET_STATE);
		break;

	case ADS_GET_APP_SIGNAL_STATE:
		onGetAppSignalStateReply(replyData, replyDataSize);
		sendNextRequest(ADS_GET_APP_SIGNAL_STATE);
		break;

	case ADS_GET_APP_DATA_SOURCES_STATES:
		onGetAppDataSourcesStatesReply(replyData, replyDataSize);
		sendNextRequest(ADS_GET_APP_DATA_SOURCES_STATES);
		break;

	case RQID_GET_CLIENT_LIST:
		onGetClientList(replyData, replyDataSize);
		sendNextRequest(RQID_GET_CLIENT_LIST);
		break;

	default:
		assert(false);
	}
}

QString TcpAppDataClient::configServiceConnectionState()
{
	if (m_getAppDataServiceState.cfgserviceisconnected())
	{
		QHostAddress addr(m_getAppDataServiceState.cfgserviceip());
		return addr.toString() + ":" + QString::number(m_getAppDataServiceState.cfgserviceport());
	}
	return "No";
}

QString TcpAppDataClient::archiveServiceConnectionState()
{
	if (m_getAppDataServiceState.archiveserviceisconnected())
	{
		QHostAddress addr(m_getAppDataServiceState.archiveserviceip());
		return addr.toString() + ":" + QString::number(m_getAppDataServiceState.archiveserviceport());
	}
	return "No";
}


void TcpAppDataClient::onGetAppDataSourcesInfoReply(const char* replyData, quint32 replyDataSize)
{
	bool result = m_getDataSourcesInfoReply.ParseFromArray(reinterpret_cast<const void*>(replyData), replyDataSize);

	if (result == false)
	{
		assert(false);
		return;
	}

	if (m_getDataSourcesInfoReply.error() != TO_INT(NetworkError::Success))
	{
		assert(false);
		return;
	}

	clearDataSources();

	int sourcesCount = m_getDataSourcesInfoReply.datasourceinfo_size();

	for(int i = 0; i < sourcesCount; i++)
	{
		AppDataSource* source = new AppDataSource();

		source->setInfo(m_getDataSourcesInfoReply.datasourceinfo(i));

		if (m_appDataSources.contains(source->ID()))
		{
			assert(false);
			delete source;
			continue;
		}

		m_appDataSources.insert(source->ID(), source);
	}

	emit dataSourcesInfoLoaded();
}


void TcpAppDataClient::onGetAppDataSourcesStatesReply(const char* replyData, quint32 replyDataSize)
{
	bool result = m_getAppDataSourcesStatesReply.ParseFromArray(reinterpret_cast<const void*>(replyData), replyDataSize);

	if (result == false)
	{
		assert(false);
		return;
	}

	if (m_getAppDataSourcesStatesReply.error() != TO_INT(NetworkError::Success))
	{
		assert(false);
		return;
	}

	int statesCount = m_getAppDataSourcesStatesReply.appdatasourcesstates_size();

	for(int i = 0; i < statesCount; i++)
	{
		auto id = m_getAppDataSourcesStatesReply.appdatasourcesstates(i).id();
		if (!m_appDataSources.contains(id))
		{
			assert(m_appDataSources.contains(id));
			continue;
		}

		AppDataSource* source = m_appDataSources.value(id);

		source->setState(m_getAppDataSourcesStatesReply.appdatasourcesstates(i));

		assert(source->lmEquipmentID().toStdString() == m_getAppDataSourcesStatesReply.appdatasourcesstates(i).lmequipmentid());
	}

	emit dataSoursesStateUpdated();
}


void TcpAppDataClient::onGetAppSignalListStartReply(const char* replyData, quint32 replyDataSize)
{
	bool result = m_getSignalListStartReply.ParseFromArray(reinterpret_cast<const void*>(replyData), replyDataSize);

	if (result == false)
	{
		assert(false);
		return;
	}

	m_totalItemsCount = m_getSignalListStartReply.totalitemcount();
	m_partCount = m_getSignalListStartReply.partcount();
	m_itemsPerPart = m_getSignalListStartReply.itemsperpart();
}


void TcpAppDataClient::getNextItemsPart()
{
	m_getSignalListNextRequest.Clear();

	m_getSignalListNextRequest.set_part(m_currentPart);

	sendRequest(ADS_GET_APP_SIGNAL_LIST_NEXT, m_getSignalListNextRequest);
}


void TcpAppDataClient::onGetAppSignalListNextReply(const char* replyData, quint32 replyDataSize)
{
	bool result = m_getSignalListNextReply.ParseFromArray(reinterpret_cast<const void*>(replyData), replyDataSize);

	if (result == false)
	{
		assert(false);
		return;
	}

	int stringCount = m_getSignalListNextReply.appsignalids_size();

	for(int i = 0; i < stringCount; i++)
	{
		Hash hash = calcHash(QString::fromStdString(m_getSignalListNextReply.appsignalids(i)));

		m_signalHashes.append(hash);

		m_hash2Index.insert(hash, m_signalHashes.count() - 1);
	}

	m_currentPart++;
}


void TcpAppDataClient::getNextParamPart()
{
	int startIndex = m_getParamsCurrentPart * ADS_GET_APP_SIGNAL_PARAM_MAX;

	int paramsInPartCount = m_totalItemsCount - startIndex;

	if (paramsInPartCount > ADS_GET_APP_SIGNAL_PARAM_MAX)
	{
		paramsInPartCount = ADS_GET_APP_SIGNAL_PARAM_MAX;
	}

	m_getSignalsRequest.Clear();

	for(int i = 0; i < paramsInPartCount; i++)
	{
		m_getSignalsRequest.add_signalhashes(m_signalHashes[startIndex + i]);
	}

	sendRequest(ADS_GET_APP_SIGNAL_PARAM, m_getSignalsRequest);
}


void TcpAppDataClient::onGetAppSignalReply(const char* replyData, quint32 replyDataSize)
{
	bool result = m_getSignalsReply.ParseFromArray(reinterpret_cast<const void*>(replyData), replyDataSize);

	if (result == false)
	{
		assert(false);
		return;
	}

	int paramCount = m_getSignalsReply.appsignals_size();

	int startIndex = m_getParamsCurrentPart * ADS_GET_APP_SIGNAL_PARAM_MAX;

	int signalCount = m_signalParams.count();

	for(int i = 0; i < paramCount; i++)
	{
		if (startIndex + i >= signalCount)
		{
			assert(false);
			break;
		}

		m_signalParams[startIndex + i].serializeFrom(m_getSignalsReply.appsignals(i));
	}

	m_getParamsCurrentPart++;
}


void TcpAppDataClient::getNextStatePart()
{
	if (isClearToSendRequest() == false)	// New update could be started while current is processing (waiting for reply)
	{
		return;
	}

	int startIndex = m_getStatesCurrentPart * ADS_GET_APP_SIGNAL_STATE_MAX;

	int paramsInPartCount = m_totalItemsCount - startIndex;

	if (paramsInPartCount > ADS_GET_APP_SIGNAL_STATE_MAX)
	{
		paramsInPartCount = ADS_GET_APP_SIGNAL_STATE_MAX;
	}

	m_getSignalStateRequest.Clear();

	for(int i = 0; i < paramsInPartCount; i++)
	{
		m_getSignalStateRequest.add_signalhashes(m_signalHashes[startIndex + i]);
	}

	sendRequest(ADS_GET_APP_SIGNAL_STATE, m_getSignalStateRequest);
}

void TcpAppDataClient::onGetClientList(const char *replyData, quint32 replyDataSize)
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


void TcpAppDataClient::onGetAppSignalStateReply(const char* replyData, quint32 replyDataSize)
{
	bool result = m_getSignalStateReply.ParseFromArray(reinterpret_cast<const void*>(replyData), replyDataSize);

	if (result == false)
	{
		assert(false);
		return;
	}

	int stateCount = m_getSignalStateReply.appsignalstates_size();

	int startIndex = m_getStatesCurrentPart * ADS_GET_APP_SIGNAL_STATE_MAX;

	int signalCount = m_signalParams.count();

	for(int i = 0; i < stateCount; i++)
	{
		if (startIndex + i >= signalCount)
		{
			assert(false);
			break;
		}

		Hash hash = m_getSignalStateReply.appsignalstates(i).hash();

		if (m_hash2Index.contains(hash) == false)
		{
			assert(false);
			continue;
		}

		int index = m_hash2Index[hash];

		m_states[index].load(m_getSignalStateReply.appsignalstates(i));
	}

	m_getStatesCurrentPart++;
}

void TcpAppDataClient::onGetUnitsReply(const char* replyData, quint32 replyDataSize)
{
	bool result = m_getUnitsReply.ParseFromArray(reinterpret_cast<const void*>(replyData), replyDataSize);

	if (result == false)
	{
		assert(false);
		return;
	}
}

void TcpAppDataClient::onGetServiceState(const char* replyData, quint32 replyDataSize)
{
	bool result = m_getAppDataServiceState.ParseFromArray(reinterpret_cast<const void*>(replyData), replyDataSize);

	if (result == false)
	{
		assert(false);
		return;
	}

	m_stateIsReady = true;
	emit stateLoaded();
}

void TcpAppDataClient::onGetServiceSettings(const char* replyData, quint32 replyDataSize)
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

void TcpAppDataClient::sendNextRequest(quint32 processedRequestID)
{
	switch(processedRequestID)
	{
	// Static data requests
	//
	case ADS_GET_APP_DATA_SOURCES_INFO:
		sendRequest(ADS_GET_UNITS);
		break;

	case ADS_GET_UNITS:
		sendRequest(ADS_GET_APP_SIGNAL_LIST_START);
		break;

	case ADS_GET_APP_SIGNAL_LIST_START:
		if (m_partCount > 0)
		{
			m_hash2Index.clear();
			m_hash2Index.reserve(m_signalHashes.count() * 1.3);

			m_currentPart = 0;
			getNextItemsPart();
		}
		else
		{
			sendRequest(ADS_GET_SETTINGS);
		}
		break;

	case ADS_GET_APP_SIGNAL_LIST_NEXT:
		if (m_currentPart >= m_partCount)
		{
			// all hashes received
			//
			m_signalParams.resize(m_signalHashes.count());
			m_getParamsCurrentPart = 0;
			getNextParamPart();
		}
		else
		{
			getNextItemsPart();
		}
		break;

	case ADS_GET_APP_SIGNAL_PARAM:	// Is same as ADS_GET_APP_SIGNAL
	case ADS_GET_APP_SIGNAL:
		{
			int paramsTotalParts = m_totalItemsCount / ADS_GET_APP_SIGNAL_PARAM_MAX +
					((m_totalItemsCount % ADS_GET_APP_SIGNAL_PARAM_MAX) == 0 ? 0 : 1);

			if (m_getParamsCurrentPart >= paramsTotalParts)
			{
				m_states.resize(m_signalParams.count());

				emit appSignalListLoaded();

				sendRequest(ADS_GET_SETTINGS);
			}
			else
			{
				getNextParamPart();
			}
		}
		break;

	case ADS_GET_SETTINGS:
		startStateUpdating();
		updateStates();	// First time update immediately, while waiting for timer
		break;

	// Dynamic data requests
	//
	case ADS_GET_STATE:
		m_getStatesCurrentPart = 0;
		getNextStatePart();
		break;

	case ADS_GET_APP_SIGNAL_STATE:
		{
			int statesTotalParts = m_totalItemsCount / ADS_GET_APP_SIGNAL_STATE_MAX +
					((m_totalItemsCount % ADS_GET_APP_SIGNAL_STATE_MAX) == 0 ? 0 : 1);

			if (m_getStatesCurrentPart >= statesTotalParts)
			{
				emit appSignalsStateUpdated();

				m_getStatesCurrentPart = 0;
				sendRequest(ADS_GET_APP_DATA_SOURCES_STATES);
			}
			else
			{
				getNextStatePart();
			}
		}
		break;

	case ADS_GET_APP_DATA_SOURCES_STATES:
		sendRequest(RQID_GET_CLIENT_LIST);
		break;

	case RQID_GET_CLIENT_LIST:
		// Last request - do nothing
		break;

	default:
		assert(false);
	}
}
