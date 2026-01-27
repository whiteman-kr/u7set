#include "TestSettings.hpp"

#include <AdsGatewayLib/../../src/AdsGwConnImpl.hpp>
#include <AdsGatewayLib/Logger.hpp>
#include <AdsGatewayLib/SignalManager.hpp>

#include <gmock/gmock-matchers.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>


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

		virtual void run(std::stop_token /*stoken*/, std::string_view address, uint16_t port, std::string_view /*equipmentId*/) override
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

		virtual void run(std::stop_token /*stoken*/, std::string_view address, uint16_t port, std::string_view equipmentId) override
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

		virtual void run(std::stop_token /*stoken*/, std::string_view address, uint16_t port, std::string_view equipmentId) override
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

		virtual void run(std::stop_token /*stoken*/, std::string_view address, uint16_t port, std::string_view /*equipmentId*/) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);

				AdsGatewayLib::GwHandshakeRequest request{};
				AdsGatewayLib::GwHandshakeResponse response{};

				m_logger.logTrace("Sending handshake request to ADS Gateway...");
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

		virtual void run(std::stop_token /*stoken*/, std::string_view address, uint16_t port, std::string_view /*equipmentId*/) override
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

		virtual void run(std::stop_token /*stoken*/, std::string_view address, uint16_t port, std::string_view /*equipmentId*/) override
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
