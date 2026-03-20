#pragma once

#include <cassert>
#include <cstdint>
#include <format>
#include <string_view>

namespace GatewayClientLib
{
	constexpr uint32_t GW_MAX_PAYLOAD_SIZE = 2 * 1024 * 1024; // 2 MB

	constexpr size_t STRING_LENGTH_128 = 128;
	constexpr size_t STRING_LENGTH_256 = 256;

	constexpr size_t GW_APP_SIGNAL_ID_SIZE = STRING_LENGTH_128;

	enum class GwErrorCode : uint32_t
	{
		GWC_SUCCESS = 0,

		GWC_WRONG_PART_NO = 0x0001,
		GWC_REQUEST_PARAM_EXCEED = 0x0002,
		GWC_REQUEST_STATE_EXCEED = 0x0003,
		GWC_PARSE_REQUEST_ERROR = 0x0004,
		GWC_REQUEST_DATA_SOURCES_STATES_EXCEED = 0x0005,
		GWC_UNKNOWN_TUNING_CLIENT_ID = 0x0007,
		GWC_UNKNOWN_SIGNAL_HASH = 0x0008,
		GWC_INTERNAL_ERROR = 0x0009,
		GWC_TUNING_VALUE_OUT_OF_RANGE = 0x000C,
		GWC_SINGLE_LM_CONTROL_DISABLED = 0x000D,
		GWC_LM_CONTROL_IS_NOT_ACTIVE = 0x000E,
		GWC_CLIENT_IS_NOT_ACTIVE = 0x000F,
		GWC_TUNING_NO_REPLY = 0x0010,
		GWC_TUNING_VALUE_CORRUPTED = 0x0011,
		GWC_UNKNOWN_MATS_USER = 0x0013,
		GWC_DISABLED_MATS_USER = 0x0014,
		GWC_NO_SIGNALS_ALLOWED_TO_CONTROL = 0x0015,
		GWC_SIGNAL_IS_NOT_ALLOWED_TO_CONTROL = 0x0016,
		GWC_UNKNOWN_TUNING_SOURCE_ID = 0x0017,

		GWC_INVALID_REQUEST = 0x0201,
		GWC_UNSUPPORTED_VERSION = 0x0202,
		GWC_NO_ADS_CONNECTION = 0x0203,
		GWC_TOO_MANY_SIGNALS = 0x0204,
		GWC_HANDSHAKE_REQUIRED = 0x0205,
		GWC_REQUEST_FORMAT_ERROR = 0x0206,
		GWC_GATEWAY_INTERNAL_ERROR = 0x0207,
		GWC_NO_TS_CONNECTION = 0x0208,
		GWC_CRC_ERROR = 0x020A
	};

	constexpr std::string_view to_string(GwErrorCode ec) noexcept
	{
		// clang-format off
		switch (ec)
		{
		using enum GwErrorCode;
        case GWC_SUCCESS:							return "GWC_SUCCESS(0x0000)";
		case GWC_WRONG_PART_NO:						return "GWC_WRONG_PART_NO(0x0001)";
		case GWC_REQUEST_PARAM_EXCEED:				return "GWC_REQUEST_PARAM_EXCEED(0x0002)";
		case GWC_REQUEST_STATE_EXCEED:				return "GWC_REQUEST_STATE_EXCEED(0x0003)";
		case GWC_PARSE_REQUEST_ERROR:				return "GWC_PARSE_REQUEST_ERROR(0x0004)";
		case GWC_REQUEST_DATA_SOURCES_STATES_EXCEED:return "GWC_REQUEST_DATA_SOURCES_STATES_EXCEED(0x0005)";
		case GWC_UNKNOWN_TUNING_CLIENT_ID:			return "GWC_UNKNOWN_TUNING_CLIENT_ID(0x0007)";
		case GWC_UNKNOWN_SIGNAL_HASH:				return "GWC_UNKNOWN_SIGNAL_HASH(0x0008)";
		case GWC_INTERNAL_ERROR:					return "GWC_INTERNAL_ERROR(0x0009)";
		case GWC_TUNING_VALUE_OUT_OF_RANGE:			return "GWC_TUNING_VALUE_OUT_OF_RANGE(0x000C)";
		case GWC_SINGLE_LM_CONTROL_DISABLED:		return "GWC_SINGLE_LM_CONTROL_DISABLED(0x000D)";
		case GWC_LM_CONTROL_IS_NOT_ACTIVE:			return "GWC_LM_CONTROL_IS_NOT_ACTIVE(0x000E)";
		case GWC_CLIENT_IS_NOT_ACTIVE:				return "GWC_CLIENT_IS_NOT_ACTIVE(0x000F)";
		case GWC_TUNING_NO_REPLY:					return "GWC_TUNING_NO_REPLY(0x0010)";
		case GWC_TUNING_VALUE_CORRUPTED:			return "GWC_TUNING_VALUE_CORRUPTED(0x0011)";
		case GWC_UNKNOWN_MATS_USER:					return "GWC_UNKNOWN_MATS_USER(0x0013)";
		case GWC_DISABLED_MATS_USER:				return "GWC_DISABLED_MATS_USER(0x0014)";
		case GWC_NO_SIGNALS_ALLOWED_TO_CONTROL:		return "GWC_NO_SIGNALS_ALLOWED_TO_CONTROL(0x0015)";
		case GWC_SIGNAL_IS_NOT_ALLOWED_TO_CONTROL:	return "GWC_SIGNAL_IS_NOT_ALLOWED_TO_CONTROL(0x0016)";
		case GWC_UNKNOWN_TUNING_SOURCE_ID:			return "GWC_UNKNOWN_TUNING_SOURCE_ID(0x0017)";
		case GWC_INVALID_REQUEST:					return "GWC_INVALID_REQUEST(0x0201)";
		case GWC_UNSUPPORTED_VERSION:				return "GWC_UNSUPPORTED_VERSION(0x0202)";
		case GWC_NO_ADS_CONNECTION:					return "GWC_NO_ADS_CONNECTION(0x0203)";
		case GWC_TOO_MANY_SIGNALS:					return "GWC_TOO_MANY_SIGNALS(0x0204)";
		case GWC_HANDSHAKE_REQUIRED:				return "GWC_HANDSHAKE_REQUIRED(0x0205)";
		case GWC_REQUEST_FORMAT_ERROR:				return "GWC_REQUEST_FORMAT_ERROR(0x0206)";
		case GWC_GATEWAY_INTERNAL_ERROR:			return "GWC_GATEWAY_INTERNAL_ERROR(0x0207)";
		case GWC_NO_TS_CONNECTION:					return "GWC_NO_TS_CONNECTION(0x0208)";
		case GWC_CRC_ERROR:							return "GWC_CRC_ERROR(0x020A)";
		}
		// clang-format on

		assert(false);
		return "GwErrorCode(unknown)";
	}
} // namespace GatewayClientLib

template<>
struct std::formatter<GatewayClientLib::GwErrorCode> : std::formatter<std::string_view>
{
	template<typename FormatContext>
	auto format(GatewayClientLib::GwErrorCode code, FormatContext& ctx) const
	{
		return std::formatter<std::string_view>::format(to_string(code), ctx);
	}
};

namespace GatewayClientLib
{
	struct GwMessageHeader
	{
		uint32_t requestID;
		uint32_t payloadSize;
		uint32_t statusCode;
	};

	static_assert(sizeof(GwMessageHeader) == 12);

	constexpr size_t GW_MSG_HEADER_SIZE = sizeof(GwMessageHeader);
	constexpr size_t GW_MSG_CRC_SIZE = sizeof(uint32_t);
	constexpr size_t GW_APP_SIGNAL_HASH_SIZE = sizeof(uint64_t);

	constexpr size_t GW_MAX_MSG_PAYLOAD_SIZE = GW_MAX_PAYLOAD_SIZE - GW_MSG_HEADER_SIZE - GW_MSG_CRC_SIZE;
} // namespace GatewayClientLib