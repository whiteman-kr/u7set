#include <algorithm>
#include "AsyncTcpServer.h"

AsyncTcpSession::AsyncTcpSession(asio::ip::tcp::socket socket, CircularLoggerShared log) :
	LogWrapper(log),
	m_socket(std::move(socket)),
	m_strand(asio::make_strand(m_socket.get_executor()))
{
	auto className = QString("AsyncTcpSession of %1").arg(remoteIpPortStr());
	setClassName(className);

	m_readBuffer.resize(MIN_READ_BUFFER_SIZE);
	m_writeBuffer.reserve(MIN_READ_BUFFER_SIZE);
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
					   self->startRead();
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
	m_readBuffer.resize(size);
	m_writeBuffer.reserve(size);
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
	(void)ec;
}

void AsyncTcpSession::send(const std::vector<char>& data)
{
	if (data.empty())
	{
		Q_ASSERT(false);
		return;
	}

	if (m_writeInProgress)
	{
		Q_ASSERT(false);
		return;
	}

	m_writeBuffer = data;
	startWrite();
}

void AsyncTcpSession::send(const char* data, size_t dataSize)
{
	if (data == nullptr || dataSize == 0)
	{
		Q_ASSERT(false);
		return;
	}

	if (m_writeInProgress)
	{
		Q_ASSERT(false);
		return;
	}

	m_writeBuffer.assign(data, data + dataSize);
	startWrite();
}

void AsyncTcpSession::startRead()
{
	m_socket.async_read_some(asio::buffer(m_readBuffer),
							 asio::bind_executor(m_strand,
												 [self = shared_from_this()](const std::error_code& ec, std::size_t size)
												 {
													 if (ec)
													 {
														 self->onError(ec);
														 return;
													 }

													 self->onDataReceived(self->m_readBuffer.data(), size);
												 }));
}

void AsyncTcpSession::startWrite()
{
	Q_ASSERT(m_writeInProgress == false);

	if (m_writeBuffer.empty())
	{
		Q_ASSERT(false);
		return;
	}

	m_writeInProgress = true;

	asio::async_write(m_socket,
					  asio::buffer(m_writeBuffer),
					  asio::bind_executor(m_strand,
										  [self = shared_from_this()](const std::error_code& ec, std::size_t)
										  {
											  self->m_writeInProgress = false;

											  if (ec)
											  {
												  self->onError(ec);
												  return;
											  }

											  self->m_writeBuffer.clear();

											  self->startRead();
										  }));
}