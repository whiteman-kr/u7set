#include "TestSettings.hpp"

#include <GatewayClientLib/../../src/TuningGwConnImpl.hpp>
#include <GatewayClientLib/Logger.hpp>
#include <GatewayClientLib/TuningGwProtocol.hpp>
#include <GatewayClientLib/TuningSignalManager.hpp>

#include <gmock/gmock-matchers.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <array>
#include <chrono>
#include <cmath>
#include <numeric>


class TuningGatewayTests : public testing::Test
{
public:
	GatewayClientLib::TuningSignalManager signalManager{};
	GatewayClientLib::ConsoleLogger logger{};

	std::string clientEquipmentId = "TEST_CLIENT_EQUIPMENT_ID";
};

namespace
{
	class TestTuningGwConnectionAccessor : public GatewayClientLib::TuningGwConnImpl
	{
	public:
		using TuningGwConnImpl::TuningGwConnImpl;
		using TuningGwConnImpl::requestActivateTuningSource;
		using TuningGwConnImpl::requestHandshake;
		using TuningGwConnImpl::requestSignalStates;
		using TuningGwConnImpl::requestTuningSourceStates;
		using TuningGwConnImpl::requestTuningSources;

		bool connect(std::string_view address, uint16_t port)
		{
			m_connected = m_conn.connect(address, port);
			return m_connected;
		}

		template<typename RequestT, typename ResponseT>
		GatewayClientLib::GwErrorCode sendRawRequest(GatewayClientLib::TuningGwRequestId requestId,
													 const RequestT& request,
													 ResponseT& response)
		{
			return this->sendRequest(requestId, request, response, {});
		}

		template<typename RequestT, typename RequestVariablePartT, typename ResponseT, typename ResponseVariablePartT>
		GatewayClientLib::GwErrorCode sendRawRequest(GatewayClientLib::TuningGwRequestId requestId,
													 const RequestT& request,
													 std::span<const RequestVariablePartT> requestVariablePart,
													 ResponseT& response,
													 std::span<ResponseVariablePartT> responseVariablePart)
		{
			return this->sendRequest(requestId, request, requestVariablePart, response, responseVariablePart, {});
		}

		auto& lastStatusCode() const { return m_lastStatusCode; }
		auto& handshakeResponse() const { return m_workset.handshakeResponse; }
		const auto& workset() const { return m_workset; }

		bool m_connected = false;
	};

	std::vector<Radiy::Hash> gptKnownTuningSignalHashes()
	{
		return {Radiy::calcHash("#CLIENTTEST_TUNING_SAFE_D1"), Radiy::calcHash("#TGW_D1")};
	}

	std::array<std::string_view, 3> gptKnownTuningSourceModuleIds()
	{
		return {"SYSTEMID_CLIENTTEST_CH12_MD00", "SYSTEMID_CLIENTTEST_CH13_MD00", "SYSTEMID_CLIENTTEST_CH14_MD00"};
	}
} // namespace

// Test that connection fails when no server is available
//
TEST_F(TuningGatewayTests, NoConnection)
{
	class TestTuningGwConnection : public GatewayClientLib::TuningGwConnImpl
	{
	public:
		using TuningGwConnImpl::TuningGwConnImpl;

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

	TestTuningGwConnection tuningConn{signalManager, logger};
	tuningConn.run({}, "127.0.0.1", 3551, "EQUIPMENTID");

	ASSERT_EQ(tuningConn.m_connected, false);

	return;
}

// Expecting successful connection and handshake
//
TEST_F(TuningGatewayTests, ConnectAndHandshake)
{
	class TestTuningGwConnection : public GatewayClientLib::TuningGwConnImpl
	{
	public:
		using TuningGwConnImpl::TuningGwConnImpl;

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
		auto& handshakeResponse() const { return m_workset.handshakeResponse; }
	};

	TestTuningGwConnection tuningConn{signalManager, logger};
	tuningConn.run({}, TuningTestSettings::Address, TuningTestSettings::Port, clientEquipmentId);
	ASSERT_TRUE(tuningConn.m_connected);

	ASSERT_TRUE(tuningConn.lastStatusCode().has_value());
	EXPECT_EQ(tuningConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_SUCCESS);
	EXPECT_EQ(tuningConn.handshakeResponse().protocolVersion, GatewayClientLib::TUNING_GW_PROTOCOL_VERSION);
	EXPECT_EQ(tuningConn.handshakeResponse().sizeof_GwTuningSourceState, sizeof(GatewayClientLib::GwTuningSourceState));
	EXPECT_EQ(tuningConn.handshakeResponse().sizeof_GwTuningSignalState, sizeof(GatewayClientLib::GwTuningSignalState));

	return;
}

// Send unsupported protocol version in handshake request
// Expecting GWC_UNSUPPORTED_VERSION error code in response
// 3.1
TEST_F(TuningGatewayTests, SendUnsupportedProtocolVersion)
{
	class TestTuningGwConnection : public GatewayClientLib::TuningGwConnImpl
	{
	public:
		using TuningGwConnImpl::TuningGwConnImpl;

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
		auto& handshakeResponse() const { return m_workset.handshakeResponse; }
	};

	TestTuningGwConnection tuningConn{signalManager, logger};
	tuningConn.run({}, TuningTestSettings::Address, TuningTestSettings::Port, clientEquipmentId);
	ASSERT_TRUE(tuningConn.m_connected);

	ASSERT_TRUE(tuningConn.lastStatusCode().has_value());
	EXPECT_EQ(tuningConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_UNSUPPORTED_VERSION);

	return;
}

// Send unknown RequestID
// Expected: GWC_INVALID_REQUEST
//
TEST_F(TuningGatewayTests, SendInvalidRequest)
{
	class TestGwConnection : public GatewayClientLib::TuningGwConnImpl
	{
	public:
		using TuningGwConnImpl::TuningGwConnImpl;

		virtual void run(std::stop_token, std::string_view address, uint16_t port, std::string_view /*equipmentId*/) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);

				GatewayClientLib::TuningGwHandshakeRequest request{};
				GatewayClientLib::TuningGwHandshakeResponse response{};

				sendRequest(static_cast<GatewayClientLib::TuningGwRequestId>(199999), request, response, {});
			}
			catch (const std::runtime_error&)
			{
			}
		}

	public:
		bool m_connected = false;

		auto& lastStatusCode() const { return m_lastStatusCode; }
		auto& handshakeResponse() const { return m_workset.handshakeResponse; }
	};

	TestGwConnection tuningConn{signalManager, logger};
	tuningConn.run({}, TuningTestSettings::Address, TuningTestSettings::Port, clientEquipmentId);

	ASSERT_TRUE(tuningConn.m_connected);

	ASSERT_TRUE(tuningConn.lastStatusCode().has_value());
	EXPECT_EQ(tuningConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_INVALID_REQUEST);

	return;
}

// Send invalid CRC in request
// Expected: GWC_CRC_ERROR
//
TEST_F(TuningGatewayTests, SendInvalidCrc32)
{
	class TestGwConnection : public GatewayClientLib::TuningGwConnImpl
	{
	public:
		using TuningGwConnImpl::TuningGwConnImpl;

		virtual void run(std::stop_token, std::string_view address, uint16_t port, std::string_view equipmentId) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);

				// Compose handshake request with invalid CRC32
				//
				GatewayClientLib::TuningGwHandshakeRequest request{};
				GatewayClientLib::TuningGwHandshakeResponse response{};

				request.protocolVersion = GatewayClientLib::TUNING_GW_PROTOCOL_VERSION;
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

				writeUint32(static_cast<uint32_t>(GatewayClientLib::TuningGwRequestId::TGW_HANDSHAKE));
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
					receiveResponsePacket<GatewayClientLib::TuningGwHandshakeResponse>(GatewayClientLib::TuningGwRequestId::TGW_HANDSHAKE,
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
		auto& handshakeResponse() const { return m_workset.handshakeResponse; }
	};

	TestGwConnection tuningConn{signalManager, logger};
	tuningConn.run({}, TuningTestSettings::Address, TuningTestSettings::Port, clientEquipmentId);

	ASSERT_EQ(tuningConn.m_connected, true);

	ASSERT_TRUE(tuningConn.lastStatusCode().has_value());
	EXPECT_EQ(tuningConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_CRC_ERROR);

	return;
}

// Expecting GWC_HANDSHAKE_REQUIRED error code when requesting signal list without handshake
// TGW_GET_TUNING_SOURCES_START/TGW_GET_TUNING_SOURCES_NEXT
//
TEST_F(TuningGatewayTests, RequestTuningSourceWithoutHandshake)
{
	class TestAdsGwConnection : public GatewayClientLib::TuningGwConnImpl
	{
	public:
		using TuningGwConnImpl::TuningGwConnImpl;

		virtual void run(std::stop_token, std::string_view address, uint16_t port, std::string_view /*equipmentId*/) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);
				requestTuningSources();
			}
			catch (const std::runtime_error&)
			{
				// Exceptions are expected
			}
		}

	public:
		bool m_connected = false;

		auto& lastStatusCode() const { return m_lastStatusCode; }
		auto& handshakeResponse() const { return m_workset.handshakeResponse; }
	};

	TestAdsGwConnection tuningConn{signalManager, logger};
	tuningConn.run({}, TuningTestSettings::Address, TuningTestSettings::Port, clientEquipmentId);

	ASSERT_TRUE(tuningConn.m_connected);

	ASSERT_TRUE(tuningConn.lastStatusCode().has_value());
	EXPECT_EQ(tuningConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_HANDSHAKE_REQUIRED);

	return;
}

// Expecting GWC_HANDSHAKE_REQUIRED error code when requesting source states without handshake
// TGW_GET_TUNING_SOURCE_STATES
//
TEST_F(TuningGatewayTests, RequestSignalStatesWithoutHandshake)
{
	class TestAdsGwConnection : public GatewayClientLib::TuningGwConnImpl
	{
	public:
		using TuningGwConnImpl::TuningGwConnImpl;

		virtual void run(std::stop_token, std::string_view address, uint16_t port, std::string_view /*equipmentId*/) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);

				m_workset.handshakeResponse.maxStateRequest =
					100; // It is used in getting states, but we did not do handshake, so set it manually.

				auto hashes = {Radiy::Hash{123}, Radiy::Hash{456}};
				std::ignore = requestSignalStates(hashes);
			}
			catch (const std::runtime_error&)
			{
			}
		}

	public:
		bool m_connected = false;

		auto& lastStatusCode() const { return m_lastStatusCode; }
		auto& handshakeResponse() const { return m_workset.handshakeResponse; }
	};

	TestAdsGwConnection tuningConn{signalManager, logger};


	tuningConn.run({}, TuningTestSettings::Address, TuningTestSettings::Port, clientEquipmentId);

	ASSERT_EQ(tuningConn.m_connected, true);
	ASSERT_TRUE(tuningConn.lastStatusCode().has_value());
	EXPECT_EQ(tuningConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_HANDSHAKE_REQUIRED);

	return;
}

// Expecting GWC_HANDSHAKE_REQUIRED error code when requesting TGW_GET_TUNING_SOURCE_STATES without handshake
//
TEST_F(TuningGatewayTests, RequestSignalStateChangesWithoutHandshake)
{
	class TestAdsGwConnection : public GatewayClientLib::TuningGwConnImpl
	{
	public:
		using TuningGwConnImpl::TuningGwConnImpl;

		virtual void run(std::stop_token, std::string_view address, uint16_t port, std::string_view /*equipmentId*/) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);
				requestTuningSourceStates();
			}
			catch (const std::runtime_error&)
			{
			}
		}

	public:
		bool m_connected = false;

		auto& lastStatusCode() const { return m_lastStatusCode; }
		auto& handshakeResponse() const { return m_workset.handshakeResponse; }
	};

	TestAdsGwConnection tuningConn{signalManager, logger};
	tuningConn.run({}, TuningTestSettings::Address, TuningTestSettings::Port, clientEquipmentId);

	ASSERT_EQ(tuningConn.m_connected, true);
	ASSERT_TRUE(tuningConn.lastStatusCode().has_value());
	EXPECT_EQ(tuningConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_HANDSHAKE_REQUIRED);

	return;
}

// Expecting GWC_HANDSHAKE_REQUIRED error code when requesting TGW_CHANGE_CONTROLLED_TUNING_SOURCE without handshake
//
TEST_F(TuningGatewayTests, RequestActivateSourceWithoutHandshake)
{
	class TestAdsGwConnection : public GatewayClientLib::TuningGwConnImpl
	{
	public:
		using TuningGwConnImpl::TuningGwConnImpl;

		virtual void run(std::stop_token, std::string_view address, uint16_t port, std::string_view /*equipmentId*/) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);
				auto err = requestActivateTuningSource("ABC", true);
				m_lastStatusCode = err;
			}
			catch (const std::runtime_error&)
			{
				// Exception is not expected to command requests.
				//
				ASSERT_TRUE(false);
			}
		}

	public:
		bool m_connected = false;

		auto& lastStatusCode() const { return m_lastStatusCode; }
		auto& handshakeResponse() const { return m_workset.handshakeResponse; }
	};

	TestAdsGwConnection tuningConn{signalManager, logger};
	tuningConn.run({}, TuningTestSettings::Address, TuningTestSettings::Port, clientEquipmentId);

	ASSERT_EQ(tuningConn.m_connected, true);
	ASSERT_TRUE(tuningConn.lastStatusCode().has_value());
	EXPECT_EQ(tuningConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_HANDSHAKE_REQUIRED);

	return;
}

// Expecting GWC_HANDSHAKE_REQUIRED error code when requesting TGW_TUNING_SIGNALS_WRITE without handshake
//
TEST_F(TuningGatewayTests, RequestWriteWithoutHandshake)
{
	class TestAdsGwConnection : public GatewayClientLib::TuningGwConnImpl
	{
	public:
		using TuningGwConnImpl::TuningGwConnImpl;

		virtual void run(std::stop_token, std::string_view address, uint16_t port, std::string_view /*equipmentId*/) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);

				std::array<GatewayClientLib::GwTuningWriteValue, 2> states = {GatewayClientLib::GwTuningWriteValue{},
																			  GatewayClientLib::GwTuningWriteValue{}};

				auto err = requestWriteSignalValues(states, "Vasiliy", true);
				m_lastStatusCode = err;
			}
			catch (const std::runtime_error&)
			{
				// Exception is not expected to command requests.
				//
				ASSERT_TRUE(false);
			}
		}

	public:
		bool m_connected = false;

		auto& lastStatusCode() const { return m_lastStatusCode; }
		auto& handshakeResponse() const { return m_workset.handshakeResponse; }
	};

	TestAdsGwConnection tuningConn{signalManager, logger};
	tuningConn.run({}, TuningTestSettings::Address, TuningTestSettings::Port, clientEquipmentId);

	ASSERT_EQ(tuningConn.m_connected, true);
	ASSERT_TRUE(tuningConn.lastStatusCode().has_value());
	EXPECT_EQ(tuningConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_HANDSHAKE_REQUIRED);

	return;
}

// Expecting GWC_HANDSHAKE_REQUIRED error code when requesting TGW_TUNING_SIGNALS_APPLY without handshake
//
TEST_F(TuningGatewayTests, RequestApplyWithoutHandshake)
{
	class TestAdsGwConnection : public GatewayClientLib::TuningGwConnImpl
	{
	public:
		using TuningGwConnImpl::TuningGwConnImpl;

		virtual void run(std::stop_token, std::string_view address, uint16_t port, std::string_view /*equipmentId*/) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);

				auto err = requestApplyWrittenSignals();
				m_lastStatusCode = err;
			}
			catch (const std::runtime_error&)
			{
				// Exception is not expected to command requests.
				//
				ASSERT_TRUE(false);
			}
		}

	public:
		bool m_connected = false;

		auto& lastStatusCode() const { return m_lastStatusCode; }
		auto& handshakeResponse() const { return m_workset.handshakeResponse; }
	};

	TestAdsGwConnection tuningConn{signalManager, logger};
	tuningConn.run({}, TuningTestSettings::Address, TuningTestSettings::Port, clientEquipmentId);

	ASSERT_EQ(tuningConn.m_connected, true);
	ASSERT_TRUE(tuningConn.lastStatusCode().has_value());
	EXPECT_EQ(tuningConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_HANDSHAKE_REQUIRED);

	return;
}

// Request TuningSources.xml (TGW_GET_TUNING_SOURCES_START/TGW_GET_TUNING_SOURCES_NEXT)
//
TEST_F(TuningGatewayTests, RequestTuningSources)
{
	class TestAdsGwConnection : public GatewayClientLib::TuningGwConnImpl
	{
	public:
		using TuningGwConnImpl::TuningGwConnImpl;

		virtual void run(std::stop_token, std::string_view address, uint16_t port, std::string_view equipmentId) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);
				requestHandshake(equipmentId);

				requestTuningSources();
			}
			catch (const std::runtime_error& e)
			{
				std::cout << "Exception: " << e.what() << std::endl;
				ASSERT_TRUE(false);
			}
		}

	public:
		bool m_connected = false;
		std::vector<std::string> m_receivedSignalList;

		auto& lastStatusCode() const { return m_lastStatusCode; }
		auto& handshakeResponse() const { return m_workset.handshakeResponse; }

		const auto& workset() { return m_workset; }
	};

	TestAdsGwConnection tuningConn{signalManager, logger};
	tuningConn.run({}, TuningTestSettings::Address, TuningTestSettings::Port, clientEquipmentId);

	EXPECT_EQ(tuningConn.m_connected, true);
	ASSERT_TRUE(tuningConn.lastStatusCode().has_value());
	EXPECT_EQ(tuningConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_SUCCESS);

	// --
	ASSERT_FALSE(tuningConn.workset().project.name.empty());
	ASSERT_FALSE(tuningConn.workset().project.buildDate.empty());
	ASSERT_GT(tuningConn.workset().project.buildNo, 10);
	ASSERT_FALSE(tuningConn.workset().project.name.empty());

	ASSERT_EQ(tuningConn.workset().tuningSources.size(), 3);

	const auto& source = tuningConn.workset().tuningSources[0];
	ASSERT_FALSE(source.signalIds.empty());
	ASSERT_FALSE(source.signals.empty());
	ASSERT_EQ(source.signals.size(), source.signalIds.size());

	// Verify that known signal hashes are present in the received signals
	//

	// Check signal:
	//
	{
		// <Signal ID="2875" AppSignalID="#TEST_TUNING_LIMITS_INT32" CustomAppSignalID="TEST_TUNING_LIMITS_INT32" Caption="App signal
		// #TEST_TUNING_LIMITS_INT32 in schema SYSTEMID_CLIENTTEST_CH12_MD00" EquipmentID="SYSTEMID_CLIENTTEST_CH12_MD00" Channel="A"
		// ChannelVal="0" SignalGroupID="0" SignalInstanceID="3045" Type="Analog" TypeVal="0" InOutType="Internal" InOutTypeVal="2"
		// ByteOrder="BigEndian" ByteOrderVal="1" DataSize="32" AnalogSignalFormat="SignedInt32" AnalogSignalFormatVal="1" BusTypeID=""
		// InvertSignal="false" Acquire="true" Archive="true" Log="false" Reserved="false" ApertureType="RangePercent" ApertureTypeVal="0"
		// FineAperture="0.5" CoarseAperture="1" Unit="mm" DecimalPlaces="3" Tags="" UalAddr="46336:0" RegValueAddr="0:0"
		// RegValidityAddr="-1:-1" EnableTuning="true" TuningValueType="1" TuningValueTypeStr="SignedInt32" TuningDefaultValue="99999"
		// TuningLowBound="-2147483648" TuningHighBound="2147483647" TuningDefaultValueHex="0x0001869F" TuningLowBoundHex="0x80000000"
		// TuningHighBoundHex="0x7FFFFFFF" TuningAddr="0:0" TuningAbsAddr="0:0" HighEngineeringUnits="100" LowEngineeringUnits="0"/>
		//
		// SignedInt32
		//
		ASSERT_TRUE(source.signals.contains(Radiy::calcHash("#TEST_TUNING_LIMITS_INT32")));

		const auto& sp = source.signals.at(Radiy::calcHash("#TEST_TUNING_LIMITS_INT32"));

		EXPECT_EQ(sp.hash, Radiy::calcHash("#TEST_TUNING_LIMITS_INT32"));
		EXPECT_STREQ(sp.appSignalId, "#TEST_TUNING_LIMITS_INT32");
		EXPECT_STREQ(sp.customSignalId, "TEST_TUNING_LIMITS_INT32");

		EXPECT_EQ(static_cast<int32_t>(sp.tuningDefaultValue), 99999);
		EXPECT_EQ(static_cast<int32_t>(sp.tuningLowBound), -2147483648);
		EXPECT_EQ(static_cast<int32_t>(sp.tuningHighBound), 2147483647);
	}

	// Check signal:
	//
	{
		// <Signal ID="2947" AppSignalID="#TEST_TUNING_LIMITS_FP32" CustomAppSignalID="TEST_TUNING_LIMITS_FP32" Caption="App signal
		// #TEST_TUNING_LIMITS_FP32 in schema SYSTEMID_CLIENTTEST_CH12_MD00" EquipmentID="SYSTEMID_CLIENTTEST_CH12_MD00" Channel="A"
		// ChannelVal="0" SignalGroupID="0" SignalInstanceID="3117" Type="Analog" TypeVal="0" InOutType="Internal" InOutTypeVal="2"
		// ByteOrder="BigEndian" ByteOrderVal="1" DataSize="32" AnalogSignalFormat="Float32" AnalogSignalFormatVal="2" BusTypeID=""
		// InvertSignal="false" Acquire="true" Archive="true" Log="false" Reserved="false" ApertureType="RangePercent" ApertureTypeVal="0"
		// FineAperture="0.5" CoarseAperture="1" Unit="mm" DecimalPlaces="3" Tags="" UalAddr="46336:0" RegValueAddr="0:0"
		// RegValidityAddr="-1:-1" EnableTuning="true" TuningValueType="3" TuningValueTypeStr="Float" TuningDefaultValue="999"
		// TuningLowBound="-10345.5" TuningHighBound="18345.5" TuningDefaultValueHex="0x4479C000" TuningLowBoundHex="0xC621A600"
		// TuningHighBoundHex="0x468F5300" TuningAddr="0:0" TuningAbsAddr="0:0" HighEngineeringUnits="100" LowEngineeringUnits="0"/>
		//
		// Float32
		//
		ASSERT_TRUE(source.signals.contains(Radiy::calcHash("#TEST_TUNING_LIMITS_FP32")));

		const auto& sp = source.signals.at(Radiy::calcHash("#TEST_TUNING_LIMITS_FP32"));

		EXPECT_EQ(sp.hash, Radiy::calcHash("#TEST_TUNING_LIMITS_FP32"));
		EXPECT_STREQ(sp.appSignalId, "#TEST_TUNING_LIMITS_FP32");
		EXPECT_STREQ(sp.customSignalId, "TEST_TUNING_LIMITS_FP32");

		EXPECT_EQ(sp.tuningDefaultValue, 999.0);
		EXPECT_EQ(sp.tuningLowBound, -10345.5);
		EXPECT_EQ(sp.tuningHighBound, 18345.5);
	}


	return;
}

// Test Format Error for Handshake: Send excessive payload.
// Expected error code: GWC_REQUEST_FORMAT_ERROR
//
TEST_F(TuningGatewayTests, HandshakeSendExcessivePayload)
{
	class TestAdsGwConnection : public GatewayClientLib::TuningGwConnImpl
	{
	public:
		using TuningGwConnImpl::TuningGwConnImpl;

		virtual void run(std::stop_token, std::string_view address, uint16_t port, std::string_view /*equipmentId*/) override
		{
			try
			{
				m_connected = m_conn.connect(address, port);

				GatewayClientLib::TuningGwHandshakeRequest request{};
				GatewayClientLib::TuningGwHandshakeResponse response{};

				std::array<std::byte, 32> payload{};

				m_lastStatusCode = sendRequest<GatewayClientLib::TuningGwHandshakeRequest,
											   std::byte,
											   GatewayClientLib::TuningGwHandshakeResponse,
											   std::byte>(GatewayClientLib::TuningGwRequestId::TGW_HANDSHAKE,
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
		auto& handshakeResponse() const { return m_workset.handshakeResponse; }
	};

	TestAdsGwConnection tuningConn{signalManager, logger};
	tuningConn.run({}, TuningTestSettings::Address, TuningTestSettings::Port, clientEquipmentId);

	ASSERT_EQ(tuningConn.m_connected, true);

	ASSERT_TRUE(tuningConn.lastStatusCode().has_value());
	EXPECT_EQ(tuningConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);

	return;
}

// Test Format Error for Handshake: Send not less payload then required
// Expected error code: GWC_REQUEST_FORMAT_ERROR
//
TEST_F(TuningGatewayTests, HandshakeSendLessPayload)
{
	class TestAdsGwConnection : public GatewayClientLib::TuningGwConnImpl
	{
	public:
		using TuningGwConnImpl::TuningGwConnImpl;

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

				m_lastStatusCode = sendRequest(GatewayClientLib::TuningGwRequestId::TGW_HANDSHAKE, request, response);
			}
			catch (const std::runtime_error&)
			{
			}
		}

	public:
		bool m_connected = false;

		auto& lastStatusCode() const { return m_lastStatusCode; }
		auto& handshakeResponse() const { return m_workset.handshakeResponse; }
	};

	TestAdsGwConnection tuningConn{signalManager, logger};
	tuningConn.run({}, TuningTestSettings::Address, TuningTestSettings::Port, clientEquipmentId);

	ASSERT_EQ(tuningConn.m_connected, true);

	ASSERT_TRUE(tuningConn.lastStatusCode().has_value());
	EXPECT_EQ(tuningConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);

	return;
}

//
// GPT Tests
//
TEST_F(TuningGatewayTests, GptTuningSourcesStartReturnsConsistentMetadata)
{
	TestTuningGwConnectionAccessor tuningConn{signalManager, logger};

	ASSERT_TRUE(tuningConn.connect(TuningTestSettings::Address, TuningTestSettings::Port));
	ASSERT_NO_THROW(tuningConn.requestHandshake(clientEquipmentId));

	GatewayClientLib::GwGetTuningSourcesStartRequest request{};
	GatewayClientLib::GwGetTuningSourcesStartResponse response{};
	GatewayClientLib::GwErrorCode status{};

	for (int i = 0; i < 10; i++)
	{
		status = tuningConn.sendRawRequest(GatewayClientLib::TuningGwRequestId::TGW_GET_TUNING_SOURCES_START, request, response);
		if (status == GatewayClientLib::GwErrorCode::GWC_TUNING_SOURCES_FILE_NOT_READY)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}

		break;
	}

	ASSERT_EQ(status, GatewayClientLib::GwErrorCode::GWC_SUCCESS);
	EXPECT_GT(response.totalSize, 0u);
	EXPECT_GT(response.maxPartSize, 0u);
	EXPECT_GT(response.partCount, 0u);
	EXPECT_EQ(response.partCount, (response.totalSize + response.maxPartSize - 1) / response.maxPartSize);
	EXPECT_LE(response.totalSize, response.partCount * response.maxPartSize);
}

TEST_F(TuningGatewayTests, GptTuningSourceStatesResponseCountMatchesParsedSourceCount)
{
	TestTuningGwConnectionAccessor tuningConn{signalManager, logger};

	ASSERT_TRUE(tuningConn.connect(TuningTestSettings::Address, TuningTestSettings::Port));
	ASSERT_NO_THROW(tuningConn.requestHandshake(clientEquipmentId));
	ASSERT_NO_THROW(tuningConn.requestTuningSources());
	ASSERT_NO_THROW(tuningConn.requestTuningSourceStates());

	const auto sourceStates = tuningConn.tuningSources();
	EXPECT_EQ(sourceStates.size(), tuningConn.workset().tuningSources.size());
}

TEST_F(TuningGatewayTests, GptTuningSourceStatesModuleEquipmentIdMatchesTuningSourcesXml)
{
	TestTuningGwConnectionAccessor tuningConn{signalManager, logger};

	ASSERT_TRUE(tuningConn.connect(TuningTestSettings::Address, TuningTestSettings::Port));
	ASSERT_NO_THROW(tuningConn.requestHandshake(clientEquipmentId));

	ASSERT_NO_THROW(tuningConn.requestTuningSources());
	ASSERT_NO_THROW(tuningConn.requestTuningSourceStates());

	const auto sourceStates = tuningConn.tuningSources();
	ASSERT_EQ(sourceStates.size(), tuningConn.workset().tuningSources.size());

	for (size_t i = 0; i < sourceStates.size(); ++i)
	{
		EXPECT_EQ(std::string{sourceStates[i].moduleEquipmentId}, tuningConn.workset().tuningSources[i].moduleEquipmentId);
	}
}


TEST_F(TuningGatewayTests, GptReadSignalStatesReturnsKnownHashesInRequestOrder)
{
	TestTuningGwConnectionAccessor tuningConn{signalManager, logger};

	ASSERT_TRUE(tuningConn.connect(TuningTestSettings::Address, TuningTestSettings::Port));
	ASSERT_NO_THROW(tuningConn.requestHandshake(clientEquipmentId));

	const std::vector<Radiy::Hash> hashes{Radiy::calcHash("#TGW_D1"), Radiy::calcHash("#CLIENTTEST_TUNING_SAFE_D1")};
	const auto states = tuningConn.requestSignalStates(hashes);

	ASSERT_EQ(states.size(), hashes.size());
	for (size_t i = 0; i < hashes.size(); ++i)
	{
		EXPECT_EQ(states[i].hash, hashes[i]);
	}
}


TEST_F(TuningGatewayTests, GptReadSignalStatesDuplicateHashesPreserveDuplicates)
{
	TestTuningGwConnectionAccessor tuningConn{signalManager, logger};

	ASSERT_TRUE(tuningConn.connect(TuningTestSettings::Address, TuningTestSettings::Port));
	ASSERT_NO_THROW(tuningConn.requestHandshake(clientEquipmentId));

	const Radiy::Hash duplicateHash = Radiy::calcHash("#TGW_D1");
	const std::vector<Radiy::Hash> hashes{duplicateHash, duplicateHash, duplicateHash};
	const auto states = tuningConn.requestSignalStates(hashes);

	ASSERT_EQ(states.size(), hashes.size());
	for (const auto& state : states)
	{
		EXPECT_EQ(state.hash, duplicateHash);
	}
}


TEST_F(TuningGatewayTests, GptReadSignalStatesSupportsExactlyMaxStateRequest)
{
	TestTuningGwConnectionAccessor tuningConn{signalManager, logger};

	ASSERT_TRUE(tuningConn.connect(TuningTestSettings::Address, TuningTestSettings::Port));
	ASSERT_NO_THROW(tuningConn.requestHandshake(clientEquipmentId));

	const auto knownHashes = gptKnownTuningSignalHashes();
	std::vector<Radiy::Hash> hashes;
	hashes.reserve(tuningConn.handshakeResponse().maxStateRequest);

	for (uint32_t i = 0; i < tuningConn.handshakeResponse().maxStateRequest; ++i)
	{
		hashes.push_back(knownHashes[i % knownHashes.size()]);
	}

	const auto states = tuningConn.requestSignalStates(hashes);

	ASSERT_EQ(states.size(), hashes.size());
	for (size_t i = 0; i < hashes.size(); ++i)
	{
		EXPECT_EQ(states[i].hash, hashes[i]);
	}
}


TEST_F(TuningGatewayTests, GptReadSignalStatesOneKnownSignal)
{
	TestTuningGwConnectionAccessor tuningConn{signalManager, logger};

	ASSERT_TRUE(tuningConn.connect(TuningTestSettings::Address, TuningTestSettings::Port));
	ASSERT_NO_THROW(tuningConn.requestHandshake(clientEquipmentId));

	const std::vector<Radiy::Hash> hashes{Radiy::calcHash("#TGW_D1")};
	const auto states = tuningConn.requestSignalStates(hashes);

	ASSERT_EQ(states.size(), 1u);
	EXPECT_EQ(states[0].hash, hashes[0]);
	// Commented as out error can be GWC_LM_CONTROL_IS_NOT_ACTIVE
	// EXPECT_EQ(states[0].errorCode, static_cast<uint32_t>(GatewayClientLib::GwErrorCode::GWC_SUCCESS));
	EXPECT_TRUE(std::isfinite(states[0].value));
}


TEST_F(TuningGatewayTests, GptReadSignalStatesSkipsUnknownHashes)
{
	TestTuningGwConnectionAccessor tuningConn{signalManager, logger};

	ASSERT_TRUE(tuningConn.connect(TuningTestSettings::Address, TuningTestSettings::Port));
	ASSERT_NO_THROW(tuningConn.requestHandshake(clientEquipmentId));

	auto hashes = gptKnownTuningSignalHashes();
	hashes.insert(hashes.begin() + 1, Radiy::calcHash("#GPT_UNKNOWN_SIGNAL"));

	auto states = tuningConn.requestSignalStates(hashes);

	ASSERT_EQ(states.size(), 3u);

	EXPECT_EQ(states[0].hash, Radiy::calcHash("#CLIENTTEST_TUNING_SAFE_D1"));
	EXPECT_TRUE(states[0].errorCode == static_cast<uint32_t>(GatewayClientLib::GwErrorCode::GWC_SUCCESS) ||
				states[0].errorCode == static_cast<uint32_t>(GatewayClientLib::GwErrorCode::GWC_LM_CONTROL_IS_NOT_ACTIVE));

	EXPECT_EQ(states[1].hash, Radiy::calcHash("#GPT_UNKNOWN_SIGNAL"));
	EXPECT_EQ(states[1].errorCode, static_cast<uint32_t>(GatewayClientLib::GwErrorCode::GWC_UNKNOWN_SIGNAL_HASH));

	EXPECT_EQ(states[2].hash, Radiy::calcHash("#TGW_D1"));
	EXPECT_TRUE(states[2].errorCode == static_cast<uint32_t>(GatewayClientLib::GwErrorCode::GWC_SUCCESS) ||
				states[2].errorCode == static_cast<uint32_t>(GatewayClientLib::GwErrorCode::GWC_LM_CONTROL_IS_NOT_ACTIVE));
}

TEST_F(TuningGatewayTests, GptReadSignalStatesRejectsTooManySignals)
{
	TestTuningGwConnectionAccessor tuningConn{signalManager, logger};

	ASSERT_TRUE(tuningConn.connect(TuningTestSettings::Address, TuningTestSettings::Port));
	ASSERT_NO_THROW(tuningConn.requestHandshake(clientEquipmentId));

	GatewayClientLib::GwTuningSignalsReadRequest request{};
	request.count = tuningConn.handshakeResponse().maxStateRequest + 1;

	auto knownHashes = gptKnownTuningSignalHashes();
	std::vector<Radiy::Hash> hashes(request.count, knownHashes.front());
	std::vector<GatewayClientLib::GwTuningSignalState> states(request.count);
	GatewayClientLib::GwTuningSignalsReadResponse response{};

	auto status = tuningConn.sendRawRequest(GatewayClientLib::TuningGwRequestId::TGW_TUNING_SIGNALS_READ,
											request,
											std::span<const Radiy::Hash>{hashes},
											response,
											std::span<GatewayClientLib::GwTuningSignalState>{states});

	EXPECT_EQ(status, GatewayClientLib::GwErrorCode::GWC_TOO_MANY_SIGNALS);
}

TEST_F(TuningGatewayTests, GptChangeControlledTuningSourceAcceptsKnownModuleIdAndEchoesActivatedState)
{
	TestTuningGwConnectionAccessor tuningConn{signalManager, logger};

	ASSERT_TRUE(tuningConn.connect(TuningTestSettings::Address, TuningTestSettings::Port));
	ASSERT_NO_THROW(tuningConn.requestHandshake(clientEquipmentId));
	ASSERT_NO_THROW(tuningConn.requestTuningSources());

	GatewayClientLib::GwChangeControlledTuningSourceRequest request{};
	GatewayClientLib::GwChangeControlledTuningSourceResponse response{};

	constexpr std::string_view moduleEquipmentId = "SYSTEMID_CLIENTTEST_CH12_MD00";
	std::snprintf(request.moduleEquipmentId, sizeof(request.moduleEquipmentId), "%s", moduleEquipmentId.data());
	request.activateControl = 1;

	auto status = tuningConn.sendRawRequest(GatewayClientLib::TuningGwRequestId::TGW_CHANGE_CONTROLLED_TUNING_SOURCE, request, response);

	ASSERT_EQ(status, GatewayClientLib::GwErrorCode::GWC_SUCCESS);
	EXPECT_STREQ(response.controlledModuleEquipmentId, moduleEquipmentId.data());
	EXPECT_EQ(response.controlIsActive, 1u);
	EXPECT_EQ(response.reserved[0], 0u);
	EXPECT_EQ(response.reserved[1], 0u);
	EXPECT_EQ(response.reserved[2], 0u);

	// It takes time to activate source, so wait a bit
	bool passed = false;
	for (int i = 0; i < 20; i++)
	{
		ASSERT_NO_THROW(tuningConn.requestTuningSourceStates());

		const auto sourceStates = tuningConn.tuningSources();
		ASSERT_EQ(sourceStates.size(), gptKnownTuningSourceModuleIds().size());

		for (const auto& sourceState : sourceStates)
		{
			if (std::string_view{sourceState.moduleEquipmentId} == moduleEquipmentId)
			{
				if (sourceState.controlIsActive == 1u)
				{
					passed = true;
				}
			}
			else
			{
				EXPECT_EQ(sourceState.controlIsActive, 0u);
			}
		}

		if (passed == true)
		{
			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds{200});
	}

	EXPECT_TRUE(passed);

	return;
}


TEST_F(TuningGatewayTests, GptChangeControlledTuningSourceDeactivatesKnownModuleIdAndEchoesInactiveState)
{
	TestTuningGwConnectionAccessor tuningConn{signalManager, logger};

	ASSERT_TRUE(tuningConn.connect(TuningTestSettings::Address, TuningTestSettings::Port));
	ASSERT_NO_THROW(tuningConn.requestHandshake(clientEquipmentId));
	ASSERT_NO_THROW(tuningConn.requestTuningSources());

	GatewayClientLib::GwChangeControlledTuningSourceRequest request{};
	GatewayClientLib::GwChangeControlledTuningSourceResponse response{};

	constexpr std::string_view moduleEquipmentId = "SYSTEMID_CLIENTTEST_CH12_MD00";
	std::snprintf(request.moduleEquipmentId, sizeof(request.moduleEquipmentId), "%s", moduleEquipmentId.data());
	request.activateControl = 1;

	ASSERT_EQ(tuningConn.sendRawRequest(GatewayClientLib::TuningGwRequestId::TGW_CHANGE_CONTROLLED_TUNING_SOURCE, request, response),
			  GatewayClientLib::GwErrorCode::GWC_SUCCESS);

	request.activateControl = 0;
	response = {};

	auto status = tuningConn.sendRawRequest(GatewayClientLib::TuningGwRequestId::TGW_CHANGE_CONTROLLED_TUNING_SOURCE, request, response);

	ASSERT_EQ(status, GatewayClientLib::GwErrorCode::GWC_SUCCESS);
	EXPECT_STREQ(response.controlledModuleEquipmentId, moduleEquipmentId.data());
	EXPECT_EQ(response.controlIsActive, 0u);
	EXPECT_EQ(response.reserved[0], 0u);
	EXPECT_EQ(response.reserved[1], 0u);
	EXPECT_EQ(response.reserved[2], 0u);
}

TEST_F(TuningGatewayTests, GptChangeControlledTuningSourceRejectsUnknownModuleId)
{
	TestTuningGwConnectionAccessor tuningConn{signalManager, logger};

	ASSERT_TRUE(tuningConn.connect(TuningTestSettings::Address, TuningTestSettings::Port));
	ASSERT_NO_THROW(tuningConn.requestHandshake(clientEquipmentId));
	ASSERT_NO_THROW(tuningConn.requestTuningSources());

	GatewayClientLib::GwChangeControlledTuningSourceRequest request{};
	GatewayClientLib::GwChangeControlledTuningSourceResponse response{};

	std::snprintf(request.moduleEquipmentId, sizeof(request.moduleEquipmentId), "%s", "SYSTEMID_CLIENTTEST_CH99_MD00");
	request.activateControl = 1;

	auto status = tuningConn.sendRawRequest(GatewayClientLib::TuningGwRequestId::TGW_CHANGE_CONTROLLED_TUNING_SOURCE, request, response);

	EXPECT_EQ(status, GatewayClientLib::GwErrorCode::GWC_UNKNOWN_TUNING_SOURCE_ID);
}

TEST_F(TuningGatewayTests, GptChangeControlledTuningSourceRejectsExcessivePayload)
{
	TestTuningGwConnectionAccessor tuningConn{signalManager, logger};

	ASSERT_TRUE(tuningConn.connect(TuningTestSettings::Address, TuningTestSettings::Port));
	ASSERT_NO_THROW(tuningConn.requestHandshake(clientEquipmentId));
	ASSERT_NO_THROW(tuningConn.requestTuningSources());

	GatewayClientLib::GwChangeControlledTuningSourceRequest request{};
	GatewayClientLib::GwChangeControlledTuningSourceResponse response{};
	std::array<std::byte, 8> payload{};

	constexpr std::string_view moduleEquipmentId = "SYSTEMID_CLIENTTEST_CH12_MD00";
	std::snprintf(request.moduleEquipmentId, sizeof(request.moduleEquipmentId), "%s", moduleEquipmentId.data());
	request.activateControl = 1;

	auto status = tuningConn.sendRawRequest(GatewayClientLib::TuningGwRequestId::TGW_CHANGE_CONTROLLED_TUNING_SOURCE,
											request,
											std::span<const std::byte>(payload),
											response,
											std::span<std::byte>{});

	EXPECT_EQ(status, GatewayClientLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
}

TEST_F(TuningGatewayTests, GptRequestActivateTuningSourceActivatesKnownModuleId)
{
	TestTuningGwConnectionAccessor tuningConn{signalManager, logger};

	ASSERT_TRUE(tuningConn.connect(TuningTestSettings::Address, TuningTestSettings::Port));
	ASSERT_NO_THROW(tuningConn.requestHandshake(clientEquipmentId));
	ASSERT_NO_THROW(tuningConn.requestTuningSources());

	constexpr std::string_view moduleEquipmentId = "SYSTEMID_CLIENTTEST_CH12_MD00";

	const auto status = tuningConn.requestActivateTuningSource(moduleEquipmentId, true);
	ASSERT_EQ(status, GatewayClientLib::GwErrorCode::GWC_SUCCESS);

	bool passed = false;
	for (int i = 0; i < 20; ++i)
	{
		ASSERT_NO_THROW(tuningConn.requestTuningSourceStates());

		const auto sourceStates = tuningConn.tuningSources();
		ASSERT_EQ(sourceStates.size(), gptKnownTuningSourceModuleIds().size());

		for (const auto& sourceState : sourceStates)
		{
			if (std::string_view{sourceState.moduleEquipmentId} == moduleEquipmentId)
			{
				if (sourceState.controlIsActive == 1u)
				{
					passed = true;
				}
			}
			else
			{
				EXPECT_EQ(sourceState.controlIsActive, 0u);
			}
		}

		if (passed)
		{
			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds{200});
	}

	EXPECT_TRUE(passed);
}

TEST_F(TuningGatewayTests, GptRequestActivateTuningSourceDeactivatesKnownModuleId)
{
	TestTuningGwConnectionAccessor tuningConn{signalManager, logger};

	ASSERT_TRUE(tuningConn.connect(TuningTestSettings::Address, TuningTestSettings::Port));
	ASSERT_NO_THROW(tuningConn.requestHandshake(clientEquipmentId));
	ASSERT_NO_THROW(tuningConn.requestTuningSources());

	constexpr std::string_view moduleEquipmentId = "SYSTEMID_CLIENTTEST_CH12_MD00";

	ASSERT_EQ(tuningConn.requestActivateTuningSource(moduleEquipmentId, true), GatewayClientLib::GwErrorCode::GWC_SUCCESS);
	std::this_thread::sleep_for(std::chrono::milliseconds{500});

	ASSERT_EQ(tuningConn.requestActivateTuningSource(moduleEquipmentId, false), GatewayClientLib::GwErrorCode::GWC_SUCCESS);

	bool passed = false;
	for (int i = 0; i < 20; ++i)
	{
		ASSERT_NO_THROW(tuningConn.requestTuningSourceStates());

		const auto sourceStates = tuningConn.tuningSources();
		ASSERT_EQ(sourceStates.size(), gptKnownTuningSourceModuleIds().size());

		bool allDeactivated = std::all_of(sourceStates.begin(),
										  sourceStates.end(),
										  [](const auto& sourceState)
										  {
											  return sourceState.controlIsActive == 0u;
										  });
		if (allDeactivated == true)
		{
			passed = true;
			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds{200});
	}

	EXPECT_TRUE(passed);
}

TEST_F(TuningGatewayTests, GptChangeControlledTuningSourceRejectsLessPayload)
{
	class FakeChangeControlledTuningSourceRequest
	{
	public:
		char moduleEquipmentId[127];
		uint8_t activateControl;
		uint8_t reserved[3];
	};

	TestTuningGwConnectionAccessor tuningConn{signalManager, logger};

	ASSERT_TRUE(tuningConn.connect(TuningTestSettings::Address, TuningTestSettings::Port));
	ASSERT_NO_THROW(tuningConn.requestHandshake(clientEquipmentId));
	ASSERT_NO_THROW(tuningConn.requestTuningSources());

	FakeChangeControlledTuningSourceRequest request{};
	GatewayClientLib::GwChangeControlledTuningSourceResponse response{};

	std::snprintf(request.moduleEquipmentId, sizeof(request.moduleEquipmentId), "%s", "SYSTEMID_CLIENTTEST_CH12_MD00");
	request.activateControl = 1;

	auto status = tuningConn.sendRawRequest(GatewayClientLib::TuningGwRequestId::TGW_CHANGE_CONTROLLED_TUNING_SOURCE, request, response);

	EXPECT_EQ(status, GatewayClientLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
}

#if 0
// Test Format Error for ADSGW_SIGNAL_LIST_START: send excessive payload.
// Expected error code: GWC_REQUEST_FORMAT_ERROR
//
TEST_F(TuningGatewayTests, SignalListStartSendExcessivePayload)
{
	class TestAdsGwConnection : public GatewayClientLib::TuningGwConnImpl
	{
	public:
		using TuningGwConnImpl::TuningGwConnImpl;

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

	TestAdsGwConnection tuningConn{signalManager, logger};
	tuningConn.run({}, TuningTestSettings::Address, TuningTestSettings::Port, clientEquipmentId);

	ASSERT_TRUE(tuningConn.m_connected);
	ASSERT_TRUE(tuningConn.lastStatusCode().has_value());
	EXPECT_EQ(tuningConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
}

// Test Format Error for ADSGW_SIGNAL_LIST_START: send less payload than required.
// Expected error code: GWC_REQUEST_FORMAT_ERROR
//
TEST_F(TuningGatewayTests, SignalListStartSendLessPayload)
{
	class TestAdsGwConnection : public GatewayClientLib::TuningGwConnImpl
	{
	public:
		using TuningGwConnImpl::TuningGwConnImpl;

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

	TestAdsGwConnection tuningConn{signalManager, logger};
	tuningConn.run({}, TuningTestSettings::Address, TuningTestSettings::Port, clientEquipmentId);

	ASSERT_TRUE(tuningConn.m_connected);
	ASSERT_TRUE(tuningConn.lastStatusCode().has_value());
	EXPECT_EQ(tuningConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
}

// Test Format Error for ADSGW_SIGNAL_LIST_NEXT: send excessive payload.
// Expected error code: GWC_REQUEST_FORMAT_ERROR
//
TEST_F(TuningGatewayTests, SignalListNextSendExcessivePayload)
{
	class TestAdsGwConnection : public GatewayClientLib::TuningGwConnImpl
	{
	public:
		using TuningGwConnImpl::TuningGwConnImpl;

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

	TestAdsGwConnection tuningConn{signalManager, logger};
	tuningConn.run({}, TuningTestSettings::Address, TuningTestSettings::Port, clientEquipmentId);

	ASSERT_TRUE(tuningConn.m_connected);
	ASSERT_TRUE(tuningConn.lastStatusCode().has_value());
	EXPECT_EQ(tuningConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
}

// Test Format Error for ADSGW_SIGNAL_LIST_NEXT: send less payload than required.
// Expected error code: GWC_REQUEST_FORMAT_ERROR
//
TEST_F(TuningGatewayTests, SignalListNextSendLessPayload)
{
	class TestAdsGwConnection : public GatewayClientLib::TuningGwConnImpl
	{
	public:
		using TuningGwConnImpl::TuningGwConnImpl;

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

	TestAdsGwConnection tuningConn{signalManager, logger};
	tuningConn.run({}, TuningTestSettings::Address, TuningTestSettings::Port, clientEquipmentId);

	ASSERT_TRUE(tuningConn.m_connected);
	ASSERT_TRUE(tuningConn.lastStatusCode().has_value());
	EXPECT_EQ(tuningConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
}

// Test Format Error for ADSGW_SIGNAL_PARAM_START: send excessive payload.
// Expected error code: GWC_REQUEST_FORMAT_ERROR
//
TEST_F(TuningGatewayTests, SignalParamStartSendExcessivePayload)
{
	class TestAdsGwConnection : public GatewayClientLib::TuningGwConnImpl
	{
	public:
		using TuningGwConnImpl::TuningGwConnImpl;

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

	TestAdsGwConnection tuningConn{signalManager, logger};
	tuningConn.run({}, TuningTestSettings::Address, TuningTestSettings::Port, clientEquipmentId);

	ASSERT_TRUE(tuningConn.m_connected);
	ASSERT_TRUE(tuningConn.lastStatusCode().has_value());
	EXPECT_EQ(tuningConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
}

// Test Format Error for ADSGW_SIGNAL_PARAM_START: send less payload than required.
// Expected error code: GWC_REQUEST_FORMAT_ERROR
//
TEST_F(TuningGatewayTests, SignalParamStartSendLessPayload)
{
	class TestAdsGwConnection : public GatewayClientLib::TuningGwConnImpl
	{
	public:
		using TuningGwConnImpl::TuningGwConnImpl;

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

	TestAdsGwConnection tuningConn{signalManager, logger};
	tuningConn.run({}, TuningTestSettings::Address, TuningTestSettings::Port, clientEquipmentId);

	ASSERT_TRUE(tuningConn.m_connected);
	ASSERT_TRUE(tuningConn.lastStatusCode().has_value());
	EXPECT_EQ(tuningConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
}

// Test Format Error for ADSGW_SIGNAL_PARAM_NEXT: send excessive payload.
// Expected error code: GWC_REQUEST_FORMAT_ERROR
//
TEST_F(TuningGatewayTests, SignalParamNextSendExcessivePayload)
{
	class TestAdsGwConnection : public GatewayClientLib::TuningGwConnImpl
	{
	public:
		using TuningGwConnImpl::TuningGwConnImpl;

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

	TestAdsGwConnection tuningConn{signalManager, logger};
	tuningConn.run({}, TuningTestSettings::Address, TuningTestSettings::Port, clientEquipmentId);

	ASSERT_TRUE(tuningConn.m_connected);
	ASSERT_TRUE(tuningConn.lastStatusCode().has_value());
	EXPECT_EQ(tuningConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
}

// Test Format Error for ADSGW_SIGNAL_PARAM_NEXT: send less payload than required.
// Expected error code: GWC_REQUEST_FORMAT_ERROR
//
TEST_F(TuningGatewayTests, SignalParamNextSendLessPayload)
{
	class TestAdsGwConnection : public GatewayClientLib::TuningGwConnImpl
	{
	public:
		using TuningGwConnImpl::TuningGwConnImpl;

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

	TestAdsGwConnection tuningConn{signalManager, logger};
	tuningConn.run({}, TuningTestSettings::Address, TuningTestSettings::Port, clientEquipmentId);

	ASSERT_TRUE(tuningConn.m_connected);
	ASSERT_TRUE(tuningConn.lastStatusCode().has_value());
	EXPECT_EQ(tuningConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
}

// Test Format Error for ADSGW_SIGNAL_STATE: send excessive payload.
// Expected error code: GWC_REQUEST_FORMAT_ERROR
//
TEST_F(TuningGatewayTests, SignalStateSendExcessivePayload)
{
	class TestAdsGwConnection : public GatewayClientLib::TuningGwConnImpl
	{
	public:
		using TuningGwConnImpl::TuningGwConnImpl;

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

	TestAdsGwConnection tuningConn{signalManager, logger};
	tuningConn.run({}, TuningTestSettings::Address, TuningTestSettings::Port, clientEquipmentId);

	ASSERT_TRUE(tuningConn.m_connected);
	ASSERT_TRUE(tuningConn.lastStatusCode().has_value());
	EXPECT_EQ(tuningConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
}

// Test Format Error for ADSGW_SIGNAL_STATE: send less payload than required.
// Expected error code: GWC_REQUEST_FORMAT_ERROR
//
TEST_F(TuningGatewayTests, SignalStateSendLessPayload)
{
	class TestAdsGwConnection : public GatewayClientLib::TuningGwConnImpl
	{
	public:
		using TuningGwConnImpl::TuningGwConnImpl;

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

	TestAdsGwConnection tuningConn{signalManager, logger};
	tuningConn.run({}, TuningTestSettings::Address, TuningTestSettings::Port, clientEquipmentId);

	ASSERT_TRUE(tuningConn.m_connected);
	ASSERT_TRUE(tuningConn.lastStatusCode().has_value());
	EXPECT_EQ(tuningConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
}

// Test Format Error for ADSGW_SIGNAL_STATE_CHANGES: send excessive payload.
// Expected error code: GWC_REQUEST_FORMAT_ERROR
//
TEST_F(TuningGatewayTests, SignalStateChangesSendExcessivePayload)
{
	class TestAdsGwConnection : public GatewayClientLib::TuningGwConnImpl
	{
	public:
		using TuningGwConnImpl::TuningGwConnImpl;

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

	TestAdsGwConnection tuningConn{signalManager, logger};
	tuningConn.run({}, TuningTestSettings::Address, TuningTestSettings::Port, clientEquipmentId);

	ASSERT_TRUE(tuningConn.m_connected);
	ASSERT_TRUE(tuningConn.lastStatusCode().has_value());
	EXPECT_EQ(tuningConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
}

// Test Format Error for ADSGW_SIGNAL_STATE_CHANGES: send excessive payload.
// Expected error code: GWC_REQUEST_FORMAT_ERROR
//
TEST_F(TuningGatewayTests, SignalStateChangesSendLessPayload)
{
	class TestAdsGwConnection : public GatewayClientLib::TuningGwConnImpl
	{
	public:
		using TuningGwConnImpl::TuningGwConnImpl;

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

	TestAdsGwConnection tuningConn{signalManager, logger};
	tuningConn.run({}, TuningTestSettings::Address, TuningTestSettings::Port, clientEquipmentId);

	ASSERT_TRUE(tuningConn.m_connected);
	ASSERT_TRUE(tuningConn.lastStatusCode().has_value());
	EXPECT_EQ(tuningConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
}

// Request too big part for ADSGW_SIGNAL_LIST_NEXT:
// Expected error code: GWC_REQUEST_FORMAT_ERROR -- it is not stated directly in the protocol doc but makes sense to have it.
//
TEST_F(TuningGatewayTests, SignalListGetInvalidPart)
{
	class TestAdsGwConnection : public GatewayClientLib::TuningGwConnImpl
	{
	public:
		using TuningGwConnImpl::TuningGwConnImpl;

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

	TestAdsGwConnection tuningConn{signalManager, logger};
	tuningConn.run({}, TuningTestSettings::Address, TuningTestSettings::Port, clientEquipmentId);

	ASSERT_TRUE(tuningConn.m_connected);
	ASSERT_TRUE(tuningConn.lastStatusCode().has_value());
	EXPECT_EQ(tuningConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
}

// Request too big part for ADSGW_SIGNAL_LIST_NEXT:
// Expected error code: GWC_REQUEST_FORMAT_ERROR -- it is not stated directly in the protocol doc but makes sense to have it.
//
TEST_F(TuningGatewayTests, SignalParamGetInvalidPart)
{
	class TestAdsGwConnection : public GatewayClientLib::TuningGwConnImpl
	{
	public:
		using TuningGwConnImpl::TuningGwConnImpl;

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

	TestAdsGwConnection tuningConn{signalManager, logger};
	tuningConn.run({}, TuningTestSettings::Address, TuningTestSettings::Port, clientEquipmentId);

	ASSERT_TRUE(tuningConn.m_connected);
	ASSERT_TRUE(tuningConn.lastStatusCode().has_value());
	EXPECT_EQ(tuningConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_REQUEST_FORMAT_ERROR);
}

// Request several signal state which are not existing
// Expecting: These signals are not returned.
//
TEST_F(TuningGatewayTests, RequestNonexistingSignalStates)
{
	class TestAdsGwConnection : public GatewayClientLib::TuningGwConnImpl
	{
	public:
		using TuningGwConnImpl::TuningGwConnImpl;

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

	TestAdsGwConnection tuningConn{signalManager, logger};
	tuningConn.run({}, TuningTestSettings::Address, TuningTestSettings::Port, clientEquipmentId);

	ASSERT_TRUE(tuningConn.m_connected);
	ASSERT_TRUE(tuningConn.lastStatusCode().has_value());
	EXPECT_EQ(tuningConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_SUCCESS);
	EXPECT_EQ(tuningConn.returnedSignalStates, 0);
}

// Request too many signal states
// Expecting: GWC_TOO_MANY_SIGNALS
//
TEST_F(TuningGatewayTests, RequestTooManySignalStates)
{
	class TestAdsGwConnection : public GatewayClientLib::TuningGwConnImpl
	{
	public:
		using TuningGwConnImpl::TuningGwConnImpl;

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

	TestAdsGwConnection tuningConn{signalManager, logger};
	tuningConn.run({}, TuningTestSettings::Address, TuningTestSettings::Port, clientEquipmentId);

	ASSERT_TRUE(tuningConn.m_connected);
	ASSERT_TRUE(tuningConn.lastStatusCode().has_value());
	EXPECT_EQ(tuningConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_TOO_MANY_SIGNALS);
}

// Request several signal states.
// Expecting: Normal behavior.
//
TEST_F(TuningGatewayTests, RequestSignalStates)
{
	class TestAdsGwConnection : public GatewayClientLib::TuningGwConnImpl
	{
	public:
		using TuningGwConnImpl::TuningGwConnImpl;

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

	TestAdsGwConnection tuningConn{signalManager, logger};
	tuningConn.run({}, TuningTestSettings::Address, TuningTestSettings::Port, clientEquipmentId);

	ASSERT_TRUE(tuningConn.m_connected);
	ASSERT_TRUE(tuningConn.lastStatusCode().has_value());
	EXPECT_EQ(tuningConn.lastStatusCode().value(), GatewayClientLib::GwErrorCode::GWC_SUCCESS);

	EXPECT_EQ(tuningConn.returnedSignalStates, tuningConn.projectSignals.size());

	for (const auto& s : tuningConn.projectSignals)
	{
		auto it = std::find_if(tuningConn.states.begin(),
							   tuningConn.states.end(),
							   [&s](const GatewayClientLib::GwAppSignalState& state)
							   {
								   return state.hash == s.hash;
							   });

		EXPECT_NE(it, tuningConn.states.end());

		if (std::isnan(s.expectedValue) == false) // nan - means do not check expected value
		{
			EXPECT_NEAR(it->value, s.expectedValue, 1e-3);
		}
	}
}

#endif