#include "AdsGwConnection.hpp"
#include "MiniLogger.hpp"
#include "TcpConnection.hpp"

#include <AdsgatewayLib/AdsGwProtocol.hpp>
#include <AdsgatewayLib/GwCrc32.hpp>

#include <array>
#include <cassert>
#include <cstring>
#include <exception>
#include <format>

namespace
{
	using namespace adsgw;

	class AdsGwConnImpl final
	{
	public:
		AdsGwConnImpl(IMiniLogger& logger) :
			m_logger{logger}
		{
		}

		void run(std::stop_token stoken, std::string_view address, uint16_t port, std::string_view equipmentId);

	private:
		// Sends a request and receives a response from the ADS Gateway.
		// Only for request/response pairs where both RequestT and ResponseT are POD types with fixed sizes.
		// Throws std::runtime_error on communication errors.
		//
		template<typename RequestT, typename ResponseT, typename cancellableFuncT = std::function<bool()>>
		GwErrorCode sendRequest(GwRequestId requestId,
								const RequestT& request,
								ResponseT& response,
								const cancellableFuncT& isCancelledFunc = {})
		{
			// Send request and receive response, can throw exceptions.
			//
			m_logger.logTraceFormat("Sending request ID {}", static_cast<uint32_t>(requestId));
			sendRequestPacket<RequestT>(requestId, request, {}, isCancelledFunc);

			m_logger.logTraceFormat("Receiving response for request ID {}", static_cast<uint32_t>(requestId));
			return receiveResponsePacket<ResponseT>(requestId, response, {}, isCancelledFunc);
		}

		template<typename RequestT,
				 typename RequestVariablePartT,
				 typename ResponseT,
				 typename ResponseVariablePartT,
				 typename cancellableFuncT = std::function<bool()>>
		GwErrorCode sendRequest(GwRequestId requestId,
								const RequestT& request,
								std::span<const RequestVariablePartT> requestVariablePart,
								ResponseT& response,
								std::span<ResponseVariablePartT> responseVariablePart,
								const cancellableFuncT& isCancelledFunc = {})
		{
			// Send request and receive response, can throw exceptions.
			//
			m_logger.logTraceFormat("Sending request ID {}", static_cast<uint32_t>(requestId));
			sendRequestPacket<RequestT>(requestId, request, requestVariablePart, isCancelledFunc);

			m_logger.logTraceFormat("Receiving response for request ID {}", static_cast<uint32_t>(requestId));
			return receiveResponsePacket<ResponseT>(requestId, response, responseVariablePart, isCancelledFunc);
		}

		template<typename RequestT, typename RequestVariablePartT = char, typename cancellableFuncT = std::function<bool()>>
		void sendRequestPacket(GwRequestId requestId,
							   const RequestT& request,
							   std::span<const RequestVariablePartT> requestVariablePart,
							   const cancellableFuncT& isCancelledFunc)
		{
			/*
			**General Message Format**
			+------------------+------------------+------------------+------------------+------------------+
			| Request ID       | Payload Size     | Status Code      | Payload          | CRC32            |
			| (4 bytes)        | (4 bytes)        | (4 bytes)        | (variable)       | (4 bytes)        |
			+------------------+------------------+------------------+------------------+------------------+
			*/
			thread_local std::vector<std::byte> requestBuffer{};

			// Size: Request ID + Payload Size + Status Code + Payload (struct + variable part) + CRC32
			//
			requestBuffer.clear();
			requestBuffer.resize(4 + 4 + 4 + sizeof(RequestT) + requestVariablePart.size() * sizeof(RequestVariablePartT) + 4);

			size_t offset = 0;
			auto writeUint32 = [&offset](uint32_t value)
			{
				std::memcpy(requestBuffer.data() + offset, &value, sizeof(value));
				offset += sizeof(value);
			};

			writeUint32(static_cast<uint32_t>(requestId));
			writeUint32(
				static_cast<uint32_t>(sizeof(RequestT) + requestVariablePart.size() * sizeof(RequestVariablePartT))); // Payload size
			writeUint32(static_cast<uint32_t>(GWC_SUCCESS)); // Status code for request is always 0

			// Write request payload (struct + variable part)
			//
			std::memcpy(requestBuffer.data() + offset, &request, sizeof(RequestT));
			offset += sizeof(RequestT);

			if (requestVariablePart.size() > 0)
			{
				std::memcpy(requestBuffer.data() + offset,
							requestVariablePart.data(),
							requestVariablePart.size() * sizeof(RequestVariablePartT));
				offset += requestVariablePart.size() * sizeof(RequestVariablePartT);
			}

			uint32_t crc = Radiy::CRC32(std::span<const std::byte>{requestBuffer.data(), offset});
			writeUint32(crc);

			assert(offset == requestBuffer.size());

			bool sendOk = m_conn.send(std::span<const std::byte>{requestBuffer.data(), requestBuffer.size()}, isCancelledFunc);
			if (sendOk == false)
			{
				throw std::runtime_error{std::format("Send error: {}", m_conn.lastError())};
			}

			return;
		}

		template<typename ResponseT, typename ResponseVariablePartT = char, typename cancellableFuncT = std::function<bool()>>
		GwErrorCode receiveResponsePacket(GwRequestId requestId,
										  ResponseT& response,
										  std::span<ResponseVariablePartT> responseVariablePart,
										  const cancellableFuncT& isCancelledFunc = {})
		{
			// Receive and parse response header
			//
			std::array<std::byte, 4 + 4 + 4> responseHeader{}; // Request ID + Payload Size + Status Code (up to payload)

			bool receiveOk = m_conn.receive(std::span<std::byte>{responseHeader.data(), responseHeader.size()}, isCancelledFunc);
			if (receiveOk == false)
			{
				throw std::runtime_error{std::format("Receive error: {}", m_conn.lastError())};
			}

			size_t offset = 0;
			auto readUint32 = [&offset](const std::array<std::byte, 4 + 4 + 4>& buffer) -> uint32_t
			{
				uint32_t value = 0;
				std::memcpy(&value, buffer.data() + offset, sizeof(value));
				offset += sizeof(value);
				return value;
			};
			uint32_t respRequestId = readUint32(responseHeader);
			uint32_t respPayloadSize = readUint32(responseHeader);
			uint32_t respStatusCode = readUint32(responseHeader);

			if (respRequestId != static_cast<uint32_t>(requestId))
			{
				throw std::runtime_error{std::format("Invalid response request ID: {}", respRequestId)};
			}

			if (respStatusCode != GWC_SUCCESS)
			{
				// Receive CRC32 for error response (no payload)
				//
				std::array<std::byte, 4> responseCrcBuffer{};
				receiveOk = m_conn.receive(std::span<std::byte>{responseCrcBuffer.data(), responseCrcBuffer.size()}, isCancelledFunc);
				if (receiveOk == false)
				{
					throw std::runtime_error{std::format("Receive error: {}", m_conn.lastError())};
				}

				uint32_t respCrc = 0;
				std::memcpy(&respCrc, responseCrcBuffer.data(), sizeof(respCrc));

				uint32_t computedCrc = Radiy::CRC32(responseHeader);
				if (respCrc != computedCrc)
				{
					throw std::runtime_error{"Response CRC32 mismatch for error response"};
				}

				return static_cast<GwErrorCode>(respStatusCode);
			}

			if (respPayloadSize > ADSGW_MAX_PAYLOAD_SIZE)
			{
				throw std::runtime_error{std::format("Response payload size too large: {}", respPayloadSize)};
			}

			if (respPayloadSize != sizeof(ResponseT) + responseVariablePart.size() * sizeof(ResponseVariablePartT))
			{
				throw std::runtime_error{std::format("Invalid response payload size: {}", respPayloadSize)};
			}

			// Receive and parse response payload + CRC32
			//
			thread_local std::vector<std::byte> payloadCrcBuffer{}; // Payload + CRC32
			payloadCrcBuffer.clear();
			payloadCrcBuffer.resize(respPayloadSize + 4);           // + CRC32

			receiveOk = m_conn.receive(std::span<std::byte>{payloadCrcBuffer.data(), payloadCrcBuffer.size()}, isCancelledFunc);
			if (receiveOk == false)
			{
				throw std::runtime_error{std::format("Receive error: {}", m_conn.lastError())};
			}

			// Copy payload to output structures
			//
			std::memcpy(&response, payloadCrcBuffer.data(), sizeof(ResponseT));
			std::memcpy(responseVariablePart.data(),
						payloadCrcBuffer.data() + sizeof(ResponseT),
						responseVariablePart.size() * sizeof(ResponseVariablePartT));

			// Check packet CRC
			// Calc CRC over header and payload (different buffers)
			//
			auto computedCrc = Radiy::CRC32(responseHeader, false);
			computedCrc = Radiy::CRC32(payloadCrcBuffer, true, computedCrc);

			// CRC calculation includes CRC itself, thus final value must match the residue.
			//
			if (computedCrc != Radiy::Crc32Residue)
			{
				throw std::runtime_error{"Response CRC32 mismatch"};
			}

			return GWC_SUCCESS;
		}

		void requestHandshake(std::string_view equipmentId);

	private:
		IMiniLogger& m_logger;
		TcpConnection m_conn;
		std::function<bool()> m_isCancelledFunc;
	};

	void AdsGwConnImpl::run(std::stop_token stoken, std::string_view address, uint16_t port, std::string_view equipmentId)
	{
		m_isCancelledFunc = [stoken]()
		{
			return stoken.stop_requested();
		};

		while (stoken.stop_requested() == false)
		{
			try
			{
				// Establish TCP Connections
				//
				m_logger.logTraceFormat("Connecting to ADS Gateway at {}:{}", address, port);
				bool ok = m_conn.connect(address, port, m_isCancelledFunc);
				if (ok == false)
				{
					throw std::runtime_error{std::format("Connect error: {}", m_conn.lastError())};
				}
				m_logger.logTraceFormat("Connected to ADS Gateway {}:{}", address, port);

				// Send Handshake
				//
				requestHandshake(equipmentId);

				while (stoken.stop_requested() == false)
				{
					// TODO: Main communication loop
					//
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
				}
			}
			catch (const std::runtime_error& e)
			{
				m_conn.close();

				// todo: Logger, now just print to stdout.
				//
				m_logger.logError(e.what());

				std::this_thread::sleep_for(std::chrono::seconds(1));
			}
		}

		if (m_conn.isOpen() == true)
		{
			m_conn.close();
		}

		m_isCancelledFunc = {};

		return;
	}

	void AdsGwConnImpl::requestHandshake(std::string_view equipmentId)
	{
		GwHandshakeRequest request{};
		GwHandshakeResponse response{};

		request.protocolVersion = ADSGW_PROTOCOL_VERSION;
		std::snprintf(request.clientName, sizeof(request.clientName), "%s", equipmentId.data());

		GwErrorCode requestResult{};

		try
		{
			m_logger.logTraceFormat("Sending handshake request to ADS Gateway...");
			requestResult = sendRequest(ADSGW_HANDSHAKE, request, response, m_isCancelledFunc);
		}
		catch (const std::runtime_error& e)
		{
			throw std::runtime_error{std::format("Handshake error: {}", e.what())};
		}

		if (requestResult != GWC_SUCCESS)
		{
			throw std::runtime_error{std::format("Handshake error: {}", static_cast<int>(requestResult))};
		}

		if (response.protocolVersion != ADSGW_PROTOCOL_VERSION)
		{
			m_conn.close();
			throw std::runtime_error{std::format("Handshake error: Unsupported protocol version {}", response.protocolVersion)};
		}

		if (response.sizeof_GwAppSignalParam != sizeof(GwAppSignalParam))
		{
			m_conn.close();
			throw std::runtime_error{
				std::format("Handshake error: Incompatible GwAppSignalParam size {}", response.sizeof_GwAppSignalParam)};
		}

		if (response.sizeof_GwAppSignalState != sizeof(GwAppSignalState))
		{
			m_conn.close();
			throw std::runtime_error{
				std::format("Handshake error: Incompatible GwAppSignalState size {}", response.sizeof_GwAppSignalState)};
		}

		return;
	}
} // namespace

namespace adsgw
{
	AdsGwConnection::AdsGwConnection(IMiniLogger& logger) :
		m_logger{logger}
	{
	}

	void AdsGwConnection::connect(std::string_view address, uint16_t port, std::string_view equipmentId)
	{
		m_thread =
			std::jthread{[addressStr = std::string{address}, port, equipmentIdStr = std::string{equipmentId}, this](std::stop_token stoken)
						 {
							 AdsGwConnImpl conn{m_logger};
							 conn.run(stoken, addressStr, port, equipmentIdStr);
						 }};
	}

	void AdsGwConnection::close() {}
} // namespace adsgw