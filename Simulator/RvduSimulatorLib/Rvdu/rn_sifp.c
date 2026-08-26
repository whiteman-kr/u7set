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

#include <string.h>
#include <stdlib.h>

#include "rn_sifp.h"
#include "rn_diag.h"
#include "rn_utils.h"
#include "rn_opto.h"

#include "defs.h"

#include "fat_filelib.h"

// Private module data
//

static struct AppSignalsFile* m_pAppSignalsFile = NULL;
static struct VduAppSignalParam* m_pSignalEntry = NULL;
static struct VduAppSignalState* m_pSignalState = NULL;
static struct VduHash32ToIndex* m_pSignalHashTable = NULL;

static struct VduOptoPort* m_pVduOptoPorts = NULL;
static struct VduTxRxAppSignal* m_pTxRxAppSignals = NULL;

static alt_u16 m_outputDiscreteState = 0u;	// Keeps 4 bts of output discretes (OUT_CTRL_* signals)
//
// End of Private data

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
bool cacheSignalsFile(const char* const signalsFile, alt_u8* const startAddr, const alt_u32 maxFileSize, alt_u32* bytesRead)
{
	if ((signalsFile == NULL) || (startAddr == NULL) || (bytesRead == NULL))
	{
		RN_ASSERT(signalsFile != NULL);
		RN_ASSERT(startAddr != NULL);
		RN_ASSERT(bytesRead != NULL);
		return false;
	}

	char msg[STRING_MAX_LEN];
	(void)snprintf(msg, STRING_MAX_LEN, "cacheSignalsFile: loading from %s.", signalsFile);
	logMessage(msg);

	void* f = fl_fopen(signalsFile, "rb");
	if (f == NULL)
	{
		logError(OpenFileError, "cacheSignalsFile: configuration file read error.");
		return false;
	}
	
	bool readOk = true;
	
	// Check file size
	//
	alt_u32 size = fileSize(f);

	if ((size == WRONG_FILE_SIZE) ||
		(size > maxFileSize))
	{
		(void)snprintf(msg, STRING_MAX_LEN, "cacheSignalsFile: file has wrong size (%d), max size is %d bytes.",
				(int)size,
				(int)maxFileSize);
		logError(FileSizeError, msg);

		readOk = false;
	}

	// Read file data
	//
	if (readOk == true)
	{
		if (freadBigData(startAddr, size, f) == false)
		{
			(void)snprintf(msg, STRING_MAX_LEN, "cacheSignalsFile: file %s read error.", signalsFile);
			logError(ReadFileError, msg);

			readOk = false;
		}
	}

	(void)fl_fclose(f);

	if (readOk == true)
	{
		m_pAppSignalsFile = (struct AppSignalsFile*)startAddr;

		if (m_pAppSignalsFile->fileVersion > FILE_SIGNALS_VERSION)
		{
			(void)snprintf(msg, STRING_MAX_LEN, "cacheSignalsFile: file '%s' version (%u) is not supported, max supported version is (%u).",
				signalsFile,
				m_pAppSignalsFile->fileVersion,
				FILE_SIGNALS_VERSION);
			logError(UnsupportedVersion, msg);

			readOk = false;
		}
	}

	alt_u32 readbytescount = size;

	if (readOk == true)
	{
		// Check crc64
		//
		unsigned long long crc = calcCrc64(startAddr, readbytescount - sizeof(alt_u64));
		const alt_u64* pFileCrc64 = ((alt_u64*)(startAddr + readbytescount - sizeof(alt_u64)));
		if (crc != *pFileCrc64)
		{
			logError(FileFormatError, "cacheSignalsFile: file CRC64 is incorrect.");
			readOk = false;
		}
	}

	if (readOk == true)
	{
		// Check the size for run-time structures
		//
		alt_u32 requiredSize = size
			+ sizeof(struct VduAppSignalState) * m_pAppSignalsFile->appSignalsCount;

		if (requiredSize > maxFileSize)
		{
			(void)snprintf(msg, STRING_MAX_LEN, "cacheSignalsFile: application signals and runitime data size is too large (%d), max size is %d bytes.",
				(int)requiredSize,
				(int)maxFileSize);
			logError(FileSizeError, msg);

			readOk = false;
		}
	}

	if (readOk == true)
	{
		m_pSignalEntry = (struct VduAppSignalParam*)(startAddr + sizeof(struct AppSignalsFile));

		m_pVduOptoPorts = (struct VduOptoPort*)(startAddr + m_pAppSignalsFile->refOptoPorts);
		m_pTxRxAppSignals = (struct VduTxRxAppSignal*)(startAddr + m_pAppSignalsFile->refTxAppSignals);

		// convert offsets in optic channels to offsets in component TO_EXTERNAL_BUS_BRIDGE_1
		//
		for (alt_u32 index = 0; index < m_pAppSignalsFile->appSignalsCount; index++)
		{
			if (m_pSignalEntry[index].vduSignalInOutType == (alt_u16)RxSignal)
			{
				m_pSignalEntry[index].optoAddr.offset +=
					m_pSignalEntry[index].rxPortIndex * WORDS_PER_OPTO_CHANEL;
			}
		}

		// Allocate memory for signals states
		//
		readbytescount = alignAddressToDword(readbytescount);

		m_pSignalState = (struct VduAppSignalState*)(startAddr + readbytescount);
		readbytescount += sizeof(struct VduAppSignalState) * m_pAppSignalsFile->appSignalsCount;

		// Setting default values of internal signals
		//
		for (alt_u32 index = 0; index < m_pAppSignalsFile->appSignalsCount; index++)
		{
			if (m_pSignalEntry[index].vduSignalInOutType == (alt_u16)Internal)
			{
				m_pSignalState[index].value.intValue = m_pSignalEntry[index].defaultValue.intValue;	// float value will also be overwritten
			}
			else
			{
				m_pSignalState[index].value.intValue = 0;
			}
		}

		// Create hash table for all signals
		//
		m_pSignalHashTable = (struct VduHash32ToIndex*)((alt_u8*)m_pAppSignalsFile + m_pAppSignalsFile->refHash32ToIndex);
	}

	*bytesRead = readbytescount;
	return readOk;
}

// Function return VDU Config ID (8 bytes array)
//
const alt_u8* const vduConfigID(void) 
{
	if (m_pAppSignalsFile == NULL)
	{
		RN_ASSERT(m_pAppSignalsFile != NULL);
		return NULL;
	}
	return m_pAppSignalsFile->vduConfigID;
}

// Function return application signals count
//
alt_u16 appSignalsCount(void)
{
	if (m_pAppSignalsFile == NULL)
	{
		return 0;
	}
	return 	m_pAppSignalsFile->appSignalsCount;
}

// Function return opto port count
//
alt_u16 optoPortsCount(void)
{
	if (m_pAppSignalsFile == NULL)
	{
		return 0;
	}
	return 	m_pAppSignalsFile->optoPortsCount;
}

// Function return RX signals count
//
alt_u16 txAppSignalsCount(void)
{
	if (m_pAppSignalsFile == NULL)
	{
		return 0;
	}
	return 	m_pAppSignalsFile->txAppSignalsCount;
}

// Function returns VduAppSignalParam by index
//
const struct VduAppSignalParam* const appSignalParamByIndex(alt_u16 index)
{
	if ((m_pAppSignalsFile == NULL)
		|| (index >= m_pAppSignalsFile->appSignalsCount)
		|| (m_pSignalEntry[index].signalIndex != index))
	{
		return NULL;
	}
	return &m_pSignalEntry[index];
}

// Function returns VduAppSignalState by index
//
const struct VduAppSignalState* const appSignalStateByIndex(alt_u16 index)
{
	if ((m_pAppSignalsFile == NULL)
		|| (index >= m_pAppSignalsFile->appSignalsCount))
	{
		return NULL;
	}
	return &m_pSignalState[index];
}

// Function return signal index by appSignalId or INVALID_INDEX if signal is not found
//
alt_u16 appSignalIndex(const char* const appSignalId)
{
	if ((appSignalId == NULL) ||
		(m_pAppSignalsFile == NULL) ||
		(m_pSignalHashTable == NULL) ||
		(m_pSignalEntry == NULL))

	{
		RN_ASSERT(false);
		return INVALID_INDEX;
	}


	alt_u32 hash = calcHash32((const alt_u8*)appSignalId, (alt_u8)strlen(appSignalId));

	alt_u16 left = 0u;
	alt_u16 right = m_pAppSignalsFile->hash32ToIndexCount;

	while (left < right)
	{
		const alt_u16 mid = left + (right - left) / 2u;
		const alt_u32 midHash = m_pSignalHashTable[mid].hash32;

		if (midHash < hash)
		{
			left = mid + 1u;
		}
		else if (midHash > hash)
		{
			right = mid;
		}
		else
		{
			return (alt_u16)m_pSignalHashTable[mid].signalIndex;
		}
	}

	return INVALID_INDEX;
}

// Function return SignalID by index
//
const char* const appSignalID(alt_u16 index)
{
	const struct VduAppSignalParam* const param = appSignalParamByIndex(index);
	if (param == NULL)
	{
		return NULL;
	}
	return (char*)m_pAppSignalsFile + param->refAppSignalID + sizeof(alt_u16) /*skip string size*/;
}

// Function return CustomSignalID by index
//
const char* const customAppSignalID(alt_u16 index)
{
	const struct VduAppSignalParam* const param = appSignalParamByIndex(index);
	if (param == NULL)
	{
		return NULL;
	}
	return (char*)m_pAppSignalsFile + param->refCustomAppSignalID + sizeof(alt_u16) /*skip string size*/;
}

// Function return signalCaption by index
//
const char* const signalCaption(alt_u16 index)
{
	const struct VduAppSignalParam* const param = appSignalParamByIndex(index);
	if (param == NULL)
	{
		return NULL;
	}
	return (char*)m_pAppSignalsFile + param->refCaption + sizeof(alt_u16) /*skip string size*/;
}

// Function return signalUnit by index
//
const char* const signalUnit(alt_u16 index)
{
	const struct VduAppSignalParam* const param = appSignalParamByIndex(index);
	if (param == NULL)
	{
		return NULL;
	}
	return (char*)m_pAppSignalsFile + param->refUnit + sizeof(alt_u16) /*skip string size*/;
}

// Function returns VduOptoPort by index
//
const struct VduOptoPort* const vduOptoPort(alt_u16 index)
{
	if ((m_pVduOptoPorts == NULL)
			|| (index >= m_pAppSignalsFile->optoPortsCount)
			|| (m_pVduOptoPorts[index].optoPortIndex != index))
	{
		return NULL;
	}
	return &m_pVduOptoPorts[index];
}

// Function returns VduTxRxAppSignal by index
//
const struct VduTxRxAppSignal* const txRxAppSignal(alt_u16 index)
{
	if ((m_pTxRxAppSignals == NULL) ||
			(index >= m_pAppSignalsFile->txAppSignalsCount))
	{
		return NULL;
	}
	return &m_pTxRxAppSignals[index];
}

// At the beginning of the work loop, the states of output signals must be saved to perform their writing during the loop
//
void outputSignalsPreproc(void)
{
	externalBusBridgeReadLock();

	// In RVDU the state of 6 output discretes are stored in corresponding Input signals.
	// We need only 4 output discretes with numbers [3..6].
	m_outputDiscreteState = (readEBB16(ADDR_DISCRETE_OUTPUTS_READ) >> ADDR_DISCRETE_OUTPUTS_RW_BITS_DELTA);
	
	externalBusBridgeReadUnlock();
}

// Function returns signal is available by index
// 
bool signalAvailableByIndex(alt_u16 signalIndex)
{
	const struct VduAppSignalParam* const param = appSignalParamByIndex(signalIndex);
	if (param == NULL)
	{
		return false;
	}

	bool res = true;

	if (param->vduSignalInOutType == (alt_u16)RxSignal)
	{
		res = (readEBB16(ADDR_OPTO_VALIDITY) & (1u << param->rxPortIndex)) != 0u;
	}

	return res;
}

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
bool readSignalValueIntByIndex(alt_u16 signalIndex, alt_32* res)
{
	if (res == NULL)
	{
		RN_ASSERT(res != NULL);
		return false;
	}
	*res = 0;

	const struct VduAppSignalParam* const param = appSignalParamByIndex(signalIndex);
	if (param == NULL)
	{
		return false;
	}

	bool result = false;

	if (param->vduSignalInOutType == (alt_u16)RxSignal)
	{
		if (param->vduSignalType == (alt_u16)Discrete)
		{
			alt_u16 wData = readEBB16(param->optoAddr.offset);
				
			if ((wData & (1u << param->optoAddr.bitNo)) != 0u)
			{
				*res = 1;
			}
			else
			{
				*res = 0;
			}
			result = true;
		}
		else if (param->vduSignalType == (alt_u16)AnalogSignedInt32)
		{
			alt_u16 wDataHi = readEBB16(param->optoAddr.offset);
			alt_u16 wDataLo = readEBB16(param->optoAddr.offset + 1u);
			
			alt_u32 uRes = (((alt_u32)wDataHi << 16u) | wDataLo); // compose a 32-bits value from two 16-bit values
			*res = (alt_32)(uRes);
			result = true;
		}
		else
		{
		}
	}
	else if (param->vduSignalInOutType == (alt_u16)Input)
	{
		if (param->vduSignalType == (alt_u16)Discrete)
		{
			alt_u16 wData = readEBB16(param->optoAddr.offset);

			if ((wData & (1u << param->optoAddr.bitNo)) != 0u)
			{
				*res = 1;
			}
			else
			{
				*res = 0;
			}

			result = true;
		}
		else if (param->vduSignalType == (alt_u16)AnalogSignedInt32)
		{
			*res = (alt_32)readEBB16(param->optoAddr.offset);
			result = true;
		}
		else
		{
		}
	}
	else if (param->vduSignalInOutType == (alt_u16)Internal)
	{
		*res = m_pSignalState[signalIndex].value.intValue;

		if ((param->vduSignalType == (alt_u16)Discrete) ||
			(param->vduSignalType == (alt_u16)AnalogSignedInt32))
		{
			result = true;
		}
	}
	else
	{
	}

	return result;
}

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
bool readSignalValueFloatByIndex(alt_u16 signalIndex, float* res)
{
	if (res == NULL)
	{
		RN_ASSERT(res != NULL);
		return false;
	}
	*res = 0.0f;

	const struct VduAppSignalParam* const param = appSignalParamByIndex(signalIndex);
	if (param == NULL)
	{
		return false;
	}

	bool result = false;

	if (param->vduSignalType == (alt_u16)AnalogFloat32)
	{
		if ((param->vduSignalInOutType == (alt_u16)RxSignal) || (param->vduSignalInOutType == (alt_u16)Input))
		{
			alt_u16 wDataHi = readEBB16(param->optoAddr.offset);
			alt_u16 wDataLo = readEBB16(param->optoAddr.offset + 1u);

			alt_u32 iRes = (((alt_u32)wDataHi << 16u) | wDataLo); // compose a 32-bits value from two 16-bit values
			(void)memcpy(res, &iRes, sizeof(*res));	// alt_u32-to-float bit reinterpretation

			result = true;
		}
		else if (param->vduSignalInOutType == (alt_u16)Internal)
		{
			*res = m_pSignalState[signalIndex].value.floatValue;
			result = true;
		}
		else
		{
		}
	}

	return result;
}

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
bool writeSignalValueIntByIndex(alt_u16 signalIndex, alt_32 value)
{
	const struct VduAppSignalParam* const param = appSignalParamByIndex(signalIndex);
	if (param == NULL)
	{
		return false;
	}

	bool result = false;

	if ((param->vduSignalInOutType == (alt_u16)Internal) &&
		((param->vduSignalType == (alt_u16)Discrete) || (param->vduSignalType == (alt_u16)AnalogSignedInt32)))
	{
		m_pSignalState[signalIndex].value.intValue = value;
		result = true;
	}
	else if ((param->vduSignalInOutType == (alt_u16)Output) &&
		(param->vduSignalType == (alt_u16)Discrete))
	{
		externalBusBridgeWriteLock();
			
		if (value != 0)
		{
			m_outputDiscreteState = m_outputDiscreteState | (1u << (param->optoAddr.bitNo));
		}
		else
		{
			m_outputDiscreteState = m_outputDiscreteState & (~(1u << (param->optoAddr.bitNo)));
		}

		// Only output disretes address shall be written
		//
		if (param->optoAddr.offset == ADDR_DISCRETE_OUTPUTS_WRITE)
		{
			writeEBB16(param->optoAddr.offset, m_outputDiscreteState);
		}
		else
		{
			RN_ASSERT(param->optoAddr.offset == ADDR_DISCRETE_OUTPUTS_WRITE);
		}
		
		externalBusBridgeWriteUnlock();

		result = true;
	}
	else
	{
	}

	return result;
}

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
bool writeSignalValueFloatByIndex(alt_u16 signalIndex, float value)
{
	const struct VduAppSignalParam* const param = appSignalParamByIndex(signalIndex);
	if (param == NULL)
	{
		return false;
	}

	bool result = false;

	if ((param->vduSignalInOutType == (alt_u16)Internal) && 
		(param->vduSignalType == (alt_u16)AnalogFloat32))
	{
		m_pSignalState[signalIndex].value.floatValue = value;
		result = true;
	}

	return result;
}

