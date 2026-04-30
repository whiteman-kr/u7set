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


class TuningGatewayTestsNoTuningService : public testing::Test
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
       using TuningGwConnImpl::requestApplyWrittenSignals;
		using TuningGwConnImpl::requestHandshake;
		using TuningGwConnImpl::requestSignalStates;
		using TuningGwConnImpl::requestWriteSignalValues;
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

	std::vector<GatewayClientLib::GwTuningWriteValue> gptKnownWritableSignalValues()
	{
		return {
			{Radiy::calcHash("#CLIENTTEST_TUNING_SAFE_D1"), 1.0},
			{Radiy::calcHash("#TGW_D1"), 0.0},
		};
	}

	std::array<std::string_view, 3> gptKnownTuningSourceModuleIds()
	{
		return {"SYSTEMID_CLIENTTEST_CH12_MD00", "SYSTEMID_CLIENTTEST_CH13_MD00", "SYSTEMID_CLIENTTEST_CH14_MD00"};
	}
} // namespace

TEST_F(TuningGatewayTestsNoTuningService, GptRequestTuningSourcesReturnsNoTsConnection)
{
   TestTuningGwConnectionAccessor tuningConn{signalManager, logger};

	ASSERT_TRUE(tuningConn.connect(TuningTestSettings::Address, TuningTestSettings::Port));
	ASSERT_NO_THROW(tuningConn.requestHandshake(clientEquipmentId));

	GatewayClientLib::GwGetTuningSourcesStartRequest request{};
	GatewayClientLib::GwGetTuningSourcesStartResponse response{};

	auto status = tuningConn.sendRawRequest(GatewayClientLib::TuningGwRequestId::TGW_GET_TUNING_SOURCES_START, request, response);

	EXPECT_EQ(status, GatewayClientLib::GwErrorCode::GWC_NO_TS_CONNECTION);
}

TEST_F(TuningGatewayTestsNoTuningService, GptRequestTuningSourcesNextReturnsNoTsConnection)
{
	TestTuningGwConnectionAccessor tuningConn{signalManager, logger};

	ASSERT_TRUE(tuningConn.connect(TuningTestSettings::Address, TuningTestSettings::Port));
	ASSERT_NO_THROW(tuningConn.requestHandshake(clientEquipmentId));

	GatewayClientLib::GwGetTuningSourcesNextRequest request{};
	request.part = 0;
	GatewayClientLib::GwGetTuningSourcesNextResponse response{};
	std::array<std::byte, 256> xmlPart{};

	auto status = tuningConn.sendRawRequest(GatewayClientLib::TuningGwRequestId::TGW_GET_TUNING_SOURCES_NEXT,
		request,
		std::span<const std::byte>{},
		response,
		std::span<std::byte>{xmlPart});

	EXPECT_EQ(status, GatewayClientLib::GwErrorCode::GWC_NO_TS_CONNECTION);
}

TEST_F(TuningGatewayTestsNoTuningService, GptRequestTuningSourceStatesReturnsNoTsConnection)
{
	TestTuningGwConnectionAccessor tuningConn{signalManager, logger};

	ASSERT_TRUE(tuningConn.connect(TuningTestSettings::Address, TuningTestSettings::Port));
	ASSERT_NO_THROW(tuningConn.requestHandshake(clientEquipmentId));

	GatewayClientLib::GwGetTuningSourceStatesRequest request{};
	GatewayClientLib::GwGetTuningSourceStatesResponse response{};
	std::array<GatewayClientLib::GwTuningSourceState, 8> sourceStates{};

	auto status = tuningConn.sendRawRequest(GatewayClientLib::TuningGwRequestId::TGW_GET_TUNING_SOURCE_STATES,
		request,
       std::span<const std::byte>{},
		response,
		std::span<GatewayClientLib::GwTuningSourceState>{sourceStates});

	EXPECT_EQ(status, GatewayClientLib::GwErrorCode::GWC_NO_TS_CONNECTION);
}

TEST_F(TuningGatewayTestsNoTuningService, GptRequestSignalStatesReturnsNoTsConnection)
{
	TestTuningGwConnectionAccessor tuningConn{signalManager, logger};

	ASSERT_TRUE(tuningConn.connect(TuningTestSettings::Address, TuningTestSettings::Port));
	ASSERT_NO_THROW(tuningConn.requestHandshake(clientEquipmentId));

	const auto hashes = gptKnownTuningSignalHashes();
	GatewayClientLib::GwTuningSignalsReadRequest request{};
	request.count = static_cast<uint32_t>(hashes.size());

	GatewayClientLib::GwTuningSignalsReadResponse response{};
	std::vector<GatewayClientLib::GwTuningSignalState> states(hashes.size());

	auto status = tuningConn.sendRawRequest(GatewayClientLib::TuningGwRequestId::TGW_TUNING_SIGNALS_READ,
		request,
		std::span<const Radiy::Hash>{hashes},
		response,
		std::span<GatewayClientLib::GwTuningSignalState>{states});

	EXPECT_EQ(status, GatewayClientLib::GwErrorCode::GWC_NO_TS_CONNECTION);
}

TEST_F(TuningGatewayTestsNoTuningService, GptRequestWriteSignalValuesReturnsNoTsConnection)
{
	TestTuningGwConnectionAccessor tuningConn{signalManager, logger};

	ASSERT_TRUE(tuningConn.connect(TuningTestSettings::Address, TuningTestSettings::Port));
	ASSERT_NO_THROW(tuningConn.requestHandshake(clientEquipmentId));

	auto values = gptKnownWritableSignalValues();
	auto result = tuningConn.requestWriteSignalValues(values, "TuningUser1", false);

	EXPECT_EQ(result.errorCode, GatewayClientLib::GwErrorCode::GWC_NO_TS_CONNECTION);
}

TEST_F(TuningGatewayTestsNoTuningService, GptRequestApplyWrittenSignalsReturnsNoTsConnection)
{
	TestTuningGwConnectionAccessor tuningConn{signalManager, logger};

	ASSERT_TRUE(tuningConn.connect(TuningTestSettings::Address, TuningTestSettings::Port));
	ASSERT_NO_THROW(tuningConn.requestHandshake(clientEquipmentId));

	const auto status = tuningConn.requestApplyWrittenSignals();

	EXPECT_EQ(status, GatewayClientLib::GwErrorCode::GWC_NO_TS_CONNECTION);
}

TEST_F(TuningGatewayTestsNoTuningService, GptRequestActivateTuningSourceReturnsNoTsConnection)
{
	TestTuningGwConnectionAccessor tuningConn{signalManager, logger};

	ASSERT_TRUE(tuningConn.connect(TuningTestSettings::Address, TuningTestSettings::Port));
	ASSERT_NO_THROW(tuningConn.requestHandshake(clientEquipmentId));

	const auto status = tuningConn.requestActivateTuningSource("SYSTEMID_CLIENTTEST_CH12_MD00", true);

	EXPECT_EQ(status, GatewayClientLib::GwErrorCode::GWC_NO_TS_CONNECTION);
}

