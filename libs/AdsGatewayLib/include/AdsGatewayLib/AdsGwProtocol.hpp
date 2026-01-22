#pragma once

#include <cstdint>

namespace adsgw
{
	constexpr uint16_t ADSGW_PORT = 5566;
	constexpr uint16_t ADSGW_PROTOCOL_VERSION = 0x0100;
	constexpr uint32_t ADSGW_MAX_PAYLOAD_SIZE = 2 * 1024 * 1024; // 2 MB

	enum GwErrorCode : uint16_t
	{
		GWC_SUCCESS = 0,
		GWC_INVALID_REQUEST = 1,
		GWC_UNSUPPORTED_VERSION = 2,
		GWC_NO_ADS_CONNECTION = 3,
		GWC_TOO_MANY_SIGNALS = 4,
		GWC_HANDSHAKE_REQUIRED = 5,
		GWC_INTERNAL_ERROR = 7,
		GWC_CRC_ERROR = 10
	};

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

	// Request ADSGW_HANDSHAKE
	//
	struct GwHandshakeRequest
	{
		uint16_t protocolVersion; // Protocol version client supports (e.g., 0x0100 for v1.0)
		char clientName[64];      // Null-terminated client name
	};

	static_assert(sizeof(GwHandshakeRequest) == 66);

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

	// Structure defining application signal parameters
	//
	struct GwAppSignalParam
	{
		uint64_t hash;             // Signal hash (as defined in Section 5.2)
		char appSignalId[64];      // AppSignalID (ASCII, null-terminated, as defined in Section 5.1)
		char customSignalId[64];   // Custom Signal ID (UTF-8, null-terminated)

		char caption[256];         // Signal caption/description (UTF-8, null-terminated)
		char equipmentId[64];      // EquipmentID (ASCII, null-terminated)
		char lmEquipmentId[64];    // LogicModule EquipmentID (ASCII, null-terminated)
		char units[64];            // Engineering units (UTF-8, null-terminated)
		char tags[256];            // Tags, space-separated (ASCII, null-terminated)

		uint8_t channel;           // Channel code (see Section 7.3)
		uint8_t inOutType;         // I/O type code (see Section 7.4)
		uint8_t type;              // Signal type code (see Section 7.5)
		uint8_t decimalPlaces;     // Number of decimal places for analog signals

		double lowValidRange;      // Low valid range for analog signals
		double highValidRange;     // High valid range for analog signals

		uint8_t tuning;            // Tuning flag (0 = non-tunable, 1 = tunable)
		double tuningDefaultValue; // Default tuning value
		double tuningLowBound;     // Low bound for tuning value
		double tuningHighBound;    // High bound for tuning value
	};

	static_assert(sizeof(GwAppSignalParam) == 896);

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
} // namespace adsgw
