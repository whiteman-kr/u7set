#include "TcpConnection.hpp"

#ifdef _WIN32
	#include "TcpConnWindows.hpp"
#else
	#include "TcpConnLinux.hpp"
#endif

namespace adsgw
{
	TcpConnection::TcpConnection() :
		m_impl{std::make_unique<TcpConnType>()}
	{
	}

	TcpConnection::~TcpConnection() = default;

	bool TcpConnection::isOpen() const
	{
		return m_impl->isOpen();
	}

	bool TcpConnection::connect(std::string_view address,
								uint16_t port,
								std::function<bool()> isCancelled,
								std::chrono::milliseconds timeout)
	{
		return m_impl->connect(address, port, isCancelled, timeout);
	}

	bool TcpConnection::close()
	{
		return m_impl->close();
	}

	bool TcpConnection::send(std::span<const std::byte> data, std::function<bool()> isCancelled, std::chrono::milliseconds timeout)
	{
		return m_impl->send(data, isCancelled, timeout);
	}

	bool TcpConnection::receive(std::span<std::byte> buffer, std::function<bool()> isCancelled, std::chrono::milliseconds timeout)
	{
		return m_impl->receive(buffer, isCancelled, timeout);
	}

	std::string TcpConnection::lastError() const
	{
		return m_impl->lastError();
	}
} // namespace adsgw