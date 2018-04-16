#pragma once

#include <QObject>
#include <assert.h>
#include <QThread>
#include <QTimer>
#include <QUuid>
#include <QUdpSocket>
#include <QTimerEvent>
#include <QMutexLocker>
#include <QQueue>
#include <QDateTime>

#include "../lib/SocketIO.h"
#include "../lib/SimpleThread.h"
#include "../lib/CircularLogger.h"
#include "../Proto/network.pb.h"


class UdpClientSocket;
class UdpServerSocket;


// -------------------------------------------------------------------
//
// UDP Request class
//
// -------------------------------------------------------------------

struct RequestHeader
{
	quint32 id;
	quint32 clientID;
	quint32 version;
	quint32 no;
	quint32 errorCode;
	quint32 dataSize;
};


class UdpRequest
{
private:
	QHostAddress m_address;
	quint16 m_port = 0;

	char m_rawData[MAX_DATAGRAM_SIZE];
	quint32 m_rawDataSize = sizeof(RequestHeader);

	unsigned int m_writeDataIndex = 0;
	unsigned int m_readDataIndex = 0;

private:
	char* rawData() { return m_rawData; }				// return pointer on request header
	RequestHeader* header() { return reinterpret_cast<RequestHeader*>(m_rawData); }
	char* data() { return m_rawData + sizeof(RequestHeader); }						// return pointer on request data after header

	char* writeDataPtr() { return m_rawData + sizeof(RequestHeader) + m_writeDataIndex; }	// pointer for write request data
	char* readDataPtr() { return m_rawData + sizeof(RequestHeader) + m_readDataIndex; }	// pointer for read request data


	void setRawDataSize(quint32 rawDataSize) { m_rawDataSize = rawDataSize; }

friend class UdpClientSocket;
friend class UdpServerSocket;

public:
	UdpRequest();
	UdpRequest(const QHostAddress& senderAddress, qint16 senderPort, char* receivedData, quint32 receivedDataSize);

	UdpRequest& operator = (const UdpRequest& request);

	QHostAddress address() const { return m_address; }
	void setAddress(const QHostAddress& address) { m_address = address; }

	quint16 port() const { return m_port; }
	void setPort(quint16 port) { m_port = port; }

	const char* rawData() const { return m_rawData; }							// return pointer on request header
	const RequestHeader* header() const { return reinterpret_cast<const RequestHeader*>(m_rawData); }
	const char* data() const { return m_rawData + sizeof(RequestHeader); }									// return pointer on request data after header

	quint32 rawDataSize() const { return m_rawDataSize; }						// full request length with header
	quint32 dataSize() const { return m_rawDataSize - sizeof(RequestHeader); }	// request length without header

	quint32 ID() const { return header()->id; }
	void setID(quint32 id) { header()->id = id; }

	quint32 clientID() const { return header()->clientID; }
	void setClientID(quint32 clientID) { header()->clientID = clientID; }

	quint32 version() const { return header()->version; }
	void setVersion(quint32 version) { header()->version = version; }

	quint32 no() const { return header()->no; }
	void setNo(quint32 no) { header()->no = no; }

	quint32 errorCode() const { return header()->errorCode; }
	void setErrorCode(quint32 errorCode) { header()->errorCode = errorCode; }

	quint32 headerDataSize() const { return header()->dataSize; }

	bool isEmpty() const { return m_rawDataSize < sizeof(RequestHeader); }

	void initAck(const UdpRequest& request);

	void initWrite()
	{
		m_writeDataIndex = 0;
		header()->dataSize = 0;
		m_rawDataSize = sizeof(RequestHeader);
	}

	bool writeDword(quint32 dw);
	bool writeData(const char* data, quint32 dataSize);
	bool writeData(const QByteArray& data);
	bool writeData(google::protobuf::Message& protobufMessage);

	void initRead()
	{
		m_readDataIndex = 0;
	}

	quint32 readDword();
};


// -------------------------------------------------------------------
//
// UDP Sockets' Base class
//
// -------------------------------------------------------------------

class UdpSocket : public SimpleThreadWorker
{
private:
	static bool metaTypesRegistered;

protected:
	QUdpSocket m_socket;
	QTimer m_timer;

	UdpRequest m_request;
	UdpRequest m_ack;

public:
	UdpSocket();
	virtual ~UdpSocket();

protected:
	virtual void registerMetaTypes();
};


// -------------------------------------------------------------------
//
// UDP Client classes
//
// -------------------------------------------------------------------

class UdpClientSocket : public UdpSocket
{
	Q_OBJECT

private:
	enum State
	{
		ReadyToSend,
		WaitingForAck
	};

	mutable QMutex m_mutex;

	QHostAddress m_serverAddress;
	qint16 m_port = 0;

	State m_state = State::ReadyToSend;
	quint32 m_requestNo = 1;
	quint32 m_protocolVersion = 1;
	quint32 m_clientID = 0;

	int m_msTimeout = 100;
	int m_retryCount = 0;
	int m_retryCtr = 0;
	quint32 m_ackTimeoutCtr = 0;

private:
	void retryRequest();

	virtual void onThreadStarted() final;
	virtual void onThreadFinished() final;

public:
	UdpClientSocket(const QHostAddress& serverAddress, quint16 port);
	virtual ~UdpClientSocket();

	const QHostAddress& serverAddress() const;
	void setServerAddress(const QHostAddress& serverAddress);

	quint16 port() const;
	void setPort(quint16 port);

	bool isWaitingForAck() const;
	bool isReadyToSend() const;

	void setProtocolVersion(quint32 version) { m_protocolVersion = version; }
	void setTimeout(int msTimeout) { m_msTimeout = msTimeout; }
	void setRetryCount(int retryCount) { m_retryCount = retryCount; }

	virtual void onSocketThreadStarted() {}
	virtual void onSocketThreadFinished() {}

	void sendRequest(const UdpRequest& udpRequest);
	void sendRequest(quint32 requestID);

signals:
	void sendRequestSignal(const UdpRequest& udpRequest);

	void ackTimeout(UdpRequest udpRequest);
	void ackReceived(UdpRequest udpRequest);

	void unknownAckReceived(UdpRequest udpRequest);

private slots:
	void onSendRequest(UdpRequest request);
	void onSocketReadyRead();
	void onAckTimerTimeout();
};


// -------------------------------------------------------------------
//
// UDP Server classes
//
// -------------------------------------------------------------------

class UdpClientRequestHandler;


// UdpRequestProcessor is a base class for request's handlers
//
class UdpRequestProcessor : public QObject
{
	Q_OBJECT

private:
	UdpClientRequestHandler* m_clientRequestHandler;

public:
	UdpRequestProcessor();

	virtual void onThreadStarted() { qDebug() << "UdpRequestProcessor thread started"; }
	virtual void onThreadFinished() { qDebug() << "UdpRequestProcessor thread finished"; }

	virtual void processRequest(const UdpRequest& request) = 0;

	void setClientRequestHandler(UdpClientRequestHandler* clientRequestHandler);

public slots:
	void onThreadStartedSlot();
	void onThreadFinishedSlot();

	void onRequestQueueIsNotEmpty();
};


class UdpClientRequestHandler : public QObject
{
	Q_OBJECT

private:
	QMutex m_queueMutex;
	QThread m_handlerThread;

	QQueue<UdpRequest> requestQueue;

	qint64 m_lastRequestTime;

public:
	UdpClientRequestHandler(UdpRequestProcessor* udpRequestProcessor);
	virtual ~UdpClientRequestHandler();

	void putRequest(const QHostAddress& senderAddress, qint16 senderPort, char* receivedData, quint32 recevedDataSize);
	UdpRequest getRequest();
	bool hasRequest();

	qint64 lastRequestTime() const;

signals:
	void requestQueueIsNotEmpty();
};



// -------------------------------------------------------------------
//
// UDP Server classes
//
// -------------------------------------------------------------------

class UdpServerSocket : public UdpSocket
{
	Q_OBJECT

private:
	QHostAddress m_bindToAddress;
	qint16 m_port;
	std::shared_ptr<CircularLogger> m_logger;

	QMutex m_clientMapMutex;

	QHash<quint32, UdpClientRequestHandler*> clientRequestHandlerMap;

	void bind();

	virtual void onThreadStarted() final;
	virtual void onThreadFinished() final;

public:
	UdpServerSocket(const QHostAddress& bindToAddress, quint16 port, std::shared_ptr<CircularLogger> logger);
	virtual ~UdpServerSocket();

	virtual void onSocketThreadStarted();
	virtual void onSocketThreadFinished();

	virtual UdpRequestProcessor* createUdpRequestProcessor() { return nullptr; }

signals:
	void receiveRequest(UdpRequest request);

public slots:
	void sendAck(UdpRequest m_request);

private slots:
	void onTimer();
	void onSocketReadyRead();
};


typedef SimpleThread UdpSocketThread;
