#include <algorithm>

#include "../../OnlineLib/CircularLogger.h"
#include "../../UtilsLib/Crc.h"
#include "../../UtilsLib/WUtils.h"
#include <QNetworkDatagram>

#include <HardwareLib/DataProtocols.h>

#include "ModelLinkThread.h"

using namespace RvUdpSim;

// -------------------------------------------------------------------------
//
//	TuningSocketListener class implementaton
//
// -------------------------------------------------------------------------

UdpModelLink::UdpModelLink(const HostAddressPort& listenAddress,
						   int replyPort,
						   std::shared_ptr<CircularLogger> appLogger,
						   std::shared_ptr<CircularLogger> simLogger) :
	m_listenAddress(listenAddress),
	m_replyPort(replyPort),
	m_appLogger(appLogger),
	m_simLogger(simLogger)
{
}

UdpModelLink::~UdpModelLink() {}

std::queue<RvUdpSim::SimRequest> UdpModelLink::popAllRequests()
{
	std::queue<RvUdpSim::SimRequest> result;

	QMutexLocker l(&m_mutex);
	result = std::move(m_requests);
	m_requests = {};

	return result;
}

void UdpModelLink::pushReplies(std::queue<RvUdpSim::SimReply>& replies)
{
	QMutexLocker l(&m_mutex);

	while (replies.empty() == false)
	{
		m_replies.push(replies.front());
		replies.pop();
	}
}

void UdpModelLink::onThreadStarted()
{
	DEBUG_LOG_MSG(m_appLogger, QString(tr("Model Link listening thread is started on address %1.")).arg(m_listenAddress.addressPortStr()));

	initTimer();
}

void UdpModelLink::onThreadFinished()
{
	shutdownTimer();

	closeSocket();

	DEBUG_LOG_MSG(m_appLogger, QString(tr("Model Link listening thread is finished on address %1.")).arg(m_listenAddress.addressPortStr()));
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
	}

	if (m_socket == nullptr)
	{
		return;
	}

	writeSocket();
}

void UdpModelLink::initTimer()
{
	Q_ASSERT(m_timer == nullptr);

	m_timer = new QBasicTimer();
	m_timer->start(10, Qt::PreciseTimer, this);
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
	if (m_requestBuffer != nullptr)
	{
		assert(false);
		return;
	}

	if (m_replyBuffer != nullptr)
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

	connect(m_socket,
			&QUdpSocket::errorOccurred,
			[this](QAbstractSocket::SocketError socketError)
			{
				DEBUG_LOG_ERR(m_appLogger,
							  QString("Socket error - (%1) %2").arg(static_cast<int>(socketError)).arg(m_socket->errorString()));
			});

	connect(m_socket, &QUdpSocket::readyRead, this, &UdpModelLink::onReadyRead, Qt::DirectConnection);

	bool bindResult =
		m_socket->bind(m_listenAddress.address32() == 0 ? QHostAddress::Any : m_listenAddress.address(), m_listenAddress.port());

	if (bindResult == true)
	{
		QVariant osRecvBufSize = m_socket->socketOption(QAbstractSocket::ReceiveBufferSizeSocketOption);

		// successful binding
		//
		DEBUG_LOG_MSG(m_appLogger,
					  QString(tr("Model Link listening socket is created and bound to %1 (OS defined receive buffur size - %2 bytes))"))
						  .arg(m_listenAddress.addressPortStr())
						  .arg(osRecvBufSize.toInt()));

		if (osRecvBufSize.toInt() < 65536)
		{
			QVariant newRecvBufSize(static_cast<int>(65536));

			m_socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, newRecvBufSize);

			QVariant currentBufSize = m_socket->socketOption(QAbstractSocket::ReceiveBufferSizeSocketOption);

			DEBUG_LOG_MSG(m_appLogger, (QString("UdpModelLink: new receive buffer size is set - %1 bytes").arg(currentBufSize.toInt())));

			if (newRecvBufSize.toInt() != currentBufSize.toInt())
			{
				qDebug() << "";
				DEBUG_LOG_WRN(m_appLogger, QString("WARNING!!! Receive buffer size is not changed to required size."));
				DEBUG_LOG_MSG(m_appLogger, QString("Try change value of registry key (create if key is not exist)"));
				DEBUG_LOG_MSG(m_appLogger,
							  QString("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\AFD\\Parameters\\DefaultReceiveWindow"));
				qDebug() << "";
			}
		}

		// Allocate memory for input buffer
		//
		m_requestBufferSize = osRecvBufSize.toInt();
		if (m_requestBufferSize < 0 || m_requestBufferSize > 1048576)
		{
			Q_ASSERT(false);
			m_requestBufferSize = 65536;
		}

		m_requestBuffer = new char[m_requestBufferSize];

		// Allocate memory for output buffer
		//
		m_replyBufferSize = osRecvBufSize.toInt();
		if (m_replyBufferSize < 0 || m_replyBufferSize > 1048576)
		{
			Q_ASSERT(false);
			m_replyBufferSize = 65536;
		}

		m_replyBuffer = new char[m_replyBufferSize];
	}
	else
	{
		DEBUG_LOG_ERR(m_appLogger, QString(tr("Model Link listening socket error binding to %1")).arg(m_listenAddress.addressPortStr()));

		// error binding
		//
		closeSocket();
	}
}

void UdpModelLink::closeSocket()
{
	if (m_socket != nullptr)
	{
		m_socket->close();
		delete m_socket;
		m_socket = nullptr;
	}

	if (m_requestBuffer != nullptr)
	{
		delete[] m_requestBuffer;
		m_requestBuffer = nullptr;
		m_requestBufferSize = 0;
	}

	if (m_replyBuffer != nullptr)
	{
		delete[] m_replyBuffer;
		m_replyBuffer = nullptr;
		m_replyBufferSize = 0;
	}
}

bool UdpModelLink::readSocket()
{
	if (m_socket == nullptr)
	{
		Q_ASSERT(false);
		return false;
	}

	qint64 size = m_socket->pendingDatagramSize();
	if (size == -1)
	{
		DEBUG_LOG_ERR(m_appLogger, tr("Model Link socket pending datagram size: %1").arg(size));

		closeSocket(); // why hasPendingDatagrams returns TRUE?
		return false;
	}

	if (size == 0)
	{
		DEBUG_LOG_ERR(m_appLogger, tr("Empty datagram has arrived (size = %1), discarded.").arg(size));

		size = m_socket->readDatagram(m_requestBuffer, 0);
		return true;
	}

	// DEBUG_LOG_MSG(m_logger, tr("Pending datagram: size: %1").arg(size));

	QHostAddress sourceIP;
	quint16 sourcePort;

	// QNetworkDatagram datagram = m_socket->receiveDatagram();
	// size = datagram.data().size();

	size = m_socket->readDatagram(m_requestBuffer, m_requestBufferSize, &sourceIP, &sourcePort);
	if (size == -1)
	{
		DEBUG_LOG_ERR(m_appLogger, tr("Model Link socket read datagram error: size: %1").arg(size));

		m_errReadSocket++;
		closeSocket();
		return false;
	}

	if (size >= sizeof(SimulatorBridgePacketHeader) + sizeof(int16_t))
	{
		// memcpy(m_requestBuffer, datagram.data().constData(), size);
		SimulatorBridgePacketHeader* packet = reinterpret_cast<SimulatorBridgePacketHeader*>(m_requestBuffer);

		// sourceIP = datagram.senderAddress();
		// sourcePort = datagram.senderPort();

		bool packetOk = true;

		if (packet->marker != SGW_MARKER)
		{
			m_errRequestMarker++;
			DEBUG_LOG_ERR(m_appLogger, tr("Model Link packet received: marker: %1 (expected %2):").arg(packet->marker).arg(SGW_MARKER));
			packetOk = false;
		}

		// Check packet size
		//
		if (size != packet->size)
		{
			m_errRequestSize++;
			DEBUG_LOG_ERR(m_appLogger,
						  tr("Model Link packet received: size: %1 (expected %2), version: %3, type: %4: Wrong size!")
							  .arg(size)
							  .arg(packet->size)
							  .arg(packet->packetVersion)
							  .arg(packet->packetType));
			packetOk = false;
		}

		// Check CRC
		//
		uint16_t crc = calcCrc16(m_requestBuffer, static_cast<uint16_t>(size - sizeof(uint16_t)));
		uint16_t packetCrc = *reinterpret_cast<uint16_t*>(m_requestBuffer + size - sizeof(uint16_t));
		if (crc != packetCrc)
		{
			m_errCrc++;
			DEBUG_LOG_ERR(m_appLogger,
						  tr("Model Link packet received: size: %1, version: %2, type: %3: Wrong CRC!")
							  .arg(packet->size)
							  .arg(packet->packetVersion)
							  .arg(packet->packetType));
			packetOk = false;
		}

		if (packetOk == true)
		{
			/*
			DEBUG_LOG_MSG(m_logger,
						  tr("Model Link packet received: size: %1, version: %2, type: %3")
							  .arg(size)
							  .arg(packet->packetVersion)
							  .arg(packet->packetType));*/

			// Check packet version
			//
			switch (packet->packetVersion)
			{
			case SGW_VERSION_1:
				{
					processModelPacket_V1(packet, {sourceIP.toIPv4Address(), sourcePort});
					break;
				}
			default:
				m_errVersion++;
				Q_ASSERT(false);
				DEBUG_LOG_ERR(m_appLogger,
							  tr("Model Link packet received: size: %1, version: %2, type: %3: Unsupported version!")
								  .arg(packet->size)
								  .arg(packet->packetVersion)
								  .arg(packet->packetType));
			}
		}
	}
	else
	{
		DEBUG_LOG_ERR(m_appLogger, tr("Unknown UDP packet size: %1 bytes").arg(size));
	}

	return true;
}

bool UdpModelLink::writeSocket()
{
	int count = 0;

	do
	{
		QMutexLocker l(&m_mutex);
		if (m_replies.empty() == true)
		{
			break;
		}

		// Take the top reply
		//
		SimReply reply = m_replies.front();
		m_replies.pop();

		l.unlock();

		// Prepare the reply packet
		//
		qint64 size = 0;
		bool ok = prepareReplyPacket(reply, size);
		if (ok == false)
		{
			continue;
		}

		// Send packet to the socket
		//
		int result = m_socket->writeDatagram(m_replyBuffer, size, QHostAddress(reply.addressTo.address32()), m_replyPort);
		if (result == -1)
		{
			DEBUG_LOG_ERR(m_appLogger, tr("UdpModelLink::writeSocket(): error sending data."));
			m_errWriteSocket++;
		}
		else
		{
			if (result < size)
			{
				DEBUG_LOG_ERR(m_appLogger,
							  tr("UdpModelLink::writeSocket(): no all data was sent: %1 bytes, expected: %2.").arg(result).arg(size));
				m_errReplySize++;
			}
		}
	} while (count < 100);

	return true;
}

bool UdpModelLink::processModelPacket_V1(const SimulatorBridgePacketHeader_v1* packet, const HostAddressPort& address)
{
	if (packet == nullptr)
	{
		Q_ASSERT(packet);
		return false;
	}

	if (packet->marker != SGW_MARKER)
	{
		Q_ASSERT(false);
		return false;
	}

	SimRequest r = {
		.type = packet->packetType,
		.addressFrom = address,
	};

	const char* data = reinterpret_cast<const char*>(packet);
	data += sizeof(SimulatorBridgePacketHeader_v1);

	switch (packet->packetType)
	{
	case SGW_SIGNAL_READ:
		{
			SignalsReadRequest rr;

			int16_t count = *reinterpret_cast<const int16_t*>(data);       // Number of signals to read 1..READ_SIGNALS_MAX_COUNT
			data += sizeof(int16_t);

			if (count > READ_SIGNALS_MAX_COUNT)
			{
				Q_ASSERT(count <= READ_SIGNALS_MAX_COUNT);
				count = std::clamp<int16_t>(count, 0, READ_SIGNALS_MAX_COUNT);
			}

			for (int i = 0; i < count; i++) // Signal hashes
			{
				Hash hash = *reinterpret_cast<const Hash*>(data);
				data += sizeof(Hash);
				
				if (hash == 0 || hash == UNDEFINED_HASH) 
				{
					Q_ASSERT(false);
					DEBUG_LOG_ERR(m_appLogger, tr("SGW_SIGNAL_READ: invalid hash arrived: %1.").arg(hash));
				}
				else
				{
					rr.hashes.push_back(hash);
				}
			}

			r.readRequest = rr;
		}
		break;
	case SGW_SIGNAL_WRITE:
		{
			SignalsWriteRequest rw;

			int16_t count = *reinterpret_cast<const int16_t*>(data);        // Number of signals to read 1..WRITE_SIGNALS_MAX_COUNT
			data += sizeof(int16_t);

			if (count > WRITE_SIGNALS_MAX_COUNT) 
			{
				Q_ASSERT(count <= WRITE_SIGNALS_MAX_COUNT);
				count = std::clamp<int16_t>(count, 0, WRITE_SIGNALS_MAX_COUNT);
			}

			const Hash *hashData = reinterpret_cast<const Hash*>(data);
			data += sizeof(Hash) * count;

			const SignalValue* signalValueData = reinterpret_cast<const SignalValue*>(data);
			data += sizeof(SignalValue) * count;


			for (int i = 0; i < count; i++) // Signal hashes and Values
			{
				Hash hash = *hashData;
				hashData++;

				SignalValue sv = *signalValueData;
				signalValueData++;

				if (hash == 0 || hash == UNDEFINED_HASH)
				{
					Q_ASSERT(false);
					DEBUG_LOG_ERR(m_appLogger, tr("SGW_SIGNAL_WRITE: invalid hash arrived: %1.").arg(hash));
				}
				else 
				{
					rw.hashes.push_back(hash);
					rw.values.push_back(sv);
				}
			}

			r.writeRequest = rw;
		}
		break;

	case SGW_COMMAND_GET_STATE:
	case SGW_COMMAND_START:
	case SGW_COMMAND_STOP:
	case SGW_COMMAND_PAUSE:
	case SGW_COMMAND_RESUME:
		{
			// Do nothing
		}
		break;

	default:
		DEBUG_LOG_ERR(m_appLogger, tr("UdpModelLink::processModelPacket: unknown packet type (%1)").arg(packet->packetType));
		return false;
	}

	QMutexLocker l(&m_mutex);
	m_requests.push(r);

	return true;
}

bool UdpModelLink::prepareReplyPacket(const RvUdpSim::SimReply& reply, qint64& size)
{
	SimulatorBridgePacketHeader* header = reinterpret_cast<SimulatorBridgePacketHeader*>(m_replyBuffer);
	header->marker = SGW_MARKER;
	header->packetVersion = SGW_VERSION_1;
	header->reserve0 = 0;
	header->packetType = reply.type;

	char* data = m_replyBuffer + sizeof(SimulatorBridgePacketHeader);

	// the reply
	//
	switch (reply.type)
	{
	case SGW_SIGNAL_READ:
		{
			Q_ASSERT(reply.readReply.has_value());

			Q_ASSERT(reply.readReply.value().states.size() <= READ_SIGNALS_MAX_COUNT);

			// Number of states
			//
			int16_t* count = reinterpret_cast<int16_t*>(data);
			*count = static_cast<int16_t>(reply.readReply.value().states.size());
			data += sizeof(int16_t);

			// States
			//
			for (const auto& state : reply.readReply.value().states)
			{
				SignalState* pState = reinterpret_cast<SignalState*>(data);
				*pState = state;
				data += sizeof(SignalState);
			}

			// Size
			//
			size = sizeof(SimulatorBridgePacketHeader) + sizeof(int16_t) + sizeof(SignalState) * (*count) + sizeof(uint16_t);
		}
		break;
	case SGW_SIGNAL_WRITE:
		{
			Q_ASSERT(reply.writeReply.has_value());

			Q_ASSERT(reply.writeReply.value().errorCodes.size() <= WRITE_SIGNALS_MAX_COUNT);

			// Number of states
			//
			int16_t* count = reinterpret_cast<int16_t*>(data);
			*count = static_cast<int16_t>(reply.writeReply.value().errorCodes.size());
			data += sizeof(int16_t);

			// States
			//
			for (const auto& errorCode : reply.writeReply.value().errorCodes)
			{
				ErrorCode* pErrorCode = reinterpret_cast<ErrorCode*>(data);
				*pErrorCode = errorCode;
				data += sizeof(ErrorCode);
			}

			// Size
			//
			size = sizeof(SimulatorBridgePacketHeader) + sizeof(int16_t) + sizeof(ErrorCode) * (*count) + sizeof(uint16_t);
		}
		break;
	case SGW_COMMAND_GET_STATE:
	case SGW_COMMAND_START:
	case SGW_COMMAND_STOP:
	case SGW_COMMAND_PAUSE:
	case SGW_COMMAND_RESUME:
		{
			Q_ASSERT(reply.stateReply.has_value());

			ErrorCode* errorCode = reinterpret_cast<ErrorCode*>(data);
			*errorCode = reply.stateReply.value().errorCode;
			data += sizeof(ErrorCode);

			SimulatorStateCode* stateCode = reinterpret_cast<SimulatorStateCode*>(data);
			*stateCode = reply.stateReply.value().state;
			data += sizeof(SimulatorStateCode);

			// Size
			//
			size = sizeof(SimulatorBridgePacketHeader) + sizeof(ErrorCode) + sizeof(SimulatorStateCode) + sizeof(uint16_t);
		}
		break;
	default:
		Q_ASSERT(false);
		DEBUG_LOG_ERR(m_appLogger, tr("UdpModelLink::prepareReplyPacket: unknown reply type (%1)").arg(reply.type));

		return false;
	}

	// Set reply size
	//
	header->size = size;

	// Calculate CRC
	//
	uint16_t* pCrc = reinterpret_cast<uint16_t*>(m_replyBuffer + header->size - sizeof(uint16_t));
	*pCrc = ::calcCrc16(m_replyBuffer, header->size - sizeof(uint16_t));

	return true;
}

void UdpModelLink::onReadyRead()
{
	// Read data from socket
	//
	while (m_socket->hasPendingDatagrams() == true)
	{
		if (readSocket() == false)
		{
			break;
		}
	}

	// Signalize that we have some requests
	//
	QMutexLocker l(&m_mutex);
	if (m_requests.empty() == false)
	{
		l.unlock();
		emit requestsArrived();
	}
}

UdpModelLinkThread::UdpModelLinkThread(UdpModelLink* worker) :
	m_worker(worker)
{
	addWorker(worker);

	connect(m_worker,
			&UdpModelLink::requestsArrived,
			this,
			[this]()
			{
				emit requestsArrived();
			});
}

std::queue<RvUdpSim::SimRequest> UdpModelLinkThread::popAllRequests()
{
	return m_worker->popAllRequests();
}

void UdpModelLinkThread::pushReplies(std::queue<RvUdpSim::SimReply> replies)
{
	m_worker->pushReplies(replies);
}
