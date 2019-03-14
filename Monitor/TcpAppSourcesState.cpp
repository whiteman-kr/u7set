#include "TcpAppSourcesState.h"
#include "Settings.h"

#include "../lib/Tuning/TuningSourceState.h"

//
//TuningSource
//

AppDataSourceState::AppDataSourceState()
{
	m_perviousStateLastUpdateTime = QDateTime::currentDateTime();
}

Hash AppDataSourceState::id() const
{
	return info.id();
}

QString AppDataSourceState::equipmentId() const
{
	return info.lmequipmentid().c_str();
}

void AppDataSourceState::setNewState(const ::Network::AppDataSourceState& newState)
{
	QDateTime ct = QDateTime::currentDateTime();

	int secsTo = m_perviousStateLastUpdateTime.secsTo(ct);

	if (secsTo > m_previousStateUpdatePeriod)
	{
		m_previousState = state;
		m_perviousStateLastUpdateTime = ct;
	}

	state = newState;
}

int AppDataSourceState::getErrorsCount() const
{
	int result = 0;

	// Errors counter

	if (state.errorprotocolversion() > m_previousState.errorprotocolversion())
	{
		result++;
	}

	if (state.errorframesquantity() > m_previousState.errorframesquantity())
	{
		result++;
	}

	if (state.errorframeno() > m_previousState.errorframeno())
	{
		result++;
	}

	if (state.errordataid() > m_previousState.errordataid())
	{
		result++;
	}

	if (state.errorframesize() > m_previousState.errorframesize())
	{
		result++;
	}

	if (state.errorduplicateplanttime() > m_previousState.errorduplicateplanttime())
	{
		result++;
	}

	if (state.errornonmonotonicplanttime() > m_previousState.errornonmonotonicplanttime())
	{
		result++;
	}

	if (state.errordataid() > m_previousState.errordataid())
	{
		result++;
	}

	return result;
}

bool AppDataSourceState::valid() const
{
	return m_valid;
}

void AppDataSourceState::invalidate()
{
	m_valid = false;
}

const ::Network::AppDataSourceState& AppDataSourceState::previousState() const
{
	return m_previousState;
}

//
//
//

TcpAppSourcesState::TcpAppSourcesState(MonitorConfigController* configController, const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2) :
	Tcp::Client(configController->softwareInfo(), serverAddressPort1, serverAddressPort2),
	m_cfgController(configController)
{
	assert(m_cfgController);

	setObjectName("TcpSourcesStateClient");

	qDebug() << "TcpSourcesStateClient::TcpSourcesStateClient(...)";
}

TcpAppSourcesState::~TcpAppSourcesState()
{
	qDebug() << "TcpSourcesStateClient::~TcpSourcesStateClient()";
}

std::vector<Hash> TcpAppSourcesState::appDataSourceHashes()
{
	std::vector<Hash> result;

	QMutexLocker l(&m_appDataSourceStatesMutex);

	for (auto it : m_appDataSourceStates)
	{
		result.push_back(it.first);
	}

	return result;
}

AppDataSourceState TcpAppSourcesState::appDataSourceState(Hash id, bool* ok)
{
	QMutexLocker l(&m_appDataSourceStatesMutex);

	auto it = m_appDataSourceStates.find(id);
	if (it == m_appDataSourceStates.end())
	{
		if (ok != nullptr)
		{
			*ok = false;
		}
		return AppDataSourceState();
	}

	if (ok != nullptr)
	{
		*ok = true;
	}

	return it->second;
}

int TcpAppSourcesState::sourceErrorCount()
{
	QMutexLocker l(&m_appDataSourceStatesMutex);

	int result = 0;

	for (const auto& it : m_appDataSourceStates)
	{
		const AppDataSourceState& ads = it.second;

		if (ads.state.datareceives() == false)
		{
			result++;
			continue;
		}

		result += ads.getErrorsCount();
	}

	return result;
}

void TcpAppSourcesState::onClientThreadStarted()
{
	qDebug() << "TcpSourcesStateClient::onClientThreadStarted()";

	connect(m_cfgController, &MonitorConfigController::configurationArrived,
			this, &TcpAppSourcesState::slot_configurationArrived,
			Qt::QueuedConnection);

	return;
}

void TcpAppSourcesState::onClientThreadFinished()
{
	qDebug() << "TcpSourcesStateClient::onClientThreadFinished()";
}

void TcpAppSourcesState::onConnection()
{
	qDebug() << "TcpSourcesStateClient::onConnection()";

	assert(isClearToSendRequest() == true);

	QMutexLocker l(&m_appDataSourceStatesMutex);
	m_appDataSourceStates.clear();
	l.unlock();

	resetToGetAppDataSourcesInfo();

	return;
}

void TcpAppSourcesState::onDisconnection()
{
	qDebug() << "TcpSourcesStateClient::onDisconnection";

	{
		QMutexLocker l(&m_appDataSourceStatesMutex);

		for (auto& it : m_appDataSourceStates)
		{
			AppDataSourceState& ads = it.second;

			ads.invalidate();
		}
	}
	emit connectionReset();
}

void TcpAppSourcesState::onReplyTimeout()
{
	qDebug() << "TcpSourcesStateClient::onReplyTimeout()";
}

void TcpAppSourcesState::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
{
	if (replyData == nullptr)
	{
		assert(replyData);
		return;
	}

	QByteArray data = QByteArray::fromRawData(replyData, replyDataSize);

	switch (requestID)
	{
	case ADS_GET_APP_DATA_SOURCES_INFO:
		processAppDataSourcesInfo(data);
		break;

	case ADS_GET_APP_DATA_SOURCES_STATES:
		processAppDataSourcesState(data);
		break;

	default:
		assert(false);
		qDebug() << "Wrong requestID in TcpAppDataSourcesStateClient::processReply()";

		resetToGetAppDataSourcesInfo();
	}

	return;
}

void TcpAppSourcesState::resetToGetAppDataSourcesInfo()
{
	QThread::msleep(m_requestPeriod);

	requestAppDataSourcesInfo();

	return;
}

void TcpAppSourcesState::resetToGetAppDataSourcesState()
{
	QThread::msleep(m_requestPeriod);

	requestAppDataSourcesState();

	return;
}


void TcpAppSourcesState::requestAppDataSourcesInfo()
{
	if (isClearToSendRequest() == false)
	{
		qDebug() << tr("TcpAppDataSourcesStateClient::requestTuningSourcesInfo, isClearToSendRequest() == false, reconnecting.");
		closeConnection();
		return;
	}

	QMutexLocker l(&m_appDataSourceStatesMutex);
	m_appDataSourceStates.clear();

	sendRequest(ADS_GET_APP_DATA_SOURCES_INFO);

}

void TcpAppSourcesState::processAppDataSourcesInfo(const QByteArray& data)
{
	bool ok = m_getDataSourcesInfoReply.ParseFromArray(data.constData(), data.size());

	if (ok == false)
	{
		assert(ok);
		resetToGetAppDataSourcesInfo();
		return;
	}

	if (m_getDataSourcesInfoReply.error() != static_cast<int>(NetworkError::Success))
	{
		qDebug() << tr("TcpAppDataSourcesStateClient::m_getDataSourcesInfoReply, error received: %1")
					  .arg(getNetworkErrorStr(static_cast<NetworkError>(m_getDataSourcesInfoReply.error())));

		resetToGetAppDataSourcesInfo();
		return;
	}

	QMutexLocker l(&m_appDataSourceStatesMutex);
	m_appDataSourceStates.clear();

	for (int i = 0; i < m_getDataSourcesInfoReply.datasourceinfo_size(); i++)
	{
		const ::Network::DataSourceInfo& dsi = m_getDataSourcesInfoReply.datasourceinfo(i);

		AppDataSourceState ads;
		ads.info = dsi;

		Hash hash = ::calcHash(QString::fromStdString(ads.info.lmequipmentid()));

		assert(m_appDataSourceStates.count(hash) == 0);

		m_appDataSourceStates[hash] = ads;
	}

	resetToGetAppDataSourcesState();
}


void TcpAppSourcesState::requestAppDataSourcesState()
{
	assert(isClearToSendRequest());
	sendRequest(ADS_GET_APP_DATA_SOURCES_STATES);
}

void TcpAppSourcesState::processAppDataSourcesState(const QByteArray& data)
{
	QMutexLocker l(&m_appDataSourceStatesMutex);

	bool ok = m_getAppDataSourcesStateReply.ParseFromArray(data.constData(), data.size());

	if (ok == false)
	{
		assert(ok);
		resetToGetAppDataSourcesState();
		return;
	}

	if (m_getAppDataSourcesStateReply.error() != static_cast<int>(NetworkError::Success))
	{
		qDebug() << "TcpSourcesStateClient::processAppDataSourcesState, error received: " << m_getAppDataSourcesStateReply.error();
		assert(m_getAppDataSourcesStateReply.error() != static_cast<int>(NetworkError::Success));

		resetToGetAppDataSourcesState();
		return;
	}

	//
	for (int i = 0; i < m_getAppDataSourcesStateReply.appdatasourcesstates_size(); i++)
	{
		const ::Network::AppDataSourceState& state = m_getAppDataSourcesStateReply.appdatasourcesstates(i);

		Hash id = state.id();

		bool found = false;

		for (auto& it : m_appDataSourceStates)
		{
			AppDataSourceState& ads = it.second;

			if (ads.id() == id)
			{
				ads.setNewState(state);

				found = true;

				break;
			}
		}

		if (found == false)
		{
			assert(false);
		}
	}

	//

	resetToGetAppDataSourcesState();

	return;
}
void TcpAppSourcesState::slot_configurationArrived(ConfigSettings configuration)
{
	HostAddressPort s1 = configuration.appDataService1.address();
	HostAddressPort s2 = configuration.appDataService2.address();

	if (serverAddressPort(0) != s1 ||
		serverAddressPort(1) != s2)
	{
		setServers(s1, s2, true);
	}

	return;
}

