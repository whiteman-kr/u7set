#include <algorithm>

#include "../../OnlineLib/CircularLogger.h"
#include "../../UtilsLib/Crc.h"
#include "../../UtilsLib/WUtils.h"

#include <HardwareLib/DataProtocols.h>

#include "ModelLinkThread.h"

using namespace ModelLink;

// -------------------------------------------------------------------------
//
//	TuningSocketListener class implementaton
//
// -------------------------------------------------------------------------

UdpModelLink::UdpModelLink(const HostAddressPort& listenIP, std::shared_ptr<CircularLogger> logger) :
	m_listenIP(listenIP),
	m_logger(logger)
{
}

UdpModelLink::~UdpModelLink() {}

void UdpModelLink::onThreadStarted()
{
	DEBUG_LOG_MSG(m_logger, QString(tr("Model Link listening thread is started on address %1.")).arg(m_listenIP.addressPortStr()));

	initTimer();
}

void UdpModelLink::onThreadFinished()
{
	shutdownTimer();

	closeSocket();

	DEBUG_LOG_MSG(m_logger, QString(tr("Model Link listening thread is finished on address %1.")).arg(m_listenIP.addressPortStr()));
}

void UdpModelLink::timerEvent(QTimerEvent* event)
{
	TEST_PTR_RETURN(m_timer);

	if (event->timerId() != m_timer->timerId())
	{
		return;
	}

	if (m_socket == nullptr)
	{
		// Socket is not opened
		//
		createSocket();

		if (m_socket == nullptr)
		{
			return;
		}
	}

	// Socket is created
	//
	if (m_socket->hasPendingDatagrams() == true)
	{
		int count = 0;

		do
		{
			count++;

			bool result = readSocket();

			if (result == false)
			{
				break;
			}
		} while (m_socket->hasPendingDatagrams() == true && count < 500);
	}
}

void UdpModelLink::initTimer()
{
	Q_ASSERT(m_timer == nullptr);

	m_timer = new QBasicTimer();

	m_timer->start(1, Qt::PreciseTimer, this);
}

void UdpModelLink::shutdownTimer()
{
	TEST_PTR_RETURN(m_timer);

	m_timer->stop();

	delete m_timer;

	m_timer = nullptr;
}

void UdpModelLink::createSocket()
{
	if (m_socket != nullptr)
	{
		assert(false);
		return;
	}

	qint64 now = QDateTime::currentMSecsSinceEpoch();

	if (now - m_socketCreateLastTime < 1000)
	{
		return;
	}

	m_socketCreateLastTime = now;

	m_socket = new QUdpSocket();

	bool bindResult = m_socket->bind(m_listenIP.address(), m_listenIP.port());

	if (bindResult == true)
	{
		QVariant osRecvBufSize = m_socket->socketOption(QAbstractSocket::ReceiveBufferSizeSocketOption);

		// successful binding
		//
		DEBUG_LOG_MSG(m_logger,
					  QString(tr("Model Link listening socket is created and bound to %1 (OS defined receive buffur size - %2 bytes))"))
						  .arg(m_listenIP.addressPortStr())
						  .arg(osRecvBufSize.toInt()));

		if (osRecvBufSize.toInt() < 65536)
		{
			QVariant newRecvBufSize(static_cast<int>(65536));

			m_socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, newRecvBufSize);

			QVariant currentBufSize = m_socket->socketOption(QAbstractSocket::ReceiveBufferSizeSocketOption);

			DEBUG_LOG_MSG(m_logger, (QString("UdpModelLink: new receive buffer size is set - %1 bytes").arg(currentBufSize.toInt())));

			if (newRecvBufSize.toInt() != currentBufSize.toInt())
			{
				qDebug() << "";
				DEBUG_LOG_WRN(m_logger, QString("WARNING!!! Receive buffer size is not changed to required size."));
				DEBUG_LOG_MSG(m_logger, QString("Try change value of registry key (create if key is not exist)"));
				DEBUG_LOG_MSG(m_logger,
							  QString("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\AFD\\Parameters\\DefaultReceiveWindow"));
				qDebug() << "";
			}
		}
	}
	else
	{
		DEBUG_LOG_ERR(m_logger, QString(tr("TModel Link listening socket error binding to %1")).arg(m_listenIP.addressPortStr()));

		// error binding
		//
		closeSocket();
	}
}

void UdpModelLink::closeSocket()
{
	if (m_socket == nullptr)
	{
		return;
	}

	m_socket->close();
	delete m_socket;
	m_socket = nullptr;
}

bool UdpModelLink::readSocket()
{
	if (m_socket == nullptr)
	{
		Q_ASSERT(false);
		return false;
	}

	QHostAddress sourceIP;
	// SimRupFotip reply;

	char reply[65536];

	qint64 size = m_socket->pendingDatagramSize();

	if (size == -1)
	{
		closeSocket(); // why hasPendingDatagrams returns TRUE?
		return false;
	}

	/*
	if (size != sizeof(RupFotip) && size != sizeof(SimRupFotip))
	{
		m_errReplySize++;

		// anyway read datagram but don't process it
		//
		m_socket->readDatagram(reinterpret_cast<char*>(&reply), sizeof(reply), &sourceIP);

		incErrReplySizeOfTuningSource(tuningSourceIP);

		qDebug() << C_STR(QString("Wrong datagram size from %1. Reply rejected.").arg(tuningSourceIP.toString()));
		return true;
	}*/

	size = m_socket->readDatagram(reinterpret_cast<char*>(&reply), sizeof(reply), &sourceIP);

	if (size == -1)
	{
		m_errReadSocket++;
		closeSocket();
		return false;
	}

	SimulatorBridgePacket* packet = reinterpret_cast<SimulatorBridgePacket*>(&reply);

	if (size >= sizeof(SimulatorBridgePacket) && packet->marker == SGW_MARKER && packet->packetVersion == 1)
	{
		uint16_t crc = calcCrc16(reply, size);
		uint16_t packetCrc = *(reply + size - sizeof(uint16_t));

		if (crc != packetCrc)
		{
			DEBUG_LOG_ERR(m_logger,
						  tr("Model Link packet received: size: %1, version: %2, type: %3: Wrong CRC!")
							  .arg(size)
							  .arg(packet->packetVersion)
							  .arg(packet->packetType));
		}
		else
		{
			DEBUG_LOG_MSG(m_logger,
						  tr("Model Link packet received: size: %1, version: %2, type: %3")
							  .arg(size)
							  .arg(packet->packetVersion)
							  .arg(packet->packetType));

			processModelPacket(*packet);
		}
	}
	else
	{
		DEBUG_LOG_MSG(m_logger, tr("UDP packet received: %1 bytes").arg(size));
	}

	return true;
}

bool UdpModelLink::writeSocket()
{
	/*
	Q_ASSERT(sizeof(Rup::Frame) == Socket::ENTIRE_UDP_SIZE);
	Q_ASSERT(sizeof(RupFotip) == Socket::ENTIRE_UDP_SIZE);
	Q_ASSERT(sizeof(Fotip::Frame) == Rup::FRAME_DATA_SIZE);
	Q_ASSERT(sizeof(Fotip::Header) == 128);

	Q_ASSERT(m_waitReply == false);

	RupFotip& rupFotip = request.rupFotip;

	if (retry == true && m_fotipVersion >= Fotip::V3)
	{
		m_fotipRequestNumerator++;
		rupFotip.fotipFrame.header.requestNumerator = m_fotipRequestNumerator;
	}

	// convert headers to BigEndian
	//
	rupFotip.rupHeader.reverseBytes();
	rupFotip.fotipFrame.header.reverseBytes();

	//

	rupFotip.calcCRC64();

	qint64 sent = 0;

	if (m_isSimulationMode == false)
	{
		// packet sending to real LM
		//
		sent = m_socket.writeDatagram(reinterpret_cast<char*>(&rupFotip), sizeof(rupFotip), m_sourceIP.address(), m_sourceIP.port());
	}
	else
	{
		// packet sending to Simulator
		//
		request.simVersion = reverseUint16(1);
		request.tuningSourceIP = reverseUint32(m_sourceIP.address32());

		sent = m_socket.writeDatagram(reinterpret_cast<char*>(&request), sizeof(request), m_tuningSimIP.address(), m_tuningSimIP.port());
	}

	m_state.requestCount++;

	// revert headers to LittleEndian
	//
	rupFotip.rupHeader.reverseBytes();
	rupFotip.fotipFrame.header.reverseBytes();

	//

	m_sourceThread.service().logTuningPacket(true,
											 static_cast<Fotip::OpCode>(rupFotip.fotipFrame.header.operationCode),
											 rupFotip.rupHeader.numerator,
											 rupFotip.fotipFrame.header.requestNumerator);
	//

	quint32 rawDiscreteValue = rupFotip.fotipFrame.write.discreteValue;
	quint32 rawBitmask = rupFotip.fotipFrame.write.bitMask;
	quint16 requestID = rupFotip.rupHeader.numerator;

	//

	m_lastRequestTime = QDateTime::currentMSecsSinceEpoch();

	m_waitReply = true;

	if (sent == -1)
	{
		m_state.errSent++;
		return;
	}

	if (sent < static_cast<qint64>(sizeof(m_request)))
	{
		m_state.errPartialSent++;
	}

	// logging
	//
	switch (static_cast<Fotip::OpCode>(rupFotip.fotipFrame.header.operationCode))
	{
	case Fotip::OpCode::Write:
		{
			QString valueStr = rupFotip.fotipFrame.valueStr(true);

			if (rupFotip.fotipFrame.isDiscreteData() == true)
			{
				DEBUG_LOG_MSG(m_logger,
							  QString("%1 RupFotip WRITE request is sent to %2 (%3), signal %4 value %5."
									  "StartAddrW %6, OffsetInFrameW %7, RawValue32 %8 BE, Bitmask32 %9 BE")
								  .arg(toHex(requestID))
								  .arg(sourceEquipmentID())
								  .arg(m_sourceIP.addressStr())
								  .arg(appSignalID)
								  .arg(valueStr)
								  .arg(rupFotip.fotipFrame.header.startAddressW)
								  .arg(rupFotip.fotipFrame.header.offsetInFrameW)
								  .arg(rawDiscreteValue, 8, 16, QLatin1Char('0'))
								  .arg(rawBitmask, 8, 16, QLatin1Char('0')));
			}
			else
			{
				DEBUG_LOG_MSG(m_logger,
							  QString("%1 RupFotip WRITE request is sent to %2 (%3), signal %4 value %5")
								  .arg(toHex(requestID))
								  .arg(sourceEquipmentID())
								  .arg(m_sourceIP.addressStr())
								  .arg(appSignalID)
								  .arg(valueStr));
			}
		}
		break;

	case Fotip::OpCode::Apply:
		DEBUG_LOG_MSG(m_logger,
					  QString("%1 RupFotip APPLY request is sent to %2 (%3)")
						  .arg(toHex(requestID))
						  .arg(sourceEquipmentID())
						  .arg(m_sourceIP.addressStr()));
		break;

	case Fotip::OpCode::Read:
		break;

	default:
		assert(false);
	}*/
	return true;
}

bool UdpModelLink::processModelPacket(const SimulatorBridgePacket& packet)
{
	return true;
}


UdpModelLinkThread::UdpModelLinkThread(UdpModelLink* worker)
{
	addWorker(worker);
}
