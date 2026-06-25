#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <span>
#include <string>


namespace GatewayClientLib
{
	class TcpConnLinux;
	class TcpConnWindows;

	class TcpConnection final
	{
	public:
		TcpConnection();

		TcpConnection(const TcpConnection&) = default;
		TcpConnection(TcpConnection&& rhs) noexcept = default;
		TcpConnection& operator=(const TcpConnection&) = default;
		TcpConnection& operator=(TcpConnection&& rhs) noexcept = default;

		~TcpConnection();

	public:
		// Checks if the TCP connection is currently open.
		// Returns true if open, false otherwise.
		//
		[[nodiscard]] bool isOpen() const;

		// Establishes a TCP connection to the specified address and port.
		// Returns true on success, false on failure.
		// On failure, lastError() can be used to get the error description.
		//
		[[nodiscard]] bool connect(std::string_view address,
								   uint16_t port,
								   std::function<bool()> isCancelled = {},
								   std::chrono::milliseconds timeout = std::chrono::milliseconds(5000));

		// Closes the TCP connection.
		// Returns true on success, false on failure.
		// On failure, lastError() can be used to get the error description.
		//
		bool close();

		// Sends data from the provided buffer.
		// Returns true on success, false on failure.
		// On failure, lastError() can be used to get the error description.
		// No partial writes are possible; the function will attempt to send the entire buffer before returning.
		//
		[[nodiscard]] bool send(std::span<const std::byte> data,
								std::function<bool()> isCancelled = {},
								std::chrono::milliseconds timeout = std::chrono::milliseconds(2000));

		// Receives data into the provided buffer.
		// Returns true on success, false on failure.
		// On failure, lastError() can be used to get the error description.
		// No partial reads are possible; the function will attempt to fill the entire buffer before returning.
		//
		[[nodiscard]] bool receive(std::span<std::byte> buffer,
								   std::function<bool()> isCancelled = {},
								   std::chrono::milliseconds timeout = std::chrono::milliseconds(2000));

		// Retrieves the last error message.
		//
		[[nodiscard]] std::string lastError() const;

	private:
#ifdef _WIN32
		using TcpConnType = TcpConnWindows;
#else
		using TcpConnType = TcpConnLinux;
#endif
		std::unique_ptr<TcpConnType> m_impl;
	};
} // namespace GatewayClientLib