#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SGW_MARKER 0x1643
#define SGW_VERSION 1

//extern const int16_t crc16tab[];
//
//// Hash calculation
////
//typedef uint64_t Hash;
//typedef uint32_t Hash32;
//#define UNDEFINED_HASH 0x0000000000000000ULL
//
//// CRC16 calculation function (x^16 + x^12 + x^2 + 1)
////
//uint16_t calcCrc16(const void* buf, int len);
//
//// Hash function for Latin ASCII/UTF-8 characters
////
//Hash calcHash(const char* str);

#pragma pack(push, 4)

//
// Protocol description for SimulatorModelBridge exchange
//

// Maximum number of signals to read/write
//
#define READ_SIGNALS_MAX_COUNT 32
#define WRITE_SIGNALS_MAX_COUNT 32

#define SGW_SNAPSHOTID_MAX_SIZE 64 // Including NULL terminator.

// Packet types
//
#define SGW_SIGNAL_READ 1               // Request and response for reading signals
#define SGW_SIGNAL_WRITE 2              // Request and response for writing signals

#define SGW_COMMAND_GET_STATE 10        // Get simulator state
#define SGW_COMMAND_START 11            // Start simulator
#define SGW_COMMAND_STOP 12             // Stop simulator
#define SGW_COMMAND_PAUSE 13            // Pause simulator
#define SGW_COMMAND_RESUME 14           // Resume simulator

#define SGW_COMMAND_SAVE_SNAPSHOT 20    // Save simulator snapshot
#define SGW_COMMAND_RESTORE_SNAPSHOT 21 // Restore simulator snapshot

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
		uint16_t valid : 1;          //	0 - signal is invalid, 1 - valid
		uint16_t stateAvailable : 1; //	1 - signal value is simulated by the simulator, 0 - not available

		uint16_t simulated : 1;      //	2	1 - signal is simulated
		uint16_t blocked : 1;        //	3	1 - signal is blocked
		uint16_t mismatch : 1;       //	4	1 - signal is inconsistent between channels

		uint16_t aboveHighLimit : 1; //	5	1 - signal value is above upper range
		uint16_t belowLowLimit : 1;  //	6	1 - signal value is below lower range

		// reserved bits
		//
		uint16_t _bit7 : 1;  //  7	reserved
		uint16_t _bit8 : 1;  //  8,	reserved

		uint16_t _bit9 : 1;  //	9
		uint16_t _bit10 : 1; //	10
		uint16_t _bit11 : 1; //	11
		uint16_t _bit12 : 1; //	12
		uint16_t _bit13 : 1; //	13
		uint16_t _bit14 : 1; //	14
		uint16_t _bit15 : 1; //	15
	} bits;

	uint16_t all;
};

// Error codes
//
enum ErrorCode
{
	Success,
	NoConnection,     // no connection to simulator program
	SnapshotNotFound, // simulator snapshot not found during restore

	SignalNotFound,   // signal not found during read/write
	OutOfRange,       // signal value is out of allowed range
	CannotWrite,      // cannot write value (e.g., signal is constant)
};

// Convert error code to string
//
const char* errorCodeToString(enum ErrorCode code);


// Signal state
//
struct SignalState
{
	Hash hash;
	uint64_t time;
	union SignalValue value;
	union SignalFlags flags;
};

// Request to read signal states
//
struct SignalReadRequest
{
	int16_t count; // Number of signals to read 1..READ_SIGNALS_MAX_COUNT
	// Hash hash[count];                // Signal hashes
};

// Response to signal state read request
//
struct SignalReadReply
{
	int16_t count; // Number of responses 1..READ_SIGNALS_MAX_COUNT
	// struct SignalState state[count];        // Signal states
};

// Request to write signal states
//
struct SignalWriteRequest
{
	int16_t count; // Number of signals to write 1..WRITE_SIGNALS_MAX_COUNT
	// Hash hash[count];                // Signal hashes
	// union SignalValue value[count];        // Signal values
};

// Response to signal state write request
//
struct SignalWriteReply
{
	int16_t count; // Number of signals 1..WRITE_SIGNALS_MAX_COUNT
	// enum ErrorCode errorCode[count];      // Error codes
};

struct SimulatorSaveSnapshotRequest
{
	char name[SGW_SNAPSHOTID_MAX_SIZE]; // Simulator snapshot name, null-terminated C-string.
};

struct SimulatorRestoreSnapshotRequest
{
	char name[SGW_SNAPSHOTID_MAX_SIZE]; // Simulator snapshot name, null-terminated C-string.
};

// Simulator state code
//
enum SimulatorStateCode
{
	Unavailable,
	Stopped,
	Running,
	Paused
};

// Convert simulator state code to string
//
const char* simStateToString(enum SimulatorStateCode code);

// Simulator control responses
//
struct SimulatorStateReply
{
	enum ErrorCode errorCode;      // Error code
	enum SimulatorStateCode state; // current simulator state
};

//
// UDP packet for SimulatorModelBridge exchange
//
struct SimulatorBridgePacket
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

	// The following content depends on the packet type.

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

#pragma pack(pop)