#include "TcpSignalClient.h"
#include "Settings.h"

TcpSignalClient::TcpSignalClient(const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2) :
	Tcp::Client(serverAddressPort1, serverAddressPort2)
{
	qDebug() << "TcpSignalClient::TcpSignalClient(const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2)";

	m_startStateTimerId = startTimer(theSettings.requestTimeInterval());

	resetToGetState();
}

TcpSignalClient::~TcpSignalClient()
{
	qDebug() << "TcpSignalClient::~TcpSignalClient()";
}

void TcpSignalClient::timerEvent(QTimerEvent* event)
{
	assert(event);
	return;
}

void TcpSignalClient::onClientThreadStarted()
{
	qDebug() << "TcpSignalClient::onClientThreadStarted()";
}

void TcpSignalClient::onClientThreadFinished()
{
	qDebug() << "TcpSignalClient::onClientThreadFinished()";
}

void TcpSignalClient::onConnection()
{
	qDebug() << "TcpSignalClient::onConnection()";

	assert(isClearToSendRequest() == true);

	resetToGetSignalList();

	return;
}

void TcpSignalClient::onDisconnection()
{
	qDebug() << "TcpSignalClient::onDisconnection";
}

void TcpSignalClient::onReplyTimeout()
{
	qDebug() << "TcpSignalClient::onReplyTimeout()";
}

void TcpSignalClient::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
{
	if (replyData == nullptr)
	{
		assert(replyData);
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

	default:
		assert(false);
		qDebug() << "Wrong requestID in TcpSignalClient::processReply()";

		resetToGetState();
	}

	return;
}

void TcpSignalClient::resetToGetSignalList()
{
	QThread::msleep(theSettings.requestTimeInterval());

	theSignals.reset();
	m_signalList.clear();

	requestSignalListStart();
	return;
}

void TcpSignalClient::resetToGetState()
{
	QThread::msleep(theSettings.requestTimeInterval());
	// requestSignalState....
	//
}

void TcpSignalClient::requestSignalListStart()
{
	assert(isClearToSendRequest());
	sendRequest(ADS_GET_APP_SIGNAL_LIST_START);
}

void TcpSignalClient::processSignalListStart(const QByteArray& data)
{
	bool ok = m_getSignalListStartReply.ParseFromArray(data.constData(), data.size());

	if (ok == false)
	{
		assert(ok);
		resetToGetSignalList();
		return;
	}

	if (m_getSignalListStartReply.error() != 0)
	{
		qDebug() << "TcpSignalClient::processSignalListNext, error received: " << m_getSignalListStartReply.error();
		assert(m_getSignalListStartReply.error() != 0);

		resetToGetSignalList();
		return;
	}

	qDebug() << "----------------- processSignalListStart -----------------";
	qDebug() << "error: " << m_getSignalListStartReply.error();
	qDebug() << "totalItemCount: " << m_getSignalListStartReply.totalitemcount();
	qDebug() << "partCount: " << m_getSignalListStartReply.partcount();
	qDebug() << "itemsPerPart: " << m_getSignalListStartReply.itemsperpart();

	if (m_getSignalListStartReply.totalitemcount() == 0 ||
		m_getSignalListStartReply.partcount() == 0)
	{
		assert(m_getSignalListStartReply.totalitemcount() == 0);
		assert(m_getSignalListStartReply.partcount() == 0);

		// request params
		//
		requestSignalParam(0);
	}

	m_signalList.clear();
	m_signalList.reserve(m_getSignalListStartReply.totalitemcount());

	requestSignalListNext(0);

	return;
}

void TcpSignalClient::requestSignalListNext(int part)
{
	assert(isClearToSendRequest());

	// if all parts were requested then sitch to next reply
	//
	if (part >= m_getSignalListStartReply.partcount())
	{
		if (m_signalList.size() != m_getSignalListStartReply.totalitemcount())
		{
			assert(m_signalList.size() != m_getSignalListStartReply.totalitemcount());
		}

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
	bool ok = m_getSignalListNextReply.ParseFromArray(data.constData(), data.size());

	if (ok == false)
	{
		assert(ok);
		resetToGetSignalList();
		return;
	}

	if (m_getSignalListNextReply.error() != 0)
	{
		qDebug() << "TcpSignalClient::processSignalListNext, error received: " << m_getSignalListNextReply.error();
		assert(m_getSignalListNextReply.error() != 0);

		resetToGetSignalList();
		return;
	}

	if (m_getSignalListNextReply.part() != m_getSignalListNextRequest.part())
	{
		// Asked for one part but got different
		//
		assert(m_getSignalListNextReply.part() == m_getSignalListNextRequest.part());
		resetToGetSignalList();
		return;
	}

	qDebug() << "----------------- processSignalListNext -----------------";
	qDebug() << "error: " << m_getSignalListNextReply.error();
	qDebug() << "part: " << m_getSignalListNextReply.part();

	for (int i = 0; i < m_getSignalListNextReply.appsignalids_size(); i++)
	{
		m_signalList.push_back(QString::fromStdString(m_getSignalListNextReply.appsignalids(i)));
	}

	// Next request
	//
	requestSignalListNext(m_getSignalListNextReply.part() + 1);

	return;
}

void TcpSignalClient::requestSignalParam(int startIndex)
{
	assert(isClearToSendRequest());
	m_lastSignalParamStartIndex = startIndex;

	if (startIndex == 0)
	{
		theSignals.reset();
	}

	if (startIndex >= m_signalList.size())
	{
		resetToGetState();
		return;
	}

	m_getSignalParamRequest.mutable_signalhashes()->Clear();
	m_getSignalParamRequest.mutable_signalhashes()->Reserve(ADS_GET_APP_SIGNAL_PARAM_MAX);

	for (int i = startIndex;
		 i < startIndex + ADS_GET_APP_SIGNAL_PARAM_MAX &&
		 i < m_signalList.size();
		 i++)
	{
		Hash signalHash = calcHash(m_signalList[i]);
		m_getSignalParamRequest.add_signalhashes(signalHash);
	}

	sendRequest(ADS_GET_APP_SIGNAL_PARAM, m_getSignalParamRequest);
	return;
}

void TcpSignalClient::processSignalParam(const QByteArray& data)
{
	bool ok = m_getSignalParamReply.ParseFromArray(data.constData(), data.size());

	if (ok == false)
	{
		assert(ok);
		resetToGetSignalList();
		return;
	}

	if (m_getSignalParamReply.error() != 0)
	{
		qDebug() << "TcpSignalClient::processSignalParam, error received: " << m_getSignalParamReply.error();
		assert(m_getSignalParamReply.error() != 0);

		resetToGetSignalList();
		return;
	}

	for (int i = 0; i < m_getSignalParamReply.appsignalparams_size(); i++)
	{
		const ::Proto::AppSignal& protoSignal = m_getSignalParamReply.appsignalparams(i);

		Signal s(false);
		s.serializeFromProtoAppSignal(&protoSignal);

		assert(s.hash() != 0);
		assert(s.appSignalID().isEmpty() == false);

		if (s.hash() != 0 && s.appSignalID().isEmpty() == false)
		{
			theSignals.addSignal(s);
		}
	}

	requestSignalParam(m_lastSignalParamStartIndex + ADS_GET_APP_SIGNAL_PARAM_MAX);

//	for (int i = 0; i < m_getSignalListNextReply.appsignalids_size(); i++)
//	{
//		m_signalList.push_back(QString::fromStdString(m_getSignalListNextReply.appsignalids(i)));
//	}

//	// Next request
//	//
//	requestSignalListNext(m_getSignalListNextReply.part() + 1);

	return;
}

