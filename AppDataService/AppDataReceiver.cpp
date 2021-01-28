#include "AppDataReceiver.h"
#include "../lib/AppSignal.h"
#include "../lib/WUtils.h"
#include "../lib/Socket.h"

// -------------------------------------------------------------------------------
//
// AppDataReceiverThread class implementation
//
// -------------------------------------------------------------------------------

AppDataReceiverThread::AppDataReceiverThread(const HostAddressPort& dataReceivingIP,
								 const AppDataSourcesIP& appDataSourcesIP,
								 E::SoftwareRunMode swRunMode,
								 CircularLoggerShared log) :
	m_dataReceivingIP(dataReceivingIP),
	m_appDataSourcesIP(appDataSourcesIP),
	m_log(log)
{
	m_isSimulationMode = (swRunMode == E::SoftwareRunMode::Simulation);

	setPriority(QThread::Priority::HighPriority);
}

AppDataReceiverThread::~AppDataReceiverThread()
{
}

void AppDataReceiverThread::fillAppDataReceiveState(Network::AppDataReceiveState* adrs)
{
	adrs->set_receivingrate(m_receivingRate);
	adrs->set_udpreceivingrate(m_udpReceivingRate);
	adrs->set_rupframesreceivingrate(m_rupFramesReceivingRate);

	adrs->set_rupframescount(m_rupFramesCount);
	adrs->set_simframescount(m_simFramesCount);

	adrs->set_errdatagramsize(m_errDatagramSize);
	adrs->set_errsimversion(m_errSimVersion);
	adrs->set_errunknownappdatasourceip(m_errUnknownAppDataSourceIP);
	adrs->set_errrupframecrc(m_errRupFrameCRC);

	adrs->set_errnotexpectedsimpacket(m_errNotExpectedSimPacket);
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
			msleep(200);
			continue;
		}

		prevServerTime = serverTime;

		qDebug() << C_STR(QString("Try create AppDataReceiverThread listening socket on %1").arg(m_dataReceivingIP.addressPortStr()));

		m_socket = new QUdpSocket();

		bool result = m_socket->bind(m_dataReceivingIP.address(), m_dataReceivingIP.port());

		if (result == false)
		{
			qDebug() << C_STR(QString("AppDataReceiverThread listening socket binding error to %1").arg(m_dataReceivingIP.addressPortStr()));

			closeSocket();

			msleep(200);

			continue;
		}

		// bind Ok

		DEBUG_LOG_MSG(m_log, QString("AppDataReceiver listening socket is created and bound to %1").arg(m_dataReceivingIP.addressPortStr()));

		QVariant osRecvBufSize = m_socket->socketOption(QAbstractSocket::ReceiveBufferSizeSocketOption);

		DEBUG_LOG_MSG(m_log, QString("AppDataReceiver: OS defined receive buffer size - %1 bytes").arg(osRecvBufSize.toInt()));

		QVariant newRecvBufSize(static_cast<int>(2 * 1024 * 1024));

		m_socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, newRecvBufSize);

		QVariant currentBufSize = m_socket->socketOption(QAbstractSocket::ReceiveBufferSizeSocketOption);

		DEBUG_LOG_MSG(m_log, (QString("AppDataReceiver: new receive buffer size is set - %1 bytes").arg(currentBufSize.toInt())));

		if (newRecvBufSize.toInt() != currentBufSize.toInt())
		{
			qDebug() << "";
			DEBUG_LOG_WRN(m_log, QString("WARNING!!! Receive buffer size is not changed to required size."));
			DEBUG_LOG_MSG(m_log, QString("Try change value of registry key (create if key is not exist)"));
			DEBUG_LOG_MSG(m_log, QString("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\AFD\\Parameters\\DefaultReceiveWindow"));
			qDebug() << "";
		}

		break;
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

	const int BUFFER_SIZE = sizeof(Rup::SimFrame) + 1;

	char receiveBuffer[BUFFER_SIZE];

	Rup::SimFrame& simFrame = *reinterpret_cast<Rup::SimFrame*>(receiveBuffer);

	qint64 prevServerTime = QDateTime::currentMSecsSinceEpoch();
	qint64 lastPacketTime = prevServerTime;

	while(isQuitRequested() == false)
	{
		qint64 serverTime = QDateTime::currentMSecsSinceEpoch();

		if (serverTime - prevServerTime > 1000)
		{
			prevServerTime = serverTime;

			m_receivingRate.store(m_receivedPerSecond);
			m_receivedPerSecond = 0;

			m_udpReceivingRate.store(m_udpReceivedPerSecond);
			m_udpReceivedPerSecond = 0;

			m_rupFramesReceivingRate.store(m_rupFramesReceivedPerSecond);
			m_rupFramesReceivedPerSecond = 0;

			qDebug() << C_STR(QString("Receive RUP frames %1").arg(m_rupFramesReceivingRate));
		}

		qint64 size = m_socket->readDatagram(receiveBuffer, BUFFER_SIZE, &from);

		if (size == -1)
		{
			if (serverTime - lastPacketTime > 3000)
			{
				qDebug() << "No RUP packets received in 3 seconds";
				closeSocket();
				return;
			}

			msleep(5);
			continue;
		}

		m_udpReceivedPerSecond++;
		m_receivedPerSecond += static_cast<int>(size);

		lastPacketTime = serverTime;

		quint32 ip = 0;

		if (size == sizeof(Rup::Frame))
		{
			ip = from.toIPv4Address();
		}
		else
		{
			if (m_isSimulationMode == false)
			{
				m_errNotExpectedSimPacket++;

				if ((m_errNotExpectedSimPacket % 1000) == 0)
				{
					qDebug() << C_STR(QString("Software is not in SIMULATION mode, %1 sim packets has been ignored.").
									  arg(m_errNotExpectedSimPacket));
				}

				continue;
			}

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
				// received datagram  has unknown size, skip this datagram
				//
				m_errDatagramSize++;
				continue;
			}
		}

		if (simFrame.rupFrame.checkCRC64() == false)
		{
			m_errRupFrameCRC++;
			continue;
		}

		m_rupFramesReceivedPerSecond++;
		m_rupFramesCount++;

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

		dataSource->pushRupFrame(serverTime, simFrame.rupFrame, m_thisThread);
	}
}
