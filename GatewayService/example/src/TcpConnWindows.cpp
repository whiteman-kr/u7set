#include "TcpConnWindows.hpp"

#include <cassert>
#include <optional>
#include <system_error>
#include <thread>
#include <utility>

#include <WS2tcpip.h>
#include <WinSock2.h>

#pragma comment(lib, "Ws2_32.lib")

// #include <fcntl.h>
// #include <poll.h>
// #include <unistd.h>
// #include <arpa/inet.h>
// #include <netinet/in.h>
// #include <sys/socket.h>

namespace adsgw
{

	TcpConnWindows::TcpConnWindows() :
		m_socket{INVALID_SOCKET}
	{
		static_assert(std::is_same_v<decltype(TcpConnWindows::m_socket), SOCKET>);
		initializeSocketsSystem();
	}

	TcpConnWindows::TcpConnWindows(TcpConnWindows&& rhs) noexcept :
		m_socket{std::exchange(rhs.m_socket, INVALID_SOCKET)},
		m_lastError{std::exchange(rhs.m_lastError, std::string{})}
	{
		initializeSocketsSystem();
	}

	TcpConnWindows& TcpConnWindows::operator=(TcpConnWindows&& rhs) noexcept
	{
		if (this != &rhs)
		{
			close();

			m_socket = std::exchange(rhs.m_socket, INVALID_SOCKET);
			m_lastError = std::exchange(rhs.m_lastError, std::string{});
		}

		return *this;
	}

	TcpConnWindows::~TcpConnWindows()
	{
		close();

		cleanupSocketsSystem();
	}

	bool TcpConnWindows::isOpen() const
	{
		return m_socket != INVALID_SOCKET;
	}

	bool TcpConnWindows::connect(std::string_view address,
								 uint16_t port,
								 std::function<bool()> isCancelled,
								 std::chrono::milliseconds timeout)
	{
		if (isOpen() == true)
		{
			setError("Already open.");
			return false;
		}

		resetError();
		std::optional<int> flags;

		try
		{
			m_socket = ::socket(AF_INET, SOCK_STREAM, 0);
			if (m_socket == INVALID_SOCKET)
			{
				throw ::WSAGetLastError();
			}

			u_long nonBlocking = 1;
			::ioctlsocket(m_socket, FIONBIO, &nonBlocking);

			sockaddr_in addr{};
			addr.sin_family = AF_INET;
			addr.sin_port = htons(port);

			int addressConvertResult = inet_pton(AF_INET, std::string{address}.c_str(), &addr.sin_addr);
			if (addressConvertResult == 0)
			{
				throw "Address does not contain a character representing a valid network address";
			}
			if (addressConvertResult == -1)
			{
				throw ::WSAGetLastError();
			}

			int res = ::connect(m_socket, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr));
			if (res == 0)
			{
				// Connected immediately!
				//
			}
			else
			{
				if (auto lastErr = ::WSAGetLastError(); //
					lastErr != WSAEWOULDBLOCK)
				{
					throw lastErr;
				}

														//        // Wait for connection
				//        //
				//        bool connected = false;
				//        auto startedAt = std::chrono::steady_clock::now();

				//        while (connected == false)
				//        {
				//            if (isCancelled && isCancelled() == true)
				//            {
				//                throw "Connection cancelled";
				//            }

				//            auto now = std::chrono::steady_clock::now();
				//            auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - startedAt);
				//            if (elapsedMs >= timeout)
				//            {
				//                throw "Connection timeout";
				//            }

				//            pollfd fds{};
				//            fds.fd = m_fd;
				//            fds.events = POLLOUT;

				//            auto remaining = static_cast<int>((timeout - elapsedMs).count());
				//            int waitMs = std::min(remaining, 200); // Timeout 200ms or less depending on remaining time

				//            int pollRes = ::poll(&fds, 1, waitMs);
				//            if (pollRes < 0 && errno != EINTR)
				//            {
				//                throw errno;
				//            }

				//            connected = (pollRes > 0);
				//        }

				//        // Socket is ready - check if connection succeeded
				//        //
				//        {
				//            int error{};
				//            socklen_t errorLength = sizeof(error);
				//            if (::getsockopt(m_fd, SOL_SOCKET, SO_ERROR, &error, &errorLength) < 0)
				//            {
				//                throw errno;
				//            }

				//            if (error != 0)
				//            {
				//                throw error;
				//            }
				//        }
			}
		}
		catch (const char* error)
		{
			if (isOpen() == true)
			{
				close();
			}

			setError(error);
			return false;
		}
		catch (int error)
		{
			if (isOpen() == true)
			{
				close();
			}

			setError(error);
			return false;
		}

		// Restore blocking mode if it was changed before.
		//
		if (flags.has_value() == true)
		{
			u_long nonBlocking = 0;
			::ioctlsocket(m_socket, FIONBIO, &nonBlocking);
		}

		return true;
	}

	bool TcpConnWindows::close()
	{
		resetError();

		if (isOpen() == true)
		{
			auto s = std::exchange(m_socket, INVALID_SOCKET);

			if (::closesocket(s) == SOCKET_ERROR)
			{
				setError(WSAGetLastError());
				return false;
			}

			return true;
		}

		return true;
	}

	bool TcpConnWindows::send(std::span<const std::byte> data, std::function<bool()> isCancelled, std::chrono::milliseconds timeout)
	{
		if (isOpen() == false)
		{
			setError("Not connected");
			return false;
		}

		resetError();

		// size_t total = 0;
		// auto startedAt = std::chrono::steady_clock::now();

		// while (total < data.size())
		//{
		//     if (isCancelled && isCancelled() == true)
		//     {
		//         setError("Send cancelled");
		//         return false;
		//     }

		//    auto now = std::chrono::steady_clock::now();
		//    if (now - startedAt >= timeout)
		//    {
		//        setError("Send timeout");
		//        return false;
		//    }

		//    int sendResult = ::send(m_fd, data.data() + total, data.size() - total, MSG_NOSIGNAL);
		//    if (sendResult == -1)
		//    {
		//        auto err = errno;
		//        if (err == EINTR)
		//        {
		//            // Interrupted by a signal, retry the operation
		//            //
		//            continue;
		//        }

		//        if (err == EAGAIN || err == EWOULDBLOCK)
		//        {
		//            // Socket not ready for writing, try again later.
		//            // Here should be a wait with timeout, bussy wait for simplicity.
		//            //
		//            std::this_thread::yield();
		//            continue;
		//        }

		//        setError(err);
		//        return false;
		//    }

		//    total += static_cast<size_t>(sendResult);
		//}

		return true;
	}

	bool TcpConnWindows::receive(std::span<std::byte> buffer, std::function<bool()> isCancelled, std::chrono::milliseconds timeout)
	{
		if (isOpen() == false)
		{
			setError("Not connected");
			return false;
		}

		resetError();

		// auto startedAt = std::chrono::steady_clock::now();
		// size_t received = 0;

		// while (received < buffer.size())
		//{
		//     if (isCancelled && isCancelled() == true)
		//     {
		//         setError("Receive cancelled");
		//         return false;
		//     }

		//    auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startedAt);
		//    if (elapsedMs >= timeout)
		//    {
		//        setError("Receive timeout");
		//        return false;
		//    }

		//    pollfd fds{};
		//    fds.fd = m_fd;
		//    fds.events = POLLIN;

		//    auto waitMs = std::clamp<int>((timeout - elapsedMs).count(), 1, 100);

		//    int pollResult = ::poll(&fds, 1, waitMs);
		//    if (pollResult < 0)
		//    {
		//        if (errno == EINTR)
		//        {
		//            continue;
		//        }
		//        setError(errno);
		//        return false;
		//    }

		//    if (pollResult == 0)
		//    {
		//        continue; // timeout slice, loop will re-check overall timeout
		//    }

		//    if ((fds.revents & (POLLERR | POLLHUP)) != 0)
		//    {
		//        setError("Connection closed");
		//        return false;
		//    }

		//    int recvResult = ::recv(m_fd, buffer.data() + received, buffer.size() - received, MSG_DONTWAIT);
		//    if (recvResult == 0)
		//    {
		//        setError("Connection closed");
		//        return false;
		//    }
		//    if (recvResult == -1)
		//    {
		//        int err = errno;
		//        if (err == EINTR || err == EAGAIN || err == EWOULDBLOCK)
		//        {
		//            // Retry
		//            //
		//            continue;
		//        }

		//        setError(err);
		//        return false;
		//    }

		//    received += static_cast<size_t>(recvResult);
		//}

		return true;
	}

	std::string TcpConnWindows::lastError() const
	{
		return m_lastError;
	}

	bool TcpConnWindows::initializeSocketsSystem()
	{
		WSADATA wsaData{};
		int result = ::WSAStartup(MAKEWORD(2, 2), &wsaData);
		return (result == 0);
	}

	void TcpConnWindows::cleanupSocketsSystem()
	{
		::WSACleanup();
	}

	void TcpConnWindows::setError(int err)
	{
#if 0
        std::error_code ec{ err, std::system_category() };
        m_lastError = ec.message();
#else
		m_lastError = std::system_category().message(err);
#endif
	}

	void TcpConnWindows::setError(std::string_view err)
	{
		m_lastError = err;
	}

	void TcpConnWindows::resetError()
	{
		m_lastError.clear();
	}
} // namespace adsgw