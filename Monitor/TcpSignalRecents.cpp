#include "TcpSignalRecents.h"
#include "Settings.h"


RecentUsed::RecentUsed(int maxSize /*= 750*/) :
	m_maxSize(maxSize)
{
}

void RecentUsed::add(Hash hash)
{
	qint64 now = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();

	auto it = m_signalToTime.find(hash);
	if (it == m_signalToTime.end())
	{
		if (m_signalToTime.size() >= m_maxSize)
		{
			auto lastTimeIt = m_timeToSignal.begin();

			m_signalToTime.erase(lastTimeIt->second);
			m_timeToSignal.erase(lastTimeIt);
		}

		m_signalToTime.insert({hash, now});
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
		Q_ASSERT(updated == true);

		it->second = now;
	}

	Q_ASSERT(m_signalToTime.size() == m_timeToSignal.size());
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
	auto it = m_signalToTime.find(hash);
	if (it == m_signalToTime.end())
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
	Q_ASSERT(removedFromTimeMap == true);

	m_signalToTime.erase(it);

	Q_ASSERT(m_signalToTime.size() == m_timeToSignal.size());
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
	return static_cast<int>(m_signalToTime.size());
}

const std::map<Hash, qint64>& RecentUsed::rawHashes() const
{
	return m_signalToTime;
}

std::vector<Hash> RecentUsed::hashes() const
{
	std::vector<Hash> result;
	result.reserve(m_signalToTime.size());

	for (auto p : m_signalToTime)
	{
		result.push_back(p.first);
	}

	return result;
}



TcpSignalRecents::TcpSignalRecents(const MonitorConfigController& configController,
								   const MonitorSettings::AppDataService& adsInfo,
								   MonitorSignalManager& signalManager,
								   ILogFile* logFile) :
	Tcp::Client(configController.softwareInfo(), adsInfo.address, "TcpSignalRecents"),
	TcpClientStatistics(this),
	HasLogFile(logFile, QString("Recent ") + adsInfo.equipmentId),
	m_cfgController(configController),
	m_serverSettings(adsInfo),
	m_signalManager(signalManager)
{
	setObjectName("TcpSignalRecents " + adsInfo.equipmentId);

	Q_ASSERT(this->logFile());
	qDebug() << "TcpSignalRecents::TcpSignalRecents(...)";

	return;
}

TcpSignalRecents::~TcpSignalRecents()
{
	qDebug() << "TcpSignalRecents::~TcpSignalRecents()";
}


void TcpSignalRecents::onClientThreadStarted()
{
	qDebug() << "TcpSignalRecents::onClientThreadStarted()";
	writeMessage("TcpSignalRecents::onClientThreadStarted()");

	return;
}

void TcpSignalRecents::onClientThreadFinished()
{
	qDebug() << "TcpSignalRecents::onClientThreadFinished()";
	writeMessage("TcpSignalRecents::onClientThreadFinished()");

	//theSignals.reset();	!signal reset moved to AdsConnection::configurationArrived
}

void TcpSignalRecents::onConnection()
{
	qDebug() << "TcpSignalRecents::onConnection()";
	writeMessage("TcpSignalRecents::onConnection()");

	Q_ASSERT(isClearToSendRequest() == true);

	requestSignalState();

	return;
}

void TcpSignalRecents::onDisconnection()
{
	qDebug() << "TcpSignalRecents::onDisconnection";
	writeMessage("TcpSignalRecents::onDisconnection()");

	m_signalManager.invalidateSignalStates(QThread::currentThreadId());

	emit connectionReset();

	return;
}

void TcpSignalRecents::onReplyTimeout()
{
	qDebug() << "TcpSignalRecents::onReplyTimeout()";
	writeWarning("TcpSignalRecents::onReplyTimeout()");
}

void TcpSignalRecents::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
{
	if (replyData == nullptr)
	{
		Q_ASSERT(replyData);
		return;
	}

	QByteArray data = QByteArray::fromRawData(replyData, replyDataSize);

	switch (requestID)
	{

	case ADS_GET_APP_SIGNAL_STATE:
		processSignalState(data);
		break;

	default:
		Q_ASSERT(false);

		qDebug() << "Wrong requestID in TcpSignalRecents::processReply()";
		writeError(QString("Wrong requestID in TcpSignalRecents::processReply(), requestId %1").arg(requestID));

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

	Q_ASSERT(isClearToSendRequest());

	const std::map<Hash, qint64>& recentRecords = m_recents.rawHashes();
	if (recentRecords.empty() == true)
	{
		QThread::yieldCurrentThread();
	}

	std::vector<Hash> hashesToRemove;
	hashesToRemove.reserve(recentRecords.size());

	if (recentRecords.size() > ADS_GET_APP_SIGNAL_STATE_MAX)
	{
		Q_ASSERT(recentRecords.size() <= ADS_GET_APP_SIGNAL_STATE_MAX);
	}

	m_getSignalStateRequest.mutable_signalhashes()->Clear();
	m_getSignalStateRequest.mutable_signalhashes()->Reserve(static_cast<int>(recentRecords.size()));

	qint64 now = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();

	int index = 0;
	for (const auto&[hash, lastAccessTime] : recentRecords)
	{
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
	bool ok = m_getSignalStateReply.ParseFromArray(data.constData(), static_cast<int>(data.size()));

	if (ok == false)
	{
		Q_ASSERT(ok);
		requestSignalState();
		return;
	}

	if (m_getSignalStateReply.error() != 0)
	{
		qDebug() << "TcpSignalRecents::processSignalState, error received: " << m_getSignalStateReply.error();
		writeError(QString("processSignalState, error received %1").arg(m_getSignalStateReply.error()));

		Q_ASSERT(m_getSignalStateReply.error() != 0);

		requestSignalState();
		return;
	}

	int signalStateCount = m_getSignalStateReply.appsignalstates_size();

	std::vector<AppSignalState> states;
	states.reserve(signalStateCount);

	for (int i = 0; i < signalStateCount; i++)
	{
		const ::Proto::AppSignalState& protoState = m_getSignalStateReply.appsignalstates(i);
		Q_ASSERT(protoState.hash() != 0);

		states.emplace_back(protoState);
	}

	m_signalManager.setState(states, QThread::currentThreadId());

	//qDebug() << "Priority updates state count  "  << states.size();

	requestSignalState();
	return;
}


