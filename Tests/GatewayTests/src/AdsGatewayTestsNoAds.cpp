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


class AdsGatewayTestsNoAds : public testing::Test
{
public:
	GatewayClientLib::AdsSignalManager signalManager{};
	GatewayClientLib::ConsoleLogger logger{};

	std::string clientEquipmentId = "TEST_CLIENT_EQUIPMENT_ID";
};

// Expecting successful connection and handshake, even with no ADS connection established
//
TEST_F(AdsGatewayTestsNoAds, ConnectAndHandshake)
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
	adsConn.run({}, AdsTestSettings::Address, AdsTestSettings::Port, clientEquipmentId);

	ASSERT_EQ(adsConn.m_connected, true);

	ASSERT_TRUE(adsConn.lastStatusCode().has_value());
	EXPECT_EQ(adsConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_SUCCESS);
	EXPECT_EQ(adsConn.handshakeResponse().protocolVersion, GatewayClientLib::ADS_GW_PROTOCOL_VERSION);
	EXPECT_EQ(adsConn.handshakeResponse().sizeof_GwAppSignalParam, sizeof(GatewayClientLib::GwAppSignalParam));
	EXPECT_EQ(adsConn.handshakeResponse().sizeof_GwAppSignalState, sizeof(GatewayClientLib::GwAppSignalState));

	return;
}

// Expecting GWC_NO_ADS_CONNECTION error code when requesting signal states with no ADS connection
// for request ADSGW_SIGNAL_STATE
// Precondition: ADS Gateway server is running but no ADS connection is established
//
TEST_F(AdsGatewayTestsNoAds, RequestSignalStatesWithoutAdsConnection)
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
	adsConn.run({}, AdsTestSettings::Address, AdsTestSettings::Port, clientEquipmentId);

	ASSERT_EQ(adsConn.m_connected, false);
	ASSERT_TRUE(adsConn.lastStatusCode().has_value());
	EXPECT_EQ(adsConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_NO_ADS_CONNECTION);

	return;
}

// Expecting GWC_NO_ADS_CONNECTION error code when requesting signal states with no ADS connection
// for request ADSGW_SIGNAL_STATE_CHANGES
// Precondition: ADS Gateway server is running but no ADS connection is established
//
TEST_F(AdsGatewayTestsNoAds, RequestSignalStateChangesWithoutAdsConnection)
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
	adsConn.run({}, AdsTestSettings::Address, AdsTestSettings::Port, clientEquipmentId);

	ASSERT_EQ(adsConn.m_connected, false);
	ASSERT_TRUE(adsConn.lastStatusCode().has_value());
	EXPECT_EQ(adsConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_NO_ADS_CONNECTION);

	return;
}