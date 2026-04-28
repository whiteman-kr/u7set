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

	constexpr size_t TUNING_GW_TUNING_SOURCE_STATE_SIZE = sizeof(GwTuningSourceState);

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

	constexpr size_t TUNING_GW_TUNING_SIGNAL_STATE_SIZE = sizeof(GwTuningSignalState);

	enum GwTuningSignalStateFlags : uint32_t
	{
		TGWF_VALID = 0x00000001,              // Signal value is valid and reliable
		TGWF_OUT_OF_RANGE = 0x00000002,       // Signal value is outside [lowBound, highBound]
		TGWF_WRITE_IN_PROGRESS = 0x00000004,  // A write request is currently in progress
		TGWF_CONTROL_IS_ENABLED = 0x00000008, // LogicModule control is enabled for the tuning source
		TGWF_WRITING_IS_ENABLED = 0x00000010, // Signal allows writing (tuning is enabled and not administratively blocked)
		TGWF_TUNING_DEFAULT = 0x00000020      // Current value equals tuning default value
	};

	constexpr std::string to_string(GwTuningSignalStateFlags f) noexcept
	{
		std::string result;

		if (f & TGWF_VALID)
		{
			result += "VLD ";
		}
		else
		{
			result += "NONVLD ";
		}

		if (f & TGWF_OUT_OF_RANGE)
		{
			result += "OUT_OF_RANGE ";
		}

		if (f & TGWF_WRITE_IN_PROGRESS)
		{
			result += "WRITE_IN_PROGRESS ";
		}

		if (f & TGWF_CONTROL_IS_ENABLED)
		{
			result += "CONTROL_IS_ENABLED ";
		}

		if (f & TGWF_WRITING_IS_ENABLED)
		{
			result += "WRITING_IS_ENABLED ";
		}

		if (f & TGWF_TUNING_DEFAULT)
		{
			result += "TUNING_DEFAULT ";
		}

		if (result.empty() == false && result.back() == ' ')
		{
			result.pop_back();
		}

		return result;
	}
} // namespace GatewayClientLib

namespace GatewayClientLib
{
	//
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

	//
	// Request TGW_GET_TUNING_SOURCES_START
	//
	struct GwGetTuningSourcesStartRequest
	{
		uint32_t reserved;
	};

	static_assert(sizeof(GwGetTuningSourcesStartRequest) == 4);

	constexpr size_t TUNING_GW_GET_TUNING_SOURCES_START_REQUEST_SIZE = sizeof(GwGetTuningSourcesStartRequest);

	struct GwGetTuningSourcesStartResponse
	{
		uint32_t totalSize;   // Total file size in bytes
		uint32_t maxPartSize; // Maximum size of each part in bytes
		uint32_t partCount;   // Total number of parts to retrieve via TGW_GET_TUNING_SOURCES_NEXT
	};

	static_assert(sizeof(GwGetTuningSourcesStartResponse) == 12);

	//
	// Request TGW_GET_TUNING_SOURCES_NEXT
	//
	struct GwGetTuningSourcesNextRequest
	{
		uint32_t part; // Part number to retrieve (0-based)
	};

	static_assert(sizeof(GwGetTuningSourcesNextRequest) == 4);

	constexpr size_t TUNING_GW_GET_TUNING_SOURCES_NEXT_REQUEST_SIZE = sizeof(GwGetTuningSourcesNextRequest);

	struct GwGetTuningSourcesNextResponse
	{
		uint32_t part;     // Current part number (matches request)
		uint32_t partSize; // Size of data in this part (bytes)
#if 0
		char data[partSize]; // Part data (UTF-8 encoded)
#endif
	};

	constexpr size_t TUNING_GW_GET_TUNING_SOURCES_NEXT_RESPONSE_SIZE = sizeof(GwGetTuningSourcesNextResponse);


	//
	// Request TGW_GET_TUNING_SOURCE_STATES
	//
	struct GwGetTuningSourceStatesRequest
	{
		uint32_t reserved;
	};

	static_assert(sizeof(GwGetTuningSourceStatesRequest) == 4);

	constexpr size_t TUNING_GW_GET_TUNING_SOURCE_STATES_REQUEST_SIZE = sizeof(GwGetTuningSourceStatesRequest);

	struct GwGetTuningSourceStatesResponse
	{
		uint32_t count;         // Number of tuning source states in response
		uint8_t clientIsActive; // Current client is active
		uint8_t reserved[3];
#if 0
		GwTuningSourceState sourceStates[count]; // Array of tuning source states
#endif
	};

	constexpr size_t TUNING_GW_GET_TUNING_SOURCE_STATES_RESPONSE_SIZE = sizeof(GwGetTuningSourceStatesResponse);

	//
	// Request TGW_TUNING_SIGNALS_READ
	//
	struct GwTuningSignalsReadRequest
	{
		uint32_t count;    // Number of signal hashes in request
		uint32_t reserved; // Reserved (must be 0)
#if 0
		uint64_t hashes[count]; // Array of AppSignalID hashes (see Section 1.7.2)
#endif
	};

	constexpr size_t TUNING_GW_TUNING_SIGNALS_READ_REQUEST_SIZE = sizeof(GwTuningSignalsReadRequest);

	struct GwTuningSignalsReadResponse
	{
		uint32_t count;    // Number of states in response
		uint32_t reserved; // Reserved
#if 0
		GwTuningSignalState states[count]; // Array of states
#endif
	};

	constexpr size_t TUNING_GW_TUNING_SIGNALS_READ_RESPONSE_SIZE = sizeof(GwTuningSignalsReadResponse);
	constexpr size_t TUNING_GW_MAX_SIGNAL_STATES = (GW_MAX_MSG_PAYLOAD_SIZE - TUNING_GW_TUNING_SIGNALS_READ_RESPONSE_SIZE) / TUNING_GW_TUNING_SIGNAL_STATE_SIZE;

	//
	// Request TGW_TUNING_SIGNALS_APPLY
	//
	struct GwTuningSignalsApplyRequest
	{
		uint32_t reserved; // Reserved (must be 0)
	};

	static_assert(sizeof(GwTuningSignalsApplyRequest) == 4);

	constexpr size_t TUNING_GW_TUNING_SIGNALS_APPLY_REQUEST_SIZE = sizeof(GwTuningSignalsApplyRequest);

	struct GwTuningSignalsApplyResponse
	{
		uint32_t reserved; // Reserved
	};

	static_assert(sizeof(GwTuningSignalsApplyResponse) == 4);

	//
	// Request TGW_TUNING_SIGNALS_WRITE
	//
	struct GwTuningWriteValue
	{
		uint64_t hash; // AppSignalID hash (see Section 1.7.2)
		double value;  // New tuning value (must be finite: not NaN, +Inf, or -Inf)
	};

	static_assert(sizeof(GwTuningWriteValue) == 16);

	constexpr size_t TUNING_GW_TUNING_WRITE_VALUE_SIZE = sizeof(GwTuningWriteValue);

	struct GwTuningSignalsWriteRequest
	{
		char user[128];      // User name (ASCII, null-terminated)
		uint8_t apply;       // 1 = apply values after write (auto-apply),
							 // 0 = write only, requires separate TGW_TUNING_SIGNALS_APPLY
		uint8_t reserved[3]; // Reserved (must be 0)
		uint32_t count;      // Number of write commands
#if 0
		GwTuningWriteValue values[count]; // Array of write commands
#endif
	};

	constexpr size_t TUNING_GW_TUNING_SIGNALS_WRITE_REQUEST_SIZE = sizeof(GwTuningSignalsWriteRequest);
	constexpr size_t TUNING_GW_MAX_WRITE_VALUES = (GW_MAX_MSG_PAYLOAD_SIZE - TUNING_GW_TUNING_SIGNALS_WRITE_REQUEST_SIZE) / TUNING_GW_TUNING_WRITE_VALUE_SIZE;

	static_assert(sizeof(GwTuningSignalsWriteRequest) == 136);

	struct GwTuningSignalWriteResult
	{
		uint64_t hash;     // AppSignalID hash
		int32_t status;    // Per-signal status: 0 = GWC_SUCCESS (command queued),
						   // non-zero = error code (see Section 7.2)
		uint32_t reserved; // Reserved
	};

	static_assert(sizeof(GwTuningSignalWriteResult) == 16);

	struct GwTuningSignalsWriteResponse
	{
		uint32_t count;    // Number of results
		uint32_t reserved; // Reserved
#if 0
		GwTuningSignalWriteResult results[count]; // Per-signal write results
#endif
	};

	static_assert(sizeof(GwTuningSignalsWriteResponse) == 8);

	//
	// Request TGW_CHANGE_CONTROLLED_TUNING_SOURCE
	//
	struct GwChangeControlledTuningSourceRequest
	{
		char moduleEquipmentId[128]; // Tuning source module equipment ID
									 // (ASCII, null-terminated)
		uint8_t activateControl;     // 1 = activate tuning source,
									 // 0 = deactivate tuning source
		uint8_t reserved[3];         // Reserved
	};

	static_assert(sizeof(GwChangeControlledTuningSourceRequest) == 132);

	constexpr size_t TUNING_GW_CHANGE_CONTROLLED_TUNING_SOURCE_REQUEST_SIZE = sizeof(GwChangeControlledTuningSourceRequest);

	struct GwChangeControlledTuningSourceResponse
	{
		char controlledModuleEquipmentId[128]; // Controlled tuning source module equipment ID
											   // (ASCII, null-terminated)
		uint8_t controlIsActive;               // 1 = control is active,
											   // 0 = control is not active
		uint8_t reserved[3];                   // Reserved
	};

	static_assert(sizeof(GwChangeControlledTuningSourceResponse) == 132);

	constexpr size_t TUNING_GW_CHANGE_CONTROLLED_TUNING_SOURCE_RESPONSE_SIZE = sizeof(GwChangeControlledTuningSourceResponse);

}; // namespace GatewayClientLib
