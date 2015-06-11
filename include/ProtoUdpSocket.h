#pragma once

#include <QObject>
#include <QtGlobal>
#include <QTimer>
#include <QThread>
#include <QUuid>
#include <QUdpSocket>
#include <cassert>


#pragma pack(push, 1)

const int PROTO_UDP_DATAGRAM_SIZE = 8192;


struct ProtoUdpFrameHeader
{
	enum Type
	{
		Request = 1,
		Replay = 2,
		Ack = 3
	};

	Type type = Type::Request;
	quint32 clientID = 0;
	quint32 sessionID = 0;
	quint32 requestID = 0;
	quint32 frameNumber = 0;			// zero-based!
	quint32 totalDataSize = 0;
	quint32 errorCode = 0;

	void clear()
	{
		type = Type::Request;
		clientID = 0;				// assign by client
		sessionID = 0;				// assign by server
		requestID = 0;				// assign by client
		frameNumber = 0;			// zero-based!
		totalDataSize = 0;
		errorCode = 0;
	}
};


const int PROTO_UDP_FRAME_DATA_SIZE = PROTO_UDP_DATAGRAM_SIZE - sizeof(ProtoUdpFrameHeader);


const int PROTO_UDP_REPLY_TIMEOUT = 50;		// 50 milliseconds
const int PROTO_UDP_RETRY_COUNT = 2;


union ProtoUdpFrame
{
	struct
	{
		ProtoUdpFrameHeader header;

		char frameData[PROTO_UDP_FRAME_DATA_SIZE];
	};

	char rawData[PROTO_UDP_DATAGRAM_SIZE];
};


#pragma pack(pop)


class ProtoUdpSocket : public QObject
{
	Q_OBJECT
/*private:
	static bool metaTypesRegistered;*/

protected:

	QUdpSocket m_socket;
	QTimer m_timer;
	QMutex m_sync;

	QByteArray m_requestData;
	QByteArray m_replyData;

	ProtoUdpFrame m_requestFrame;
	ProtoUdpFrame m_replayFrame;

	int m_replayTimeout = PROTO_UDP_REPLY_TIMEOUT;

	quint32 getTotalFramesNumber(quint32 dataSize);
	quint32 getFrameDataSize(quint32 frameNumber, quint32 dataSize);

	void copyDataToFrame(ProtoUdpFrame& protoUdpFrame, const QByteArray& data);

public:
	ProtoUdpSocket() {}
	~ProtoUdpSocket() {}

public slots:
	void onThreadStarted() {};
	void onThreadFinished() { deleteLater(); }
};


enum ProtoUdpError
{
	Ok,
	NoReplayFromServer
};


enum ProtoUdpClientState
{
	ReadyToSendRequest,
	RequestSending,
	ReplyReceiving,
	ReplyReady,
};


struct ProtoUdpClientStatus
{
	ProtoUdpClientState state = ProtoUdpClientState::ReadyToSendRequest;
	ProtoUdpError error = ProtoUdpError::Ok;

	int requestSendingProgress = 0;				// 0 - 100%
	int replayReceivingProgress = 0;			// 0 - 100%
};


class ProtoUdpClient : public ProtoUdpSocket
{
private:
	ProtoUdpClientStatus m_status;

	quint32 m_clientID = 0;

	int m_retryCount = 0;

	QHostAddress m_serverAddress;
	quint16 m_serverPort = 0;

	quint32 m_requestID = 0;

	void sendRequestFrame();

	void onTimerTimeout();
	void onSocketReadyRead();

	ProtoUdpClientState state() { return m_status.state; }
	void setState(ProtoUdpClientState state) { m_status.state = state; }

	void setError(ProtoUdpError error) { m_status.error = error; }

public:
	ProtoUdpClient(const QHostAddress& serverAddress, quint16 serverPort);
	~ProtoUdpClient();

	QHostAddress serverAddress() const { return m_serverAddress; }
	void setServerAddress(const QHostAddress& address) { m_serverAddress = address; }

	quint16 serverPort() const { return m_serverPort; }
	void setServerPort(quint16 port) { m_serverPort = port; }

	bool isReadyToSendRequest() { return state() == ProtoUdpClientState::ReadyToSendRequest; }
	bool sendRequest(quint32 requestID, QByteArray requestData);
};



class ProtoUdpServer : public ProtoUdpSocket
{

};


class ProtoUdpClientThread : public QObject
{
	Q_OBJECT

public:
	ProtoUdpClientThread(const QHostAddress& serverAddress, quint16 serverPort);
	~ProtoUdpClientThread();

	void run();
	void quit() { m_thread.quit(); }

private:
	QThread m_thread;
	ProtoUdpClient* m_protoUdpClient = nullptr;
};

