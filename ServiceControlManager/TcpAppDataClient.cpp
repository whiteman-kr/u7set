#include "TcpAppDataClient.h"
#include "version.h"


TcpAppDataClient::TcpAppDataClient(const HostAddressPort& serverAddressPort,
								   const Tcp::SoftwareInfo& softwareInfo) :
	Tcp::Client(serverAddressPort, softwareInfo)
{
}


TcpAppDataClient::TcpAppDataClient(const HostAddressPort& serverAddressPort1,
								   const HostAddressPort& serverAddressPort2,
								   const Tcp::SoftwareInfo& softwareInfo) :
	Tcp::Client(serverAddressPort1, serverAddressPort2, softwareInfo)
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


void TcpAppDataClient::updateStates()
{
	m_getStatesCurrentPart = 0;
	getNextStatePart();
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

	sendRequest(ADS_GET_DATA_SOURCES_INFO);
}


void TcpAppDataClient::onDisconnection()
{
	if (m_updateStatesTimer != nullptr)
	{
		m_updateStatesTimer->stop();
	}

	m_signalHahes.clear();
	m_signalParams.clear();
	m_states.clear();
	emit disconnected();
}


void TcpAppDataClient::onReplyTimeout()
{

}


void TcpAppDataClient::init()
{
	m_signalHahes.clear();

	m_totalItemsCount = 0;
	m_partCount = 0;
	m_itemsPerPart = 0;
	m_currentPart = 0;

	m_getParamsCurrentPart = 0;
	m_getStatesCurrentPart = 0;
}


void TcpAppDataClient::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
{
	switch(requestID)
	{
	case ADS_GET_DATA_SOURCES_INFO:
		onGetDataSourcesInfoReply(replyData, replyDataSize);
		break;

	case ADS_GET_DATA_SOURCES_STATES:
		onGetDataSourcesStatesReply(replyData, replyDataSize);
		break;

	case ADS_GET_APP_SIGNAL_LIST_START:
		onGetAppSignalListStartReply(replyData, replyDataSize);
		break;

	case ADS_GET_APP_SIGNAL_LIST_NEXT:
		onGetAppSignalListNextReply(replyData, replyDataSize);
		break;

	case ADS_GET_APP_SIGNAL:
		onGetAppSignalReply(replyData, replyDataSize);
		break;

	case ADS_GET_APP_SIGNAL_STATE:
		onGetAppSignalStateReply(replyData, replyDataSize);
		break;

	case ADS_GET_UNITS:
		onGetUnitsReply(replyData, replyDataSize);
		break;

	case ADS_GET_APP_SIGNAL_PARAM:
		break;

	default:
		assert(false);
	}
}


void TcpAppDataClient::onGetDataSourcesInfoReply(const char* replyData, quint32 replyDataSize)
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

	sendRequest(ADS_GET_UNITS);
}


void TcpAppDataClient::onGetDataSourcesStatesReply(const char* replyData, quint32 replyDataSize)
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

	if (m_partCount > 0)
	{
		m_hash2Index.clear();
		m_hash2Index.reserve(m_signalHahes.count() * 1.3);

		m_currentPart = 0;
		getNextItemsPart();
	}
}


void TcpAppDataClient::getNextItemsPart()
{
	if (m_currentPart >= m_partCount)
	{
		// all hashes received
		//
		m_signalParams.resize(m_signalHahes.count());
		m_getParamsCurrentPart = 0;
		getNextParamPart();
		return;
	}

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

		m_signalHahes.append(hash);

		m_hash2Index.insert(hash, m_signalHahes.count() - 1);
	}

	m_currentPart++;

	getNextItemsPart();
}


void TcpAppDataClient::getNextParamPart()
{
	int paramsTotalParts = m_totalItemsCount / ADS_GET_APP_SIGNAL_PARAM_MAX +
							((m_totalItemsCount % ADS_GET_APP_SIGNAL_PARAM_MAX) == 0 ? 0 : 1);

	if (m_getParamsCurrentPart >= paramsTotalParts)
	{
		m_states.resize(m_signalParams.count());

		emit appSignalListLoaded();

		if (m_updateStatesTimer == nullptr)
		{
			m_updateStatesTimer = new QTimer(this);
			connect(m_updateStatesTimer, &QTimer::timeout, this, &TcpAppDataClient::updateStates);
		}

		m_updateStatesTimer->start(200);
		return;
	}

	int startIndex = m_getParamsCurrentPart * ADS_GET_APP_SIGNAL_PARAM_MAX;

	int paramsInPartCount = m_totalItemsCount - startIndex;

	if (paramsInPartCount > ADS_GET_APP_SIGNAL_PARAM_MAX)
	{
		paramsInPartCount = ADS_GET_APP_SIGNAL_PARAM_MAX;
	}

	m_getSignalsRequest.Clear();

	for(int i = 0; i < paramsInPartCount; i++)
	{
		m_getSignalsRequest.add_signalhashes(m_signalHahes[startIndex + i]);
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

	getNextParamPart();
}


void TcpAppDataClient::getNextStatePart()
{
	if (isClearToSendRequest() == false)
	{
		return;
	}

	int statesTotalParts = m_totalItemsCount / ADS_GET_APP_SIGNAL_STATE_MAX +
							((m_totalItemsCount % ADS_GET_APP_SIGNAL_STATE_MAX) == 0 ? 0 : 1);

	if (m_getStatesCurrentPart >= statesTotalParts)
	{
		m_getStatesCurrentPart = 0;
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
		m_getSignalStateRequest.add_signalhashes(m_signalHahes[startIndex + i]);
	}

	sendRequest(ADS_GET_APP_SIGNAL_STATE, m_getSignalStateRequest);
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

	m_getParamsCurrentPart++;

	if (startIndex + stateCount >= signalCount)
	{
		emit appSignalsStateUpdated();

		sendRequest(ADS_GET_DATA_SOURCES_STATES);
		return;
	}

	getNextStatePart();
}

void TcpAppDataClient::onGetUnitsReply(const char* replyData, quint32 replyDataSize)
{
	bool result = m_getUnitsReply.ParseFromArray(reinterpret_cast<const void*>(replyData), replyDataSize);

	if (result == false)
	{
		assert(false);
		return;
	}

	sendRequest(ADS_GET_APP_SIGNAL_LIST_START);
}
