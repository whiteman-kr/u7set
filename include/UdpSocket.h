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
//#include <QByteArray>

#include "../include/SocketIO.h"

const int MAX_DATAGRAM_SIZE = 4096;

// -------------------------------------------------------------------
// UDP Client classes
//

class UdpClientSocket : public QObject
{
    Q_OBJECT

private:
    enum UdpClientSocketState
    {
        readyToSend,
        waitingForAck
    };

    QMutex m_mutex;

    QHostAddress m_serverAddress;
    qint16 m_port;
    quint32 m_protocolVersion;
    int m_msTimeout;
    int m_retryCount;
    int m_retryCtr;
    quint32 m_ackTimeoutCtr;
    quint32 m_clientID;

    UdpClientSocketState m_state;
    quint32 m_requestNo;

    QUdpSocket m_socket;
    QTimer m_ackTimer;

    char m_receivedData[MAX_DATAGRAM_SIZE];
    quint32 m_recevedDataSize;
    REQUEST_HEADER* m_receivedHeader;

    char m_sentData[MAX_DATAGRAM_SIZE];
    quint32 m_sentDataSize;
    REQUEST_HEADER* m_sentHeader;

    QHostAddress m_senderHostAddr;
    quint16 m_senderPort;

private:
    void retryRequest();

public:
    UdpClientSocket(const QHostAddress& serverAddress, quint16 port);
    virtual ~UdpClientSocket();

    const QHostAddress& serverAddress() const;
    void setServerAddress(const QHostAddress& serverAddress);

    const quint16 port() const;
    void setPort(quint16 port);

    bool isWaitingForAck() { return m_state == waitingForAck; }

    void setProtocolVersion(quint32 version) { m_protocolVersion = version; }
    void setTimeout(int msTimeout) { m_msTimeout = msTimeout; }
    void setRetryCount(int retryCount) { m_retryCount = retryCount; }

    virtual void onSocketThreadStarted();
    virtual void onSocketThreadFinished();
    virtual void onAckTimeout();
    virtual void onRequestTimeout(const REQUEST_HEADER& requestHeader);
    virtual void onRequestAck(const REQUEST_HEADER& ackHeader, char* ackData, quint32 ackDataSize);
    virtual void onUnknownRequestAck(const REQUEST_HEADER& ackHeader, char* ackData, quint32 ackDataSize);

signals:
    void ackTimeout();
    void ackReceived(REQUEST_HEADER header, QByteArray data);

public slots:
    void onSocketThreadStartedSlot();
    void onSocketThreadFinishedSlot();
    void sendRequest(quint32 requestID, char* requestData, quint32 requestDataSize);

private slots:
    void onSocketReadyRead();
    void onAckTimerTimeout();
};


// -------------------------------------------------------------------
// UDP Server classes
//


class UdpRequest
{
private:
    QHostAddress m_address;
    qint16 m_port;
    char m_requestData[MAX_DATAGRAM_SIZE];
    quint32 m_requestDataSize;

    char* m_dataPtr;

public:
    UdpRequest();
    UdpRequest(const QHostAddress& senderAddress, qint16 senderPort, char* receivedData, quint32 receivedDataSize);

    REQUEST_HEADER* header() { return reinterpret_cast<REQUEST_HEADER*>(m_requestData); }
    quint32 id() { return header()->ID; }
    char* data() { return m_requestData; }
    quint32 dataSize() { return m_requestDataSize; }
    QHostAddress address() { return m_address; }
    quint16 port() { return m_port; }

    bool isEmpty() const;

    void initAck(const UdpRequest& request);

    bool writeDword(quint32 dw);
};



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


class UdpServerSocket : public QObject
{
    Q_OBJECT

private:
    QMutex m_clientMapMutex;

    QHostAddress m_bindToAddress;
    qint16 m_port;

    QUdpSocket m_socket;
    QTimer m_timer;

    qint64 m_recevedDataSize;
    char m_receivedData[MAX_DATAGRAM_SIZE];
    REQUEST_HEADER* requestHeader;
    QHostAddress m_senderHostAddr;
    quint16 m_senderPort;

    QHash<quint32, UdpClientRequestHandler*> clientRequestHandlerMap;

public:
    UdpServerSocket(const QHostAddress& bindToAddress, quint16 port);
    virtual ~UdpServerSocket();

    virtual void datagramReceived();

    virtual void onSocketThreadStarted();
    virtual void onSocketThreadFinished();

    virtual UdpRequestProcessor* createUdpRequestProcessor() { return nullptr; }

signals:
    void request(UdpRequest request);

public slots:
    void onSocketThreadStartedSlot();
    void onSocketThreadFinishedSlot();
    void sendAck(UdpRequest request);

private slots:
    void onTimer();
    void onSocketReadyReadSlot();
};



// -------------------------------------------------------------------
// UDP sockets' thread
//

class UdpSocketThread : public QObject
{
    Q_OBJECT

public:
    UdpSocketThread();
    void run(UdpClientSocket* clientSocket);
    void run(UdpServerSocket* serverSocket);

    virtual ~UdpSocketThread();

private:
    QThread m_socketThread;
};
