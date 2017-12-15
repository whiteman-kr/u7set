#include "TcpSignalRecents.h"
#include "Settings.h"
#include "version.h"


RecentUsed::RecentUsed(int maxSize /*= 750*/) :
	m_maxSize(maxSize)
{
}

void RecentUsed::add(Hash hash)
{
	qint64 now = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();

	auto it = m_signalToTile.find(hash);
	if (it == m_signalToTile.end())
	{
		if (m_signalToTile.size() >= m_maxSize)
		{
			auto lastTimeIt = m_timeToSignal.begin();

			m_signalToTile.erase(lastTimeIt->second);
			m_timeToSignal.erase(lastTimeIt);
		}

		m_signalToTile.insert({hash, now});
		m_timeToSignal.insert({now, hash});
	}
	else
	{
		// Update add time
		//
		qint64 itemTime = it->second;
		bool updated = false;

		auto range = m_timeToSignal.equal_range(itemTime);
		for (auto th = range.first; th != range.second; ++th)
		{
			if (th->second == hash)
			{
				m_timeToSignal.erase(th);
				m_timeToSignal.insert({now, hash});
				updated = true;
				break;
			}
		}
		assert(updated == true);

		it->second = now;
	}

	assert(m_signalToTile.size() == m_timeToSignal.size());
	return;
}

void RecentUsed::add(const QVector<Hash>& hashes)
{
	for (Hash hash : hashes)
	{
		add(hash);
	}

	return;
}

bool RecentUsed::remove(Hash hash)
{
	auto it = m_signalToTile.find(hash);
	if (it == m_signalToTile.end())
	{
		return false;
	}

	qint64 itemTime = it->second;
	bool removedFromTimeMap = false;

	auto range = m_timeToSignal.equal_range(itemTime);
	for (auto th = range.first; th != range.second; ++th)
	{
		if (th->second == hash)
		{
			m_timeToSignal.erase(th);
			removedFromTimeMap = true;
			break;
		}
	}
	assert(removedFromTimeMap == true);

	m_signalToTile.erase(it);

	assert(m_signalToTile.size() == m_timeToSignal.size());
	return true;
}

bool RecentUsed::remove(const std::vector<Hash>& hashes)
{
	bool ok = true;
	for (Hash hash : hashes)
	{
		ok &= remove(hash);
	}

	return ok;
}

int RecentUsed::size() const
{
	return static_cast<int>(m_signalToTile.size());
}

const std::map<Hash, qint64>& RecentUsed::rawHashes() const
{
	return m_signalToTile;
}

std::vector<Hash> RecentUsed::hashes() const
{
	std::vector<Hash> result;
	result.reserve(m_signalToTile.size());

	for (auto p : m_signalToTile)
	{
		result.push_back(p.first);
	}

	return result;
}



TcpSignalRecents::TcpSignalRecents(MonitorConfigController* configController, const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2) :
	Tcp::Client(configController->softwareInfo(), serverAddressPort1, serverAddressPort2),
	m_cfgController(configController)
{
	assert(m_cfgController);
	qDebug() << "TcpSignalRecents::TcpSignalRecents(...)";
}

TcpSignalRecents::~TcpSignalRecents()
{
	qDebug() << "TcpSignalRecents::~TcpSignalRecents()";
}


void TcpSignalRecents::onClientThreadStarted()
{
	qDebug() << "TcpSignalRecents::onClientThreadStarted()";

	connect(m_cfgController, &MonitorConfigController::configurationArrived,
			this, &TcpSignalRecents::slot_configurationArrived,
			Qt::QueuedConnection);

	return;
}

void TcpSignalRecents::onClientThreadFinished()
{
	qDebug() << "TcpSignalRecents::onClientThreadFinished()";

	theSignals.reset();
}

void TcpSignalRecents::onConnection()
{
	qDebug() << "TcpSignalRecents::onConnection()";

	assert(isClearToSendRequest() == true);

	requestSignalState();

	return;
}

void TcpSignalRecents::onDisconnection()
{
	qDebug() << "TcpSignalRecents::onDisconnection";
}

void TcpSignalRecents::onReplyTimeout()
{
	qDebug() << "TcpSignalRecents::onReplyTimeout()";
}

void TcpSignalRecents::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
{
	if (replyData == nullptr)
	{
		assert(replyData);
		return;
	}

	QByteArray data = QByteArray::fromRawData(replyData, replyDataSize);

	switch (requestID)
	{

	case ADS_GET_APP_SIGNAL_STATE:
		processSignalState(data);
		break;

	default:
		assert(false);
		qDebug() << "Wrong requestID in TcpSignalRecents::processReply()";

		requestSignalState();
	}

	return;
}

void TcpSignalRecents::addSignal(Hash hash)
{
	m_recents.add(hash);
	return;
}

void TcpSignalRecents::addSignals(QVector<Hash> hashes)
{
	m_recents.add(hashes);
	return;
}

// AppSignalState
//
void TcpSignalRecents::requestSignalState()
{
	QThread::msleep(100);

	assert(isClearToSendRequest());

	const std::map<Hash, qint64>& recentRecords = m_recents.rawHashes();
	if (recentRecords.empty() == true)
	{
		QThread::yieldCurrentThread();
	}

	std::vector<Hash> hashesToRemove;
	hashesToRemove.reserve(recentRecords.size());

	if (recentRecords.size() > ADS_GET_APP_SIGNAL_STATE_MAX)
	{
		assert(recentRecords.size() <= ADS_GET_APP_SIGNAL_STATE_MAX);
	}

	m_getSignalStateRequest.mutable_signalhashes()->Clear();
	m_getSignalStateRequest.mutable_signalhashes()->Reserve(static_cast<int>(recentRecords.size()));

	qint64 now = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();

	int index = 0;
	for (const std::pair<Hash, qint64>& p : recentRecords)
	{
		const Hash& hash = p.first;
		const qint64& lastAccessTime = p.second;

		if (now - lastAccessTime > 5_sec)
		{
			hashesToRemove.push_back(hash);
			continue;
		}
		else
		{
			m_getSignalStateRequest.add_signalhashes(hash);
		}

		index ++;
		if (index > ADS_GET_APP_SIGNAL_STATE_MAX)
		{
			// Make break at the and of the loop, as before threre is a condition to remove items
			//
			break;
		}
	}

	m_recents.remove(hashesToRemove);

	sendRequest(ADS_GET_APP_SIGNAL_STATE, m_getSignalStateRequest);
	return;
}

void TcpSignalRecents::processSignalState(const QByteArray& data)
{
	bool ok = m_getSignalStateReply.ParseFromArray(data.constData(), data.size());

	if (ok == false)
	{
		assert(ok);
		requestSignalState();
		return;
	}

	if (m_getSignalStateReply.error() != 0)
	{
		qDebug() << "TcpSignalRecents::processSignalState, error received: " << m_getSignalStateReply.error();
		assert(m_getSignalStateReply.error() != 0);

		requestSignalState();
		return;
	}

	int signalStateCount = m_getSignalStateReply.appsignalstates_size();

	std::vector<AppSignalState> states;
	states.reserve(signalStateCount);

	for (int i = 0; i < signalStateCount; i++)
	{
		const ::Proto::AppSignalState& protoState = m_getSignalStateReply.appsignalstates(i);
		assert(protoState.hash() != 0);

		states.emplace_back(protoState);
	}

	theSignals.setState(states);

	//qDebug() << "Priority updates state count  "  << states.size();

	requestSignalState();
	return;
}

void TcpSignalRecents::slot_configurationArrived(ConfigSettings configuration)
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

