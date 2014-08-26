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


class UdpSocketThread : public QObject
{
    Q_OBJECT

public:
    UdpSocketThread();
    void runClientSocket(UdpClientSocket* clientSocket);

    virtual ~UdpSocketThread();

private:
    QThread m_socketThread;
};


// -------------------------------------------------------------------
// UDP Server classes
//


class UdpServerSocketWorker : public QObject
{
    Q_OBJECT

public:
    UdpServerSocketWorker(const QHostAddress& bindToAddress, quint16 port);
    virtual ~UdpServerSocketWorker();

    virtual void datagramReceived();

signals:

public slots:
    void onSocketThreadStarted();


private slots:
    void onSecondsTimer();
    void onSocketReadyRead();

private:
    QHostAddress m_bindToAddress;
    qint16 m_port;

    QUdpSocket m_socket;
    QTimer m_secondsTimer;

    qint64 m_recevedDatagramSize;
    char m_receivedDatagram[MAX_DATAGRAM_SIZE];
    QHostAddress m_senderHostAddr;
    quint16 m_senderPort;
};


class UdpServerSocket : public QObject
{
    Q_OBJECT

public:
    UdpServerSocket(const QHostAddress& address, qint16 port);
    virtual ~UdpServerSocket();

private:
    QThread m_socketThread;
};
