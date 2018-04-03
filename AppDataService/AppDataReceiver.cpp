#include "AppDataReceiver.h"
#include "../lib/AppSignal.h"
#include "../lib/WUtils.h"


// -------------------------------------------------------------------------------
//
// AppDataReceiver class implementation
//
// -------------------------------------------------------------------------------

AppDataReceiver::AppDataReceiver(const HostAddressPort& dataReceivingIP,
								 const AppDataSourcesIP& appDataSourcesIP,
								 CircularLoggerShared log) :
	m_dataReceivingIP(dataReceivingIP),
	m_appDataSourcesIP(appDataSourcesIP),
	m_log(log),
	m_timer1s(this),
	m_shortTimer(this)
{
}

AppDataReceiver::~AppDataReceiver()
{
}

void AppDataReceiver::onThreadStarted()
{
	connect(&m_timer1s, &QTimer::timeout, this, &AppDataReceiver::onTimer1s);
	connect(&m_shortTimer, &QTimer::timeout, this, &AppDataReceiver::onSocketReadyRead);
	
	m_timer1s.setInterval(1000);
	m_timer1s.start();

	m_shortTimer.setInterval(5);
	m_shortTimer.start();

	DEBUG_LOG_MSG(m_log, QString("AppDataReceiver thread is started (receiving IP %1)").arg(m_dataReceivingIP.addressPortStr()));
}

void AppDataReceiver::onThreadFinished()
{
	m_timer1s.stop();

	closeSocket();

	DEBUG_LOG_MSG(m_log, QString("AppDataReceiver thread is finished (receiving IP %1)").arg(m_dataReceivingIP.addressPortStr()));
}

void AppDataReceiver::createAndBindSocket()
{
	if (m_socket == nullptr)
	{
		m_socket = new QUdpSocket(this);

		DEBUG_LOG_MSG(m_log, QString("AppDataReceiver listening socket is created"));

		connect(m_socket, &QUdpSocket::readyRead, this, &AppDataReceiver::onSocketReadyRead);
	}

	if (m_socketBound == false)
	{
		m_socketBound = m_socket->bind(m_dataReceivingIP.address(), m_dataReceivingIP.port());

		if (m_socketBound == true)
		{
			DEBUG_LOG_MSG(m_log, QString("AppDataReceiver listening socket is bound to %1").arg(m_dataReceivingIP.addressPortStr()));
		}
	}
}

void AppDataReceiver::closeSocket()
{
	if (m_socket != nullptr)
	{
		m_socket->close();
		delete m_socket;
		m_socket = nullptr;

		DEBUG_LOG_WRN(m_log, QString("AppDataReceiver listening socket %1 is closed").arg(m_dataReceivingIP.addressPortStr()));
	}

	m_socketBound = false;
}

void AppDataReceiver::onTimer1s()
{
	createAndBindSocket();

	if (m_receivedFramesCount != 0)
	{
		qDebug() << "Receive per second " << m_receivedFramesCount;

		m_receivedFramesCount = 0;
	}
}

void AppDataReceiver::onSocketReadyRead()
{
	if (m_socket == nullptr)
	{
		return;
	}

	QHostAddress from;
	Rup::SimFrame simFrame;

	do
	{
		bool isSimFrame = false;

		qint64 size = m_socket->pendingDatagramSize();

		if (size == -1 || size > sizeof(simFrame))
		{
			break;				// exit from loop if no pending datagram exists
								// or datagram size is exceede sizeof(simFrame)
		}

		size = m_socket->readDatagram(reinterpret_cast<char*>(&simFrame), size, &from);

		if (size == -1)
		{
			DEBUG_LOG_ERR(m_log, QString("AppDataReceiver %1 read socket error %2").
								arg(m_dataReceivingIP.addressPortStr()).arg(m_socket->error()));

			closeSocket();

			return;
		}

		quint32 ip = 0;

		if (size == sizeof(Rup::Frame))
		{
			ip = from.toIPv4Address();
		}
		else
		{
			if (size == sizeof(Rup::SimFrame))
			{
				quint16 simVersion = reverseUint16(simFrame.simVersion);

				if (simVersion != 1)
				{
					m_errSimVersion++;
					continue;
				}

				ip = reverseUint32(simFrame.sourceIP);

				m_simFrameCount++;
			}
			else
			{
				m_errDatagramSize++;
				continue;
			}
		}

		m_receivedFramesCount++;

		qint64 serverTime = QDateTime::currentMSecsSinceEpoch();

		AppDataSourceShared dataSource = m_appDataSourcesIP.value(ip, nullptr);

		if (dataSource == nullptr)
		{
			if (m_unknownAppDataSourcesIP.contains(ip) == false && m_unknownAppDataSourcesIP.count() < 500)
			{
				m_unknownAppDataSourcesIP.insert(ip, ip);
			}

			continue;
		}

		dataSource->pushRupFrame(serverTime, simFrame.rupFrame);

		//	emit rupFrameIsReceived(ip);			uncomment if using AppDataProcessingThread class to process data
		//
		//											for AppDataProcessingThread2 class this imit is not requred!
	}
	while(quitRequested() == false);
}


// -------------------------------------------------------------------------------
//
// AppDataChannelThread class implementation
//
// -------------------------------------------------------------------------------

AppDataReceiverThread::AppDataReceiverThread(const HostAddressPort& dataRecievingIP,
											 const AppDataSourcesIP& appDataSourcesIP,
											 CircularLoggerShared log)
{
	m_appDataReceiver = new AppDataReceiver(dataRecievingIP, appDataSourcesIP, log);

	addWorker(m_appDataReceiver);
}
