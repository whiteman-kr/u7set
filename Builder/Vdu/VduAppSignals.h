#pragma once

#include "VduTypes.h"

// VDU Application Signals info file, extension *.vas
//
// Data stored in little-endian format.
// The file is a binary file with the following high-level structure:
// 1. VduAppSignalsFileHeader
// 2. VduAppSignal array[appSignalsCount]
// 3. VduOptoPort array[optoPortsCount]
// 4. VduTxRxAppSignal array[rxAppSignalsCount] - signals received by VDU
// 5. VduTxRxAppSignal array[txAppSignalsCount] - signals transmitted by VDU
// 6. Strings
// 7. crc64

#pragma pack(push, 1)

inline const uint16_t VAS_FILE_VERSION = 1;

struct VduAppSignalsFileHeader
{
	char magic[4];							// 'VAS\0'
	uint16_t fileVersion;					// == VAS_FILE_VERSION

	uint16_t appSignalsCount;
	uint16_t hashToIndexCount;
	uint16_t optoPortsCount;
	uint16_t rxAppSignalsCount;
	uint16_t txAppSignalsCount;

	vdu_file_ref refAppSignals;				// ref to array of VduAppSignal structures,
											// sorted by VduAppSignal.signalIndex ascending

	vdu_file_ref refHashToIndex;			// ref to array of VduHashToIndex structures,
											// sorted by VduHashToIndex.hash ascending

	vdu_file_ref refOptoPorts;				// ref to array of VduOptoPort structures,
											// sorted by VduOptoPort.portIndex ascending

	vdu_file_ref refRxAppSignals;			// ref to array of VduTxRxAppSignal structures,
											// sortred by fields: portIndex

	vdu_file_ref refTxAppSignals;			// ref to array of VduTxRxAppSignal structures,
											// sortred by fields: portIndex
	vdu_file_ref refStrings;

	//

//	uint32_t reserve1;

};

enum class VduSignalType
{
	Unknown = 0,
	Discrete = 1,
	AnalogFloat32 = 2,
	AnalogSignedInt32 = 3
};

enum class VduSignalInOutType
{
	Unknown = 0,
	Internal = 1,
	Input = 2,
	Output = 3,
};

// table of VduAppSignal struct sorted by VduAppSignal.signalIndex ascending
//
struct VduAppSignal
{
	uint16_t signalIndex;

	uint16_t vduSignalInOutType;	// values of VduSignalInOutType enum
	uint16_t vduSignalType;			// values of VduSignalType enum

	// boolean properties of signal
	//
	union
	{
		struct
		{
			uint16_t enableTuning : 1;		// boolProps.bit0
		};

		uint16_t boolProps;
	};

	vdu_string_ref refAppSignalID;
	vdu_string_ref refCustomAppSignalID;
	vdu_string_ref refCaption;
	vdu_string_ref refUnit;

	// real type of next values depends from vduSignalType
	//
	uint32_t tuningDefaultValue;
	uint32_t tuningLowBound;
	uint32_t tuningHighBound;

	// in/out signals addresses in VDU memory
	//
	uint16_t ioOffset;
	uint16_t ioBit;
};

struct VduHashToIndex
{
	uint32_t hash;					// calcHash32(appSignalID.toUtf8())
	uint32_t signalIndex;			// 32-bit! signal index
};

struct VduOptoPort
{
	uint16_t optoPortIndex;			// 0..7
	uint16_t linkID;				// 0..999

	uint16_t rxDataSizeW;			// received data size in words (2 bytes)
	uint16_t txDataSizeW;			// transmitted data size in words

	uint32_t rxDataUID;				// received DataUID from LM to VDU
	uint32_t txDataUID;				// transmitted DataUID from VDU to LM
};

struct VduTxRxAppSignal
{
	uint16_t optoPortIndex;			// index in OptoPorts table

	uint16_t signalIndex;			// index in AppSignals table

	uint16_t valueOffsetW;			// offset in opto port buffer in words
	uint16_t valueBitNo;
};

#pragma pack(pop)
