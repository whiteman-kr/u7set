#ifndef CLIENT_LIB_DOMAIN
#error Don't include this file in the project! Link ClientLib instead.
#endif

#include "TcpSignalClient.h"

namespace ClientLib
{

	TcpSignalClient::TcpSignalClient(const SoftwareInfo& softwareInfo,
									 const SoftwareEndpoint::AppDataService& adsInfo,
									 IAppSignalUpdater& signalUpdater,
									 ILogFile* logFile) :
		Tcp::Client(softwareInfo, adsInfo.address, "TcpSignalClient", adsInfo.equipmentId),
		TcpClientStatistics(this),
		HasLogFile(logFile, QString("ADS ") + adsInfo.shortenId),
		m_serverSettings(adsInfo),
		m_signalUpdater(signalUpdater)
	{
		setObjectName("TcpSignalClient " + adsInfo.equipmentId);

		Q_ASSERT(this->logFile());
		qDebug() << "TcpSignalClient::TcpSignalClient() " << adsInfo.equipmentId << ", " << serverAddressPort1().addressPortStr();

		return;
	}

	TcpSignalClient::~TcpSignalClient()
	{
		qDebug() << "TcpSignalClient::~TcpSignalClient() " << serverAddressPort1().addressPortStr();
	}

	void TcpSignalClient::onClientThreadStarted()
	{
		qDebug() << "TcpSignalClient::onClientThreadStarted()" << this->serverAddressPort1().addressPortStr();
		writeMessage("TcpSignalClient::onClientThreadStarted()");

		return;
	}

	void TcpSignalClient::onClientThreadFinished()
	{
		qDebug() << "TcpSignalClient::onClientThreadFinished()" << this->serverAddressPort1().addressPortStr();
		writeMessage("TcpSignalClient::onClientThreadFinished()");

		// theSignals.reset();	!!!signal reset moved to AdsConnection::configurationArrived
		return;
	}

	void TcpSignalClient::onConnection()
	{
		qDebug() << "TcpSignalClient::onConnection()" << this->serverAddressPort1().addressPortStr();
		writeMessage("TcpSignalClient::onConnection()");

		Q_ASSERT(isClearToSendRequest() == true);

		resetToGetSignalList();

		return;
	}

	void TcpSignalClient::onDisconnection()
	{
		qDebug() << "TcpSignalClient::onDisconnection" << this->serverAddressPort1().addressPortStr();
		writeMessage("onDisconnection()");

		m_signalUpdater.invalidateSignalStates(QThread::currentThreadId());

		return;
	}

	void TcpSignalClient::onReplyTimeout()
	{
		qDebug() << "TcpSignalClient::onReplyTimeout()" << this->serverAddressPort1().addressPortStr();
		writeWarning("TcpSignalClient::onReplyTimeout()");
	}

	void TcpSignalClient::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
	{
		if (replyData == nullptr)
		{
			Q_ASSERT(replyData);
			return;
		}

		QByteArray data = QByteArray::fromRawData(replyData, replyDataSize);

		switch (requestID)
		{
		case ADS_GET_APP_SIGNAL_LIST_START:
			processSignalListStart(data);
			break;

		case ADS_GET_APP_SIGNAL_LIST_NEXT:
			processSignalListNext(data);
			break;

		case ADS_GET_APP_SIGNAL_PARAM:
			processSignalParam(data);
			break;

		case ADS_GET_APP_SIGNAL_STATE_CHANGES:
			processSignalStateChanges(data);
			break;

		case ADS_GET_APP_SIGNAL_STATE:
			processSignalState(data);
			break;

		default:
			Q_ASSERT(false);
			qDebug() << "Wrong requestID in TcpSignalClient::processReply()" << this->serverAddressPort1().addressPortStr();
			writeError(QString("Wrong requestID in TcpSignalClient::processReply(), %1").arg(requestID));

			resetToGetState(true);
		}

		return;
	}

	void TcpSignalClient::resetToGetSignalList()
	{
		QThread::msleep(RequestTimeInterval);

		m_signalList.clear();
		m_lastSignalParamStartIndex = 0;
		m_lastSignalStateStartIndex = 0;

		requestSignalListStart();
		return;
	}

	void TcpSignalClient::resetToGetState(bool resetStateIndex)
	{
		QThread::msleep(RequestTimeInterval);

		if (resetStateIndex == true)
		{
			m_lastSignalStateStartIndex = 0;
		}

		if (m_signalList.empty() == false)
		{
			requestSignalStateChanges();
		}
		else
		{
			// There is no signals stae to request list again
			//
			resetToGetSignalList();
		}

		return;
	}

	void TcpSignalClient::requestSignalListStart()
	{
		Q_ASSERT(isClearToSendRequest());
		sendRequest(ADS_GET_APP_SIGNAL_LIST_START);
	}

	void TcpSignalClient::processSignalListStart(const QByteArray& data)
	{
		bool ok = m_getSignalListStartReply.ParseFromArray(data.constData(), static_cast<int>(data.size()));
		if (ok == false)
		{
			Q_ASSERT(ok);
			resetToGetSignalList();
			return;
		}

		if (m_getSignalListStartReply.error() != 0)
		{
			qDebug() << "TcpSignalClient::processSignalListNext, error received: " << m_getSignalListStartReply.error();
			writeError(QString("processSignalListNext, error received: %1").arg(m_getSignalListStartReply.error()));

			Q_ASSERT(m_getSignalListStartReply.error() != 0);

			resetToGetSignalList();
			return;
		}

		qDebug() << "----------------- processSignalListStart -----------------"  << this->serverAddressPort1().addressPortStr();;
		qDebug() << "error: " << m_getSignalListStartReply.error();
		qDebug() << "totalItemCount: " << m_getSignalListStartReply.totalitemcount();
		qDebug() << "partCount: " << m_getSignalListStartReply.partcount();
		qDebug() << "itemsPerPart: " << m_getSignalListStartReply.itemsperpart();

		writeMessage("----------------- processSignalListStart -----------------");
		if (m_getSignalListStartReply.error() == 0)
		{
			writeMessage(QString("-- error: %1").arg(m_getSignalListStartReply.error()));
		}
		else
		{
			writeError(QString("-- error: %1").arg(m_getSignalListStartReply.error()));
		}
		writeMessage(QString("-- totalItemCount: %1").arg(m_getSignalListStartReply.totalitemcount()));
		writeMessage(QString("-- partCount: %1").arg(m_getSignalListStartReply.partcount()));
		writeMessage(QString("-- itemsPerPart: %1").arg(m_getSignalListStartReply.itemsperpart()));

		if (m_getSignalListStartReply.totalitemcount() == 0 ||
			m_getSignalListStartReply.partcount() == 0)
		{
			// There is no signals, useless but can be
			//
			Q_ASSERT(m_getSignalListStartReply.totalitemcount() == 0);
			Q_ASSERT(m_getSignalListStartReply.partcount() == 0);

			m_signalList.clear();

			// request params
			//
			requestSignalParam(0);
			return;
		}

		m_signalList.clear();
		m_signalList.reserve(m_getSignalListStartReply.totalitemcount());

		requestSignalListNext(0);

		return;
	}

	void TcpSignalClient::requestSignalListNext(int part)
	{
		Q_ASSERT(isClearToSendRequest());

		// if all parts were requested then switch to next reply
		//
		if (part >= m_getSignalListStartReply.partcount())
		{
			Q_ASSERT(std::ssize(m_signalList) == m_getSignalListStartReply.totalitemcount());

			// Request params
			//
			requestSignalParam(0);
			return;
		}

		// Request part, partNo is set in processSignalListStart and is incremented in processSignalListNext
		//
		m_getSignalListNextRequest.set_part(part);

		sendRequest(ADS_GET_APP_SIGNAL_LIST_NEXT, m_getSignalListNextRequest);
		return;
	}

	void TcpSignalClient::processSignalListNext(const QByteArray& data)
	{
		bool ok = m_getSignalListNextReply.ParseFromArray(data.constData(), static_cast<int>(data.size()));

		if (ok == false)
		{
			Q_ASSERT(ok);
			resetToGetSignalList();
			return;
		}

		if (m_getSignalListNextReply.error() != 0)
		{
			qDebug() << "TcpSignalClient::processSignalListNext, error received: " << m_getSignalListNextReply.error();
			writeError(QString("processSignalListNext, error received: %1").arg(m_getSignalListNextReply.error()));

			Q_ASSERT(m_getSignalListNextReply.error() != 0);

			resetToGetSignalList();
			return;
		}

		if (m_getSignalListNextReply.part() != m_getSignalListNextRequest.part())
		{
			// Asked for one part but got different
			//
			Q_ASSERT(m_getSignalListNextReply.part() == m_getSignalListNextRequest.part());
			resetToGetSignalList();
			return;
		}

		qDebug() << "----------------- processSignalListNext -----------------";
		qDebug() << "error: " << m_getSignalListNextReply.error();
		qDebug() << "part: " << m_getSignalListNextReply.part();

		writeMessage("----------------- processSignalListNext -----------------");
		if (m_getSignalListNextReply.error() == 0)
		{
			writeMessage(QString("-- error: %1").arg(m_getSignalListNextReply.error()));
		}
		else
		{
			writeError(QString("-- error: %1").arg(m_getSignalListNextReply.error()));
		}
		writeMessage(QString("-- part: %1").arg(m_getSignalListNextReply.part()));

		{
			for (int i = 0; i < m_getSignalListNextReply.appsignalids_size(); i++)
			{
				Hash hash = ::calcHash(QString::fromStdString(m_getSignalListNextReply.appsignalids(i)));
				m_signalList.push_back(hash);
			}
		}

		// Next request
		//
		requestSignalListNext(m_getSignalListNextReply.part() + 1);

		return;
	}

	// AppSignalParam
	//
	void TcpSignalClient::requestSignalParam(int startIndex)
	{
		Q_ASSERT(isClearToSendRequest());
		m_lastSignalParamStartIndex = startIndex;

		if (startIndex == 0)
		{
			//theSignals.reset();	no need to reset signal list, signals will be just updates,
			// if new configuration arrives (ONLY than signal list can be changed) then all signals will be reset in
			// AdsConnection::configurationArrived
			//
		}

		if (startIndex >= std::ssize(m_signalList))
		{
			m_signalUpdater.notifySignalParamsUpdated();

			resetToGetState(true);	// END OF RECEIVING SIGNALS PARAMS,
			// Here the new loop starts!!!
			return;
		}

		m_getSignalParamRequest.mutable_signalhashes()->Clear();
		m_getSignalParamRequest.mutable_signalhashes()->Reserve(ADS_GET_APP_SIGNAL_PARAM_MAX);

		for (int i = startIndex; i < startIndex + ADS_GET_APP_SIGNAL_PARAM_MAX && i < std::ssize(m_signalList); i++)
		{
			Hash signalHash = m_signalList[i];
			m_getSignalParamRequest.add_signalhashes(signalHash);
		}

		sendRequest(ADS_GET_APP_SIGNAL_PARAM, m_getSignalParamRequest);
		return;
	}

	void TcpSignalClient::processSignalParam(const QByteArray& data)
	{
		bool ok = m_getSignalParamReply.ParseFromArray(data.constData(), static_cast<int>(data.size()));

		if (ok == false)
		{
			Q_ASSERT(ok);
			resetToGetSignalList();
			return;
		}

		if (m_getSignalParamReply.error() != 0)
		{
			qDebug() << "TcpSignalClient::processSignalParam, error received: " << m_getSignalParamReply.error();
			writeError(QString("processSignalParam, error received: %1").arg(m_getSignalParamReply.error()));

			Q_ASSERT(m_getSignalParamReply.error() != 0);

			resetToGetState(true);
			return;
		}

		std::vector<AppSignalParam> appSignals;
		appSignals.reserve(m_getSignalParamReply.appsignals_size());

		for (int i = 0; i < m_getSignalParamReply.appsignals_size(); i++)
		{
			const ::Proto::AppSignal& protoSignal = m_getSignalParamReply.appsignals(i);

			AppSignalParam& s = appSignals.emplace_back();
			s.load(protoSignal);

			if (s.hash() == 0 || s.appSignalId().isEmpty() == true)
			{
				Q_ASSERT(s.hash() != 0);
				Q_ASSERT(s.appSignalId().isEmpty() == false);

				appSignals.pop_back();
			}
		}

		m_signalUpdater.addSignals(appSignals, m_serverSettings.equipmentId);

		requestSignalParam(m_lastSignalParamStartIndex + ADS_GET_APP_SIGNAL_PARAM_MAX);

		return;
	}

	// AppSignalStateChanges
	//
	void TcpSignalClient::requestSignalStateChanges()
	{
		Q_ASSERT(isClearToSendRequest());

		sendRequest(ADS_GET_APP_SIGNAL_STATE_CHANGES, m_getSignalStateChangesRequest);

		return;
	}

	void TcpSignalClient::processSignalStateChanges(const QByteArray& data)
	{
		if (bool ok = m_getSignalStateChangesReply.ParseFromArray(data.constData(), static_cast<int>(data.size()));
			ok == false)
		{
			Q_ASSERT(ok);
			resetToGetState(true);
			return;
		}

		//	optional int32 error = 1 [default = 0];
		//	optional int64 serverTimeUtc = 2;
		//	optional int64 serverTimeLocal = 3;
		//	optional int32 pendingStatesCount = 4 [default = 0];
		//	repeated Proto.AppSignalState appSignalStates = 5;		// Limited to ADS_GET_APP_SIGNAL_STATE_MAX (2000)

		if (m_getSignalStateChangesReply.error() != 0)
		{
			qDebug() << "TcpSignalClient::processSignalStateChanges, error received: " << m_getSignalStateChangesReply.error();
			writeError(QString("processSignalStateChanges, error received: %1").arg(m_getSignalStateChangesReply.error()));
			Q_ASSERT(m_getSignalStateChangesReply.error() != 0);

			resetToGetState(true);
			return;
		}

		int signalStateCount = m_getSignalStateChangesReply.appsignalstates_size();

		std::vector<AppSignalState> states;
		states.reserve(signalStateCount);

		for (int i = 0; i < signalStateCount; i++)
		{
			const AppSignalState& state = states.emplace_back(m_getSignalStateChangesReply.appsignalstates(i));
			Q_ASSERT(state.hash() != 0);
		}

		m_signalUpdater.setState(states, QThread::currentThreadId());

		if (m_getSignalStateChangesReply.pendingstatescount() >= ADS_GET_APP_SIGNAL_STATE_MAX)
		{
			// A lot of signals are in teh event queue, request one more time
			//
			requestSignalStateChanges();
		}
		else
		{
			// Update all signals
			//
			requestSignalState(m_lastSignalStateStartIndex + ADS_GET_APP_SIGNAL_STATE_MAX);
		}

		return;
	}

	// AppSignalState
	//
	void TcpSignalClient::requestSignalState(int startIndex)
	{
		Q_ASSERT(isClearToSendRequest());

		if (startIndex >= std::ssize(m_signalList))
		{
			startIndex = 0;
		}

		m_lastSignalStateStartIndex = startIndex;

		m_getSignalStateRequest.mutable_signalhashes()->Clear();
		m_getSignalStateRequest.mutable_signalhashes()->Reserve(ADS_GET_APP_SIGNAL_STATE_MAX);

		for (int i = startIndex; i < startIndex + ADS_GET_APP_SIGNAL_STATE_MAX && i < std::ssize(m_signalList); i++)
		{
			Hash signalHash = m_signalList[i];
			m_getSignalStateRequest.add_signalhashes(signalHash);
		}

		sendRequest(ADS_GET_APP_SIGNAL_STATE, m_getSignalStateRequest);
		return;
	}

	void TcpSignalClient::processSignalState(const QByteArray& data)
	{
		if (bool ok = m_getSignalStateReply.ParseFromArray(data.constData(), static_cast<int>(data.size()));
			ok == false)
		{
			Q_ASSERT(ok);
			resetToGetState(true);
			return;
		}

		if (m_getSignalStateReply.error() != 0)
		{
			qDebug() << "TcpSignalClient::processSignalState, error received: " << m_getSignalStateReply.error();
			writeError(QString("processSignalState, error received: %1").arg(m_getSignalStateReply.error()));

			Q_ASSERT(m_getSignalStateReply.error() != 0);

			resetToGetState(true);
			return;
		}

		int signalStateCount = m_getSignalStateReply.appsignalstates_size();

		std::vector<AppSignalState> states;
		states.reserve(signalStateCount);

		for (int i = 0; i < signalStateCount; i++)
		{
			const AppSignalState& state = states.emplace_back(m_getSignalStateReply.appsignalstates(i));
			Q_ASSERT(state.m_hash != 0);
		}

		m_signalUpdater.setState(states, QThread::currentThreadId());

		resetToGetState(false);
		return;
	}

}
