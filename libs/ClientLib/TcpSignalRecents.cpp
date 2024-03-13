#ifndef CLIENT_LIB_DOMAIN
#error Do not include this file in the project! Link ClientLib instead.
#endif

#include "TcpSignalRecents.h"

namespace ClientLib
{

	//
	// TcpSignalRecents
	//
	TcpSignalRecents::TcpSignalRecents(const SoftwareInfo& softwareInfo,
									   const SoftwareEndpoint::AppDataService& adsInfo,
									   IRecentAppSignals& recentAppSignals,
									   IAppSignalUpdater& signalUpdater,
									   ILogFile* logFile) :
		Tcp::Client(softwareInfo, adsInfo.address, "TcpSignalRecents", adsInfo.equipmentId),
		TcpClientStatistics(this),
		HasLogFile(logFile, QString("Recent ") + adsInfo.shortenId),
		m_serverSettings(adsInfo),
		m_recentAppSignals(recentAppSignals),
		m_signalUpdater(signalUpdater)
	{
		setObjectName("TcpSignalRecents " + adsInfo.shortenId);

		Q_ASSERT(this->logFile());
		qDebug() << "TcpSignalRecents::TcpSignalRecents(...)";

		connect(this, &Tcp::Client::signal_wrongServerID,
			[this](const QString& errorMessage)
			{
				writeError(errorMessage);
			});

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

		m_signalUpdater.invalidateSignalStates(QThread::currentThreadId());

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

	// AppSignalState
	//
	void TcpSignalRecents::requestSignalState()
	{
		QThread::msleep(100);

		Q_ASSERT(isClearToSendRequest());

		auto recentSignals = m_recentAppSignals.recentlyUsedAppSignals(connectedSoftwareInfo().equipmentID());
		if (recentSignals.empty() == true)
		{
			QThread::yieldCurrentThread();
		}

		if (recentSignals.size() > ADS_GET_APP_SIGNAL_STATE_MAX)
		{
			Q_ASSERT(recentSignals.size() <= ADS_GET_APP_SIGNAL_STATE_MAX);
		}

		thread_local ::Network::GetAppSignalStateRequest s_getSignalStateRequest;

		s_getSignalStateRequest.mutable_signalhashes()->Clear();
		s_getSignalStateRequest.mutable_signalhashes()->Reserve(std::ssize(recentSignals));

		for (Hash hash : recentSignals)
		{
			s_getSignalStateRequest.add_signalhashes(hash);
		}

		sendRequest(ADS_GET_APP_SIGNAL_STATE, s_getSignalStateRequest);

		return;
	}

	void TcpSignalRecents::processSignalState(const QByteArray& data)
	{
		thread_local ::Network::GetAppSignalStateReply s_getSignalStateReply;

		bool ok = s_getSignalStateReply.ParseFromArray(data.constData(), static_cast<int>(data.size()));

		if (ok == false)
		{
			Q_ASSERT(ok);
			requestSignalState();
			return;
		}

		if (s_getSignalStateReply.error() != 0)
		{
			qDebug() << "TcpSignalRecents::processSignalState, error received: " << s_getSignalStateReply.error();
			writeError(QString("processSignalState, error received %1").arg(s_getSignalStateReply.error()));

			Q_ASSERT(s_getSignalStateReply.error() != 0);

			requestSignalState();
			return;
		}

		int signalStateCount = s_getSignalStateReply.appsignalstates_size();

		std::vector<AppSignalState> states;
		states.reserve(signalStateCount);

		// If signal is not present in that AppDataService, it will be skipped in the answear
		//
		for (int i = 0; i < signalStateCount; i++)
		{
			const ::Proto::AppSignalState& protoState = s_getSignalStateReply.appsignalstates(i);
			Q_ASSERT(protoState.hash() != 0);

			states.emplace_back(protoState);
		}

		m_signalUpdater.setState(states, ::calcHash(m_serverSettings.equipmentId), QThread::currentThreadId());

		//qDebug() << "Priority updates state count  "  << states.size();

		requestSignalState();
		return;
	}

}
