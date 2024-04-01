#pragma once

#include "VduTypes.h"

// SVDU opto connections info file, extension *.vci
// Data stored in little-endian format.
// The file is a binary file with the following high-level structure:
// 1. VduOptoConnectionsInfoFileHeader
// 2. VduOptoConnectionsInfo array[8]
// 3. VduOptoAppSignalInfo array
// 4. Strings
// 5. crc64

#pragma pack(push, 1)


struct VduOptoConnectionsInfoFileHeader
{
	char magic[4] = { 'V', 'C', 'I', '\0' };	// "VCI\0"  VDU Connections Info
	uint16_t fileVersion = 0;

	uint16_t optoConnectionsCount = 0;

	uint32_t connectionsInfoOffset = 0;
	uint32_t appSignalsInfoOffset = 0;
	uint32_t stringsOffset = 0;

	uint32_t reserve2 = 0;

	// Next:
	//
	// VduOptoConnectionsInfo connectionsInfo[VDU_OPTO_CONNECTIONS_COUNT];
	// VduOptoAppSignalInfo appSignalInfop[]
	// uint16_t strings[]
};

struct VduOptoConnectionsInfo
{
	uint16_t connectionIndex;		// 0..7
	uint16_t connectionID;			// 1..999

	uint16_t rxDataSizeW;			// received data size in words (2 bytes)
	uint32_t rxDataUID;				// received DataUID from LM to VDU

	uint16_t txDataSizeW;			// transmitted data size in words
	uint32_t txDataUID;				// transmitted DataUID from VDU to LM
};

struct VduOptoAppSignalInfo
{
	uint16_t connectionIndex;
	uint16_t signalIndex;

	uint16_t valueOffsetW;			// in words
	uint16_t valueBitNo;

	vdu_string_ref appSignalID;
	vdu_string_ref customAppSignalID;
	vdu_string_ref caption;
	vdu_string_ref unit;
};

#pragma pack(pop)
