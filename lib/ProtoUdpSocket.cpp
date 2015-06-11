#include "..\include\ProtoUdpSocket.h"


// -------------------------------------------------------------------------------------
//
// ProtoUdpSocket class implementation
//
// -------------------------------------------------------------------------------------


ProtoUdpSocket::ProtoUdpSocket() :
	m_socket(this),
	m_timer(this)
{
}


ProtoUdpSocket::~ProtoUdpSocket()
{
}


quint32 ProtoUdpSocket::getTotalFramesNumber(quint32 dataSize)
{
	return dataSize / PROTO_UDP_FRAME_DATA_SIZE +
			(dataSize % PROTO_UDP_FRAME_DATA_SIZE ? 1 : 0);
}


quint32 ProtoUdpSocket::getFrameDataSize(quint32 frameNumber, quint32 dataSize)
{
	quint32 totalFramesNumber = getTotalFramesNumber(dataSize);

	if (frameNumber >= totalFramesNumber)
	{
		assert(false);
		return 0;
	}

	if (frameNumber == totalFramesNumber - 1)
	{
		// is a last frame
		//
		return dataSize % PROTO_UDP_FRAME_DATA_SIZE;
	}

	return PROTO_UDP_FRAME_DATA_SIZE;
}


void ProtoUdpSocket::copyDataToFrame(ProtoUdpFrame& protoUdpFrame, const QByteArray& data)
{
	const char* dataPtr = data.constData() + protoUdpFrame.header.frameNumber * PROTO_UDP_FRAME_DATA_SIZE;

	quint32 frameDataSize = getFrameDataSize(protoUdpFrame.header.frameNumber, data.size());

	memcpy(protoUdpFrame.frameData, dataPtr, static_cast<size_t>(frameDataSize));
}


// -------------------------------------------------------------------------------------
//
// ProtoUdpSocket class implementation
//
// -------------------------------------------------------------------------------------


ProtoUdpClient::ProtoUdpClient(const HostAddressPort& firstServerAddress, const HostAddressPort& secondServerAddress) :
	m_firstServerAddress(firstServerAddress),
	m_secondServerAddress(secondServerAddress)
{
	m_serverAddress = firstServerAddress;
	m_communicateWithFirstServer = true;

	m_clientID = qHash(QUuid::createUuid());

	m_socket.bind();

	connect(&m_timer, &QTimer::timeout, this, &ProtoUdpClient::onTimerTimeout);
	connect(&m_socket, &QUdpSocket::readyRead, this, &ProtoUdpClient::onSocketReadyRead);

	connect(this, &ProtoUdpClient::startSendRequest, this, &ProtoUdpClient::onStartSendRequest);
}


ProtoUdpClient::~ProtoUdpClient()
{
}


void ProtoUdpClient::setServersAddresses(const HostAddressPort& firstServerAddress, const HostAddressPort& secondServerAddress)
{
	m_sync.lock();

	m_firstServerAddress = firstServerAddress;
	m_secondServerAddress = secondServerAddress;

	emit restartCommunication();

	m_sync.unlock();
}


void ProtoUdpClient::sendRequest(quint32 requestID, QByteArray& requestData)
{
	m_sync.lock();

	if (state() == ProtoUdpClientState::ReadyToSendRequest)
	{
		m_requestData.swap(requestData);

		m_requestID = requestID;

		emit startSendRequest();
	}
	else
	{
		// m_clientState != ClientState::ReadyToSendRequest
		//
		assert(false);
	}

	m_sync.unlock();
}


void ProtoUdpClient::onStartSendRequest()
{
	m_requestFrame.header.type = ProtoUdpFrameHeader::Type::Request;
	m_requestFrame.header.clientID = m_clientID;
	m_requestFrame.header.sessionID = 0;					// must be assign on server side
	m_requestFrame.header.requestID = m_requestID;
	m_requestFrame.header.frameNumber = 0;
	m_requestFrame.header.errorCode = 0;

	copyDataToFrame(m_requestFrame, m_requestData);

	setState(ProtoUdpClientState::RequestSending);
	setError(ProtoUdpError::Ok);

	calcRequestSendingProgress();
	setReplyReceivingProgress(0);

	sendRequestFrame();
}


void ProtoUdpClient::continueSendRequest()
{
	m_requestFrame.header.frameNumber++;

	copyDataToFrame(m_requestFrame, m_requestData);

	sendRequestFrame();
}


void ProtoUdpClient::calcRequestSendingProgress()
{
	quint32 sentDataSize = (m_requestFrame.header.frameNumber + 1) * PROTO_UDP_FRAME_DATA_SIZE;

	if (sentDataSize >= m_requestFrame.header.totalDataSize)
	{
		setRequestSendingProgress(100);
	}
	else
	{
		setRequestSendingProgress(static_cast<int>(double(sentDataSize * 100) / double(m_requestFrame.header.totalDataSize)));
	}
}


void ProtoUdpClient::sendRequestFrame()
{
	qint64 senBytesCount = m_socket.writeDatagram(reinterpret_cast<const char*>(&m_requestFrame), sizeof(m_requestFrame), m_serverAddress.address(), m_serverAddress.port());

	assert(senBytesCount == sizeof(m_requestFrame));

	m_timer.start(m_replayTimeout);
}


void ProtoUdpClient::onTimerTimeout()
{
	m_sync.lock();

	switch(state())
	{
	case ProtoUdpClientState::RequestSending:

		if (m_retryCount < PROTO_UDP_RETRY_COUNT)
		{
			sendRequestFrame();
			m_retryCount++;
		}
		else
		{
			m_timer.stop();

			m_requestFrame.header.clear();

			setError(ProtoUdpError::NoReplayFromServer);
			setState(ProtoUdpClientState::ReadyToSendRequest);

			qDebug() << "NoReplayFromServer";
		}

		break;

	default:
		assert(false);
	}

	m_sync.unlock();
}

void ProtoUdpClient::onSocketReadyRead()
{

}


// -------------------------------------------------------------------------------------
//
// ProtoUdpClientThread class implementation
//
// -------------------------------------------------------------------------------------


ProtoUdpClientThread::ProtoUdpClientThread(const HostAddressPort& serverAddress)
{
	m_protoUdpClient = new ProtoUdpClient(serverAddress, serverAddress);
}


ProtoUdpClientThread::ProtoUdpClientThread(const HostAddressPort& firstServerAddress, const HostAddressPort& secondServerAddress)
{
	m_protoUdpClient = new ProtoUdpClient(firstServerAddress, secondServerAddress);
}


void ProtoUdpClientThread::run()
{
	m_protoUdpClient->moveToThread(&m_thread);

	connect(&m_thread, &QThread::started, m_protoUdpClient, &ProtoUdpClient::onThreadStarted);
	connect(&m_thread, &QThread::finished, m_protoUdpClient, &ProtoUdpClient::onThreadFinished);

	m_thread.start();
}


ProtoUdpClientThread::~ProtoUdpClientThread()
{
	m_thread.quit();
	m_thread.wait();
}


void ProtoUdpClientThread::sendRequest(quint32 requestID, QByteArray& requestData)
{
	m_protoUdpClient->sendRequest(requestID, requestData);
}


// -------------------------------------------------------------------------------------
//
// ProtoUdpServer class implementation
//
// -------------------------------------------------------------------------------------


ProtoUdpServer::ProtoUdpServer(const QHostAddress& serverAddress, quint16 serverPort)
{

}


ProtoUdpServer::~ProtoUdpServer()
{

}


