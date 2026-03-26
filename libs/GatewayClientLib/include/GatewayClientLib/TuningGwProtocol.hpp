#pragma once

#include "GwClient.hpp"

namespace GatewayClientLib
{
	constexpr uint16_t TUNING_GW_PORT = 5576;
	constexpr uint16_t TUNING_GW_PROTOCOL_VERSION = 0x0100;

	enum class TuningGwRequestId : uint32_t
	{
		TGW_HANDSHAKE = 0x1500,
		TGW_GET_TUNING_SOURCES_START = 0x1521,       // Start retrieval of tuning sources file (TuningSources.xml)
		TGW_GET_TUNING_SOURCES_NEXT = 0x1522,        // Retrieve next part of tuning sources file
		TGW_GET_TUNING_SOURCE_STATES = 0x1502,       // Retrieve tuning sources states
		TGW_TUNING_SIGNALS_READ = 0x1503,            // Read tuning signals states
		TGW_TUNING_SIGNALS_WRITE = 0x1504,           // Write tuning signals values
		TGW_TUNING_SIGNALS_APPLY = 0x1505,           // Apply(commit) written tuning values
		TGW_CHANGE_CONTROLLED_TUNING_SOURCE = 0x1506 // Enable/disable tuning source control(activate LM control)
	};

	constexpr std::string_view to_string(TuningGwRequestId requestId) noexcept
	{
		// clang-format off
		using enum TuningGwRequestId;
		switch (requestId)
		{
		case TGW_HANDSHAKE:							return "TGW_HANDSHAKE(0x1500)";
		case TGW_GET_TUNING_SOURCES_START:			return "TGW_GET_TUNING_SOURCES_START(0x1521)";
		case TGW_GET_TUNING_SOURCES_NEXT:			return "TGW_GET_TUNING_SOURCES_NEXT(0x1522)";
		case TGW_GET_TUNING_SOURCE_STATES:			return "TGW_GET_TUNING_SOURCE_STATES(0x1502)";
		case TGW_TUNING_SIGNALS_READ:				return "TGW_TUNING_SIGNALS_READ(0x1503)";
		case TGW_TUNING_SIGNALS_WRITE:				return "TGW_TUNING_SIGNALS_WRITE(0x1504)";
		case TGW_TUNING_SIGNALS_APPLY:				return "TGW_TUNING_SIGNALS_APPLY(0x1505)";
		case TGW_CHANGE_CONTROLLED_TUNING_SOURCE:	return "TGW_CHANGE_CONTROLLED_TUNING_SOURCE(0x1506)";
		}
		// clang-format on

		return "TuningGwRequestId(unknown)";
	}
} // namespace GatewayClientLib

template<>
struct std::formatter<GatewayClientLib::TuningGwRequestId> : std::formatter<std::string_view>
{
	template<typename FormatContext>
	auto format(GatewayClientLib::TuningGwRequestId requestId, FormatContext& ctx) const
	{
		return std::formatter<std::string_view>::format(to_string(requestId), ctx);
	}
};

namespace GatewayClientLib
{
	// GwTuningSourceState - contains the current state of a tuning source (LogicModule).
	//
	struct GwTuningSourceState
	{
		// Tuning Source channel identification
		//
		uint64_t sourceId;           // Unique source ID
		char moduleEquipmentId[128]; // Module equipment ID (ASCII, null-terminated)
		char lanEquipmentId[128];    // LAN equipment ID (ASCII, null-terminated)

		// Tuning Source processing states (boolean fields: 0 = false, 1 = true)
		//
		uint8_t isReplying;         // TuningService receives data from LM's tuning LAN
		uint8_t controlIsActive;    // Control is active for this tuning source
		uint8_t setSOR;             // Safety Override (SOR) will be set when LM switches
									// from TuningMode
		uint8_t writingDisabled;    // Writing to LM is disabled (non-safety LMs only;
									// ignore for safety LMs)
		uint8_t buildMismatch;      // Non-zero when LM build mismatches the loaded build
		uint8_t hasUnappliedParams; // LM has written tuning signal states that were not
									// applied yet
									// these states will be reset if LM leaves tuning mode.
									// Note: This flag is calculated by TuningService and
									// resets if TuningService is reloaded.
		uint8_t reservedFlags[2];   // Reserved (alignment to 8 bytes)

		int64_t lmTime;             // LM time: milliseconds since Unix epoch,
									// as reported by the LogicModule's own clock
	};

	static_assert(sizeof(GwTuningSourceState) == 280);

	// GwTuningSignalState - contains the current tuning state of a tunable signal.
	//
	struct GwTuningSignalState
	{
		uint64_t hash;      // AppSignalID hash (as defined in Section 1.7.2)
		uint32_t errorCode; // Error code, 0 = GWC_SUCCESS, Section 7.2

		uint32_t flags;     // TuningSignalStateFlags bitmask (see below)
		double value;       // Current signal value

							// All times are ms since Unix epoch
		int64_t successfulReadTime;        // Last successful read time, server UTC
		int64_t writeRequestTime;          // Last write request time, server UTC
		int64_t successfulWriteTime;       // Last successful write time, server UTC
		int64_t unsuccessfulWriteTime;     // Last unsuccessful write time, server UTC

		int64_t lmTime;                    // LogicModule plant time assigned to the state
		uint64_t fotipProcessingNumerator; // Source processing numerator/counter
	};

	static_assert(sizeof(GwTuningSignalState) == 72);
}

namespace GatewayClientLib
{
	// Request TGW_HANDSHAKE
	//
	struct TuningGwHandshakeRequest
	{
		uint16_t protocolVersion; // Protocol version client supports (e.g., 0x0100 for v1.0)
		uint16_t reserved1;       // Reserved for future use
		char clientName[128];     // Null-terminated client name
	};

	static_assert(sizeof(TuningGwHandshakeRequest) == 132);

	constexpr size_t TUNING_GW_HANDSHAKE_REQUEST_SIZE = sizeof(TuningGwHandshakeRequest);

	struct TuningGwHandshakeResponse
	{
		uint16_t protocolVersion;            // Server protocol version
											 // (must match request for success)
		uint16_t reserved;                   // Reserved
		uint32_t maxStateRequest;            // Max tuning signal states per request
											 // (TGW_TUNING_SIGNALS_READ)
		uint32_t maxStateWrite;              // Max tuning signal write commands per request
											 // (TGW_TUNING_SIGNALS_WRITE)

		uint32_t sizeof_GwTuningSourceState; // See Section 5.3, struct GwTuningSourceState
		uint32_t sizeof_GwTuningSignalState; // See Section 6.1
	};

	static_assert(sizeof(TuningGwHandshakeResponse) == 20);

	// Request TGW_GET_TUNING_SOURCES_START
	//
	struct GwGetTuningSourcesStartRequest
	{
		uint32_t reserved;
	};

	static_assert(sizeof(GwGetTuningSourcesStartRequest) == 4);

	struct GwGetTuningSourcesStartResponse
	{
		uint32_t totalSize;   // Total file size in bytes
		uint32_t maxPartSize; // Maximum size of each part in bytes
		uint32_t partCount;   // Total number of parts to retrieve via TGW_GET_TUNING_SOURCES_NEXT
	};

	static_assert(sizeof(GwGetTuningSourcesStartResponse) == 12);

	// Request TGW_GET_TUNING_SOURCES_NEXT
	//
	struct GwGetTuningSourcesNextRequest
	{
		uint32_t part; // Part number to retrieve (0-based)
	};

	static_assert(sizeof(GwGetTuningSourcesNextRequest) == 4);

	struct GwGetTuningSourcesNextResponse
	{
		uint32_t part;     // Current part number (matches request)
		uint32_t partSize; // Size of data in this part (bytes)
#if 0
		char data[partSize]; // Part data (UTF-8 encoded)
#endif
	};

	
	// } // namespace GatewayClientLib
//
// namespace GatewayClientLib
//{
//	constexpr size_t GW_APP_SIGNAL_HASH_SIZE = sizeof(uint64_t);
//
//	// Structure defining application signal parameters
//	//
//	struct GwAppSignalParam
//	{
//		uint64_t hash;                          // Signal hash (as defined in Section 5.2)
//		char appSignalId[STRING_LENGTH_128];    // AppSignalID (ASCII, null-terminated, as defined in Section 5.1)
//		char customSignalId[STRING_LENGTH_128]; // Custom Signal ID (UTF-8, null-terminated)
//
//		char caption[STRING_LENGTH_256];        // Signal caption/description (UTF-8, null-terminated)
//		char equipmentId[STRING_LENGTH_128];    // EquipmentID (ASCII, null-terminated)
//		char lmEquipmentId[STRING_LENGTH_128];  // LogicModule EquipmentID (ASCII, null-terminated)
//		char units[STRING_LENGTH_128];          // Engineering units (UTF-8, null-terminated)
//		char tags[STRING_LENGTH_256];           // Tags, space-separated (ASCII, null-terminated)
//
//		uint8_t channel;                        // Channel code (see Section 7.3)
//		uint8_t inOutType;                      // I/O type code (see Section 7.4)
//		uint8_t type;                           // Signal type code (see Section 7.5)
//		uint8_t decimalPlaces;                  // Number of decimal places for analog signals
//
//		uint8_t tuning;                         // Tuning flag (0 = non-tunable, 1 = tunable)
//		uint8_t reserved1;
//		uint8_t reserved2;
//		uint8_t reserved3;
//
//		double lowValidRange;                   // Low valid range for analog signals
//		double highValidRange;                  // High valid range for analog signals
//
//		double tuningDefaultValue;              // Default tuning value
//		double tuningLowBound;                  // Low bound for tuning value
//		double tuningHighBound;                 // High bound for tuning value
//	};
//
//	static_assert(sizeof(GwAppSignalParam) == 1208);
//	constexpr size_t GW_APP_SIGNAL_PARAM_SIZE = sizeof(GwAppSignalParam);
//
//	// Structure defining application signal state
//	//
//	struct GwAppSignalState
//	{
//		uint64_t hash;      // Signal hash (as defined in Section 5.2)
//		int64_t systemTime; // Server system time (UTC+0) when the state was acquired
//		int64_t localTime;  // systemTime adjusted to Local time zone
//		int64_t plantTime;  // Timestamp assigned in LogicModule (local time zone)
//		double value;       // Signal value (for discrete: 0=false, 1=true)
//		uint32_t flags;     // State flags (see Section 7.3 for bit definitions)
//		uint32_t reserved;  // Reserved for future use
//	};
//
//	static_assert(sizeof(GwAppSignalState) == 48);
//
//	constexpr size_t GW_APP_SIGNAL_STATE_SIZE = sizeof(GwAppSignalState);
//
//	// Signal state flags
//	//
//
//	enum GwAppSignalStateFlags : uint32_t
//	{
//		GWF_VALID = 0x00000001,
//		GWF_STATE_AVAILABLE = 0x00000002,
//		GWF_SIMULATED = 0x00000004,
//		GWF_BLOCKED = 0x00000008,
//		GWF_MISMATCH = 0x00000010,
//		GWF_ABOVE_HIGH_LIMIT = 0x00000020,
//		GWF_BELOW_LOW_LIMIT = 0x00000040,
//		GWF_SW_SIMULATED = 0x00000080,
//		GWF_TUNING_DEFAULT = 0x00000100
//	};
//
//	constexpr std::string to_string(GwAppSignalStateFlags f) noexcept
//	{
//		std::string result;
//
//		if (f & GWF_VALID)
//		{
//			result += "VLD ";
//		}
//		else
//		{
//			result += "NONVLD ";
//		}
//
//		if (f & GWF_STATE_AVAILABLE)
//		{
//			result += "ST_AVAIL ";
//		}
//		else
//		{
//			result += "ST_UNAVAIL ";
//		}
//
//		if (f & GWF_SIMULATED)
//		{
//			result += "SIM ";
//		}
//
//		if (f & GWF_BLOCKED)
//		{
//			result += "BLK ";
//		}
//
//		if (f & GWF_MISMATCH)
//		{
//			result += "MISMATCH ";
//		}
//
//		if (f & GWF_ABOVE_HIGH_LIMIT)
//		{
//			result += "ABOVE_HIGH_LIMIT ";
//		}
//
//		if (f & GWF_BELOW_LOW_LIMIT)
//		{
//			result += "BELOW_LOW_LIMIT ";
//		}
//
//		if (f & GWF_SW_SIMULATED)
//		{
//			result += "SW_SIMULATED ";
//		}
//
//		if (f & GWF_TUNING_DEFAULT)
//		{
//			result += "TUNING_DEFAULT ";
//		}
//
//		if (result.empty() == false && result.back() == ' ')
//		{
//			result.pop_back();
//		}
//
//		return result;
//	}
// } // namespace GatewayClientLib
//
// namespace GatewayClientLib
//{
//	// Request ADSGW_HANDSHAKE
//	//
//	struct GwHandshakeRequest
//	{
//		uint16_t protocolVersion; // Protocol version client supports (e.g., 0x0100 for v1.0)
//		uint16_t reserved1;       // Reserved for future use
//		char clientName[128];     // Null-terminated client name
//	};
//
//	static_assert(sizeof(GwHandshakeRequest) == 132);
//	constexpr size_t GW_HANDSHAKE_REQUEST_SIZE = sizeof(GwHandshakeRequest);
//
//	struct GwHandshakeResponse
//	{
//		uint16_t protocolVersion; // Server protocol version (must match request for success)
//		uint16_t reserved;        // Reserved (must be 0)
//
//		uint32_t maxStateRequest; // Max signal states per request (ADSGW_SIGNAL_STATE)
//
//		// Structure size compatibility fields (bytes)
//		uint32_t sizeof_GwAppSignalParam; // See Section 7.1
//		uint32_t sizeof_GwAppSignalState; // See Section 7.2
//	};
//
//	static_assert(sizeof(GwHandshakeResponse) == 16);
//
//	// Request ARGW_SIGNAL_LIST_START
//	//
//	struct GwSignalListStartRequest
//	{
//		uint32_t reserved;
//	};
//
//	static_assert(sizeof(GwSignalListStartRequest) == 4);
//	constexpr size_t GW_SIGNAL_LIST_START_REQUEST_SIZE = sizeof(GwSignalListStartRequest);
//
//	struct GwSignalListStartResponse
//	{
//		uint32_t totalItemCount; // Total number of AppSignalIDs in system
//		uint32_t partCount;      // Total number of parts (pages) to retrieve
//		uint32_t itemsPerPart;   // Maximum number of AppSignalIDs per part
//	};
//
//	static_assert(sizeof(GwSignalListStartResponse) == 12);
//
//	// Request ARGW_SIGNAL_LIST_NEXT
//	//
//	struct GwSignalListNextRequest
//	{
//		uint32_t part; // Part number to retrieve (0-based index)
//	};
//
//	static_assert(sizeof(GwSignalListNextRequest) == 4);
//	constexpr size_t GW_SIGNAL_LIST_NEXT_REQUEST_SIZE = sizeof(GwSignalListNextRequest);
//
//	struct GwSignalListNextResponse
//	{
//		uint32_t part;             // Part number of this response
//		uint32_t appSignalIdCount; // Number of AppSignalIDs in this response
//
//								   // Array of AppSignalID strings
// #if 0
//		struct
//		{
//			char appSignalId[STRING_LENGTH_128]; // AppSignalID (null-terminated, max STRING_LENGTH_128 bytes including '\0')
//		} appSignalIds[appSignalIdCount];
// #endif
//	};
//
//	constexpr size_t GW_SIGNAL_LIST_NEXT_RESPONSE_SIZE = sizeof(GwSignalListNextResponse);
//	constexpr size_t GW_MAX_APP_SIGNAL_ID_COUNT = (GW_MAX_MSG_PAYLOAD_SIZE - GW_SIGNAL_LIST_NEXT_RESPONSE_SIZE) / GW_APP_SIGNAL_ID_SIZE;
//
//	// Request ARGW_SIGNAL_PARAM_START
//	//
//	struct GwSignalParamStartRequest
//	{
//		uint32_t reserved;
//	};
//
//	static_assert(sizeof(GwSignalParamStartRequest) == 4);
//	constexpr size_t GW_SIGNAL_PARAM_START_REQUEST_SIZE = sizeof(GwSignalParamStartRequest);
//
//	struct GwSignalParamStartResponse
//	{
//		uint32_t totalItemCount; // Total number of GwAppSignalParams in system
//		uint32_t partCount;      // Total number of parts (pages) to retrieve
//		uint32_t itemsPerPart;   // Maximum number of GwAppSignalParams per part
//	};
//
//	static_assert(sizeof(GwSignalParamStartResponse) == 12);
//
//	// Request ARGW_SIGNAL_PARAM_NEXT
//	//
//	struct GwSignalParamNextRequest
//	{
//		uint32_t part; // Part number to retrieve (0-based index)
//	};
//
//	static_assert(sizeof(GwSignalParamNextRequest) == 4);
//	constexpr size_t GW_SIGNAL_PARAM_NEXT_REQUEST_SIZE = sizeof(GwSignalParamNextRequest);
//
//	struct GwSignalParamNextResponse
//	{
//		uint32_t part;       // Part number of this response
//		uint32_t paramCount; // Number of GwAppSignalParams in this response
// #if 0
//		GwAppSignalParam params[paramCount]; // Array of GwAppSignalParam structures
// #endif
//	};
//
//	constexpr size_t GW_SIGNAL_PARAM_NEXT_RESPONSE_SIZE = sizeof(GwSignalParamNextResponse);
//	constexpr size_t GW_MAX_SIGNAL_PARAMS = (GW_MAX_MSG_PAYLOAD_SIZE - GW_SIGNAL_PARAM_NEXT_RESPONSE_SIZE) / GW_APP_SIGNAL_PARAM_SIZE;
//
//	// Request ARGW_SIGNAL_STATE
//	//
//	struct GwSignalStateRequest
//	{
//		uint32_t signalCount; // Number of signals requested
// #if 0
//		uint64_t signalHashes[signalCount]; // Array of signal hashes
// #endif
//	};
//
//	constexpr size_t GW_SIGNAL_STATE_REQUEST_SIZE = sizeof(GwSignalStateRequest);
//
//	struct GwSignalStateResponse
//	{
//		uint32_t stateCount; // Number of states returned
//
// #if 0
//		GwAppSignalState states[stateCount]; // Array of GwAppSignalState structures
// #endif
//	};
//
//	constexpr size_t GW_SIGNAL_STATE_RESPONSE_SIZE = sizeof(GwSignalStateResponse);
//	constexpr size_t GW_MAX_SIGNAL_STATES = (GW_MAX_MSG_PAYLOAD_SIZE - GW_SIGNAL_STATE_RESPONSE_SIZE) / GW_APP_SIGNAL_STATE_SIZE;
//
//	// Request ARGW_SIGNAL_STATE_CHANGES
//	//
//	struct GwSignalStateChangesRequest
//	{
//		uint32_t reserved;
//	};
//
//	static_assert(sizeof(GwSignalStateChangesRequest) == 4);
//	constexpr size_t GW_SIGNAL_STATE_CHANGES_REQUEST_SIZE = sizeof(GwSignalStateChangesRequest);
//
//	struct GwSignalStateChangesResponse
//	{
//		uint32_t pendingStatesCount; // Number of state changes still in queue (not returned in this response)
//		uint32_t stateCount;         // Number of states in this response
// #if 0
//		GwAppSignalState states[stateCount]; // Array of GwAppSignalState structures
// #endif
//	};
//
//	constexpr size_t GW_SIGNAL_STATE_CHANGES_RESPONSE_SIZE = sizeof(GwSignalStateChangesResponse);
//	constexpr size_t GW_MAX_SIGNAL_STATE_CHANGES =
//		(GW_MAX_MSG_PAYLOAD_SIZE - GW_SIGNAL_STATE_CHANGES_RESPONSE_SIZE) / GW_APP_SIGNAL_STATE_SIZE;
//
//
// } // namespace GatewayClientLib
//
// template<>
// struct std::formatter<GatewayClientLib::GwAppSignalStateFlags> : std::formatter<std::string_view>
//{
//	template<typename FormatContext>
//	auto format(GatewayClientLib::GwAppSignalStateFlags flags, FormatContext& ctx) const
//	{
//		return std::formatter<std::string_view>::format(to_string(flags), ctx);
//	}
 };
