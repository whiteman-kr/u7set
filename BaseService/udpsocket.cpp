#include "UdpSocket.h"


// UdpServerSocketWorker class implementation
//


UdpServerSocketWorker::UdpServerSocketWorker(const QHostAddress &address, quint16 port) :
    m_hostAddress(address),
    m_port(port),
    m_secondsTimer(this),
    m_socket(this),
    m_recevedDatagramSize(0),
    m_senderPort(0)
{
}


UdpServerSocketWorker::~UdpServerSocketWorker()
{
}


void UdpServerSocketWorker::onSocketThreadStarted()
{
    m_secondsTimer.start(1000);

    connect(&m_secondsTimer, &QTimer::timeout, this, &UdpServerSocketWorker::onSecondsTimer);

    connect(&m_socket, &QUdpSocket::readyRead, this, &UdpServerSocketWorker::onSocketReadyRead);

    qDebug() << "Socket Thread started";
}


void UdpServerSocketWorker::onSecondsTimer()
{
    if (!m_socket.isValid())
    {
        qDebug() << "Socket is not valid";
    }

    if (m_socket.state() == QAbstractSocket::BoundState)
    {
        qDebug() << "Socket is bounded";
    }
    else
    {
        if (m_socket.bind(m_hostAddress, m_port) == true)
        {
            qDebug() << "Bind OK";
        }
        else
        {
            qDebug() << "Bind failed";
        }
    }

}


void UdpServerSocketWorker::onSocketReadyRead()
{
    m_recevedDatagramSize = m_socket.readDatagram(m_receivedDatagram, MAX_DATAGRAM_SIZE, &m_senderHostAddr, &m_senderPort);
}


// UdpServerSocket class implementation
//

UdpServerSocket::UdpServerSocket(const QHostAddress& address, qint16 port)
{
    UdpServerSocketWorker* serverWorker = new UdpServerSocketWorker(address, port);

    serverWorker->moveToThread(&m_socketThread);

    connect(&m_socketThread, &QThread::finished, serverWorker, &QObject::deleteLater);
    connect(&m_socketThread, &QThread::started, serverWorker, &UdpServerSocketWorker::onSocketThreadStarted);

    m_socketThread.start();
}


UdpServerSocket::~UdpServerSocket()
{
    m_socketThread.quit();
    m_socketThread.wait();
}
