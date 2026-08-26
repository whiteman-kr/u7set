/*
Module title: Signal File Processing

(C) 2026 RPC "Radiy".
All rights reserved.
The information presented here is confidential
and is the intellectual property of PJSC "RPC "Radiy".
Any use of this information is strictly
prohibited without the written consent of PJSC "RPC "Radiy".

Module description: Block loads and stores the file that contains the description of Application signals used in the control logic. 

-- *************************************************************
-- Version v1.00
-- Date:	31.03.2026.
-- Designed by: Mazyr S.M. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Vintenko B.U.

-- Changes: RADIX-3966, rn_sifp initial update

-- *************************************************************
-- Version v1.01
-- Date:	27.04.2026.
-- Designed by: Mazyr S.M. https://jira.radiy.com/browse/RADIX-3966, https://jira.radiy.com/browse/RADIX-3969
-- Reviewed by: Vintenko B.U.

-- Changes: RADIX-3966, rn_sifp module refactoring
-- Add description to function
-- Add file version control
-- Add file CRC64 checking
-- Add signalAvailableByIndex function for get signal states functions
-- Add VduAppSignalState structure
-- Now, internal signal states are stored in separated structures, not in the loaded memory area

-- Changes: RADIX-3969, add logging and debugging support
-- Add log errors and messages

-- *************************************************************

-- Version v1.02
-- Date:	09.05.2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-4009
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-4009, adding API support for working with signals by AppSignalIDs
-- RADIX-3969, input floating-point signals reading (like temperature)

-- *************************************************************

-- Version v1.03
-- Date:	28.07.2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-3966, code is made MISRA-2025 compatible

-- *************************************************************

-- Version v1.04
-- Date:	26.08.2026.
-- Designed by: Nesterenko M.V. https://jira.radiy.com/browse/RADIX-4116
-- Reviewed by: Vintenko B.U.

-- Changes: RADIX-4116, rn_sifp сorrecting errors identified by static analysis testing

-- *************************************************************
*/

#ifndef RN_SIFP_H
#define RN_SIFP_H

#include <stdbool.h>

#include <alt_types.h>

enum VduSignalType
{
	UnknownSignalType = 0,
	Discrete = 1,
	AnalogFloat32 = 2,
	AnalogSignedInt32 = 3
};

// Enum is an integer constant used as a reference.
// It represents a 16-bit unsigned integer value.
//
enum VduSignalInOutType
{
	UnknownInOutType = 0,
	Internal = 1,
	Input = 2,
	Output = 3,
	RxSignal = 4
};

struct VduTxRxAppSignal
{
	alt_u16 optoPortIndex;			// index in OptoPorts table
	alt_u16 signalIndex;			// index in AppSignals table
	alt_u16 offsetW;				// offset (in Words) in OptoPort buffer
	alt_u16 bitNo;					// 0..15
};

struct VduOptoPort
{
	alt_u16 optoPortIndex;			// 0..7
	alt_u16 linkID;					// 0..999

	alt_u16 rxDataSizeW;			// received data size in words (2 bytes)
	alt_u16 txDataSizeW;			// transmitted data size in words

	alt_u32 rxDataUID;				// received DataUID from LM to VDU
	alt_u32 txDataUID;				// transmitted DataUID from VDU to LM
};

struct VduAppSignalParam
{
	alt_u16 signalIndex;

	alt_u16 vduSignalInOutType;				// values of VduSignalInOutType enum
	alt_u16 vduSignalType;					// values of VduSignalType enum

	union
	{
		struct
		{
			bool tunable: 1;
		} bits;

		alt_u16 value;						// boolean properties of signal
	} props;

	alt_u32 refAppSignalID;					//vdu_string_ref
	alt_u32 refCustomAppSignalID;			//vdu_string_ref
	alt_u32 refCaption;						//vdu_string_ref
	alt_u32 refUnit;						//vdu_string_ref

	// real type of next values depends from vduSignalType
	//
	union
	{
		alt_32 intValue;
		float floatValue;
	} defaultValue;

	union
	{
		alt_32 intValue;
		float floatValue;
	} lowLimit;

	union
	{
		alt_32 intValue;
		float floatValue;
	} highLimit;

	alt_u16 decimalPlaces;

	// rx and in/out signals addresses in VDU memory
	//
	alt_u16 rxPortIndex;					// port index for RxSignals

	struct
	{
		// offset (in Words) and bitNo (0..15) for Input/Output signals,
		// offset (in Words) and bitNo (0..15) in appropriate opto port buffer for RxSignals
		alt_u16 offset;
		alt_u16 bitNo;
	} optoAddr;
};

struct AppSignalsFile
{
	char magic[4];							// 'VAS\0' has length of 4 bytes
	alt_u16 fileVersion;					// == VAS_FILE_VERSION

	alt_u16 appSignalsCount;
	alt_u16 hash32ToIndexCount;
	alt_u16 optoPortsCount;
	alt_u16 txAppSignalsCount;

	alt_u16 reserv1;

	alt_u8 vduConfigID[8];				// has length of 8 bytes (64-bits value)

	alt_u32 refAppSignals;				// ref to array of VduAppSignalParam structures,
											// sorted by VduAppSignalParam.signalIndex ascending

	alt_u32 refHash32ToIndex;			// ref to array of VduHashToIndex structures,
											// sorted by VduHash32ToIndex.hash32 ascending

	alt_u32 refOptoPorts;				// ref to array of VduOptoPort structures,
											// sorted by VduOptoPort.portIndex ascending

	alt_u32 refTxAppSignals;			// ref to array of VduTxRxAppSignal structures

	alt_u32 refStrings;
};

struct VduAppSignalState
{
	union
	{
		alt_32 intValue;	// Current value for internal signals
		float floatValue;	// Current value for internal signals
	} value;
};

struct VduHash32ToIndex
{
	alt_32 hash32;				// calcHash32(appSignalID.toUtf8())
	alt_32 signalIndex;			// 32-bit! signal index
};

/*
Copies the contents of the VduAppSignals.bin file to memory at the address specified in the parameter.
Returns “true” if the copy is successful.
Returns “false” if an error occurs while opening or reading the configuration file or on the cache overflow.
Parameters:
    signalsFile - configuration file name.
    startAddr - 32-bit pointer to a buffer where to store the configuration.
    maxFileSize - size, in bytes, of the buffer.
Return Value:
	bytesRead - receives number of actually read bytes.
	Returns "true" if successful, "false" on error.
*/
bool cacheSignalsFile(const char* const signalsFile, alt_u8* const startAddr, const alt_u32 maxFileSize, alt_u32* bytesRead);

//
// Warning: information access functions DO NOT LOCK the critical section for Simulator mode
// because they provide the cached data that is NOT modified.
//

// Function return VDU Config ID (8 bytes array)
//
const alt_u8* const vduConfigID(void);

//
// Object count functions
//

// Function return application signals count
//
alt_u16 appSignalsCount(void);

// Function return opto port count
//
alt_u16 optoPortsCount(void);

// Function return RX signals count
//
alt_u16 txAppSignalsCount(void);

// Function returns VduAppSignalParam by index
//
const struct VduAppSignalParam* const appSignalParamByIndex(alt_u16 index);

// Function returns VduAppSignalState by index
//
const struct VduAppSignalState* const appSignalStateByIndex(alt_u16 index);

// Function return signal index by appSignalId or INVALID_INDEX if signal is not found
//
alt_u16 appSignalIndex(const char* const appSignalId);

// Function return SignalID by index
//
const char* const appSignalID(alt_u16 index);

// Function return CustomSignalID by index
//
const char* const customAppSignalID(alt_u16 index);

// Function return signalCaption by index
//
const char* const signalCaption(alt_u16 index);

// Function return signalUnit by index
//
const char* const signalUnit(alt_u16 index);

// Function returns VduOptoPort by index
//
const struct VduOptoPort* const vduOptoPort(alt_u16 index);

// Function returns VduTxRxAppSignal by index
//
const struct VduTxRxAppSignal* const txRxAppSignal(alt_u16 index);

// At the beginning of the work loop, the states of output signals must be saved to perform their writing during the loop
//
void outputSignalsPreproc(void);

//
// Warning: read/write signals functions DO LOCK the critical section for Simulator mode
// because the Simulator can also read and write the signals memory
//

// Function returns signal is available by index
// 
bool signalAvailableByIndex(alt_u16 signalIndex);

//
// Signal states functions, return false on failure (index is incorrect, or wrong signal type)
//

/*
Copies intiger signal value by signal index.
Returns “true” if the copy is successful.
Returns “false” if signal not found or if signal is the wrong type.
Parameters:
    signalIndex - signal index.
	res - signal value.
Return Value:
	Returns "true" if successful, "false" on error.
*/
/* Signals allowed to be read :
	-	RxSignal Discrete
	-	RxSignal AnalogSignedInt32
	-	Input Discrete
	-	Input AnalogSignedInt16
	-	Internal Discrete
	-	Internal AnalogSignedInt32
*/
bool readSignalValueIntByIndex(alt_u16 signalIndex, alt_32* res);

/*
Copies float signal value by signal index.
Returns “true” if the copy is successful.
Returns “false” if signal not found or if signal is the wrong type.
Parameters:
    signalIndex - signal index.
	res - signal value.
Return Value:
	Returns "true" if successful, "false" on error.
*/
/* Signals allowed to be read :
	-	RxSignal AnalogFloat32
	-	Internal AnalogFloat32
*/
bool readSignalValueFloatByIndex(alt_u16 signalIndex, float* res);

/*
Write intiger signal value by signal index.
Returns “true” if the write is successful.
Returns “false” if signal not found or if signal is the wrong type.
Parameters:
    signalIndex - signal index.
	res - signal value.
Return Value:
	Returns "true" if successful, "false" on error.
*/
/* Signals allowed to be written :
	-	Internal Discrete
	-	Internal AnalogSignedInt32
	-	Output Discrete
*/
bool writeSignalValueIntByIndex(alt_u16 signalIndex, alt_32 value);

/*
Write float signal value by signal index.
Returns “true” if the write is successful.
Returns “false” if signal not found or if signal is the wrong type.
Parameters:
    signalIndex - signal index.
	res - signal value.
Return Value:
	Returns "true" if successful, "false" on error.
*/
/* Signals allowed to be written :
	-	Internal AnalogFloat32
*/
bool writeSignalValueFloatByIndex(alt_u16 signalIndex, float value);

#endif
