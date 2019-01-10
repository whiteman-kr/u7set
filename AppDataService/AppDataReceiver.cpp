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
}

AppDataReceiverThread::~AppDataReceiverThread()
{
}

void AppDataReceiverThread::run()
{
	DEBUG_LOG_MSG(m_log, QString("AppDataReceiver thread is started (receiving IP %1)").arg(m_dataReceivingIP.addressPortStr()));

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
	if (m_socketIsWorkable == true)
	{
		return true;
	}

	m_counter200ms++;

	if (m_counter200ms < 5)
	{
		msleep(200);
		return false;
	}

	m_counter200ms = 0;

	m_socket = new QUdpSocket(this);

	bool result = m_socket->bind(m_dataReceivingIP.address(), m_dataReceivingIP.port());

	if (result == true)
	{
		DEBUG_LOG_MSG(m_log, QString("AppDataReceiver listening socket is created and bound to %1").arg(m_dataReceivingIP.addressPortStr()));

		m_socketIsWorkable = true;

		return true;
	}

	closeSocket();

	msleep(200);

	return false;
}

void AppDataReceiverThread::closeSocket()
{
	if (m_socket != nullptr)
	{
		m_socket->close();
		delete m_socket;
		m_socket = nullptr;
	}

	m_socketIsWorkable = false;
}

void AppDataReceiverThread::receivePackets()
{
	if (m_socketIsWorkable == false || m_socket == nullptr)
	{
		assert(false);
		return;
	}

	QHostAddress from;
	Rup::SimFrame simFrame;

	while(isQuitRequested() == false)
	{
		qint64 size = m_socket->pendingDatagramSize();

		if (size == -1)
		{
			// is no datagram available, short sleep on 200mcs
			//
			usleep(200);
			continue;
		}

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
			DEBUG_LOG_ERR(m_log, QString("AppDataReceiver %1 read socket error %2").
								arg(m_dataReceivingIP.addressPortStr()).arg(m_socket->error()));

			closeSocket();

			m_counter200ms = 5;			// for immediately socket creation

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

		//

		Rup::Header h = simFrame.rupFrame.header;

		h.reverseBytes();

		//
		m_receivedFramesCount++;

		qint64 serverTime = QDateTime::currentMSecsSinceEpoch();

		AppDataSourceShared dataSource = m_appDataSourcesIP.value(ip, nullptr);

		if (dataSource == nullptr)
		{
			if (m_unknownAppDataSourcesIP.contains(ip) == false && m_unknownAppDataSourcesIP.count() < 500)
			{
				m_unknownAppDataSourcesIP.insert(ip, ip);
				m_unknownAppDataSourcesCount++;
			}

			continue;
		}

		dataSource->pushRupFrame(serverTime, simFrame.rupFrame);
	}
}
