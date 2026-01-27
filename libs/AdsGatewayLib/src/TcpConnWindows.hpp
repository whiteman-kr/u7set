#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <optional>
#include <span>
#include <string>

namespace AdsGatewayLib
{
	class TcpConnWindows final
	{
	public:
		TcpConnWindows();

		TcpConnWindows(const TcpConnWindows&) = delete;
		TcpConnWindows& operator=(const TcpConnWindows&) = delete;

		TcpConnWindows(TcpConnWindows&& rhs) noexcept;
		TcpConnWindows& operator=(TcpConnWindows&& rhs) noexcept;
		~TcpConnWindows();

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
		static bool initializeSocketsSystem();
		static void cleanupSocketsSystem();

		void setError(int err);
		void setError(std::string_view err);
		void resetError();

	private:
		std::uintptr_t m_socket; // Type must be compatible with SOCKET! Keep Windows headers inside .cpp
		std::string m_lastError;
	};
} // namespace AdsGatewayLib