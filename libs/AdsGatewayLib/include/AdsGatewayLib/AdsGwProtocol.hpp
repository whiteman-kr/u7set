#pragma once

#include <cassert>
#include <cstdint>
#include <format>
#include <string_view>

namespace AdsGatewayLib
{
	constexpr uint16_t ADSGW_PORT = 5566;

	constexpr uint16_t ADSGW_PROTOCOL_VERSION = 0x0100;
	constexpr uint32_t ADSGW_MAX_PAYLOAD_SIZE = 2 * 1024 * 1024; // 2 MB

	constexpr size_t STRING_LENGTH_128 = 128;
	constexpr size_t STRING_LENGTH_256 = 256;

	constexpr size_t GW_APP_SIGNAL_ID_SIZE = STRING_LENGTH_128;

	enum GwErrorCode : uint32_t
	{
		GWC_SUCCESS = 0,
		GWC_INVALID_REQUEST = 1,
		GWC_UNSUPPORTED_VERSION = 2,
		GWC_NO_ADS_CONNECTION = 3,
		GWC_TOO_MANY_SIGNALS = 4,
		GWC_HANDSHAKE_REQUIRED = 5,
		GWC_REQUEST_FORMAT_ERROR = 6,
		GWC_INTERNAL_ERROR = 7,
		GWC_CRC_ERROR = 10
	};

	constexpr std::string_view to_string(GwErrorCode ec) noexcept
	{
		// clang-format off
		using enum GwErrorCode;
		switch (ec)
		{
		case GWC_SUCCESS:				return "GWC_SUCCESS(0)";
		case GWC_INVALID_REQUEST:		return "GWC_INVALID_REQUEST(1)";
		case GWC_UNSUPPORTED_VERSION:	return "GWC_UNSUPPORTED_VERSION(2)";
		case GWC_NO_ADS_CONNECTION:		return "GWC_NO_ADS_CONNECTION(3)";
		case GWC_TOO_MANY_SIGNALS:		return "GWC_TOO_MANY_SIGNALS(4)";
		case GWC_HANDSHAKE_REQUIRED:	return "GWC_HANDSHAKE_REQUIRED(5)";
		case GWC_INTERNAL_ERROR:		return "GWC_INTERNAL_ERROR(7)";
		case GWC_CRC_ERROR:				return "GWC_CRC_ERROR(10)";
		}
		// clang-format on

		assert(false);
		return "GwErrorCode(unknown)";
	}
} // namespace AdsGatewayLib

template<>
struct std::formatter<AdsGatewayLib::GwErrorCode> : std::formatter<std::string_view>
{
	template<typename FormatContext>
	auto format(AdsGatewayLib::GwErrorCode code, FormatContext& ctx) const
	{
		return std::formatter<std::string_view>::format(to_string(code), ctx);
	}
};

namespace AdsGatewayLib
{
	enum GwRequestId : uint32_t
	{
		ADSGW_HANDSHAKE = 0x0001,
		ADSGW_SIGNAL_LIST_START = 0x0100,
		ADSGW_SIGNAL_LIST_NEXT = 0x0101,
		ADSGW_SIGNAL_PARAM_START = 0x0200,
		ADSGW_SIGNAL_PARAM_NEXT = 0x0201,
		ADSGW_SIGNAL_STATE = 0x0300,
		ADSGW_SIGNAL_STATE_CHANGES = 0x0301
	};

	constexpr std::string_view to_string(GwRequestId requestId) noexcept
	{
		// clang-format off
		using enum GwRequestId;
		switch (requestId)
		{
		case ADSGW_HANDSHAKE:				return "ADSGW_HANDSHAKE(0x0001)";
		case ADSGW_SIGNAL_LIST_START:		return "ADSGW_SIGNAL_LIST_START(0x0100)";
		case ADSGW_SIGNAL_LIST_NEXT:		return "ADSGW_SIGNAL_LIST_NEXT(0x0101)";
		case ADSGW_SIGNAL_PARAM_START:		return "ADSGW_SIGNAL_PARAM_START(0x0200)";
		case ADSGW_SIGNAL_PARAM_NEXT:		return "ADSGW_SIGNAL_PARAM_NEXT(0x0201)";
		case ADSGW_SIGNAL_STATE:			return "ADSGW_SIGNAL_STATE(0x0300)";
		case ADSGW_SIGNAL_STATE_CHANGES:	return "ADSGW_SIGNAL_STATE_CHANGES(0x0301)";
		}
		// clang-format on

		return "GwRequestId(unknown)";
	}
} // namespace AdsGatewayLib

template<>
struct std::formatter<AdsGatewayLib::GwRequestId> : std::formatter<std::string_view>
{
	template<typename FormatContext>
	auto format(AdsGatewayLib::GwRequestId requestId, FormatContext& ctx) const
	{
		return std::formatter<std::string_view>::format(to_string(requestId), ctx);
	}
};

namespace AdsGatewayLib
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

	constexpr size_t GW_MAX_MSG_PAYLOAD_SIZE = ADSGW_MAX_PAYLOAD_SIZE - GW_MSG_HEADER_SIZE - GW_MSG_CRC_SIZE;
} // namespace AdsGatewayLib

namespace AdsGatewayLib
{
	constexpr size_t GW_APP_SIGNAL_HASH_SIZE = sizeof(uint64_t);

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

		uint8_t channel;                        // Channel code (see Section 7.3)
		uint8_t inOutType;                      // I/O type code (see Section 7.4)
		uint8_t type;                           // Signal type code (see Section 7.5)
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

	// Structure defining application signal state
	//
	struct GwAppSignalState
	{
		uint64_t hash;      // Signal hash (as defined in Section 5.2)
		int64_t systemTime; // Server system time (UTC+0) when the state was acquired
		int64_t localTime;  // systemTime adjusted to Local time zone
		int64_t plantTime;  // Timestamp assigned in LogicModule (local time zone)
		double value;       // Signal value (for discrete: 0=false, 1=true)
		uint32_t flags;     // State flags (see Section 7.3 for bit definitions)
		uint32_t reserved;  // Reserved for future use
	};

	static_assert(sizeof(GwAppSignalState) == 48);

	constexpr size_t GW_APP_SIGNAL_STATE_SIZE = sizeof(GwAppSignalState);

	// Signal state flags
	//

	enum GwAppSignalStateFlags : uint32_t
	{
		GWF_VALID = 0x00000001,
		GWF_STATE_AVAILABLE = 0x00000002,
		GWF_SIMULATED = 0x00000004,
		GWF_BLOCKED = 0x00000008,
		GWF_MISMATCH = 0x00000010,
		GWF_ABOVE_HIGH_LIMIT = 0x00000020,
		GWF_BELOW_LOW_LIMIT = 0x00000040,
		GWF_SW_SIMULATED = 0x00000080,
		GWF_TUNING_DEFAULT = 0x00000100
	};

	constexpr std::string to_string(GwAppSignalStateFlags f) noexcept
	{
		std::string result;

		if (f & GWF_VALID)
		{
			result += "VLD ";
		}
		else
		{
			result += "NONVLD ";
		}

		if (f & GWF_STATE_AVAILABLE)
		{
			result += "ST_AVAIL ";
		}
		else
		{
			result += "ST_UNAVAIL ";
		}

		if (f & GWF_SIMULATED)
		{
			result += "SIM ";
		}

		if (f & GWF_BLOCKED)
		{
			result += "BLK ";
		}

		if (f & GWF_MISMATCH)
		{
			result += "MISMATCH ";
		}

		if (f & GWF_ABOVE_HIGH_LIMIT)
		{
			result += "ABOVE_HIGH_LIMIT ";
		}

		if (f & GWF_BELOW_LOW_LIMIT)
		{
			result += "BELOW_LOW_LIMIT ";
		}

		if (f & GWF_SW_SIMULATED)
		{
			result += "SW_SIMULATED ";
		}

		if (f & GWF_TUNING_DEFAULT)
		{
			result += "TUNING_DEFAULT ";
		}

		if (result.empty() == false && result.back() == ' ')
		{
			result.pop_back();
		}

		return result;
	}
} // namespace AdsGatewayLib

namespace AdsGatewayLib
{
	// Request ADSGW_HANDSHAKE
	//
	struct GwHandshakeRequest
	{
		uint16_t protocolVersion; // Protocol version client supports (e.g., 0x0100 for v1.0)
		uint16_t reserved1;       // Reserved for future use
		char clientName[128];     // Null-terminated client name
	};

	static_assert(sizeof(GwHandshakeRequest) == 132);
	constexpr size_t GW_HANDSHAKE_REQUEST_SIZE = sizeof(GwHandshakeRequest);

	struct GwHandshakeResponse
	{
		uint16_t protocolVersion; // Server protocol version (must match request for success)
		uint16_t reserved;        // Reserved (must be 0)

		uint32_t maxStateRequest; // Max signal states per request (ADSGW_SIGNAL_STATE)

		// Structure size compatibility fields (bytes)
		uint32_t sizeof_GwAppSignalParam; // See Section 7.1
		uint32_t sizeof_GwAppSignalState; // See Section 7.2
	};

	static_assert(sizeof(GwHandshakeResponse) == 16);

	// Request ARGW_SIGNAL_LIST_START
	//
	struct GwSignalListStartRequest
	{
		uint32_t reserved;
	};

	static_assert(sizeof(GwSignalListStartRequest) == 4);
	constexpr size_t GW_SIGNAL_LIST_START_REQUEST_SIZE = sizeof(GwSignalListStartRequest);

	struct GwSignalListStartResponse
	{
		uint32_t totalItemCount; // Total number of AppSignalIDs in system
		uint32_t partCount;      // Total number of parts (pages) to retrieve
		uint32_t itemsPerPart;   // Maximum number of AppSignalIDs per part
	};

	static_assert(sizeof(GwSignalListStartResponse) == 12);

	// Request ARGW_SIGNAL_LIST_NEXT
	//
	struct GwSignalListNextRequest
	{
		uint32_t part; // Part number to retrieve (0-based index)
	};

	static_assert(sizeof(GwSignalListNextRequest) == 4);
	constexpr size_t GW_SIGNAL_LIST_NEXT_REQUEST_SIZE = sizeof(GwSignalListNextRequest);

	struct GwSignalListNextResponse
	{
		uint32_t part;             // Part number of this response
		uint32_t appSignalIdCount; // Number of AppSignalIDs in this response

								   // Array of AppSignalID strings
#if 0
		struct
		{
			char appSignalId[STRING_LENGTH_128]; // AppSignalID (null-terminated, max STRING_LENGTH_128 bytes including '\0')
		} appSignalIds[appSignalIdCount];
#endif
	};

	constexpr size_t GW_SIGNAL_LIST_NEXT_RESPONSE_SIZE = sizeof(GwSignalListNextResponse);
	constexpr size_t GW_MAX_APP_SIGNAL_ID_COUNT = (GW_MAX_MSG_PAYLOAD_SIZE - GW_SIGNAL_LIST_NEXT_RESPONSE_SIZE) / GW_APP_SIGNAL_ID_SIZE;

	// Request ARGW_SIGNAL_PARAM_START
	//
	struct GwSignalParamStartRequest
	{
		uint32_t reserved;
	};

	static_assert(sizeof(GwSignalParamStartRequest) == 4);
	constexpr size_t GW_SIGNAL_PARAM_START_REQUEST_SIZE = sizeof(GwSignalParamStartRequest);

	struct GwSignalParamStartResponse
	{
		uint32_t totalItemCount; // Total number of GwAppSignalParams in system
		uint32_t partCount;      // Total number of parts (pages) to retrieve
		uint32_t itemsPerPart;   // Maximum number of GwAppSignalParams per part
	};

	static_assert(sizeof(GwSignalParamStartResponse) == 12);

	// Request ARGW_SIGNAL_PARAM_NEXT
	//
	struct GwSignalParamNextRequest
	{
		uint32_t part; // Part number to retrieve (0-based index)
	};

	static_assert(sizeof(GwSignalParamNextRequest) == 4);
	constexpr size_t GW_SIGNAL_PARAM_NEXT_REQUEST_SIZE = sizeof(GwSignalParamNextRequest);

	struct GwSignalParamNextResponse
	{
		uint32_t part;       // Part number of this response
		uint32_t paramCount; // Number of GwAppSignalParams in this response
#if 0
		GwAppSignalParam params[paramCount]; // Array of GwAppSignalParam structures
#endif
	};

	constexpr size_t GW_SIGNAL_PARAM_NEXT_RESPONSE_SIZE = sizeof(GwSignalParamNextResponse);
	constexpr size_t GW_MAX_SIGNAL_PARAMS = (GW_MAX_MSG_PAYLOAD_SIZE - GW_SIGNAL_PARAM_NEXT_RESPONSE_SIZE) / GW_APP_SIGNAL_PARAM_SIZE;

	// Request ARGW_SIGNAL_STATE
	//
	struct GwSignalStateRequest
	{
		uint32_t signalCount; // Number of signals requested
#if 0
		uint64_t signalHashes[signalCount]; // Array of signal hashes
#endif
	};

	constexpr size_t GW_SIGNAL_STATE_REQUEST_SIZE = sizeof(GwSignalStateRequest);

	struct GwSignalStateResponse
	{
		uint32_t stateCount; // Number of states returned

#if 0		
		GwAppSignalState states[stateCount]; // Array of GwAppSignalState structures
#endif
	};

	constexpr size_t GW_SIGNAL_STATE_RESPONSE_SIZE = sizeof(GwSignalStateResponse);
	constexpr size_t GW_MAX_SIGNAL_STATES = (GW_MAX_MSG_PAYLOAD_SIZE - GW_SIGNAL_STATE_RESPONSE_SIZE) / GW_APP_SIGNAL_STATE_SIZE;

	// Request ARGW_SIGNAL_STATE_CHANGES
	//
	struct GwSignalStateChangesRequest
	{
		uint32_t reserved;
	};

	static_assert(sizeof(GwSignalStateChangesRequest) == 4);
	constexpr size_t GW_SIGNAL_STATE_CHANGES_REQUEST_SIZE = sizeof(GwSignalStateChangesRequest);

	struct GwSignalStateChangesResponse
	{
		uint32_t pendingStatesCount; // Number of state changes still in queue (not returned in this response)
		uint32_t stateCount;         // Number of states in this response
#if 0
		GwAppSignalState states[stateCount]; // Array of GwAppSignalState structures
#endif
	};

	constexpr size_t GW_SIGNAL_STATE_CHANGES_RESPONSE_SIZE = sizeof(GwSignalStateChangesResponse);
	constexpr size_t GW_MAX_SIGNAL_STATE_CHANGES =
		(GW_MAX_MSG_PAYLOAD_SIZE - GW_SIGNAL_STATE_CHANGES_RESPONSE_SIZE) / GW_APP_SIGNAL_STATE_SIZE;


} // namespace AdsGatewayLib

template<>
struct std::formatter<AdsGatewayLib::GwAppSignalStateFlags> : std::formatter<std::string_view>
{
	template<typename FormatContext>
	auto format(AdsGatewayLib::GwAppSignalStateFlags flags, FormatContext& ctx) const
	{
		return std::formatter<std::string_view>::format(to_string(flags), ctx);
	}
};
