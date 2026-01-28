#include "TestSettings.hpp"

#include <AdsGatewayLib/../../src/AdsGwConnImpl.hpp>
#include <AdsGatewayLib/Logger.hpp>
#include <AdsGatewayLib/SignalManager.hpp>

#include <gmock/gmock-matchers.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>
#include <numeric>


class AdsGatewayTests : public testing::Test
{
public:
	AdsGatewayLib::SignalManager signalManager{};
	AdsGatewayLib::ConsoleLogger logger{};

	std::string clientEquipmentId = "TEST_CLIENT_EQUIPMENT_ID";
};

// Test that connection fails when no server is available
//
TEST_F(AdsGatewayTests, NoConnection)
{
	class TestAdsGwConnection : public AdsGatewayLib::AdsGwConnImpl
	{
	public:
		using AdsGwConnImpl::AdsGwConnImpl;

		virtual void run(std::stop_token, std::string_view address, uint16_t port, std::string_view /*equipmentId*/) override
		{
			try
			{
				m_connected = m_conn.connect(address, port, m_isCancelledFunc, std::chrono::seconds{5});
			}
			catch (const std::runtime_error&)
			{
				assert(false); // No exceptions expected
			}
		}

	public:
		bool m_connected = false;
	};

	TestAdsGwConnection adsConn{signalManager, logger};
	adsConn.run({}, "127.0.0.1", 3551, "EQUIPMENTID");

	ASSERT_EQ(adsConn.m_connected, false);

	return;
}

// Expecting successful connection and handshake
//
TEST_F(AdsGatewayTests, ConnectAndHandshake)
{
	class TestAdsGwConnection : public AdsGatewayLib::AdsGwConnImpl
	{
	public:
		using AdsGwConnImpl::AdsGwConnImpl;

		virtual void run(std::stop_token, std::string_view address, uint16_t port, std::string_view equipmentId) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);
				requestHandshake(equipmentId);
			}
			catch (const std::runtime_error&)
			{
			}
		}

	public:
		bool m_connected = false;

		auto& lastStatusCode() const { return m_lastStatusCode; }
		auto& handshakeResponse() const { return m_handshakeResponse; }
	};

	TestAdsGwConnection adsConn{signalManager, logger};
	adsConn.run({}, TestSettings::Address, TestSettings::Port, clientEquipmentId);

	ASSERT_EQ(adsConn.m_connected, true);

	ASSERT_TRUE(adsConn.lastStatusCode().has_value());
	EXPECT_EQ(adsConn.lastStatusCode().value(), AdsGatewayLib::GwErrorCode::GWC_SUCCESS);
	EXPECT_EQ(adsConn.handshakeResponse().protocolVersion, AdsGatewayLib::ADSGW_PROTOCOL_VERSION);
	EXPECT_EQ(adsConn.handshakeResponse().sizeof_GwAppSignalParam, sizeof(AdsGatewayLib::GwAppSignalParam));
	EXPECT_EQ(adsConn.handshakeResponse().sizeof_GwAppSignalState, sizeof(AdsGatewayLib::GwAppSignalState));

	return;
}

// Send unsupported protocol version in handshake request
// Expecting GWC_UNSUPPORTED_VERSION error code in response
// 3.1
TEST_F(AdsGatewayTests, SendUnsupportedProtocolVersion)
{
	class TestAdsGwConnection : public AdsGatewayLib::AdsGwConnImpl
	{
	public:
		using AdsGwConnImpl::AdsGwConnImpl;

		virtual void run(std::stop_token, std::string_view address, uint16_t port, std::string_view equipmentId) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);
				requestHandshake(equipmentId, 15); // Unsupported protocol version
			}
			catch (const std::runtime_error&)
			{
			}
		}

	public:
		bool m_connected = false;

		auto& lastStatusCode() const { return m_lastStatusCode; }
		auto& handshakeResponse() const { return m_handshakeResponse; }
	};

	TestAdsGwConnection adsConn{signalManager, logger};
	adsConn.run({}, TestSettings::Address, TestSettings::Port, clientEquipmentId);

	ASSERT_EQ(adsConn.m_connected, true);

	ASSERT_TRUE(adsConn.lastStatusCode().has_value());
	EXPECT_EQ(adsConn.lastStatusCode().value(), AdsGatewayLib::GwErrorCode::GWC_UNSUPPORTED_VERSION);

	return;
}

// Send unknown RequestID
// Expected: GWC_INVALID_REQUEST
//
TEST_F(AdsGatewayTests, SendInvalidRequest)
{
	class TestAdsGwConnection : public AdsGatewayLib::AdsGwConnImpl
	{
	public:
		using AdsGwConnImpl::AdsGwConnImpl;

		virtual void run(std::stop_token, std::string_view address, uint16_t port, std::string_view /*equipmentId*/) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);

				AdsGatewayLib::GwHandshakeRequest request{};
				AdsGatewayLib::GwHandshakeResponse response{};

				sendRequest(static_cast<AdsGatewayLib::GwRequestId>(AdsGatewayLib::ADSGW_HANDSHAKE + 19999), request, response, {});
			}
			catch (const std::runtime_error&)
			{
			}
		}

	public:
		bool m_connected = false;

		auto& lastStatusCode() const { return m_lastStatusCode; }
		auto& handshakeResponse() const { return m_handshakeResponse; }
	};

	TestAdsGwConnection adsConn{signalManager, logger};
	adsConn.run({}, TestSettings::Address, TestSettings::Port, clientEquipmentId);

	ASSERT_EQ(adsConn.m_connected, true);

	ASSERT_TRUE(adsConn.lastStatusCode().has_value());
	EXPECT_EQ(adsConn.lastStatusCode().value(), AdsGatewayLib::GwErrorCode::GWC_INVALID_REQUEST);

	return;
}

// Send invalid CRC in request
// Expected: GWC_CRC_ERROR
//
TEST_F(AdsGatewayTests, SendInvalidCrc32)
{
	class TestAdsGwConnection : public AdsGatewayLib::AdsGwConnImpl
	{
	public:
		using AdsGwConnImpl::AdsGwConnImpl;

		virtual void run(std::stop_token, std::string_view address, uint16_t port, std::string_view equipmentId) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);

				// Compose handshake request with invalid CRC32
				//
				AdsGatewayLib::GwHandshakeRequest request{};
				AdsGatewayLib::GwHandshakeResponse response{};

				request.protocolVersion = AdsGatewayLib::ADSGW_PROTOCOL_VERSION;
				std::snprintf(request.clientName, sizeof(request.clientName), "%s", equipmentId.data());


				// Size: Request ID + Payload Size + Status Code + Payload (struct + variable part) + CRC32
				//
				std::vector<std::byte> requestBuffer{};
				requestBuffer.clear();
				requestBuffer.resize(4 + 4 + 4 + sizeof(request) + 4);

				size_t offset = 0;
				auto writeUint32 = [&offset, &requestBuffer](uint32_t value)
				{
					std::memcpy(requestBuffer.data() + offset, &value, sizeof(value));
					offset += sizeof(value);
				};

				writeUint32(static_cast<uint32_t>(AdsGatewayLib::ADSGW_HANDSHAKE));
				writeUint32(static_cast<uint32_t>(sizeof(request)));            // Payload size
				writeUint32(static_cast<uint32_t>(AdsGatewayLib::GWC_SUCCESS)); // Status code for request is always 0

				// Write request payload (struct)
				//
				std::memcpy(requestBuffer.data() + offset, &request, sizeof(request));
				offset += sizeof(request);

				uint32_t crc = Radiy::CRC32(std::span<const std::byte>{requestBuffer.data(), offset});
				writeUint32(crc + 1); // SPOILTED CRC!

				assert(offset == requestBuffer.size());

				bool ok = m_conn.send(std::span<const std::byte>{requestBuffer.data(), requestBuffer.size()}, {});
				if (ok == false)
				{
					return;
				}

				// Receive response
				//
				m_lastStatusCode =
					receiveResponsePacket<AdsGatewayLib::GwHandshakeResponse>(AdsGatewayLib::ADSGW_HANDSHAKE, response, {}, {});

				return;
			}
			catch (const std::runtime_error&)
			{
			}
		}

	public:
		bool m_connected = false;

		auto& lastStatusCode() const { return m_lastStatusCode; }
		auto& handshakeResponse() const { return m_handshakeResponse; }
	};

	TestAdsGwConnection adsConn{signalManager, logger};
	adsConn.run({}, TestSettings::Address, TestSettings::Port, clientEquipmentId);

	ASSERT_EQ(adsConn.m_connected, true);

	ASSERT_TRUE(adsConn.lastStatusCode().has_value());
	EXPECT_EQ(adsConn.lastStatusCode().value(), AdsGatewayLib::GwErrorCode::GWC_CRC_ERROR);

	return;
}


// Expecting GWC_HANDSHAKE_REQUIRED error code when requesting signal list without handshake
// ADSGW_SIGNAL_LIST_START/ADSGW_SIGNAL_LIST_NEXT
//
TEST_F(AdsGatewayTests, RequestSignalListWithoutHandshake)
{
	class TestAdsGwConnection : public AdsGatewayLib::AdsGwConnImpl
	{
	public:
		using AdsGwConnImpl::AdsGwConnImpl;

		virtual void run(std::stop_token, std::string_view address, uint16_t port, std::string_view /*equipmentId*/) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);
				requestSignalList();
			}
			catch (const std::runtime_error&)
			{
				// Exceptions are expected
			}
		}

	public:
		bool m_connected = false;

		auto& lastStatusCode() const { return m_lastStatusCode; }
		auto& handshakeResponse() const { return m_handshakeResponse; }
	};

	TestAdsGwConnection adsConn{signalManager, logger};
	adsConn.run({}, TestSettings::Address, TestSettings::Port, clientEquipmentId);

	ASSERT_EQ(adsConn.m_connected, true);

	ASSERT_TRUE(adsConn.lastStatusCode().has_value());
	EXPECT_EQ(adsConn.lastStatusCode().value(), AdsGatewayLib::GwErrorCode::GWC_HANDSHAKE_REQUIRED);

	return;
}

// Expecting GWC_HANDSHAKE_REQUIRED error code when requesting signal params without handshake
// ADSGW_SIGNAL_PARAM_START/ADSGW_SIGNAL_PARAM_NEXT
//
TEST_F(AdsGatewayTests, RequestSignalParamWithoutHandshake)
{
	class TestAdsGwConnection : public AdsGatewayLib::AdsGwConnImpl
	{
	public:
		using AdsGwConnImpl::AdsGwConnImpl;

		virtual void run(std::stop_token, std::string_view address, uint16_t port, std::string_view /*equipmentId*/) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);
				requestSignalParams();
			}
			catch (const std::runtime_error&)
			{
				// Exceptions are expected
			}
		}

	public:
		bool m_connected = false;

		auto& lastStatusCode() const { return m_lastStatusCode; }
		auto& handshakeResponse() const { return m_handshakeResponse; }
	};

	TestAdsGwConnection adsConn{signalManager, logger};
	adsConn.run({}, TestSettings::Address, TestSettings::Port, clientEquipmentId);

	ASSERT_EQ(adsConn.m_connected, true);
	ASSERT_TRUE(adsConn.lastStatusCode().has_value());
	EXPECT_EQ(adsConn.lastStatusCode().value(), AdsGatewayLib::GwErrorCode::GWC_HANDSHAKE_REQUIRED);

	return;
}

// Expecting GWC_HANDSHAKE_REQUIRED error code when requesting signal states without handshake
// ADSGW_SIGNAL_STATE
//
TEST_F(AdsGatewayTests, RequestSignalStatesWithoutHandshake)
{
	class TestAdsGwConnection : public AdsGatewayLib::AdsGwConnImpl
	{
	public:
		using AdsGwConnImpl::AdsGwConnImpl;

		virtual void run(std::stop_token, std::string_view address, uint16_t port, std::string_view /*equipmentId*/) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);

				m_appSignalHashes.push_back(1234567890); // Dummy signal hash to avoid early exit
				m_appSignalHashes.push_back(3456);       // Dummy signal hash to avoid early exit

				requestSignalStates();
			}
			catch (const std::runtime_error&)
			{
				// Exceptions are expected
			}
		}

	public:
		bool m_connected = false;

		auto& lastStatusCode() const { return m_lastStatusCode; }
		auto& handshakeResponse() const { return m_handshakeResponse; }
	};

	TestAdsGwConnection adsConn{signalManager, logger};


	adsConn.run({}, TestSettings::Address, TestSettings::Port, clientEquipmentId);

	ASSERT_EQ(adsConn.m_connected, true);
	ASSERT_TRUE(adsConn.lastStatusCode().has_value());
	EXPECT_EQ(adsConn.lastStatusCode().value(), AdsGatewayLib::GwErrorCode::GWC_HANDSHAKE_REQUIRED);

	return;
}

// Expecting GWC_HANDSHAKE_REQUIRED error code when requesting signal state changes without handshake
// ADSGW_SIGNAL_STATE_CHANGES
//
TEST_F(AdsGatewayTests, RequestSignalStateChangesWithoutHandshake)
{
	class TestAdsGwConnection : public AdsGatewayLib::AdsGwConnImpl
	{
	public:
		using AdsGwConnImpl::AdsGwConnImpl;

		virtual void run(std::stop_token, std::string_view address, uint16_t port, std::string_view /*equipmentId*/) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);

				m_appSignalHashes.push_back(1234567890); // Dummy signal hash to avoid early exit
				m_appSignalHashes.push_back(3456);       // Dummy signal hash to avoid early exit

				requestStateChanges();
			}
			catch (const std::runtime_error&)
			{
				// Exceptions are expected
			}
		}

	public:
		bool m_connected = false;

		auto& lastStatusCode() const { return m_lastStatusCode; }
		auto& handshakeResponse() const { return m_handshakeResponse; }
	};

	TestAdsGwConnection adsConn{signalManager, logger};
	adsConn.run({}, TestSettings::Address, TestSettings::Port, clientEquipmentId);

	ASSERT_EQ(adsConn.m_connected, true);
	ASSERT_TRUE(adsConn.lastStatusCode().has_value());
	EXPECT_EQ(adsConn.lastStatusCode().value(), AdsGatewayLib::GwErrorCode::GWC_HANDSHAKE_REQUIRED);

	return;
}

// Expecting GWC_NO_ADS_CONNECTION error code when requesting signal states with no ADS connection
// for request ADSGW_SIGNAL_STATE
// Precondition: ADS Gateway server is running but no ADS connection is established
//
TEST_F(AdsGatewayTests, RequestSignalStatesWithoutAdsConnection)
{
	class TestAdsGwConnection : public AdsGatewayLib::AdsGwConnImpl
	{
	public:
		using AdsGwConnImpl::AdsGwConnImpl;

		virtual void run(std::stop_token, std::string_view address, uint16_t port, std::string_view equipmentId) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);
				requestHandshake(equipmentId);

				m_appSignalHashes.push_back(1234567890); // Dummy signal hash to avoid early exit
				m_appSignalHashes.push_back(3456);       // Dummy signal hash to avoid early exit

				requestSignalStates();
			}
			catch (const std::runtime_error&)
			{
				// Exceptions are expected
			}
		}

	public:
		bool m_connected = false;

		auto& lastStatusCode() const { return m_lastStatusCode; }
		auto& handshakeResponse() const { return m_handshakeResponse; }
	};

	TestAdsGwConnection adsConn{signalManager, logger};
	adsConn.run({}, TestSettings::Address, TestSettings::Port, clientEquipmentId);

	ASSERT_EQ(adsConn.m_connected, true);
	ASSERT_TRUE(adsConn.lastStatusCode().has_value());
	EXPECT_EQ(adsConn.lastStatusCode().value(), AdsGatewayLib::GwErrorCode::GWC_NO_ADS_CONNECTION);

	return;
}

// Expecting GWC_NO_ADS_CONNECTION error code when requesting signal states with no ADS connection
// for request ADSGW_SIGNAL_STATE_CHANGES
// Precondition: ADS Gateway server is running but no ADS connection is established
//
TEST_F(AdsGatewayTests, RequestSignalStateChangesWithoutAdsConnection)
{
	class TestAdsGwConnection : public AdsGatewayLib::AdsGwConnImpl
	{
	public:
		using AdsGwConnImpl::AdsGwConnImpl;

		virtual void run(std::stop_token, std::string_view address, uint16_t port, std::string_view equipmentId) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);
				requestHandshake(equipmentId);

				m_appSignalHashes.push_back(1234567890); // Dummy signal hash to avoid early exit
				m_appSignalHashes.push_back(3456);       // Dummy signal hash to avoid early exit

				requestStateChanges();
			}
			catch (const std::runtime_error&)
			{
				// Exceptions are expected
			}
		}

	public:
		bool m_connected = false;

		auto& lastStatusCode() const { return m_lastStatusCode; }
		auto& handshakeResponse() const { return m_handshakeResponse; }
	};

	TestAdsGwConnection adsConn{signalManager, logger};
	adsConn.run({}, TestSettings::Address, TestSettings::Port, clientEquipmentId);

	ASSERT_EQ(adsConn.m_connected, true);
	ASSERT_TRUE(adsConn.lastStatusCode().has_value());
	EXPECT_EQ(adsConn.lastStatusCode().value(), AdsGatewayLib::GwErrorCode::GWC_NO_ADS_CONNECTION);

	return;
}

// Request signal list after successful handshake
// Expected to receive non-empty signal list
//
TEST_F(AdsGatewayTests, RequestSignalList)
{
	class TestAdsGwConnection : public AdsGatewayLib::AdsGwConnImpl
	{
	public:
		using AdsGwConnImpl::AdsGwConnImpl;

		virtual void run(std::stop_token, std::string_view address, uint16_t port, std::string_view equipmentId) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);
				requestHandshake(equipmentId);

				m_receivedSignalList = requestSignalList();
			}
			catch (const std::runtime_error&)
			{
			}
		}

	public:
		bool m_connected = false;
		std::vector<std::string> m_receivedSignalList;

		auto& lastStatusCode() const { return m_lastStatusCode; }
		auto& handshakeResponse() const { return m_handshakeResponse; }
	};

	TestAdsGwConnection adsConn{signalManager, logger};
	adsConn.run({}, TestSettings::Address, TestSettings::Port, clientEquipmentId);

	EXPECT_EQ(adsConn.m_connected, true);
	ASSERT_TRUE(adsConn.lastStatusCode().has_value());
	EXPECT_EQ(adsConn.lastStatusCode().value(), AdsGatewayLib::GwErrorCode::GWC_SUCCESS);

	EXPECT_FALSE(adsConn.m_receivedSignalList.empty());
	EXPECT_GT(adsConn.m_receivedSignalList.size(), 3000);

	return;
}

// Request signal params after successful handshake
// Expected to receive non-empty signal param list
//
TEST_F(AdsGatewayTests, RequestSignalParams)
{
	class TestAdsGwConnection : public AdsGatewayLib::AdsGwConnImpl
	{
	public:
		using AdsGwConnImpl::AdsGwConnImpl;

		virtual void run(std::stop_token, std::string_view address, uint16_t port, std::string_view equipmentId) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);
				requestHandshake(equipmentId);

				m_receivedSignalParams = requestSignalParams();
			}
			catch (const std::runtime_error&)
			{
			}
		}

	public:
		bool m_connected = false;
		std::vector<AdsGatewayLib::GwAppSignalParam> m_receivedSignalParams;

		auto& lastStatusCode() const { return m_lastStatusCode; }
		auto& handshakeResponse() const { return m_handshakeResponse; }
	};

	TestAdsGwConnection adsConn{signalManager, logger};
	adsConn.run({}, TestSettings::Address, TestSettings::Port, clientEquipmentId);

	EXPECT_EQ(adsConn.m_connected, true);
	ASSERT_TRUE(adsConn.lastStatusCode().has_value());
	EXPECT_EQ(adsConn.lastStatusCode().value(), AdsGatewayLib::GwErrorCode::GWC_SUCCESS);

	EXPECT_FALSE(adsConn.m_receivedSignalParams.empty());
	EXPECT_GT(adsConn.m_receivedSignalParams.size(), 3000);

	return;
}


// Test Format Error for Handshake: Send excessive payload.
// Expected error code: GWC_REQUEST_FORMAT_ERROR
//
TEST_F(AdsGatewayTests, HandshakeSendExcessivePayload)
{
	class TestAdsGwConnection : public AdsGatewayLib::AdsGwConnImpl
	{
	public:
		using AdsGwConnImpl::AdsGwConnImpl;

		virtual void run(std::stop_token, std::string_view address, uint16_t port, std::string_view /*equipmentId*/) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);

				AdsGatewayLib::GwHandshakeRequest request{};
				AdsGatewayLib::GwHandshakeResponse response{};

				std::array<std::byte, 32> payload{};

				m_lastStatusCode = sendRequest<AdsGatewayLib::GwHandshakeRequest, std::byte, AdsGatewayLib::GwHandshakeResponse, std::byte>(
					AdsGatewayLib::ADSGW_HANDSHAKE,
					request,
					std::span<const std::byte>(payload),
					response,
					{},
					{});
			}
			catch (const std::runtime_error&)
			{
			}
		}

	public:
		bool m_connected = false;

		auto& lastStatusCode() const { return m_lastStatusCode; }
		auto& handshakeResponse() const { return m_handshakeResponse; }
	};

	TestAdsGwConnection adsConn{signalManager, logger};
	adsConn.run({}, TestSettings::Address, TestSettings::Port, clientEquipmentId);

	ASSERT_EQ(adsConn.m_connected, true);

	ASSERT_TRUE(adsConn.lastStatusCode().has_value());
	EXPECT_EQ(adsConn.lastStatusCode().value(), AdsGatewayLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);

	return;
}

// Test Format Error for Handshake: Send not less payload then required
// Expected error code: GWC_REQUEST_FORMAT_ERROR
//
TEST_F(AdsGatewayTests, HandshakeSendLessPayload)
{
	class TestAdsGwConnection : public AdsGatewayLib::AdsGwConnImpl
	{
	public:
		using AdsGwConnImpl::AdsGwConnImpl;

		virtual void run(std::stop_token, std::string_view address, uint16_t port, std::string_view /*equipmentId*/) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);

				struct FakeHandshakeRequest
				{
					// uint16_t protocolVersion; // Protocol version client supports (e.g., 0x0100 for v1.0)
					uint16_t reserved1;   // Reserved for future use
					char clientName[128]; // Null-terminated client name
				};

				FakeHandshakeRequest request{};
				AdsGatewayLib::GwHandshakeResponse response{};

				m_lastStatusCode = sendRequest(AdsGatewayLib::ADSGW_HANDSHAKE, request, response);
			}
			catch (const std::runtime_error&)
			{
			}
		}

	public:
		bool m_connected = false;

		auto& lastStatusCode() const { return m_lastStatusCode; }
		auto& handshakeResponse() const { return m_handshakeResponse; }
	};

	TestAdsGwConnection adsConn{signalManager, logger};
	adsConn.run({}, TestSettings::Address, TestSettings::Port, clientEquipmentId);

	ASSERT_EQ(adsConn.m_connected, true);

	ASSERT_TRUE(adsConn.lastStatusCode().has_value());
	EXPECT_EQ(adsConn.lastStatusCode().value(), AdsGatewayLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);

	return;
}

// Test Format Error for ADSGW_SIGNAL_LIST_START: send excessive payload.
// Expected error code: GWC_REQUEST_FORMAT_ERROR
//
TEST_F(AdsGatewayTests, SignalListStartSendExcessivePayload)
{
	class TestAdsGwConnection : public AdsGatewayLib::AdsGwConnImpl
	{
	public:
		using AdsGwConnImpl::AdsGwConnImpl;

		void run(std::stop_token, std::string_view address, uint16_t port, std::string_view equipmentId) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);
				requestHandshake(equipmentId);

				AdsGatewayLib::GwSignalListStartRequest request{};
				AdsGatewayLib::GwSignalListStartResponse response{};

				std::array<std::byte, 16> extraPayload;
				m_lastStatusCode =
					sendRequest<AdsGatewayLib::GwSignalListStartRequest, std::byte, AdsGatewayLib::GwSignalListStartResponse, std::byte>(
						AdsGatewayLib::ADSGW_SIGNAL_LIST_START,
						request,
						std::span<const std::byte>(extraPayload),
						response,
						std::span<std::byte>{},
						{});
			}
			catch (const std::runtime_error&)
			{
			}
		}

	public:
		bool m_connected = false;

		const auto& lastStatusCode() const { return m_lastStatusCode; }
	};

	TestAdsGwConnection adsConn{signalManager, logger};
	adsConn.run({}, TestSettings::Address, TestSettings::Port, clientEquipmentId);

	ASSERT_TRUE(adsConn.m_connected);
	ASSERT_TRUE(adsConn.lastStatusCode().has_value());
	EXPECT_EQ(adsConn.lastStatusCode().value(), AdsGatewayLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
}

// Test Format Error for ADSGW_SIGNAL_LIST_START: send less payload than required.
// Expected error code: GWC_REQUEST_FORMAT_ERROR
//
TEST_F(AdsGatewayTests, SignalListStartSendLessPayload)
{
	class TestAdsGwConnection : public AdsGatewayLib::AdsGwConnImpl
	{
	public:
		using AdsGwConnImpl::AdsGwConnImpl;

		void run(std::stop_token, std::string_view address, uint16_t port, std::string_view equipmentId) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);
				requestHandshake(equipmentId);

				struct FakeSignalListStartRequest
				{
					uint16_t reserved; // Smaller than the required 4 bytes
				};

				FakeSignalListStartRequest badRequest{};
				AdsGatewayLib::GwSignalListStartResponse response{};

				m_lastStatusCode = sendRequest(AdsGatewayLib::ADSGW_SIGNAL_LIST_START, badRequest, response);
			}
			catch (const std::runtime_error&)
			{
			}
		}

	public:
		bool m_connected = false;

		const auto& lastStatusCode() const { return m_lastStatusCode; }
	};

	TestAdsGwConnection adsConn{signalManager, logger};
	adsConn.run({}, TestSettings::Address, TestSettings::Port, clientEquipmentId);

	ASSERT_TRUE(adsConn.m_connected);
	ASSERT_TRUE(adsConn.lastStatusCode().has_value());
	EXPECT_EQ(adsConn.lastStatusCode().value(), AdsGatewayLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
}

// Test Format Error for ADSGW_SIGNAL_LIST_NEXT: send excessive payload.
// Expected error code: GWC_REQUEST_FORMAT_ERROR
//
TEST_F(AdsGatewayTests, SignalListNextSendExcessivePayload)
{
	class TestAdsGwConnection : public AdsGatewayLib::AdsGwConnImpl
	{
	public:
		using AdsGwConnImpl::AdsGwConnImpl;

		void run(std::stop_token, std::string_view address, uint16_t port, std::string_view equipmentId) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);
				requestHandshake(equipmentId);

				// Ensure server is ready to accept NEXT by issuing START once.
				AdsGatewayLib::GwSignalListStartRequest startReq{};
				AdsGatewayLib::GwSignalListStartResponse startResp{};
				auto startStatus = sendRequest(AdsGatewayLib::ADSGW_SIGNAL_LIST_START, startReq, startResp);
				if (startStatus != AdsGatewayLib::GWC_SUCCESS)
				{
					m_lastStatusCode = startStatus;
					return;
				}

				AdsGatewayLib::GwSignalListNextRequest nextReq{};
				nextReq.part = 0;

				AdsGatewayLib::GwSignalListNextResponse nextResp{};
				std::array<std::byte, 32> extraPayload{};

				m_lastStatusCode =
					sendRequest<AdsGatewayLib::GwSignalListNextRequest, std::byte, AdsGatewayLib::GwSignalListNextResponse, std::byte>(
						AdsGatewayLib::ADSGW_SIGNAL_LIST_NEXT,
						nextReq,
						std::span<const std::byte>(extraPayload),
						nextResp,
						std::span<std::byte>{},
						{});
			}
			catch (const std::runtime_error&)
			{
			}
		}

	public:
		bool m_connected = false;
		const auto& lastStatusCode() const { return m_lastStatusCode; }
	};

	TestAdsGwConnection adsConn{signalManager, logger};
	adsConn.run({}, TestSettings::Address, TestSettings::Port, clientEquipmentId);

	ASSERT_TRUE(adsConn.m_connected);
	ASSERT_TRUE(adsConn.lastStatusCode().has_value());
	EXPECT_EQ(adsConn.lastStatusCode().value(), AdsGatewayLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
}

// Test Format Error for ADSGW_SIGNAL_LIST_NEXT: send less payload than required.
// Expected error code: GWC_REQUEST_FORMAT_ERROR
//
TEST_F(AdsGatewayTests, SignalListNextSendLessPayload)
{
	class TestAdsGwConnection : public AdsGatewayLib::AdsGwConnImpl
	{
	public:
		using AdsGwConnImpl::AdsGwConnImpl;

		void run(std::stop_token, std::string_view address, uint16_t port, std::string_view equipmentId) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);
				requestHandshake(equipmentId);

				AdsGatewayLib::GwSignalListStartRequest startReq{};
				AdsGatewayLib::GwSignalListStartResponse startResp{};
				auto startStatus = sendRequest(AdsGatewayLib::ADSGW_SIGNAL_LIST_START, startReq, startResp);
				if (startStatus != AdsGatewayLib::GWC_SUCCESS)
				{
					m_lastStatusCode = startStatus;
					return;
				}

				struct FakeSignalListNextRequest
				{
					uint16_t part; // smaller than required 4-byte field
				};

				FakeSignalListNextRequest badReq{};
				badReq.part = 0;

				AdsGatewayLib::GwSignalListNextResponse response{};
				m_lastStatusCode = sendRequest(AdsGatewayLib::ADSGW_SIGNAL_LIST_NEXT, badReq, response);
			}
			catch (const std::runtime_error&)
			{
			}
		}

	public:
		bool m_connected = false;
		const auto& lastStatusCode() const { return m_lastStatusCode; }
	};

	TestAdsGwConnection adsConn{signalManager, logger};
	adsConn.run({}, TestSettings::Address, TestSettings::Port, clientEquipmentId);

	ASSERT_TRUE(adsConn.m_connected);
	ASSERT_TRUE(adsConn.lastStatusCode().has_value());
	EXPECT_EQ(adsConn.lastStatusCode().value(), AdsGatewayLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
}

// Test Format Error for ADSGW_SIGNAL_PARAM_START: send excessive payload.
// Expected error code: GWC_REQUEST_FORMAT_ERROR
//
TEST_F(AdsGatewayTests, SignalParamStartSendExcessivePayload)
{
	class TestAdsGwConnection : public AdsGatewayLib::AdsGwConnImpl
	{
	public:
		using AdsGwConnImpl::AdsGwConnImpl;

		void run(std::stop_token, std::string_view address, uint16_t port, std::string_view equipmentId) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);
				requestHandshake(equipmentId);

				AdsGatewayLib::GwSignalParamStartRequest request{};
				AdsGatewayLib::GwSignalParamStartResponse response{};

				std::array<std::byte, 32> extraPayload{};
				m_lastStatusCode =
					sendRequest<AdsGatewayLib::GwSignalParamStartRequest, std::byte, AdsGatewayLib::GwSignalParamStartResponse, std::byte>(
						AdsGatewayLib::ADSGW_SIGNAL_PARAM_START,
						request,
						std::span<const std::byte>(extraPayload),
						response,
						std::span<std::byte>{},
						{});
			}
			catch (const std::runtime_error&)
			{
			}
		}

	public:
		bool m_connected = false;
		const auto& lastStatusCode() const { return m_lastStatusCode; }
	};

	TestAdsGwConnection adsConn{signalManager, logger};
	adsConn.run({}, TestSettings::Address, TestSettings::Port, clientEquipmentId);

	ASSERT_TRUE(adsConn.m_connected);
	ASSERT_TRUE(adsConn.lastStatusCode().has_value());
	EXPECT_EQ(adsConn.lastStatusCode().value(), AdsGatewayLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
}

// Test Format Error for ADSGW_SIGNAL_PARAM_START: send less payload than required.
// Expected error code: GWC_REQUEST_FORMAT_ERROR
//
TEST_F(AdsGatewayTests, SignalParamStartSendLessPayload)
{
	class TestAdsGwConnection : public AdsGatewayLib::AdsGwConnImpl
	{
	public:
		using AdsGwConnImpl::AdsGwConnImpl;

		void run(std::stop_token, std::string_view address, uint16_t port, std::string_view equipmentId) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);
				requestHandshake(equipmentId);

				struct FakeSignalParamStartRequest
				{
					uint16_t reserved; // smaller than required 4 bytes
				};

				FakeSignalParamStartRequest badRequest{};
				AdsGatewayLib::GwSignalParamStartResponse response{};

				m_lastStatusCode = sendRequest(AdsGatewayLib::ADSGW_SIGNAL_PARAM_START, badRequest, response);
			}
			catch (const std::runtime_error&)
			{
			}
		}

	public:
		bool m_connected = false;
		const auto& lastStatusCode() const { return m_lastStatusCode; }
	};

	TestAdsGwConnection adsConn{signalManager, logger};
	adsConn.run({}, TestSettings::Address, TestSettings::Port, clientEquipmentId);

	ASSERT_TRUE(adsConn.m_connected);
	ASSERT_TRUE(adsConn.lastStatusCode().has_value());
	EXPECT_EQ(adsConn.lastStatusCode().value(), AdsGatewayLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
}

// Test Format Error for ADSGW_SIGNAL_PARAM_NEXT: send excessive payload.
// Expected error code: GWC_REQUEST_FORMAT_ERROR
//
TEST_F(AdsGatewayTests, SignalParamNextSendExcessivePayload)
{
	class TestAdsGwConnection : public AdsGatewayLib::AdsGwConnImpl
	{
	public:
		using AdsGwConnImpl::AdsGwConnImpl;

		void run(std::stop_token, std::string_view address, uint16_t port, std::string_view equipmentId) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);
				requestHandshake(equipmentId);

				// Start phase so NEXT is valid.
				AdsGatewayLib::GwSignalParamStartRequest startReq{};
				AdsGatewayLib::GwSignalParamStartResponse startResp{};
				auto startStatus = sendRequest(AdsGatewayLib::ADSGW_SIGNAL_PARAM_START, startReq, startResp);
				if (startStatus != AdsGatewayLib::GWC_SUCCESS)
				{
					m_lastStatusCode = startStatus;
					return;
				}

				AdsGatewayLib::GwSignalParamNextRequest nextReq{};
				nextReq.part = 0;

				AdsGatewayLib::GwSignalParamNextResponse nextResp{};
				std::array<std::byte, 32> extraPayload{};

				m_lastStatusCode =
					sendRequest<AdsGatewayLib::GwSignalParamNextRequest, std::byte, AdsGatewayLib::GwSignalParamNextResponse, std::byte>(
						AdsGatewayLib::ADSGW_SIGNAL_PARAM_NEXT,
						nextReq,
						std::span<const std::byte>(extraPayload),
						nextResp,
						std::span<std::byte>{},
						{});
			}
			catch (const std::runtime_error&)
			{
			}
		}

	public:
		bool m_connected = false;
		const auto& lastStatusCode() const { return m_lastStatusCode; }
	};

	TestAdsGwConnection adsConn{signalManager, logger};
	adsConn.run({}, TestSettings::Address, TestSettings::Port, clientEquipmentId);

	ASSERT_TRUE(adsConn.m_connected);
	ASSERT_TRUE(adsConn.lastStatusCode().has_value());
	EXPECT_EQ(adsConn.lastStatusCode().value(), AdsGatewayLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
}

// Test Format Error for ADSGW_SIGNAL_PARAM_NEXT: send less payload than required.
// Expected error code: GWC_REQUEST_FORMAT_ERROR
//
TEST_F(AdsGatewayTests, SignalParamNextSendLessPayload)
{
	class TestAdsGwConnection : public AdsGatewayLib::AdsGwConnImpl
	{
	public:
		using AdsGwConnImpl::AdsGwConnImpl;

		void run(std::stop_token, std::string_view address, uint16_t port, std::string_view equipmentId) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);
				requestHandshake(equipmentId);

				AdsGatewayLib::GwSignalParamStartRequest startReq{};
				AdsGatewayLib::GwSignalParamStartResponse startResp{};
				auto startStatus = sendRequest(AdsGatewayLib::ADSGW_SIGNAL_PARAM_START, startReq, startResp);
				if (startStatus != AdsGatewayLib::GWC_SUCCESS)
				{
					m_lastStatusCode = startStatus;
					return;
				}

				struct FakeSignalParamNextRequest
				{
					uint16_t part; // smaller than required 4-byte field
				};

				FakeSignalParamNextRequest badReq{};
				badReq.part = 0;

				AdsGatewayLib::GwSignalParamNextResponse response{};
				m_lastStatusCode = sendRequest(AdsGatewayLib::ADSGW_SIGNAL_PARAM_NEXT, badReq, response);
			}
			catch (const std::runtime_error&)
			{
			}
		}

	public:
		bool m_connected = false;
		const auto& lastStatusCode() const { return m_lastStatusCode; }
	};

	TestAdsGwConnection adsConn{signalManager, logger};
	adsConn.run({}, TestSettings::Address, TestSettings::Port, clientEquipmentId);

	ASSERT_TRUE(adsConn.m_connected);
	ASSERT_TRUE(adsConn.lastStatusCode().has_value());
	EXPECT_EQ(adsConn.lastStatusCode().value(), AdsGatewayLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
}

// Test Format Error for ADSGW_SIGNAL_STATE: send excessive payload.
// Expected error code: GWC_REQUEST_FORMAT_ERROR
//
TEST_F(AdsGatewayTests, SignalStateSendExcessivePayload)
{
	class TestAdsGwConnection : public AdsGatewayLib::AdsGwConnImpl
	{
	public:
		using AdsGwConnImpl::AdsGwConnImpl;

		void run(std::stop_token, std::string_view address, uint16_t port, std::string_view equipmentId) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);
				requestHandshake(equipmentId);

				AdsGatewayLib::GwSignalStateRequest request{};
				request.signalCount = 1;

				std::array<uint64_t, 2> hashes{111u, 222u}; // more hashes than requested
				AdsGatewayLib::GwSignalStateResponse response{};
				std::vector<AdsGatewayLib::GwAppSignalState> states(request.signalCount);

				m_lastStatusCode = sendRequest<AdsGatewayLib::GwSignalStateRequest,
											   uint64_t,
											   AdsGatewayLib::GwSignalStateResponse,
											   AdsGatewayLib::GwAppSignalState>(AdsGatewayLib::ADSGW_SIGNAL_STATE,
																				request,
																				std::span<const uint64_t>(hashes),
																				response,
																				std::span<AdsGatewayLib::GwAppSignalState>(states),
																				{});
			}
			catch (const std::runtime_error&)
			{
			}
		}

	public:
		bool m_connected = false;
		const auto& lastStatusCode() const { return m_lastStatusCode; }
	};

	TestAdsGwConnection adsConn{signalManager, logger};
	adsConn.run({}, TestSettings::Address, TestSettings::Port, clientEquipmentId);

	ASSERT_TRUE(adsConn.m_connected);
	ASSERT_TRUE(adsConn.lastStatusCode().has_value());
	EXPECT_EQ(adsConn.lastStatusCode().value(), AdsGatewayLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
}

// Test Format Error for ADSGW_SIGNAL_STATE: send less payload than required.
// Expected error code: GWC_REQUEST_FORMAT_ERROR
//
TEST_F(AdsGatewayTests, SignalStateSendLessPayload)
{
	class TestAdsGwConnection : public AdsGatewayLib::AdsGwConnImpl
	{
	public:
		using AdsGwConnImpl::AdsGwConnImpl;

		void run(std::stop_token, std::string_view address, uint16_t port, std::string_view equipmentId) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);
				requestHandshake(equipmentId);

				AdsGatewayLib::GwSignalStateRequest request{};
				request.signalCount = 3;              // expect 3 hashes

				std::array<uint64_t, 1> hashes{555u}; // provide fewer hashes than signalCount
				AdsGatewayLib::GwSignalStateResponse response{};
				std::vector<AdsGatewayLib::GwAppSignalState> states(request.signalCount);

				m_lastStatusCode = sendRequest<AdsGatewayLib::GwSignalStateRequest,
											   uint64_t,
											   AdsGatewayLib::GwSignalStateResponse,
											   AdsGatewayLib::GwAppSignalState>(AdsGatewayLib::ADSGW_SIGNAL_STATE,
																				request,
																				std::span<const uint64_t>(hashes),
																				response,
																				std::span<AdsGatewayLib::GwAppSignalState>(states),
																				{});
			}
			catch (const std::runtime_error&)
			{
			}
		}

	public:
		bool m_connected = false;
		const auto& lastStatusCode() const { return m_lastStatusCode; }
	};

	TestAdsGwConnection adsConn{signalManager, logger};
	adsConn.run({}, TestSettings::Address, TestSettings::Port, clientEquipmentId);

	ASSERT_TRUE(adsConn.m_connected);
	ASSERT_TRUE(adsConn.lastStatusCode().has_value());
	EXPECT_EQ(adsConn.lastStatusCode().value(), AdsGatewayLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
}

// Test Format Error for ADSGW_SIGNAL_STATE_CHANGES: send excessive payload.
// Expected error code: GWC_REQUEST_FORMAT_ERROR
//
TEST_F(AdsGatewayTests, SignalStateChangesSendExcessivePayload)
{
	class TestAdsGwConnection : public AdsGatewayLib::AdsGwConnImpl
	{
	public:
		using AdsGwConnImpl::AdsGwConnImpl;

		void run(std::stop_token, std::string_view address, uint16_t port, std::string_view equipmentId) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);
				requestHandshake(equipmentId);

				struct Larger
				{
					AdsGatewayLib::GwSignalStateChangesRequest request{};
					uint32_t extraData[32];
				} request{};

				AdsGatewayLib::GwSignalStateChangesResponse response{};

				m_lastStatusCode = sendRequest(AdsGatewayLib::ADSGW_SIGNAL_STATE_CHANGES, request, response);
			}
			catch (const std::runtime_error&)
			{
			}
		}

	public:
		bool m_connected = false;
		const auto& lastStatusCode() const { return m_lastStatusCode; }
	};

	TestAdsGwConnection adsConn{signalManager, logger};
	adsConn.run({}, TestSettings::Address, TestSettings::Port, clientEquipmentId);

	ASSERT_TRUE(adsConn.m_connected);
	ASSERT_TRUE(adsConn.lastStatusCode().has_value());
	EXPECT_EQ(adsConn.lastStatusCode().value(), AdsGatewayLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
}

// Test Format Error for ADSGW_SIGNAL_STATE_CHANGES: send excessive payload.
// Expected error code: GWC_REQUEST_FORMAT_ERROR
//
TEST_F(AdsGatewayTests, SignalStateChangesSendLessPayload)
{
	class TestAdsGwConnection : public AdsGatewayLib::AdsGwConnImpl
	{
	public:
		using AdsGwConnImpl::AdsGwConnImpl;

		void run(std::stop_token, std::string_view address, uint16_t port, std::string_view equipmentId) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);
				requestHandshake(equipmentId);

				struct Smaller
				{
				} request{};
				AdsGatewayLib::GwSignalStateChangesResponse response{};

				m_lastStatusCode = sendRequest(AdsGatewayLib::ADSGW_SIGNAL_STATE_CHANGES, request, response);
			}
			catch (const std::runtime_error&)
			{
			}
		}

	public:
		bool m_connected = false;
		const auto& lastStatusCode() const { return m_lastStatusCode; }
	};

	TestAdsGwConnection adsConn{signalManager, logger};
	adsConn.run({}, TestSettings::Address, TestSettings::Port, clientEquipmentId);

	ASSERT_TRUE(adsConn.m_connected);
	ASSERT_TRUE(adsConn.lastStatusCode().has_value());
	EXPECT_EQ(adsConn.lastStatusCode().value(), AdsGatewayLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
}

// Request too big part for ADSGW_SIGNAL_LIST_NEXT:
// Expected error code: GWC_REQUEST_FORMAT_ERROR -- it is not stated directly in the protocol doc but makes sense to have it.
//
TEST_F(AdsGatewayTests, SignalListGetInvalidPart)
{
	class TestAdsGwConnection : public AdsGatewayLib::AdsGwConnImpl
	{
	public:
		using AdsGwConnImpl::AdsGwConnImpl;

		void run(std::stop_token, std::string_view address, uint16_t port, std::string_view equipmentId) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);
				requestHandshake(equipmentId);

				// Ensure server is ready to accept NEXT by issuing START once.
				AdsGatewayLib::GwSignalListStartRequest startReq{};
				AdsGatewayLib::GwSignalListStartResponse startResp{};
				auto startStatus = sendRequest(AdsGatewayLib::ADSGW_SIGNAL_LIST_START, startReq, startResp);
				if (startStatus != AdsGatewayLib::GWC_SUCCESS)
				{
					m_lastStatusCode = startStatus;
					return;
				}

				{
					AdsGatewayLib::GwSignalListNextRequest request{};
					AdsGatewayLib::GwSignalListNextResponse response{};
					request.part = 1111; // Excessively large part number


					using AppSignalIdNetworkT = std::array<char, AdsGatewayLib::STRING_LENGTH_128>;
					std::vector<AppSignalIdNetworkT> responseVariablePartBuffer{};
					responseVariablePartBuffer.resize(startResp.itemsPerPart);
					std::memset(responseVariablePartBuffer.data(), 0, responseVariablePartBuffer.size() * sizeof(AppSignalIdNetworkT));

					m_lastStatusCode = sendRequest(AdsGatewayLib::ADSGW_SIGNAL_LIST_NEXT,
												   request,
												   std::span<const std::byte>{},
												   response,
												   std::span<AppSignalIdNetworkT>{responseVariablePartBuffer},
												   m_isCancelledFunc);
				}
			}
			catch (const std::runtime_error&)
			{
			}
		}

	public:
		bool m_connected = false;
		const auto& lastStatusCode() const { return m_lastStatusCode; }
	};

	TestAdsGwConnection adsConn{signalManager, logger};
	adsConn.run({}, TestSettings::Address, TestSettings::Port, clientEquipmentId);

	ASSERT_TRUE(adsConn.m_connected);
	ASSERT_TRUE(adsConn.lastStatusCode().has_value());
	EXPECT_EQ(adsConn.lastStatusCode().value(), AdsGatewayLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
}

// Request too big part for ADSGW_SIGNAL_LIST_NEXT:
// Expected error code: GWC_REQUEST_FORMAT_ERROR -- it is not stated directly in the protocol doc but makes sense to have it.
//
TEST_F(AdsGatewayTests, SignalParamGetInvalidPart)
{
	class TestAdsGwConnection : public AdsGatewayLib::AdsGwConnImpl
	{
	public:
		using AdsGwConnImpl::AdsGwConnImpl;

		void run(std::stop_token, std::string_view address, uint16_t port, std::string_view equipmentId) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);
				requestHandshake(equipmentId);

				// Ensure server is ready to accept NEXT by issuing START once.
				AdsGatewayLib::GwSignalParamStartRequest startReq{};
				AdsGatewayLib::GwSignalParamStartResponse startResp{};
				auto startStatus = sendRequest(AdsGatewayLib::ADSGW_SIGNAL_PARAM_START, startReq, startResp);
				if (startStatus != AdsGatewayLib::GWC_SUCCESS)
				{
					m_lastStatusCode = startStatus;
					return;
				}

				{
					AdsGatewayLib::GwSignalParamNextRequest request{};
					AdsGatewayLib::GwSignalParamNextResponse response{};
					request.part = 1111; // Excessively large part number

					std::vector<AdsGatewayLib::GwAppSignalParam> responseVariablePartBuffer{};
					responseVariablePartBuffer.resize(startResp.itemsPerPart);

					m_lastStatusCode = sendRequest(AdsGatewayLib::ADSGW_SIGNAL_PARAM_NEXT,
												   request,
												   std::span<const std::byte>{},
												   response,
												   std::span<AdsGatewayLib::GwAppSignalParam>{responseVariablePartBuffer},
												   m_isCancelledFunc);
				}
			}
			catch (const std::runtime_error&)
			{
			}
		}

	public:
		bool m_connected = false;
		const auto& lastStatusCode() const { return m_lastStatusCode; }
	};

	TestAdsGwConnection adsConn{signalManager, logger};
	adsConn.run({}, TestSettings::Address, TestSettings::Port, clientEquipmentId);

	ASSERT_TRUE(adsConn.m_connected);
	ASSERT_TRUE(adsConn.lastStatusCode().has_value());
	EXPECT_EQ(adsConn.lastStatusCode().value(), AdsGatewayLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
}

// Request several signal state which are not existing
// Expecting: These signals are not returned.
//
TEST_F(AdsGatewayTests, RequestNonexistingSignalStates)
{
	class TestAdsGwConnection : public AdsGatewayLib::AdsGwConnImpl
	{
	public:
		using AdsGwConnImpl::AdsGwConnImpl;

		void run(std::stop_token, std::string_view address, uint16_t port, std::string_view equipmentId) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);
				requestHandshake(equipmentId);

				AdsGatewayLib::GwSignalStateRequest request{};
				AdsGatewayLib::GwSignalStateResponse response{};

				request.signalCount = 3;
				std::array<uint64_t, 3> hashes{555u, 666u, 777u};

				std::vector<AdsGatewayLib::GwAppSignalState> states(request.signalCount);

				m_lastStatusCode = sendRequest<AdsGatewayLib::GwSignalStateRequest,
											   uint64_t,
											   AdsGatewayLib::GwSignalStateResponse,
											   AdsGatewayLib::GwAppSignalState>(AdsGatewayLib::ADSGW_SIGNAL_STATE,
																				request,
																				std::span<const uint64_t>(hashes),
																				response,
																				std::span<AdsGatewayLib::GwAppSignalState>(states),
																				{});
				returnedSignalStates = static_cast<int>(response.stateCount);
			}
			catch (const std::runtime_error&)
			{
			}
		}

	public:
		bool m_connected = false;
		int returnedSignalStates = -1;
		const auto& lastStatusCode() const { return m_lastStatusCode; }
	};

	TestAdsGwConnection adsConn{signalManager, logger};
	adsConn.run({}, TestSettings::Address, TestSettings::Port, clientEquipmentId);

	ASSERT_TRUE(adsConn.m_connected);
	ASSERT_TRUE(adsConn.lastStatusCode().has_value());
	EXPECT_EQ(adsConn.lastStatusCode().value(), AdsGatewayLib::GwErrorCode::GWC_SUCCESS);
	EXPECT_EQ(adsConn.returnedSignalStates, 0);
}

// Request too many signal states
// Expecting: GWC_TOO_MANY_SIGNALS
//
TEST_F(AdsGatewayTests, RequestTooManySignalStates)
{
	class TestAdsGwConnection : public AdsGatewayLib::AdsGwConnImpl
	{
	public:
		using AdsGwConnImpl::AdsGwConnImpl;

		void run(std::stop_token, std::string_view address, uint16_t port, std::string_view equipmentId) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);
				requestHandshake(equipmentId);

				AdsGatewayLib::GwSignalStateRequest request{};
				AdsGatewayLib::GwSignalStateResponse response{};

				request.signalCount = m_handshakeResponse.maxStateRequest + 1; // Exceed maximum
				std::vector<uint64_t> hashes(request.signalCount);
				std::iota(hashes.begin(), hashes.end(), 1000u);

				std::vector<AdsGatewayLib::GwAppSignalState> states(request.signalCount);

				m_lastStatusCode = sendRequest<AdsGatewayLib::GwSignalStateRequest,
											   uint64_t,
											   AdsGatewayLib::GwSignalStateResponse,
											   AdsGatewayLib::GwAppSignalState>(AdsGatewayLib::ADSGW_SIGNAL_STATE,
																				request,
																				std::span<const uint64_t>(hashes),
																				response,
																				std::span<AdsGatewayLib::GwAppSignalState>(states),
																				{});
			}
			catch (const std::runtime_error&)
			{
			}
		}

	public:
		bool m_connected = false;
		const auto& lastStatusCode() const { return m_lastStatusCode; }
	};

	TestAdsGwConnection adsConn{signalManager, logger};
	adsConn.run({}, TestSettings::Address, TestSettings::Port, clientEquipmentId);

	ASSERT_TRUE(adsConn.m_connected);
	ASSERT_TRUE(adsConn.lastStatusCode().has_value());
	EXPECT_EQ(adsConn.lastStatusCode().value(), AdsGatewayLib::GwErrorCode::GWC_TOO_MANY_SIGNALS);
}

// Request several signal states.
// Expecting: Normal behavior.
//
TEST_F(AdsGatewayTests, RequestSignalStates)
{
	class TestAdsGwConnection : public AdsGatewayLib::AdsGwConnImpl
	{
	public:
		using AdsGwConnImpl::AdsGwConnImpl;

		void run(std::stop_token, std::string_view address, uint16_t port, std::string_view equipmentId) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);
				requestHandshake(equipmentId);

				AdsGatewayLib::GwSignalStateRequest request{};
				AdsGatewayLib::GwSignalStateResponse response{};

				std::vector<Radiy::Hash> hashes;
				hashes.push_back(Radiy::calcHash("#CT_RT_NOT_0101")); // These signals exist in the project
				hashes.push_back(Radiy::calcHash("#CT_RT_ADDFP"));
				hashes.push_back(Radiy::calcHash("#SYSTEMID_CLIENTTEST_CH10_MD00_PI_BLINK"));
				hashes.push_back(Radiy::calcHash("#CLIENTTEST_TUNING_D2"));
				request.signalCount = static_cast<uint32_t>(hashes.size());

				states.resize(request.signalCount);

				m_lastStatusCode = sendRequest<AdsGatewayLib::GwSignalStateRequest,
											   Radiy::Hash,
											   AdsGatewayLib::GwSignalStateResponse,
											   AdsGatewayLib::GwAppSignalState>(AdsGatewayLib::ADSGW_SIGNAL_STATE,
																				request,
																				std::span<const Radiy::Hash>(hashes),
																				response,
																				std::span<AdsGatewayLib::GwAppSignalState>(states),
																				{});
				returnedSignalStates = response.stateCount;
			}
			catch (const std::runtime_error&)
			{
			}
		}

	public:
		bool m_connected = false;
		const auto& lastStatusCode() const { return m_lastStatusCode; }

		uint32_t returnedSignalStates = 0;
		std::vector<AdsGatewayLib::GwAppSignalState> states;
	};

	TestAdsGwConnection adsConn{signalManager, logger};
	adsConn.run({}, TestSettings::Address, TestSettings::Port, clientEquipmentId);

	ASSERT_TRUE(adsConn.m_connected);
	ASSERT_TRUE(adsConn.lastStatusCode().has_value());
	EXPECT_EQ(adsConn.lastStatusCode().value(), AdsGatewayLib::GwErrorCode::GWC_SUCCESS);

	EXPECT_EQ(adsConn.returnedSignalStates, 4);

	bool found1 = std::any_of(adsConn.states.begin(),
							  adsConn.states.end(),
							  [](const AdsGatewayLib::GwAppSignalState& state)
							  {
								  return state.hash == Radiy::calcHash("#CT_RT_NOT_0101");
							  });
	EXPECT_TRUE(found1); // Signal state #CT_RT_NOT_0101 not found

	bool found2 = std::any_of(adsConn.states.begin(),
							  adsConn.states.end(),
							  [](const AdsGatewayLib::GwAppSignalState& state)
							  {
								  return state.hash == Radiy::calcHash("#CT_RT_ADDFP");
							  });
	EXPECT_TRUE(found2); // Signal state #CT_RT_ADDFP not found

	bool found3 = std::any_of(adsConn.states.begin(),
							  adsConn.states.end(),
							  [](const AdsGatewayLib::GwAppSignalState& state)
							  {
								  return state.hash == Radiy::calcHash("#SYSTEMID_CLIENTTEST_CH10_MD00_PI_BLINK");
							  });
	EXPECT_TRUE(found3); // Signal state #SYSTEMID_CLIENTTEST_CH10_MD00_PI_BLINK not found

	bool found4 = std::any_of(adsConn.states.begin(),
							  adsConn.states.end(),
							  [](const AdsGatewayLib::GwAppSignalState& state)
							  {
								  return state.hash == Radiy::calcHash("#CLIENTTEST_TUNING_D2");
							  });
	EXPECT_TRUE(found4); // Signal state #CLIENTTEST_TUNING_D2 not found
}