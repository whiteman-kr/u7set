#include "TcpTuningServiceClient.h"


TcpTuningServiceClient::TcpTuningServiceClient(const SoftwareInfo& softwareInfo,
											   const HostAddressPort& serverAddressPort) :
	Tcp::Client(softwareInfo, serverAddressPort, "TcpTuningServiceClient")
{
}


TcpTuningServiceClient::TcpTuningServiceClient(const SoftwareInfo& softwareInfo,
											   const HostAddressPort& serverAddressPort1,
											   const HostAddressPort& serverAddressPort2) :
	Tcp::Client(softwareInfo, serverAddressPort1, serverAddressPort2, "TcpTuningServiceClient")
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
		m_tuningSignalsStateIsReady = false;

		m_updatedSignalStateQuantity = 0;

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

	m_loadedSignalParamQuantity = 0;

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
		sendRequest(TDS_GET_TUNING_SOURCE_FILLING);
		break;

	case TDS_GET_TUNING_SOURCE_FILLING:
		onGetTuningSourceFilling(replyData, replyDataSize);
		if (m_loadedSignalParamQuantity < m_signalHashes.size())
		{
			orderTuningSignalParamPortion();
		}
		else
		{
			startStateUpdating();
			updateStates();
		}
		break;

	case TDS_GET_TUNING_SIGNAL_PARAM:
		onGetTuningSignalParam(replyData, replyDataSize);
		if (m_loadedSignalParamQuantity < m_signalHashes.size())
		{
			orderTuningSignalParamPortion();
		}
		else
		{
			startStateUpdating();
			updateStates();
		}
		break;

	// Dynamic data
	//
	case TDS_GET_TUNING_SOURCES_STATES:
		onGetTuningSourcesStates(replyData, replyDataSize);
		if (m_updatedSignalStateQuantity < m_signalHashes.size())
		{
			orderTuningSignalStatePortion();
		}
		else
		{
			sendRequest(RQID_GET_CLIENT_LIST);
		}
		break;

	case TDS_TUNING_SIGNALS_READ:
		onGetTuningSignalState(replyData, replyDataSize);
		if (m_updatedSignalStateQuantity < m_signalHashes.size())
		{
			orderTuningSignalStatePortion();
		}
		else
		{
			sendRequest(RQID_GET_CLIENT_LIST);
		}
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
			if (ts.id() == dsi.id() && ts.equipmentId() == QString::fromStdString(dsi.lmequipmentid()))
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

void TcpTuningServiceClient::onGetTuningSourceFilling(const char *replyData, quint32 replyDataSize)
{
	m_getTuningSourceFillingReply.ParseFromArray(replyData, replyDataSize);

	if (m_getTuningSourceFillingReply.signalcount() == 0)
	{
		assert(false);
		return;
	}

	m_signalsSourceID.resize(static_cast<int>(m_getTuningSourceFillingReply.signalcount()));

	int totalSignalQuantity = 0;

	for (int i = 0; i < m_getTuningSourceFillingReply.signalspersource_size(); i++)
	{
		totalSignalQuantity += m_getTuningSourceFillingReply.signalspersource(i).signalhash_size();
	}

	m_signalHashes.resize(totalSignalQuantity);
	m_signalHash2SignalIndex.reserve(totalSignalQuantity);
	m_signalsSourceID.resize(totalSignalQuantity);
	m_signals.resize(totalSignalQuantity);
	m_tuningSignalState.resize(totalSignalQuantity);

	m_loadedSignalParamQuantity = 0;
	int currentSignalIndex = 0;

	for (int i = 0; i < m_getTuningSourceFillingReply.signalspersource_size(); i++)
	{
		const Network::SignalsAssociatedToTuningSource& satts = m_getTuningSourceFillingReply.signalspersource(i);

		quint64 sourceID = satts.sourceid();

		for (int j = 0; j < satts.signalhash_size(); j++)
		{
			quint64 signalHash = satts.signalhash(j);

			m_signalHashes[currentSignalIndex] = signalHash;
			m_signalHash2SignalIndex.insert(signalHash, currentSignalIndex);
			m_signalsSourceID[currentSignalIndex] = sourceID;

			currentSignalIndex++;
		}
	}
}

void TcpTuningServiceClient::orderTuningSignalParamPortion()
{
	m_getAppSignalParamRequest.Clear();

	int signalsLeft = std::min(ADS_GET_APP_SIGNAL_PARAM_MAX, m_signalHashes.size() - m_loadedSignalParamQuantity);

	if (signalsLeft <= 0)
	{
		assert(false);
		return;
	}

	for (int i = 0; i < signalsLeft; i++)
	{
		m_getAppSignalParamRequest.add_signalhashes(m_signalHashes[m_loadedSignalParamQuantity + i]);
	}

	sendRequest(TDS_GET_TUNING_SIGNAL_PARAM, m_getAppSignalParamRequest);
}

void TcpTuningServiceClient::onGetTuningSignalParam(const char *replyData, quint32 replyDataSize)
{
	bool result = m_getAppSignalParamReply.ParseFromArray(reinterpret_cast<const void*>(replyData), replyDataSize);

	if (result == false)
	{
		assert(false);
		return;
	}

	assert(m_getAppSignalParamReply.has_error() == false);

	int receivedSignalQuantity = m_getAppSignalParamReply.appsignals_size();

	Signal signal;

	for (int i = 0; i < receivedSignalQuantity; i++)
	{
		Hash signalHash = m_getAppSignalParamReply.appsignals(i).calcparam().hash();

		int signalIndex = m_signalHash2SignalIndex.value(signalHash);

		if (signalIndex < 0 || signalIndex >= m_signalHashes.size())
		{
			assert(false);
			return;
		}

		assert(signalHash == m_signalHashes[signalIndex]);

		m_signals[signalIndex].serializeFrom(m_getAppSignalParamReply.appsignals(i));

		assert(m_signals[signalIndex].hash() == m_signalHashes[signalIndex]);
	}

	m_loadedSignalParamQuantity += receivedSignalQuantity;

	if (m_loadedSignalParamQuantity == m_signalHashes.size())
	{
		m_tuningSignalsInfoIsReady = true;
		emit tuningSignalsInfoLoaded();
	}
}

void TcpTuningServiceClient::orderTuningSignalStatePortion()
{
	m_getTuningSignalStateRequest.Clear();

	int signalsLeft = std::min(TDS_TUNING_MAX_READ_STATES, m_signalHashes.size() - m_updatedSignalStateQuantity);

	if (signalsLeft <= 0)
	{
		assert(false);
		return;
	}

	for (int i = 0; i < signalsLeft; i++)
	{
		m_getTuningSignalStateRequest.add_signalhash(m_signalHashes[m_updatedSignalStateQuantity + i]);
	}

	sendRequest(TDS_TUNING_SIGNALS_READ, m_getTuningSignalStateRequest);
}

void TcpTuningServiceClient::onGetTuningSignalState(const char *replyData, quint32 replyDataSize)
{
	bool result = m_getTuningSignalStateReply.ParseFromArray(reinterpret_cast<const void*>(replyData), replyDataSize);

	if (result == false)
	{
		assert(false);
		return;
	}

	assert(m_getTuningSignalStateReply.has_error() == false || m_getTuningSignalStateReply.error() == TO_INT(NetworkError::Success));

	int receivedSignalQuantity = m_getTuningSignalStateReply.tuningsignalstate_size();

	for (int i = 0; i < receivedSignalQuantity; i++)
	{
		Hash signalHash = m_getTuningSignalStateReply.tuningsignalstate(i).signalhash();

		int signalIndex = m_signalHash2SignalIndex.value(signalHash);

		if (signalIndex < 0 || signalIndex >= m_signalHashes.size())
		{
			assert(false);
			return;
		}

		assert(signalHash == m_signalHashes[signalIndex]);

		m_tuningSignalState[signalIndex].setState(m_getTuningSignalStateReply.tuningsignalstate(i));
	}

	m_updatedSignalStateQuantity += receivedSignalQuantity;

	if (m_updatedSignalStateQuantity == m_signalHashes.size())
	{
		m_tuningSignalsStateIsReady = true;
		emit tuningSignalsStateUpdated();
	}
}

