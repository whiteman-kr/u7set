#include "AppDataReceiver.h"
#include "../lib/AppSignal.h"
#include "../lib/WUtils.h"


// -------------------------------------------------------------------------------
//
// AppDataReceiverThread class implementation
//
// -------------------------------------------------------------------------------

AppDataReceiverThread::AppDataReceiverThread(const HostAddressPort& dataReceivingIP,
								 const AppDataSourcesIP& appDataSourcesIP,
								 CircularLoggerShared log) :
	m_dataReceivingIP(dataReceivingIP),
	m_appDataSourcesIP(appDataSourcesIP),
	m_log(log)
{
	setPriority(QThread::Priority::HighPriority);
}

AppDataReceiverThread::~AppDataReceiverThread()
{
}

void AppDataReceiverThread::fillAppDataReceiveState(Network::AppDataReceiveState *adrs)
{
	adrs->set_receivedframescount(m_receivedFramesCount);

	adrs->set_simframescount(m_simFramesCount);
	adrs->set_errdatagramsize(m_errDatagramSize);
	adrs->set_errsimversion(m_errSimVersion);
	adrs->set_errunknownappdatasourceip(m_errUnknownAppDataSourceIP);
}

void AppDataReceiverThread::run()
{
	DEBUG_LOG_MSG(m_log, QString("AppDataReceiver thread is started (receiving IP %1)").arg(m_dataReceivingIP.addressPortStr()));

	m_thisThread = QThread::currentThread();

	while(isQuitRequested() == false)
	{
		bool result = tryCreateAndBindSocket();

		if (result == false)
		{
			continue;
		}

		receivePackets();
	}

	closeSocket();

	DEBUG_LOG_MSG(m_log, QString("AppDataReceiver thread is finished (receiving IP %1)").arg(m_dataReceivingIP.addressPortStr()));
}

bool AppDataReceiverThread::tryCreateAndBindSocket()
{
	if (m_socket != nullptr)
	{
		closeSocket();
	}

	qint64 prevServerTime = -1;

	while(isQuitRequested() == false)
	{
		qint64 serverTime = QDateTime::currentMSecsSinceEpoch();

		if (prevServerTime != -1 && serverTime - prevServerTime < 1000)
		{
			continue;
		}

		prevServerTime = serverTime;

		qDebug() << C_STR(QString("Try create AppDataReceiverThread listening socket on %1").arg(m_dataReceivingIP.addressPortStr()));

		m_socket = new QUdpSocket();

		bool result = m_socket->bind(m_dataReceivingIP.address(), m_dataReceivingIP.port());

		if (result == true)
		{
			DEBUG_LOG_MSG(m_log, QString("AppDataReceiver listening socket is created and bound to %1").arg(m_dataReceivingIP.addressPortStr()));
			break;
		}

		qDebug() << C_STR(QString("AppDataReceiverThread listening socket binding error to %1").arg(m_dataReceivingIP.addressPortStr()));

		closeSocket();

		msleep(200);
	}

	return m_socket != nullptr;
}

void AppDataReceiverThread::closeSocket()
{
	if (m_socket != nullptr)
	{
		m_socket->close();
		delete m_socket;
		m_socket = nullptr;
	}

	qDebug() << "AppDataReceiverThread listening socket closed";
}

void AppDataReceiverThread::receivePackets()
{
	if (m_socket == nullptr)
	{
		return;
	}

	QHostAddress from;
	Rup::SimFrame simFrame;

	qint64 prevServerTime = QDateTime::currentMSecsSinceEpoch();
	qint64 lastPacketTime = prevServerTime;

	qint64 prevReceivedFramesCount = 0;

	while(isQuitRequested() == false)
	{
		qint64 serverTime = QDateTime::currentMSecsSinceEpoch();

/*		if (m_socket->waitForReadyRead(500) == false)
		{
			if (serverTime - lastPacketTime > 3000)
			{
				qDebug() << "No RUP packets received in 3 seconds";
				closeSocket();
				return;
			}

			continue;
		}*/

		qint64 size = m_socket->pendingDatagramSize();

		if (size == -1)
		{
			if (serverTime - lastPacketTime > 3000)
			{
				qDebug() << "No RUP packets received in 3 seconds";
				closeSocket();
				return;
			}

			usleep(500);
			continue;
		}

		lastPacketTime = serverTime;

		if (size > sizeof(Rup::SimFrame))
		{
			// received datagram too large, skip this datagram
			//
			m_socket->readDatagram(reinterpret_cast<char*>(&simFrame), size, &from);
			m_errDatagramSize++;
			continue;
		}

		size = m_socket->readDatagram(reinterpret_cast<char*>(&simFrame), size, &from);

		if (size == -1)
		{
			DEBUG_LOG_ERR(m_log, QString("AppDataReceiver %1 read socket error %2.").
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

				m_simFramesCount++;
			}
			else
			{
				m_errDatagramSize++;
				continue;
			}
		}

		AppDataSourceShared dataSource = m_appDataSourcesIP.value(ip, nullptr);

		if (dataSource == nullptr)
		{
			m_errUnknownAppDataSourceIP++;

			if (m_unknownAppDataSourcesIP.contains(ip) == false && m_unknownAppDataSourcesIP.count() < 500)
			{
				m_unknownAppDataSourcesIP.insert(ip, ip);
			}

			continue;
		}

		m_receivedFramesCount++;

		dataSource->pushRupFrame(serverTime, simFrame.rupFrame, m_thisThread);

		//

		if (serverTime - prevServerTime > 1000)
		{
			prevServerTime = serverTime;

			qDebug() << C_STR(QString("Receive RUP frames %1/s").arg(m_receivedFramesCount - prevReceivedFramesCount));

			prevReceivedFramesCount = m_receivedFramesCount;
		}
	}
}
