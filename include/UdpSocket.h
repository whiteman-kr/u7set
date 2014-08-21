#pragma once

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QUdpSocket>
#include <QTimerEvent>

const int MAX_DATAGRAM_SIZE = 4096;

// -------------------------------------------------------------------
// UDP Client classes
//

class UdpClientSocketWorker : public QObject
{
    Q_OBJECT

public:
    UdpClientSocketWorker(const QHostAddress& serverAddress, quint16 port);
    virtual ~UdpClientSocketWorker();

    virtual void datagramReceived();

signals:

public slots:
    void onSocketThreadStarted();


private slots:
    void onSecondsTimer();
    void onSocketReadyRead();

private:
    QHostAddress m_serverAddress;
    qint16 m_port;

    QUdpSocket m_socket;
    QTimer m_secondsTimer;

    qint64 m_recevedDatagramSize;
    char m_receivedDatagram[MAX_DATAGRAM_SIZE];
    QHostAddress m_senderHostAddr;
    quint16 m_senderPort;
};


class UdpClientSocket : public QObject
{
    Q_OBJECT

public:
    UdpClientSocket(const QHostAddress& serverAddress, qint16 port);
    virtual ~UdpClientSocket();

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
