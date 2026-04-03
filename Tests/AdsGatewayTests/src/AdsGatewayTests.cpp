#include "TestSettings.hpp"

#include <GatewayClientLib/../../src/AdsGwConnImpl.hpp>
#include <GatewayClientLib/AdsSignalManager.hpp>
#include <GatewayClientLib/Logger.hpp>

#include <gmock/gmock-matchers.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>
#include <cmath>
#include <numeric>


class AdsGatewayTests : public testing::Test
{
public:
	GatewayClientLib::AdsSignalManager signalManager{};
	GatewayClientLib::ConsoleLogger logger{};

	std::string clientEquipmentId = "TEST_CLIENT_EQUIPMENT_ID";
};

// Test that connection fails when no server is available
//
TEST_F(AdsGatewayTests, NoConnection)
{
	class TestAdsGwConnection : public GatewayClientLib::AdsGwConnImpl
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
	class TestAdsGwConnection : public GatewayClientLib::AdsGwConnImpl
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
	EXPECT_EQ(adsConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_SUCCESS);
	EXPECT_EQ(adsConn.handshakeResponse().protocolVersion, GatewayClientLib::ADS_GW_PROTOCOL_VERSION);
	EXPECT_EQ(adsConn.handshakeResponse().sizeof_GwAppSignalParam, sizeof(GatewayClientLib::GwAppSignalParam));
	EXPECT_EQ(adsConn.handshakeResponse().sizeof_GwAppSignalState, sizeof(GatewayClientLib::GwAppSignalState));

	return;
}

// Send unsupported protocol version in handshake request
// Expecting GWC_UNSUPPORTED_VERSION error code in response
// 3.1
TEST_F(AdsGatewayTests, SendUnsupportedProtocolVersion)
{
	class TestAdsGwConnection : public GatewayClientLib::AdsGwConnImpl
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
	EXPECT_EQ(adsConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_UNSUPPORTED_VERSION);

	return;
}

// Send unknown RequestID
// Expected: GWC_INVALID_REQUEST
//
TEST_F(AdsGatewayTests, SendInvalidRequest)
{
	class TestAdsGwConnection : public GatewayClientLib::AdsGwConnImpl
	{
	public:
		using AdsGwConnImpl::AdsGwConnImpl;

		virtual void run(std::stop_token, std::string_view address, uint16_t port, std::string_view /*equipmentId*/) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);

				GatewayClientLib::AdsGwHandshakeRequest request{};
				GatewayClientLib::AdsGwHandshakeResponse response{};

				sendRequest(static_cast<GatewayClientLib::AdsGwRequestId>(199999), request, response, {});
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
	EXPECT_EQ(adsConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_INVALID_REQUEST);

	return;
}

// Send invalid CRC in request
// Expected: GWC_CRC_ERROR
//
TEST_F(AdsGatewayTests, SendInvalidCrc32)
{
	class TestAdsGwConnection : public GatewayClientLib::AdsGwConnImpl
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
				GatewayClientLib::AdsGwHandshakeRequest request{};
				GatewayClientLib::AdsGwHandshakeResponse response{};

				request.protocolVersion = GatewayClientLib::ADS_GW_PROTOCOL_VERSION;
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

				writeUint32(static_cast<uint32_t>(GatewayClientLib::AdsGwRequestId::ADSGW_HANDSHAKE));
				writeUint32(static_cast<uint32_t>(sizeof(request)));                            // Payload size
				writeUint32(static_cast<uint32_t>(GatewayClientLib::GwErrorCode::GWC_SUCCESS)); // Status code for request is always 0

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
					receiveResponsePacket<GatewayClientLib::AdsGwHandshakeResponse>(GatewayClientLib::AdsGwRequestId::ADSGW_HANDSHAKE,
																					response,
																					{},
																					{});

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
	EXPECT_EQ(adsConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_CRC_ERROR);

	return;
}


// Expecting GWC_HANDSHAKE_REQUIRED error code when requesting signal list without handshake
// ADSGW_SIGNAL_LIST_START/ADSGW_SIGNAL_LIST_NEXT
//
TEST_F(AdsGatewayTests, RequestSignalListWithoutHandshake)
{
	class TestAdsGwConnection : public GatewayClientLib::AdsGwConnImpl
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
	EXPECT_EQ(adsConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_HANDSHAKE_REQUIRED);

	return;
}

// Expecting GWC_HANDSHAKE_REQUIRED error code when requesting signal params without handshake
// ADSGW_SIGNAL_PARAM_START/ADSGW_SIGNAL_PARAM_NEXT
//
TEST_F(AdsGatewayTests, RequestSignalParamWithoutHandshake)
{
	class TestAdsGwConnection : public GatewayClientLib::AdsGwConnImpl
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
	EXPECT_EQ(adsConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_HANDSHAKE_REQUIRED);

	return;
}

// Expecting GWC_HANDSHAKE_REQUIRED error code when requesting signal states without handshake
// ADSGW_SIGNAL_STATE
//
TEST_F(AdsGatewayTests, RequestSignalStatesWithoutHandshake)
{
	class TestAdsGwConnection : public GatewayClientLib::AdsGwConnImpl
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
	EXPECT_EQ(adsConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_HANDSHAKE_REQUIRED);

	return;
}

// Expecting GWC_HANDSHAKE_REQUIRED error code when requesting signal state changes without handshake
// ADSGW_SIGNAL_STATE_CHANGES
//
TEST_F(AdsGatewayTests, RequestSignalStateChangesWithoutHandshake)
{
	class TestAdsGwConnection : public GatewayClientLib::AdsGwConnImpl
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
	EXPECT_EQ(adsConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_HANDSHAKE_REQUIRED);

	return;
}

// Expecting GWC_NO_ADS_CONNECTION error code when requesting signal states with no ADS connection
// for request ADSGW_SIGNAL_STATE
// Precondition: ADS Gateway server is running but no ADS connection is established
//
TEST_F(AdsGatewayTests, RequestSignalStatesWithoutAdsConnection)
{
	class TestAdsGwConnection : public GatewayClientLib::AdsGwConnImpl
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
	EXPECT_EQ(adsConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_NO_ADS_CONNECTION);

	return;
}

// Expecting GWC_NO_ADS_CONNECTION error code when requesting signal states with no ADS connection
// for request ADSGW_SIGNAL_STATE_CHANGES
// Precondition: ADS Gateway server is running but no ADS connection is established
//
TEST_F(AdsGatewayTests, RequestSignalStateChangesWithoutAdsConnection)
{
	class TestAdsGwConnection : public GatewayClientLib::AdsGwConnImpl
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
	EXPECT_EQ(adsConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_NO_ADS_CONNECTION);

	return;
}

// Request signal list after successful handshake
// Expected to receive non-empty signal list
//
TEST_F(AdsGatewayTests, RequestSignalList)
{
	class TestAdsGwConnection : public GatewayClientLib::AdsGwConnImpl
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
	EXPECT_EQ(adsConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_SUCCESS);

	EXPECT_FALSE(adsConn.m_receivedSignalList.empty());
	EXPECT_GT(adsConn.m_receivedSignalList.size(), 5);

	return;
}

// Request signal params after successful handshake
// Expected to receive non-empty signal param list
//
TEST_F(AdsGatewayTests, RequestSignalParams)
{
	class TestAdsGwConnection : public GatewayClientLib::AdsGwConnImpl
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
		std::vector<GatewayClientLib::GwAppSignalParam> m_receivedSignalParams;

		auto& lastStatusCode() const { return m_lastStatusCode; }
		auto& handshakeResponse() const { return m_handshakeResponse; }
	};

	TestAdsGwConnection adsConn{signalManager, logger};
	adsConn.run({}, TestSettings::Address, TestSettings::Port, clientEquipmentId);

	EXPECT_EQ(adsConn.m_connected, true);
	ASSERT_TRUE(adsConn.lastStatusCode().has_value());
	EXPECT_EQ(adsConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_SUCCESS);

	EXPECT_FALSE(adsConn.m_receivedSignalParams.empty());
	EXPECT_GT(adsConn.m_receivedSignalParams.size(), 5);

	return;
}


// Test Format Error for Handshake: Send excessive payload.
// Expected error code: GWC_REQUEST_FORMAT_ERROR
//
TEST_F(AdsGatewayTests, HandshakeSendExcessivePayload)
{
	class TestAdsGwConnection : public GatewayClientLib::AdsGwConnImpl
	{
	public:
		using AdsGwConnImpl::AdsGwConnImpl;

		virtual void run(std::stop_token, std::string_view address, uint16_t port, std::string_view /*equipmentId*/) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);

				GatewayClientLib::AdsGwHandshakeRequest request{};
				GatewayClientLib::AdsGwHandshakeResponse response{};

				std::array<std::byte, 32> payload{};

				m_lastStatusCode =
					sendRequest<GatewayClientLib::AdsGwHandshakeRequest, std::byte, GatewayClientLib::AdsGwHandshakeResponse, std::byte>(
						GatewayClientLib::AdsGwRequestId::ADSGW_HANDSHAKE,
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
	EXPECT_EQ(adsConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);

	return;
}

// Test Format Error for Handshake: Send not less payload then required
// Expected error code: GWC_REQUEST_FORMAT_ERROR
//
TEST_F(AdsGatewayTests, HandshakeSendLessPayload)
{
	class TestAdsGwConnection : public GatewayClientLib::AdsGwConnImpl
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
				GatewayClientLib::AdsGwHandshakeResponse response{};

				m_lastStatusCode = sendRequest(GatewayClientLib::AdsGwRequestId::ADSGW_HANDSHAKE, request, response);
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
	EXPECT_EQ(adsConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);

	return;
}

// Test Format Error for ADSGW_SIGNAL_LIST_START: send excessive payload.
// Expected error code: GWC_REQUEST_FORMAT_ERROR
//
TEST_F(AdsGatewayTests, SignalListStartSendExcessivePayload)
{
	class TestAdsGwConnection : public GatewayClientLib::AdsGwConnImpl
	{
	public:
		using AdsGwConnImpl::AdsGwConnImpl;

		void run(std::stop_token, std::string_view address, uint16_t port, std::string_view equipmentId) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);
				requestHandshake(equipmentId);

				GatewayClientLib::AdsGwSignalListStartRequest request{};
				GatewayClientLib::AdsGwSignalListStartResponse response{};

				std::array<std::byte, 16> extraPayload;
				m_lastStatusCode = sendRequest<GatewayClientLib::AdsGwSignalListStartRequest,
											   std::byte,
											   GatewayClientLib::AdsGwSignalListStartResponse,
											   std::byte>(GatewayClientLib::AdsGwRequestId::ADSGW_SIGNAL_LIST_START,
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
	EXPECT_EQ(adsConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
}

// Test Format Error for ADSGW_SIGNAL_LIST_START: send less payload than required.
// Expected error code: GWC_REQUEST_FORMAT_ERROR
//
TEST_F(AdsGatewayTests, SignalListStartSendLessPayload)
{
	class TestAdsGwConnection : public GatewayClientLib::AdsGwConnImpl
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
				GatewayClientLib::AdsGwSignalListStartResponse response{};

				m_lastStatusCode = sendRequest(GatewayClientLib::AdsGwRequestId::ADSGW_SIGNAL_LIST_START, badRequest, response);
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
	EXPECT_EQ(adsConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
}

// Test Format Error for ADSGW_SIGNAL_LIST_NEXT: send excessive payload.
// Expected error code: GWC_REQUEST_FORMAT_ERROR
//
TEST_F(AdsGatewayTests, SignalListNextSendExcessivePayload)
{
	class TestAdsGwConnection : public GatewayClientLib::AdsGwConnImpl
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
				GatewayClientLib::AdsGwSignalListStartRequest startReq{};
				GatewayClientLib::AdsGwSignalListStartResponse startResp{};
				auto startStatus = sendRequest(GatewayClientLib::AdsGwRequestId::ADSGW_SIGNAL_LIST_START, startReq, startResp);
				if (startStatus != GatewayClientLib::GwErrorCode::GWC_SUCCESS)
				{
					m_lastStatusCode = startStatus;
					return;
				}

				GatewayClientLib::AdsGwSignalListNextRequest nextReq{};
				nextReq.part = 0;

				GatewayClientLib::AdsGwSignalListNextResponse nextResp{};
				std::array<std::byte, 32> extraPayload{};

				m_lastStatusCode = sendRequest<GatewayClientLib::AdsGwSignalListNextRequest,
											   std::byte,
											   GatewayClientLib::AdsGwSignalListNextResponse,
											   std::byte>(GatewayClientLib::AdsGwRequestId::ADSGW_SIGNAL_LIST_NEXT,
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
	EXPECT_EQ(adsConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
}

// Test Format Error for ADSGW_SIGNAL_LIST_NEXT: send less payload than required.
// Expected error code: GWC_REQUEST_FORMAT_ERROR
//
TEST_F(AdsGatewayTests, SignalListNextSendLessPayload)
{
	class TestAdsGwConnection : public GatewayClientLib::AdsGwConnImpl
	{
	public:
		using AdsGwConnImpl::AdsGwConnImpl;

		void run(std::stop_token, std::string_view address, uint16_t port, std::string_view equipmentId) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);
				requestHandshake(equipmentId);

				GatewayClientLib::AdsGwSignalListStartRequest startReq{};
				GatewayClientLib::AdsGwSignalListStartResponse startResp{};
				auto startStatus = sendRequest(GatewayClientLib::AdsGwRequestId::ADSGW_SIGNAL_LIST_START, startReq, startResp);
				if (startStatus != GatewayClientLib::GwErrorCode::GWC_SUCCESS)
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

				GatewayClientLib::AdsGwSignalListNextResponse response{};
				m_lastStatusCode = sendRequest(GatewayClientLib::AdsGwRequestId::ADSGW_SIGNAL_LIST_NEXT, badReq, response);
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
	EXPECT_EQ(adsConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
}

// Test Format Error for ADSGW_SIGNAL_PARAM_START: send excessive payload.
// Expected error code: GWC_REQUEST_FORMAT_ERROR
//
TEST_F(AdsGatewayTests, SignalParamStartSendExcessivePayload)
{
	class TestAdsGwConnection : public GatewayClientLib::AdsGwConnImpl
	{
	public:
		using AdsGwConnImpl::AdsGwConnImpl;

		void run(std::stop_token, std::string_view address, uint16_t port, std::string_view equipmentId) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);
				requestHandshake(equipmentId);

				GatewayClientLib::AdsGwSignalParamStartRequest request{};
				GatewayClientLib::AdsGwSignalParamStartResponse response{};

				std::array<std::byte, 32> extraPayload{};
				m_lastStatusCode = sendRequest<GatewayClientLib::AdsGwSignalParamStartRequest,
											   std::byte,
											   GatewayClientLib::AdsGwSignalParamStartResponse,
											   std::byte>(GatewayClientLib::AdsGwRequestId::ADSGW_SIGNAL_PARAM_START,
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
	EXPECT_EQ(adsConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
}

// Test Format Error for ADSGW_SIGNAL_PARAM_START: send less payload than required.
// Expected error code: GWC_REQUEST_FORMAT_ERROR
//
TEST_F(AdsGatewayTests, SignalParamStartSendLessPayload)
{
	class TestAdsGwConnection : public GatewayClientLib::AdsGwConnImpl
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
				GatewayClientLib::AdsGwSignalParamStartResponse response{};

				m_lastStatusCode = sendRequest(GatewayClientLib::AdsGwRequestId::ADSGW_SIGNAL_PARAM_START, badRequest, response);
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
	EXPECT_EQ(adsConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
}

// Test Format Error for ADSGW_SIGNAL_PARAM_NEXT: send excessive payload.
// Expected error code: GWC_REQUEST_FORMAT_ERROR
//
TEST_F(AdsGatewayTests, SignalParamNextSendExcessivePayload)
{
	class TestAdsGwConnection : public GatewayClientLib::AdsGwConnImpl
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
				GatewayClientLib::AdsGwSignalParamStartRequest startReq{};
				GatewayClientLib::AdsGwSignalParamStartResponse startResp{};
				auto startStatus = sendRequest(GatewayClientLib::AdsGwRequestId::ADSGW_SIGNAL_PARAM_START, startReq, startResp);
				if (startStatus != GatewayClientLib::GwErrorCode::GWC_SUCCESS)
				{
					m_lastStatusCode = startStatus;
					return;
				}

				GatewayClientLib::AdsGwSignalParamNextRequest nextReq{};
				nextReq.part = 0;

				GatewayClientLib::AdsGwSignalParamNextResponse nextResp{};
				std::array<std::byte, 32> extraPayload{};

				m_lastStatusCode = sendRequest<GatewayClientLib::AdsGwSignalParamNextRequest,
											   std::byte,
											   GatewayClientLib::AdsGwSignalParamNextResponse,
											   std::byte>(GatewayClientLib::AdsGwRequestId::ADSGW_SIGNAL_PARAM_NEXT,
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
	EXPECT_EQ(adsConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
}

// Test Format Error for ADSGW_SIGNAL_PARAM_NEXT: send less payload than required.
// Expected error code: GWC_REQUEST_FORMAT_ERROR
//
TEST_F(AdsGatewayTests, SignalParamNextSendLessPayload)
{
	class TestAdsGwConnection : public GatewayClientLib::AdsGwConnImpl
	{
	public:
		using AdsGwConnImpl::AdsGwConnImpl;

		void run(std::stop_token, std::string_view address, uint16_t port, std::string_view equipmentId) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);
				requestHandshake(equipmentId);

				GatewayClientLib::AdsGwSignalParamStartRequest startReq{};
				GatewayClientLib::AdsGwSignalParamStartResponse startResp{};
				auto startStatus = sendRequest(GatewayClientLib::AdsGwRequestId::ADSGW_SIGNAL_PARAM_START, startReq, startResp);
				if (startStatus != GatewayClientLib::GwErrorCode::GWC_SUCCESS)
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

				GatewayClientLib::AdsGwSignalParamNextResponse response{};
				m_lastStatusCode = sendRequest(GatewayClientLib::AdsGwRequestId::ADSGW_SIGNAL_PARAM_NEXT, badReq, response);
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
	EXPECT_EQ(adsConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
}

// Test Format Error for ADSGW_SIGNAL_STATE: send excessive payload.
// Expected error code: GWC_REQUEST_FORMAT_ERROR
//
TEST_F(AdsGatewayTests, SignalStateSendExcessivePayload)
{
	class TestAdsGwConnection : public GatewayClientLib::AdsGwConnImpl
	{
	public:
		using AdsGwConnImpl::AdsGwConnImpl;

		void run(std::stop_token, std::string_view address, uint16_t port, std::string_view equipmentId) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);
				requestHandshake(equipmentId);

				GatewayClientLib::AdsGwSignalStateRequest request{};
				request.signalCount = 1;

				std::array<uint64_t, 2> hashes{111u, 222u}; // more hashes than requested
				GatewayClientLib::AdsGwSignalStateResponse response{};
				std::vector<GatewayClientLib::GwAppSignalState> states(request.signalCount);

				m_lastStatusCode = sendRequest<GatewayClientLib::AdsGwSignalStateRequest,
											   uint64_t,
											   GatewayClientLib::AdsGwSignalStateResponse,
											   GatewayClientLib::GwAppSignalState>(GatewayClientLib::AdsGwRequestId::ADSGW_SIGNAL_STATE,
																				   request,
																				   std::span<const uint64_t>(hashes),
																				   response,
																				   std::span<GatewayClientLib::GwAppSignalState>(states),
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
	EXPECT_EQ(adsConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
}

// Test Format Error for ADSGW_SIGNAL_STATE: send less payload than required.
// Expected error code: GWC_REQUEST_FORMAT_ERROR
//
TEST_F(AdsGatewayTests, SignalStateSendLessPayload)
{
	class TestAdsGwConnection : public GatewayClientLib::AdsGwConnImpl
	{
	public:
		using AdsGwConnImpl::AdsGwConnImpl;

		void run(std::stop_token, std::string_view address, uint16_t port, std::string_view equipmentId) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);
				requestHandshake(equipmentId);

				GatewayClientLib::AdsGwSignalStateRequest request{};
				request.signalCount = 3;              // expect 3 hashes

				std::array<uint64_t, 1> hashes{555u}; // provide fewer hashes than signalCount
				GatewayClientLib::AdsGwSignalStateResponse response{};
				std::vector<GatewayClientLib::GwAppSignalState> states(request.signalCount);

				m_lastStatusCode = sendRequest<GatewayClientLib::AdsGwSignalStateRequest,
											   uint64_t,
											   GatewayClientLib::AdsGwSignalStateResponse,
											   GatewayClientLib::GwAppSignalState>(GatewayClientLib::AdsGwRequestId::ADSGW_SIGNAL_STATE,
																				   request,
																				   std::span<const uint64_t>(hashes),
																				   response,
																				   std::span<GatewayClientLib::GwAppSignalState>(states),
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
	EXPECT_EQ(adsConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
}

// Test Format Error for ADSGW_SIGNAL_STATE_CHANGES: send excessive payload.
// Expected error code: GWC_REQUEST_FORMAT_ERROR
//
TEST_F(AdsGatewayTests, SignalStateChangesSendExcessivePayload)
{
	class TestAdsGwConnection : public GatewayClientLib::AdsGwConnImpl
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
					GatewayClientLib::AdsGwSignalStateChangesRequest request{};
					uint32_t extraData[32];
				} request{};

				GatewayClientLib::AdsGwSignalStateChangesResponse response{};

				m_lastStatusCode = sendRequest(GatewayClientLib::AdsGwRequestId::ADSGW_SIGNAL_STATE_CHANGES, request, response);
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
	EXPECT_EQ(adsConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
}

// Test Format Error for ADSGW_SIGNAL_STATE_CHANGES: send excessive payload.
// Expected error code: GWC_REQUEST_FORMAT_ERROR
//
TEST_F(AdsGatewayTests, SignalStateChangesSendLessPayload)
{
	class TestAdsGwConnection : public GatewayClientLib::AdsGwConnImpl
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
				GatewayClientLib::AdsGwSignalStateChangesResponse response{};

				m_lastStatusCode = sendRequest(GatewayClientLib::AdsGwRequestId::ADSGW_SIGNAL_STATE_CHANGES, request, response);
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
	EXPECT_EQ(adsConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
}

// Request too big part for ADSGW_SIGNAL_LIST_NEXT:
// Expected error code: GWC_REQUEST_FORMAT_ERROR -- it is not stated directly in the protocol doc but makes sense to have it.
//
TEST_F(AdsGatewayTests, SignalListGetInvalidPart)
{
	class TestAdsGwConnection : public GatewayClientLib::AdsGwConnImpl
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
				GatewayClientLib::AdsGwSignalListStartRequest startReq{};
				GatewayClientLib::AdsGwSignalListStartResponse startResp{};
				auto startStatus = sendRequest(GatewayClientLib::AdsGwRequestId::ADSGW_SIGNAL_LIST_START, startReq, startResp);
				if (startStatus != GatewayClientLib::GwErrorCode::GWC_SUCCESS)
				{
					m_lastStatusCode = startStatus;
					return;
				}

				{
					GatewayClientLib::AdsGwSignalListNextRequest request{};
					GatewayClientLib::AdsGwSignalListNextResponse response{};
					request.part = 1111; // Excessively large part number


					using AppSignalIdNetworkT = std::array<char, GatewayClientLib::STRING_LENGTH_128>;
					std::vector<AppSignalIdNetworkT> responseVariablePartBuffer{};
					responseVariablePartBuffer.resize(startResp.itemsPerPart);
					std::memset(responseVariablePartBuffer.data(), 0, responseVariablePartBuffer.size() * sizeof(AppSignalIdNetworkT));

					m_lastStatusCode = sendRequest(GatewayClientLib::AdsGwRequestId::ADSGW_SIGNAL_LIST_NEXT,
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
	EXPECT_EQ(adsConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
}

// Request too big part for ADSGW_SIGNAL_LIST_NEXT:
// Expected error code: GWC_REQUEST_FORMAT_ERROR -- it is not stated directly in the protocol doc but makes sense to have it.
//
TEST_F(AdsGatewayTests, SignalParamGetInvalidPart)
{
	class TestAdsGwConnection : public GatewayClientLib::AdsGwConnImpl
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
				GatewayClientLib::AdsGwSignalParamStartRequest startReq{};
				GatewayClientLib::AdsGwSignalParamStartResponse startResp{};
				auto startStatus = sendRequest(GatewayClientLib::AdsGwRequestId::ADSGW_SIGNAL_PARAM_START, startReq, startResp);
				if (startStatus != GatewayClientLib::GwErrorCode::GWC_SUCCESS)
				{
					m_lastStatusCode = startStatus;
					return;
				}

				{
					GatewayClientLib::AdsGwSignalParamNextRequest request{};
					GatewayClientLib::AdsGwSignalParamNextResponse response{};
					request.part = 1111; // Excessively large part number

					std::vector<GatewayClientLib::GwAppSignalParam> responseVariablePartBuffer{};
					responseVariablePartBuffer.resize(startResp.itemsPerPart);

					m_lastStatusCode = sendRequest(GatewayClientLib::AdsGwRequestId::ADSGW_SIGNAL_PARAM_NEXT,
												   request,
												   std::span<const std::byte>{},
												   response,
												   std::span<GatewayClientLib::GwAppSignalParam>{responseVariablePartBuffer},
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
	EXPECT_EQ(adsConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
}

// Request several signal state which are not existing
// Expecting: These signals are not returned.
//
TEST_F(AdsGatewayTests, RequestNonexistingSignalStates)
{
	class TestAdsGwConnection : public GatewayClientLib::AdsGwConnImpl
	{
	public:
		using AdsGwConnImpl::AdsGwConnImpl;

		void run(std::stop_token, std::string_view address, uint16_t port, std::string_view equipmentId) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);
				requestHandshake(equipmentId);

				GatewayClientLib::AdsGwSignalStateRequest request{};
				GatewayClientLib::AdsGwSignalStateResponse response{};

				request.signalCount = 3;
				std::array<uint64_t, 3> hashes{555u, 666u, 777u};

				std::vector<GatewayClientLib::GwAppSignalState> states(request.signalCount);

				m_lastStatusCode = sendRequest<GatewayClientLib::AdsGwSignalStateRequest,
											   uint64_t,
											   GatewayClientLib::AdsGwSignalStateResponse,
											   GatewayClientLib::GwAppSignalState>(GatewayClientLib::AdsGwRequestId::ADSGW_SIGNAL_STATE,
																				   request,
																				   std::span<const uint64_t>(hashes),
																				   response,
																				   std::span<GatewayClientLib::GwAppSignalState>(states),
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
	EXPECT_EQ(adsConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_SUCCESS);
	EXPECT_EQ(adsConn.returnedSignalStates, 0);
}

// Request too many signal states
// Expecting: GWC_TOO_MANY_SIGNALS
//
TEST_F(AdsGatewayTests, RequestTooManySignalStates)
{
	class TestAdsGwConnection : public GatewayClientLib::AdsGwConnImpl
	{
	public:
		using AdsGwConnImpl::AdsGwConnImpl;

		void run(std::stop_token, std::string_view address, uint16_t port, std::string_view equipmentId) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);
				requestHandshake(equipmentId);

				GatewayClientLib::AdsGwSignalStateRequest request{};
				GatewayClientLib::AdsGwSignalStateResponse response{};

				request.signalCount = m_handshakeResponse.maxStateRequest + 1; // Exceed maximum
				std::vector<uint64_t> hashes(request.signalCount);
				std::iota(hashes.begin(), hashes.end(), 1000u);

				std::vector<GatewayClientLib::GwAppSignalState> states(request.signalCount);

				m_lastStatusCode = sendRequest<GatewayClientLib::AdsGwSignalStateRequest,
											   uint64_t,
											   GatewayClientLib::AdsGwSignalStateResponse,
											   GatewayClientLib::GwAppSignalState>(GatewayClientLib::AdsGwRequestId::ADSGW_SIGNAL_STATE,
																				   request,
																				   std::span<const uint64_t>(hashes),
																				   response,
																				   std::span<GatewayClientLib::GwAppSignalState>(states),
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
	EXPECT_EQ(adsConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_TOO_MANY_SIGNALS);
}

// Request several signal states.
// Expecting: Normal behavior.
//
TEST_F(AdsGatewayTests, RequestSignalStates)
{
	class TestAdsGwConnection : public GatewayClientLib::AdsGwConnImpl
	{
	public:
		using AdsGwConnImpl::AdsGwConnImpl;

		void run(std::stop_token, std::string_view address, uint16_t port, std::string_view equipmentId) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);
				requestHandshake(equipmentId);

				GatewayClientLib::AdsGwSignalStateRequest request{};
				GatewayClientLib::AdsGwSignalStateResponse response{};

				if (TestSettings::projectSignals.empty() == false)
				{
					projectSignals = TestSettings::projectSignals;
				}
				else
				{
					using namespace std::string_literals;
					const double nan = std::numeric_limits<double>::quiet_NaN();

					// These signals exist in the CI project
					//
					projectSignals.emplace_back("#CT_RT_NOT_0101"s, Radiy::calcHash("#CT_RT_NOT_0101"), nan);
					projectSignals.emplace_back("#CT_RT_ADDFP"s, Radiy::calcHash("#CT_RT_ADDFP"), nan);
					projectSignals.emplace_back("#SYSTEMID_CLIENTTEST_CH10_MD00_PI_BLINK"s,
												Radiy::calcHash("#SYSTEMID_CLIENTTEST_CH10_MD00_PI_BLINK"),
												nan);
					projectSignals.emplace_back("#CLIENTTEST_TUNING_D2"s, Radiy::calcHash("#CLIENTTEST_TUNING_D2"), nan);
				}

				std::vector<Radiy::Hash> hashes;
				for (const auto& s : projectSignals)
				{
					hashes.push_back(s.hash);
				}

				request.signalCount = static_cast<uint32_t>(hashes.size());

				states.resize(request.signalCount);

				m_lastStatusCode = sendRequest<GatewayClientLib::AdsGwSignalStateRequest,
											   Radiy::Hash,
											   GatewayClientLib::AdsGwSignalStateResponse,
											   GatewayClientLib::GwAppSignalState>(GatewayClientLib::AdsGwRequestId::ADSGW_SIGNAL_STATE,
																				   request,
																				   std::span<const Radiy::Hash>(hashes),
																				   response,
																				   std::span<GatewayClientLib::GwAppSignalState>(states),
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

		std::vector<TestSettings::ProjectSignal> projectSignals;
		uint32_t returnedSignalStates = 0;
		std::vector<GatewayClientLib::GwAppSignalState> states;
	};

	TestAdsGwConnection adsConn{signalManager, logger};
	adsConn.run({}, TestSettings::Address, TestSettings::Port, clientEquipmentId);

	ASSERT_TRUE(adsConn.m_connected);
	ASSERT_TRUE(adsConn.lastStatusCode().has_value());
	EXPECT_EQ(adsConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_SUCCESS);

	EXPECT_EQ(adsConn.returnedSignalStates, adsConn.projectSignals.size());

	for (const auto& s : adsConn.projectSignals)
	{
		auto it = std::find_if(adsConn.states.begin(),
							   adsConn.states.end(),
							   [&s](const GatewayClientLib::GwAppSignalState& state)
							   {
								   return state.hash == s.hash;
							   });

		EXPECT_NE(it, adsConn.states.end());

		if (std::isnan(s.expectedValue) == false) // nan - means do not check expected value
		{
			EXPECT_NEAR(it->value, s.expectedValue, 1e-3);
		}
	}
}