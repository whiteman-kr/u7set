#include <algorithm>
#include "AsyncTcpServer.h"

AsyncTcpSession::AsyncTcpSession(const SoftwareInfo& swInfo,
								 const AppSignals& appSignals,
								 const std::vector<HostAddressPort>& serviceAddresses, 
								 asio::ip::tcp::socket socket,
								 CircularLoggerShared log) :
	LogWrapper(log),
	m_swInfo(swInfo),
	m_appSignals(appSignals),
	m_serviceAdresses(serviceAddresses),
	m_socket(std::move(socket)),
	m_strand(asio::make_strand(m_socket.get_executor()))
{
	auto className = QString("AsyncTcpSession of %1").arg(remoteIpPortStr());
	setClassName(className);

	setBufferSize(GCL::GW_MAX_PAYLOAD_SIZE);
}

AsyncTcpSession::~AsyncTcpSession() 
{
}

void AsyncTcpSession::start()
{
	asio::dispatch(m_strand,
				   [self = shared_from_this()]()
				   {
					   self->onStarted();
					   self->m_started = true;
					   self->startReceive();
				   });
}

void AsyncTcpSession::stop()
{
	asio::dispatch(m_strand,
				   [self = shared_from_this()]()
				   {
					   std::error_code ec;

					   self->m_socket.shutdown(asio::ip::tcp::socket::shutdown_both, ec);

					   self->m_socket.close(ec);

					   self->onStopped();
				   });
}

void AsyncTcpSession::setBufferSize(size_t size)
{
	if (m_started == true)
	{
		Q_ASSERT(false); // read already started, can't change buffer size
		return;
	}

	Q_ASSERT(size >= MIN_READ_BUFFER_SIZE);
	Q_ASSERT(size <= MAX_READ_BUFFER_SIZE);

	size = std::clamp(size, MIN_READ_BUFFER_SIZE, MAX_READ_BUFFER_SIZE);
	m_recvBuf.resize(size);
	m_recvBufSize = 0;
	m_sendBuf.resize(size);
}

QString AsyncTcpSession::remoteIpPortStr() const
{
	std::error_code ec;

	asio::ip::tcp::endpoint remote = m_socket.remote_endpoint(ec);

	if (ec)
	{
		return "unknown";
	}

	return QString("%1:%2").arg(qstr(remote.address().to_string())).arg(remote.port());
}

void AsyncTcpSession::onStarted() 
{
}

void AsyncTcpSession::onStopped() 
{
}

void AsyncTcpSession::onError(const std::error_code& ec)
{
	logErr(QString::fromStdString(ec.message()));
}

void AsyncTcpSession::sendErrReply(const GCL::GwMessageHeader& requestHeader, GCL::GwErrorCode errCode)
{
	sendReply(requestHeader.requestID, errCode, nullptr, 0);
}

void AsyncTcpSession::sendOkReply(const GCL::GwMessageHeader& requestHeader, const char* payloadData, size_t payloadSize)
{
	sendReply(requestHeader.requestID, GCL::GwErrorCode::GWC_SUCCESS, payloadData, payloadSize);
}

void AsyncTcpSession::sendReply(uint32_t requestID, GCL::GwErrorCode errCode, const char* payloadData, size_t payloadSize)
{
	m_sendBuf.clear();

	if (GCL::GW_MSG_HEADER_SIZE + payloadSize + GCL::GW_MSG_CRC_SIZE > GCL::GW_MAX_PAYLOAD_SIZE)
	{
		Q_ASSERT(false);
		payloadData = nullptr;
		payloadSize = 0;
		errCode = GCL::GwErrorCode::GWC_GATEWAY_INTERNAL_ERROR;
	}

	if (errCode != GCL::GwErrorCode::GWC_SUCCESS)
	{
		Q_ASSERT(payloadData == nullptr);
		Q_ASSERT(payloadSize == 0);
		payloadSize = 0;
	}

	GCL::GwMessageHeader header;

	header.requestID = requestID;
	header.payloadSize = static_cast<uint32_t>(payloadSize);
	header.statusCode = static_cast<uint32_t>(errCode);
	
	const char* headerPtr = reinterpret_cast<const char*>(&header);

	m_sendBuf.assign(headerPtr, headerPtr + GCL::GW_MSG_HEADER_SIZE);

	if (payloadSize > 0)
	{
		m_sendBuf.insert(m_sendBuf.end(), payloadData, payloadData + payloadSize);
	}

	uint32_t crc = Radiy::CRC32(m_sendBuf.data(), GCL::GW_MSG_HEADER_SIZE + payloadSize);

	const char* crcPtr = reinterpret_cast<const char*>(&crc);

	m_sendBuf.insert(m_sendBuf.end(), crcPtr, crcPtr + sizeof(crc));

	startSend();
}

size_t AsyncTcpSession::skipRequest(size_t requestSize, char* recvBuf, size_t recvBufSize)
{
	if (recvBufSize < requestSize)
	{
		Q_ASSERT(false);
		return recvBufSize;
	}

	const size_t restSize = recvBufSize - requestSize;

	if (restSize > 0)
	{
		std::memmove(recvBuf, recvBuf + requestSize, restSize);
	}

	return restSize;
}

bool AsyncTcpSession::isHandshakeCompleted() const
{
	return m_handshakeCompleted;
}
void AsyncTcpSession::setHandshakeCompleted(bool completed)
{
	m_handshakeCompleted = completed;
}

void AsyncTcpSession::startReceive()
{
	m_socket.async_read_some(asio::buffer(m_recvBuf.data() + m_recvBufSize, m_recvBuf.size() - m_recvBufSize),
							 asio::bind_executor(m_strand,
								[self = shared_from_this()](const std::error_code& ec, std::size_t size)
								{
									if (ec)
									{
										self->onError(ec);
										return;
									}

									self->m_recvBufSize += size;

									self->onDataReceived(self->m_recvBuf.data(), self->m_recvBufSize);
								}));
}

void AsyncTcpSession::startSend()
{
	if (m_writeInProgress == true)
	{
		Q_ASSERT(false);
		return;
	}

	if (m_sendBuf.empty())
	{
		Q_ASSERT(false);
		return;
	}

	m_writeInProgress = true;

	asio::async_write(m_socket,
					  asio::buffer(m_sendBuf),
					  asio::bind_executor(m_strand,
								[self = shared_from_this()](const std::error_code& ec, std::size_t)
								{
									self->m_writeInProgress = false;

									if (ec)
									{
										self->onError(ec);
										return;
									}

									self->m_sendBuf.clear();

									self->startReceive();
								}));
}

void AsyncTcpSession::onDataReceived(char* recvBuf, std::size_t recvBufSize)
{
	while (recvBufSize >= GCL::GW_MSG_HEADER_SIZE)
	{
		GCL::GwMessageHeader header;

		std::memcpy(&header, recvBuf, GCL::GW_MSG_HEADER_SIZE);

		if (header.payloadSize > GCL::GW_MAX_MSG_PAYLOAD_SIZE)
		{
			sendErrReply(header, GCL::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
			m_errCount++;
			recvBufSize = 0;
			return;
		}

		const size_t requestSize = GCL::GW_MSG_HEADER_SIZE + header.payloadSize + GCL::GW_MSG_CRC_SIZE;

		if (recvBufSize < requestSize)
		{
			return;
		}

		if (requestSize > GCL::GW_MAX_PAYLOAD_SIZE)
		{
			sendErrReply(header, GCL::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
			m_errCount++;
			recvBufSize = 0;
			return;
		}

		if (checkRequestID(header.requestID) == false)
		{
			sendErrReply(header, GCL::GwErrorCode::GWC_INVALID_REQUEST);
			m_errCount++;
			recvBufSize = skipRequest(requestSize, recvBuf, recvBufSize);
			return;
		}

		GCL::GwErrorCode errCode = GCL::GwErrorCode::GWC_SUCCESS;

		if (checkPayloadSize(header, recvBuf, recvBufSize, errCode) == false)
		{
			sendErrReply(header, errCode);
			m_errCount++;
			recvBufSize = skipRequest(requestSize, recvBuf, recvBufSize);
			continue;
		}

		uint32_t calcCrc = Radiy::CRC32(recvBuf, GCL::GW_MSG_HEADER_SIZE + header.payloadSize);

		uint32_t receivedCrc;
		std::memcpy(&receivedCrc, recvBuf + GCL::GW_MSG_HEADER_SIZE + header.payloadSize, sizeof(receivedCrc));

		if (calcCrc != receivedCrc)
		{
			logErr(QString("request %1 error CRC 0x%2 (expected 0x%3)")
					   .arg(header.requestID)
					   .arg(receivedCrc, 8, 16, QChar('0'))
					   .arg(calcCrc, 8, 16, QChar('0')));

			sendErrReply(header, GCL::GwErrorCode::GWC_CRC_ERROR);
			m_errCount++;
			recvBufSize = skipRequest(requestSize, recvBuf, recvBufSize);
			continue;
		}

		if (isHandshakeRequest(header.requestID) == false && m_handshakeCompleted == false)
		{
			sendErrReply(header, GCL::GwErrorCode::GWC_HANDSHAKE_REQUIRED);
			m_errCount++;
			recvBufSize = skipRequest(requestSize, recvBuf, recvBufSize);
			continue;
		}

		bool result = processRequest(header, recvBuf + GCL::GW_MSG_HEADER_SIZE, header.payloadSize);

		if (result == false)
		{
			m_errCount++;
		}

		recvBufSize = skipRequest(requestSize, recvBuf, recvBufSize);
	}
}