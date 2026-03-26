#pragma once
#include "TcpConnection.hpp"

#include <GatewayClientLib/Logger.hpp>
#include <GatewayClientLib/GwClient.hpp>

#include <GatewayClientLib/GwCrc32.hpp>
#include <GatewayClientLib/GwHash.hpp>

#include <array>
#include <cassert>
#include <cstring>
#include <functional>
#include <stop_token>
#include <vector>


namespace GatewayClientLib
{
	template<typename RequestIdType>
	class GwConnImpl
	{
	public:
		GwConnImpl(ILogger& logger) :
			m_logger{logger}
		{
		}

		virtual void run(std::stop_token stoken, std::string_view address, uint16_t port, std::string_view equipmentId) = 0;

	protected:
		// Sends a request and receives a response from the Gateway.
		// Only for request/response pairs where both RequestT and ResponseT are POD types with fixed sizes.
		// Throws std::runtime_error on communication errors.
		//
		template<typename RequestT, typename ResponseT, typename cancellableFuncT = std::function<bool()>>
		GwErrorCode sendRequest(RequestIdType requestId,
								const RequestT& request,
								ResponseT& response,
								const cancellableFuncT& isCancelledFunc = {})
		{
			m_lastStatusCode.reset();

			// Send request and receive response, can throw exceptions.
			//
			m_logger.logTrace("Sending request {}...", requestId);
			sendRequestPacket<RequestT>(requestId, request, {}, isCancelledFunc);

			m_logger.logTrace("Receiving response {}...", requestId);

			m_lastStatusCode = receiveResponsePacket<ResponseT>(requestId, response, {}, isCancelledFunc);
			return m_lastStatusCode.value();
		}

		template<typename RequestT,
				 typename RequestVariablePartT,
				 typename ResponseT,
				 typename ResponseVariablePartT,
				 typename cancellableFuncT = std::function<bool()>>
		GwErrorCode sendRequest(RequestIdType requestId,
								const RequestT& request,
								std::span<const RequestVariablePartT> requestVariablePart,
								ResponseT& response,
								std::span<ResponseVariablePartT> responseVariablePart,
								const cancellableFuncT& isCancelledFunc = {})
		{
			m_lastStatusCode.reset();

			// Send request and receive response, can throw exceptions.
			//
			m_logger.logTrace("Sending request {}...", requestId);
			sendRequestPacket<RequestT>(requestId, request, requestVariablePart, isCancelledFunc);

			m_logger.logTrace("Receiving response {}...", requestId);
			m_lastStatusCode = receiveResponsePacket<ResponseT>(requestId, response, responseVariablePart, isCancelledFunc);
			return m_lastStatusCode.value();
		}

		template<typename RequestT, typename RequestVariablePartT = char, typename cancellableFuncT = std::function<bool()>>
		void sendRequestPacket(RequestIdType requestId,
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
			writeUint32(static_cast<uint32_t>(GwErrorCode::GWC_SUCCESS)); // Status code for request is always 0

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
				throw std::runtime_error{std::format("Send error, request={}, error={}", requestId, m_conn.lastError())};
			}

			return;
		}

		template<typename ResponseT, typename ResponseVariablePartT = char, typename cancellableFuncT = std::function<bool()>>
		GwErrorCode receiveResponsePacket(RequestIdType requestId,
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
				throw std::runtime_error{std::format("Receive error, request={}, error={}", requestId, m_conn.lastError())};
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
			GwErrorCode respStatusCode = static_cast<GwErrorCode>(readUint32(responseHeader));

			if (respRequestId != static_cast<uint32_t>(requestId))
			{
				throw std::runtime_error{std::format("Invalid response request ID: {}", respRequestId)};
			}

			if (respStatusCode != GwErrorCode::GWC_SUCCESS)
			{
				// Receive CRC32 for error response (no payload)
				//
				std::array<std::byte, 4> responseCrcBuffer{};
				receiveOk = m_conn.receive(std::span<std::byte>{responseCrcBuffer.data(), responseCrcBuffer.size()}, isCancelledFunc);
				if (receiveOk == false)
				{
					throw std::runtime_error{std::format("Receive error, request={}, error={}", requestId, m_conn.lastError())};
				}

				uint32_t respCrc = 0;
				std::memcpy(&respCrc, responseCrcBuffer.data(), sizeof(respCrc));

				uint32_t computedCrc = Radiy::CRC32(responseHeader);
				if (respCrc != computedCrc)
				{
					throw std::runtime_error{std::format("Response CRC32 mismatch for error response, request={}", requestId)};
				}

				return static_cast<GwErrorCode>(respStatusCode);
			}

			if (respPayloadSize > GW_MAX_PAYLOAD_SIZE)
			{
				throw std::runtime_error{std::format("Response {} payload size too large: {}", requestId, respPayloadSize)};
			}

			if (respPayloadSize < sizeof(ResponseT))
			{
				throw std::runtime_error{std::format("Response {} payload size is smaller than ResponseT: {}", requestId, respPayloadSize)};
			}

			const uint32_t variablePayloadSize = respPayloadSize - static_cast<uint32_t>(sizeof(ResponseT));
			const uint32_t maxVariableBytes = static_cast<uint32_t>(responseVariablePart.size() * sizeof(ResponseVariablePartT));
			if (variablePayloadSize > maxVariableBytes)
			{
				throw std::runtime_error{
					std::format("Response {} payload variable part is greater than expected: {}", requestId, respPayloadSize)};
			}

			// Receive and parse response payload + CRC32
			//
			thread_local std::vector<std::byte> payloadCrcBuffer{};            // Payload + CRC32
			payloadCrcBuffer.clear();
			payloadCrcBuffer.resize(static_cast<size_t>(respPayloadSize) + 4); // + CRC32

			receiveOk = m_conn.receive(std::span<std::byte>{payloadCrcBuffer.data(), payloadCrcBuffer.size()}, isCancelledFunc);
			if (receiveOk == false)
			{
				throw std::runtime_error{std::format("Receive error, request={}, error={}", requestId, m_conn.lastError())};
			}

			// Copy payload to output structures
			//
			std::memcpy(&response, payloadCrcBuffer.data(), sizeof(ResponseT));
			if (variablePayloadSize > 0)
			{
				std::memcpy(responseVariablePart.data(), payloadCrcBuffer.data() + sizeof(ResponseT), variablePayloadSize);
			}

			// Check packet CRC
			// Calc CRC over header and payload (different buffers)
			//
			auto computedCrc = Radiy::CRC32(responseHeader, false);
			computedCrc = Radiy::CRC32(payloadCrcBuffer, true, computedCrc);

			// CRC calculation includes CRC itself, thus final value must match the residue.
			//
			if (computedCrc != Radiy::Crc32Residue)
			{
				throw std::runtime_error{std::format("Response CRC32 mismatch, request={}", requestId)};
			}

			return GwErrorCode::GWC_SUCCESS;
		}

	protected:
		ILogger& m_logger;
		TcpConnection m_conn;

	protected:
		std::optional<GwErrorCode> m_lastStatusCode;
	};
} // namespace GatewayClientLib