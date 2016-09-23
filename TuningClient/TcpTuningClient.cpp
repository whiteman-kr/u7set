#include "TcpTuningClient.h"
#include "Settings.h"

TcpTuningClient::TcpTuningClient(ConfigController* configController, const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2)
	:Tcp::Client(serverAddressPort1, serverAddressPort2),
	  m_cfgController(configController)
{
	assert(m_cfgController);

	qDebug() << "TcpTuningClient::TcpTuningClient(const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2)";

}


TcpTuningClient::~TcpTuningClient()
{
	qDebug() << "TcpTuningClient::~TcpTuningClient()";
}

void TcpTuningClient::onClientThreadStarted()
{
	qDebug() << "TcpTuningClient::onClientThreadStarted()";

	connect(m_cfgController, &ConfigController::configurationArrived,
			this, &TcpTuningClient::slot_configurationArrived,
			Qt::QueuedConnection);

	return;
}

void TcpTuningClient::onClientThreadFinished()
{
	qDebug() << "TcpTuningClient::onClientThreadFinished()";

	//theSignals.reset();
}

void TcpTuningClient::onConnection()
{
	qDebug() << "TcpTuningClient::onConnection()";

	assert(isClearToSendRequest() == true);

	resetToGetTuningSources();

	return;
}

void TcpTuningClient::onDisconnection()
{
	qDebug() << "TcpTuningClient::onDisconnection";

	emit connectionFailed();
}

void TcpTuningClient::onReplyTimeout()
{
	qDebug() << "TcpTuningClient::onReplyTimeout()";
}

void TcpTuningClient::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
{
	if (replyData == nullptr)
	{
		assert(replyData);
		return;
	}

	QByteArray data = QByteArray::fromRawData(replyData, replyDataSize);

	switch (requestID)
	{
	/*case ADS_GET_APP_SIGNAL_LIST_START:
		processTuningSourcesStart(data);
		break;*/

	default:
		assert(false);
		qDebug() << "Wrong requestID in TcpTuningClient::processReply()";

		resetToGetTuningSources();
	}

	return;
}

void TcpTuningClient::resetToGetTuningSources()
{
	QThread::msleep(theSettings.m_requestInterval);

	//theSignals.reset();
	//m_signalList.clear();

	requestTuningSourcesStart();
	return;
}


void TcpTuningClient::requestTuningSourcesStart()
{
	assert(isClearToSendRequest());
	sendRequest(ADS_GET_APP_SIGNAL_LIST_START);
}

void TcpTuningClient::processTuningSourcesStart(const QByteArray& data)
{
	/*bool ok = m_getSignalListStartReply.ParseFromArray(data.constData(), data.size());

	if (ok == false)
	{
		assert(ok);
		resetToGetTuningSources();
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
		// There is no signals, useless but can be
		//
		assert(m_getSignalListStartReply.totalitemcount() == 0);
		assert(m_getSignalListStartReply.partcount() == 0);

		m_signalList.clear();

		// request params
		//
		requestSignalParam(0);
		return;
	}

	m_signalList.clear();
	m_signalList.reserve(m_getSignalListStartReply.totalitemcount());

	requestSignalListNext(0);*/

	emit tuningSourcesArrived();

	return;
}

void TcpTuningClient::slot_configurationArrived(ConfigSettings configuration)
{
	HostAddressPort h1 = configuration.tuns1.address();
	HostAddressPort h2 = configuration.tuns1.address();

	setServers(h1, h2, true);

	return;
}
