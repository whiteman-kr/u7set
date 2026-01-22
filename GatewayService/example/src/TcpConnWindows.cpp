#include "TcpConnWindows.hpp"

#include <algorithm>
#include <cassert>
#include <optional>
#include <system_error>
#include <thread>
#include <utility>

#include <WS2tcpip.h>
#include <WinSock2.h>

#pragma comment(lib, "Ws2_32.lib")

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

		try
		{
			m_socket = ::socket(AF_INET, SOCK_STREAM, 0);
			if (m_socket == INVALID_SOCKET)
			{
				throw ::WSAGetLastError();
			}

			// Make socket non-blocking
			//
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
				int lastErr = ::WSAGetLastError();
				if (lastErr != WSAEWOULDBLOCK && lastErr != WSAEINPROGRESS)
				{
					throw lastErr;
				}

				// Wait for connection completion with timeout & cancellation
				//
				bool connected = false;
				auto startedAt = std::chrono::steady_clock::now();

				while (connected == false)
				{
					if (isCancelled && isCancelled() == true)
					{
						throw "Connection cancelled";
					}

					auto now = std::chrono::steady_clock::now();
					auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startedAt);
					if (elapsed >= timeout)
					{
						throw "Connection timeout";
					}

					long long remainingMs = (timeout - elapsed).count();
					// wait at most 200ms, but not more than remaining timeout, at least 1ms
					long waitMs = static_cast<long>(std::clamp(remainingMs, 1LL, 200LL));

					fd_set writeSet;
					fd_set errorSet;
					FD_ZERO(&writeSet);
					FD_ZERO(&errorSet);
					FD_SET(m_socket, &writeSet);
					FD_SET(m_socket, &errorSet);

					TIMEVAL tv{};
					tv.tv_sec = static_cast<long>(waitMs / 1000);
					tv.tv_usec = static_cast<long>((waitMs % 1000) * 1000);

					int sel = ::select(0, nullptr, &writeSet, &errorSet, &tv);
					if (sel == SOCKET_ERROR)
					{
						int selErr = ::WSAGetLastError();
						// On Windows there is no EINTR, so any error is real
						//
						throw selErr;
					}

					if (sel == 0)
					{
						// timeout slice; loop to re-check global timeout / cancellation
						//
						continue;
					}

					if (FD_ISSET(m_socket, &errorSet))
					{
						// get pending error
						//
						int so_error = 0;
						int optLen = sizeof(so_error);
						if (::getsockopt(m_socket, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&so_error), &optLen) == SOCKET_ERROR)
						{
							throw ::WSAGetLastError();
						}
						if (so_error != 0)
						{
							throw so_error;
						}
					}

					if (FD_ISSET(m_socket, &writeSet))
					{
						// socket is writable: check final connect status
						//
						int so_error = 0;
						int optLen = sizeof(so_error);
						if (::getsockopt(m_socket, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&so_error), &optLen) == SOCKET_ERROR)
						{
							throw ::WSAGetLastError();
						}
						if (so_error != 0)
						{
							throw so_error;
						}

						connected = true;
					}
				}
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

		// Restore blocking mode
		//
		u_long nonBlocking = 0;
		::ioctlsocket(m_socket, FIONBIO, &nonBlocking);

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

		size_t total = 0;
		auto startedAt = std::chrono::steady_clock::now();

		while (total < data.size())
		{
			if (isCancelled && isCancelled() == true)
			{
				setError("Send cancelled");
				return false;
			}

			auto now = std::chrono::steady_clock::now();
			if (now - startedAt >= timeout)
			{
				setError("Send timeout");
				return false;
			}

			int sendResult = ::send(m_socket, reinterpret_cast<const char*>(data.data() + total), static_cast<int>(data.size() - total), 0);
			if (sendResult == SOCKET_ERROR)
			{
				auto err = ::WSAGetLastError();

				// Transient / retryable errors
				//
				if (err == WSAEINTR)
				{
					// Interrupted, retry immediately
					//
					continue;
				}

				if (err == WSAEWOULDBLOCK)
				{
					// Socket not ready for writing yet.
					// Simple backoff: yield and retry, while still honoring timeout/cancel checks.
					//
					std::this_thread::yield();
					continue;
				}

				// Any other error is fatal for this send
				//
				setError(err);
				return false;
			}

			if (sendResult == 0)
			{
				// Peer closed the connection
				//
				setError("Connection closed");
				return false;
			}

			total += static_cast<size_t>(sendResult);
		}

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

		auto startedAt = std::chrono::steady_clock::now();
		size_t received = 0;

		while (received < buffer.size())
		{
			if (isCancelled && isCancelled() == true)
			{
				setError("Receive cancelled");
				return false;
			}

			auto now = std::chrono::steady_clock::now();
			auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startedAt);
			if (elapsed >= timeout)
			{
				setError("Receive timeout");
				return false;
			}

			auto remainingMs = (timeout - elapsed).count();
			long waitMs = static_cast<long>(std::clamp(remainingMs, 1LL, 200LL));

			fd_set readSet;
			fd_set errorSet;
			FD_ZERO(&readSet);
			FD_ZERO(&errorSet);
			FD_SET(m_socket, &readSet);
			FD_SET(m_socket, &errorSet);

			TIMEVAL tv{};
			tv.tv_sec = static_cast<long>(waitMs / 1000);
			tv.tv_usec = static_cast<long>((waitMs % 1000) * 1000);

			int sel = ::select(0, &readSet, nullptr, &errorSet, &tv);
			if (sel == SOCKET_ERROR)
			{
				setError(::WSAGetLastError());
				return false;
			}

			if (sel == 0)
			{
				continue; // slice timed out, re-check overall timeout/cancel
			}

			if (FD_ISSET(m_socket, &errorSet))
			{
				int so_error = 0;
				int optLen = sizeof(so_error);
				if (::getsockopt(m_socket, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&so_error), &optLen) == SOCKET_ERROR)
				{
					setError(::WSAGetLastError());
					return false;
				}
				if (so_error != 0)
				{
					setError(so_error);
					return false;
				}
			}

			if (FD_ISSET(m_socket, &readSet))
			{
				size_t toRead = buffer.size() - received;
				int recvResult = ::recv(m_socket, reinterpret_cast<char*>(buffer.data() + received), static_cast<int>(toRead), 0);

				if (recvResult == 0)
				{
					setError("Connection closed");
					return false;
				}

				if (recvResult == SOCKET_ERROR)
				{
					int err = ::WSAGetLastError();

					if (err == WSAEINTR)
					{
						continue; // retry
					}

					if (err == WSAEWOULDBLOCK)
					{
						std::this_thread::yield();
						continue; // not ready yet, loop again
					}

					setError(err);
					return false;
				}

				received += static_cast<size_t>(recvResult);
			}
		}

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