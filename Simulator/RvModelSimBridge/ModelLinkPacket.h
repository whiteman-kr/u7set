#pragma once

#include <CommonLib/HostAddressPort.h>
#include "../RvModelSimShared/SimModelPackets.h"

// C++ reflections of requests and replies of the protocol

struct SignalReadRequestRef
{
	// UDP structure:
	// int16_t count;							// Number of signals to read 1..READ_SIGNALS_MAX_COUNT
	// Hash hash[count];						// Signal hashes
	std::vector<Hash> hashes;
};

struct SignalReadReplyRef
{
	// UDP structure:
	// int16_t count;							// Number of responses 1..READ_SIGNALS_MAX_COUNT
	// struct SignalState state[count];			// Signal states
	std::vector<SignalState> states;
};

struct SignalWriteRequestRef
{
	// UDP structure:
	// int16_t count;							// Number of signals to write 1..WRITE_SIGNALS_MAX_COUNT
	// Hash hash[count];						// Signal hashes
	// union SignalValue value[count];			// Signal values
	std::vector<Hash> hashes;
	std::vector<SignalValue> values;
};

struct SignalWriteReplyRef
{
	// UDP structure:
	// int16_t count; // Number of signals 1..WRITE_SIGNALS_MAX_COUNT
	// enum ErrorCode errorCode[count];      // Error codes
	std::vector<ErrorCode> errorCodes;
};

struct SimRequest
{
	int type;
	HostAddressPort addressFrom;
	std::optional<SignalReadRequestRef> readRequest;
	std::optional<SignalWriteRequestRef> writeRequest;
};

struct SimReply
{
	int type;
	HostAddressPort addressTo;
	std::optional<SimulatorStateReply> stateReply;
	std::optional<SignalReadReplyRef> readReply;
	std::optional<SignalWriteReplyRef> writeReply;
};
