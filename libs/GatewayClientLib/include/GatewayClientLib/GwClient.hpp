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

		// AppDataService and TuningService error codes
		//
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

		GWC_COMMUNICATION_ERROR = 0x0018, // Not a server error code. Used internally by gateway client to indicate communication errors
										  // (e.g. connection lost, send/receive failure, etc.)
		GWC_COMMAND_CANCELED = 0x0019,    // Not a server error code. Used internally by gateway client to indicate that a command was
										  // canceled (e.g. by client shutdown)

		// Gateway service error codes - reported by gateway service when it fails to process a request (e.g. due to invalid request,
		// unsupported protocol version, etc.)
		//
		GWC_GATEWAY_SERVICE_ERROR_BASE = 0x0200, // Base code for gateway service errors (for internal use, not returned by server)
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

namespace GatewayClientLib
{
	enum class Channel : uint8_t
	{
		A = 0,
		B = 1,
		C = 2,
		D = 3
	};

	constexpr std::string_view to_string(Channel ec) noexcept
	{
		// clang-format off
		switch (ec)
		{
		using enum Channel;
		case A: return "A";
		case B: return "B";
		case C: return "C";
		case D: return "D";
		}
		// clang-format on

		assert(false);
		return "Channel(unknown)";
	}
} // namespace GatewayClientLib

template<>
struct std::formatter<GatewayClientLib::Channel> : std::formatter<std::string_view>
{
	template<typename FormatContext>
	auto format(GatewayClientLib::Channel code, FormatContext& ctx) const
	{
		return std::formatter<std::string_view>::format(to_string(code), ctx);
	}
};

namespace GatewayClientLib
{
	enum class InOutType : uint8_t
	{
		Input = 0,
		Output = 1,
		Internal = 2,
		SoftwareCalculated = 3
	};

	enum class SignalType : uint8_t
	{
		Discrete = 0x00,
		SignedInt32 = 0x10,
		Float32 = 0x11,
		Bus = 0x20
	};

	enum class AnalogFormat : uint8_t
	{
		SignedInt32 = 1,
		Float = 2
	};

	// Structure defining application signal parameters
	//
	struct GwAppSignalParam
	{
		uint64_t hash;                          // Signal hash (as defined in Section 5.2)
		char appSignalId[STRING_LENGTH_128];    // AppSignalID (ASCII, null-terminated, as defined in Section 5.1)
		char customSignalId[STRING_LENGTH_128]; // Custom Signal ID (UTF-8, null-terminated)

		char caption[STRING_LENGTH_256];        // Signal caption/description (UTF-8, null-terminated)
		char equipmentId[STRING_LENGTH_128];    // EquipmentID (ASCII, null-terminated)
		char lmEquipmentId[STRING_LENGTH_128];  // LogicModule EquipmentID (ASCII, null-terminated)
		char units[STRING_LENGTH_128];          // Engineering units (UTF-8, null-terminated)
		char tags[STRING_LENGTH_256];           // Tags, space-separated (ASCII, null-terminated)

		Channel channel;                        // Channel code (A/B/C/D. See Section 7.3)
		InOutType inOutType;                    // I/O type code (Input/Output/Internal. See Section 7.4)
		SignalType type;                        // Signal type code (Discrete/Analog/Bus. See Section 7.5)
		uint8_t decimalPlaces;                  // Number of decimal places for analog signals

		uint8_t tuning;                         // Tuning flag (0 = non-tunable, 1 = tunable)
		uint8_t reserved1;
		uint8_t reserved2;
		uint8_t reserved3;

		double lowValidRange;                   // Low valid range for analog signals
		double highValidRange;                  // High valid range for analog signals

		double tuningDefaultValue;              // Default tuning value
		double tuningLowBound;                  // Low bound for tuning value
		double tuningHighBound;                 // High bound for tuning value
	};

	static_assert(sizeof(GwAppSignalParam) == 1208);
	constexpr size_t GW_APP_SIGNAL_PARAM_SIZE = sizeof(GwAppSignalParam);
} // namespace GatewayClientLib