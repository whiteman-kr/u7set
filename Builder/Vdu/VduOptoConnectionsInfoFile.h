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
	char magic[4];
	uint16_t fileVersion;

	uint16_t optoPortsCount;

	vdu_file_ref refOptoPortsInfo;
	vdu_file_ref refRxAppSignalsInfo;
	vdu_file_ref refTxAppSignalsInfo;

	vdu_file_ref refStrings;

	//

	uint32_t reserve1;
	uint32_t reserve2;

	// Next:
	//
	// VduOptoPortInfo optoPortInfo[VDU_OPTO_PORTS_COUNT];
	// VduAppSignalInfo rxAppSignalInfo[]				// app signals received by VDU from LM
	// VduAppSignalInfo txAppSignalInfo[]				// app signals transmitted from VDU to LM
	// uint16_t strings[]
};

struct VduOptoPortInfo
{
	uint16_t optoPortIndex;			// 0..7
	uint16_t linkID;				// 0..999

	uint16_t rxDataSizeW;			// received data size in words (2 bytes)
	uint32_t rxDataUID;				// received DataUID from LM to VDU

	uint16_t txDataSizeW;			// transmitted data size in words
	uint32_t txDataUID;				// transmitted DataUID from VDU to LM
};

enum class VduSignalType
{
	Unknown = 0,
	Discrete = 1,
	AnalogFloat32 = 2,
	AnalogSignedInt32 = 3
};

struct VduAppSignalInfo
{
	uint16_t portIndex;
	uint16_t signalIndex;

	uint16_t vduSignalType;			// values of VduSignalType enum

	uint16_t valueOffsetW;			// offset in opto port buffer in words
	uint16_t valueBitNo;

	uint16_t reserv1;

	vdu_string_ref refAppSignalID;
	vdu_string_ref refCustomAppSignalID;
	vdu_string_ref refCaption;
	vdu_string_ref refUnit;

	uint32_t reserv2;
};

#pragma pack(pop)
