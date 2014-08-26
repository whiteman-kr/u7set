#pragma once

#include <QObject>
#include <assert.h>
#include <QThread>
#include <QTimer>
#include <QUuid>
#include <QUdpSocket>
#include <QTimerEvent>
#include <QMutexLocker>

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


// базовый класс для обработчиков UDP-запросов
//

class UdpRequest
{
public:
    UdpRequest(const QHostAddress& senderAddress, qint16 senderPort, char* receivedData, quint32 receivedDataSize);

    QHostAddress m_senderAddress;
    qint16 m_senderPort;
    char m_requestData[MAX_DATAGRAM_SIZE];
    quint32 m_requestDataSize;
};


class UdpRequestProcessor : public QObject
{
    Q_OBJECT

public:
    UdpRequestProcessor();

    void PutRequest(const QHostAddress& senderAddress, qint16 senderPort, char* receivedData, quint32 recevedDataSize);

public slots:


};


class UdpServerSocket : public QObject
{
    Q_OBJECT

private:
    QMutex m_mutex;

    QHostAddress m_bindToAddress;
    qint16 m_port;

    QUdpSocket m_socket;
    QTimer m_secondsTimer;

    qint64 m_recevedDataSize;
    char m_receivedData[MAX_DATAGRAM_SIZE];
    REQUEST_HEADER* requestHeader;
    QHostAddress m_senderHostAddr;
    quint16 m_senderPort;

    QHash<quint32, UdpRequestProcessor*> requestProcessorMap;

public:
    UdpServerSocket(const QHostAddress& bindToAddress, quint16 port);
    virtual ~UdpServerSocket();

    virtual void datagramReceived();

    virtual void onSocketThreadStarted();
    virtual void onSocketThreadFinished();

   // virtual UdpRequestProcessor* createUdpRequestProcessor();

signals:

public slots:
    void onSocketThreadStartedSlot();
    void onSocketThreadFinishedSlot();

private slots:
    void onSecondsTimer();
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
