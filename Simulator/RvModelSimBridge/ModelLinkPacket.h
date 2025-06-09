#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SGW_MARKER 0x1643
#define SGW_VERSION_1 1

namespace RvUdpSim
{

#pragma pack(push, 4)

//
// Description of the exchange protocol for the SimulatorModelBridge program
//

// Maximum number of signals to be read/written
//
#define READ_SIGNALS_MAX_COUNT 32
#define WRITE_SIGNALS_MAX_COUNT 32

#define SGW_SNAPSHOTID_MAX_SIZE 64 // Including NULL terminator.

// Packet types
//
#define SGW_SIGNAL_READ 1               // Request and response for reading signals
#define SGW_SIGNAL_WRITE 2              // Request and response for writing signals

#define SGW_COMMAND_GET_STATE 10        // Get simulator state
#define SGW_COMMAND_START 11            // Start the simulator
#define SGW_COMMAND_STOP 12             // Stop the simulator
#define SGW_COMMAND_PAUSE 13            // Pause the simulator
#define SGW_COMMAND_RESUME 14           // Resume simulator operation

#define SGW_COMMAND_SAVE_SNAPSHOT 20    // Save simulator state snapshot
#define SGW_COMMAND_RESTORE_SNAPSHOT 21 // Restore simulator state snapshot

	// Signal types
	//
	enum SignalType
	{
		AnalogFloat,
		AnalogInt32,
		Discrete
	};

	// Signal value
	//
	union SignalValue
	{
		int32_t iValue;
		float fValue;
		bool bValue;
	};

	// Signal state flags
	//
	union SignalFlags
	{
		struct
		{
			// flag bits
			//
			uint16_t valid : 1;          // 0 - signal is invalid, 1 - valid
			uint16_t stateAvailable : 1; // 1 - signal value is simulated by the simulator, 0 - not available

			uint16_t simulated : 1;      // 2  1 - signal is simulated
			uint16_t blocked : 1;        // 3  1 - signal is blocked
			uint16_t mismatch : 1;       // 4  1 - signal is inconsistent between channels

			uint16_t aboveHighLimit : 1; // 5  1 - signal value is above the upper range
			uint16_t belowLowLimit : 1;  // 6  1 - signal value is below the lower range

			// reserved bits
			//
			uint16_t _bit7 : 1;  // 7   reserved
			uint16_t _bit8 : 1;  // 8   reserved

			uint16_t _bit9 : 1;  // 9
			uint16_t _bit10 : 1; // 10
			uint16_t _bit11 : 1; // 11
			uint16_t _bit12 : 1; // 12
			uint16_t _bit13 : 1; // 13
			uint16_t _bit14 : 1; // 14
			uint16_t _bit15 : 1; // 15
		} bits;

		uint16_t all;
	};

	// Error codes
	//
	enum ErrorCode
	{
		Success,
		NoConnection,     // no connection to the simulator program
		SnapshotNotFound, // simulator snapshot not found during restore

		SignalNotFound,   // signal not found during read/write
		OutOfRange,       // signal value is out of the allowed range
		CannotWrite,      // unable to write signal value (e.g., signal is constant)
	};

	// Signal state
	//
	struct SignalState
	{
		Hash hash;
		qint64 time;
		union SignalValue value;
		union SignalFlags flags;
	};

	struct SimulatorSaveSnapshotRequest
	{
		char name[SGW_SNAPSHOTID_MAX_SIZE]; // Name of simulator snapshot, null-terminated C-string.
	};

	struct SimulatorRestoreSnapshotRequest
	{
		char name[SGW_SNAPSHOTID_MAX_SIZE]; // Name of simulator snapshot, null-terminated C-string.
	};

	// Simulator state codes
	//
	enum SimulatorStateCode
	{
		Unavailable,
		Stopped,
		Running,
		Paused
	};

	// Responses to simulator control
	//
	struct SimulatorStateReply
	{
		enum ErrorCode errorCode;      // Error code
		enum SimulatorStateCode state; // current simulator state
	};

	//
	// UDP packet header for SimulatorModelBridge exchange
	//
	struct SimulatorBridgePacketHeader_v1
	{
		// Packet header
		//
		int16_t marker;        // Always SGW_MARKER = 0x1643
		int16_t packetVersion; // Always SGW_VERSION = 1
		int16_t reserve0;      // Reserved, always 0
		int16_t size;          // sizeof(SimulatorBridgePacket) + data

		// Packet type
		//
		int16_t packetType; // SGW_*

							// Following is the content part depending on the packet type.

		// 1) Signal reading: SGW_SIGNAL_READ
		//
		// SignalReadRequest readRequest;   // Request
		// SignalReadReply readReply;       // Response

		// 2) Signal writing: SGW_SIGNAL_WRITE
		//
		// SignalWriteRequest writeRequest; // Request
		// SignalWriteReply writeReply;     // Response

		// 3) Simulator control:
		//
		// Requests:
		//
		// SGW_COMMAND_GET_STATE - no additional data
		// SGW_COMMAND_START - no additional data
		// SGW_COMMAND_STOP - no additional data
		// SGW_COMMAND_PAUSE - no additional data
		// SGW_COMMAND_RESUME - no additional data
		//
		// SimulatorStateReply simulatorStateReply;         // Response

		// 4) Saving and restoring simulator state:
		//
		// Requests:
		//
		// SimulatorSaveSnapshotRequest snapshotRequest;    // Request
		// SimulatorRestoreSnapshotRequest snapshotReply;   // Request
		//
		// SimulatorStateReply simulatorStateReply;         // Response
		//

		// uint16_t crc16;
	};

	using SimulatorBridgePacketHeader = SimulatorBridgePacketHeader_v1;

#pragma pack(pop)

	struct SignalsReadRequest
	{
		// UDP structure:
		// int16_t count; // Number of signals to read 1..READ_SIGNALS_MAX_COUNT
		// Hash hash[count];                // Signal hashes
		std::vector<Hash> hashes;
	};

	struct SignalsReadReply
	{
		// UDP structure:
		// int16_t count; // Number of responses 1..READ_SIGNALS_MAX_COUNT
		// struct SignalState state[count];        // Signal states
		std::vector<SignalState> states;
	};

	struct SignalsWriteRequest
	{
		// UDP structure:
		// int16_t count; // Number of signals to write 1..WRITE_SIGNALS_MAX_COUNT
		// Hash hash[count];                // Signal hashes
		// union SignalValue value[count];        // Signal values
		std::vector<Hash> hashes;
		std::vector<SignalValue> values;
	};

	struct SignalsWriteReply
	{
		// UDP structure:
		// int16_t count; // Number of signals 1..WRITE_SIGNALS_MAX_COUNT
		// enum ErrorCode errorCode[count];      // Error codes
		std::vector<ErrorCode> errorCodes;
	};

	struct SimRequest
	{
		int type;
		std::optional<SignalsReadRequest> readRequest;
		std::optional<SignalsWriteRequest> writeRequest;
	};

	struct SimReply
	{
		int type;
		std::optional<SimulatorStateReply> stateReply;
		std::optional<SignalsReadReply> readReply;
		std::optional<SignalsWriteReply> writeReply;
	};
} // namespace SimService
