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
-- Date:	31,03,2026.
-- Designed by: Mazyr S.M. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Vintenko B.U.

-- Changes: RADIX-3966, rn_sifp initial update

-- *************************************************************
-- Version v1.01
-- Date:	27,04,2026.
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
-- Date:	09,05,2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-4009
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-4009, adding API support for working with signals by AppSignalIDs
-- RADIX-3969, input floating-point signals reading (like temperature)

-- *************************************************************

-- Version v1.03
-- Date:	28,07,2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-3966, code is made MISRA-2025 compatible

-- *************************************************************
*/

#include <string.h>
#include <stdlib.h>

#include "rn_sifp.h"
#include "rn_diag.h"
#include "rn_utils.h"
#include "rn_opto.h"

#include "defs.h"

#ifndef RVDU_SIM // Microsoft Visual Studio-specific

#include "../fat_io_lib/fat_filelib.h"
#include <system.h>

#else

#include <threads.h>

static mtx_t m_cs;

#endif // End of Microsoft Visual Studio-specific

// Private module data
//
static bool m_moduleInitialized = false;

struct HashTableRecord
{
	alt_u64 hash;
	alt_u16 index;
	alt_u16 reserved;
};

static struct AppSignalsFile* m_pAppSignalsFile = NULL;
static struct VduAppSignalParam* m_pSignalEntry = NULL;
static struct VduAppSignalState* m_pSignalState = NULL;
static struct HashTableRecord* m_pSignalHashTable = NULL;

static struct VduOptoPort* m_pVduOptoPorts = NULL;
static struct VduTxRxAppSignal* m_pTxRxAppSignals = NULL;

//
// End of Private data

void moduleRnSignalsInitialize(void)
{
	if (m_moduleInitialized == true)
	{
		RN_ASSERT(false);
		return;
	}
	m_moduleInitialized = true;

#ifdef RVDU_SIM
	// Initialize critical section
	mtx_init(&m_cs, mtx_plain);
#endif
}

#ifdef RVDU_SIM
void moduleRnSignalsUninitialize(void)
{
	mtx_destroy(&m_cs);
	m_moduleInitialized = false;
}

mtx_t* rnSignalsMutex(void)
{
	RN_ASSERT(m_moduleInitialized == true);
	return &m_cs;
}
#endif

static int compareHashTableRecords(const void* a, const void* b)
{
	const struct HashTableRecord* ra = (const struct HashTableRecord*)a;
	const struct HashTableRecord* rb = (const struct HashTableRecord*)b;

	if (ra->hash < rb->hash)
	{
		return -1;
	}

	if (ra->hash > rb->hash)
	{
		return 1;
	}

	RN_ASSERT(false);
	return 0;
}

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

	if (m_moduleInitialized == false)
	{
		RN_ASSERT_MSG(false, "cacheSignalsFile: module is not initialized, initSignalsModule shall be executed!.");
		return false;
	}

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
			+ sizeof(struct VduAppSignalState) * m_pAppSignalsFile->appSignalsCount
			+ sizeof(struct HashTableRecord) * m_pAppSignalsFile->appSignalsCount;

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
		readbytescount = alignAddressToDword(readbytescount);

		m_pSignalHashTable = (struct HashTableRecord*)(startAddr + readbytescount);
		readbytescount += sizeof(struct HashTableRecord) * m_pAppSignalsFile->appSignalsCount;
		
		for (alt_u16 index = 0; index < m_pAppSignalsFile->appSignalsCount; index++)
		{
			m_pSignalHashTable[index].index = index;
			
			const char* const id = appSignalID(index);
			m_pSignalHashTable[index].hash = calcHash64(id, strlen(id));
		}

		// Sort hash table by ascending hash
		//
		qsort(m_pSignalHashTable,
			m_pAppSignalsFile->appSignalsCount,
			sizeof(struct HashTableRecord),
			&compareHashTableRecords);

	}

	*bytesRead = readbytescount;
	return readOk;
}

const alt_u8* const vduConfigID(void) 
{
	if (m_pAppSignalsFile == NULL)
	{
		RN_ASSERT(m_pAppSignalsFile != NULL);
		return NULL;
	}
	return m_pAppSignalsFile->vduConfigID;
}

alt_u16 appSignalsCount(void)
{
	if (m_pAppSignalsFile == NULL)
	{
		return 0;
	}
	return 	m_pAppSignalsFile->appSignalsCount;
}

alt_u16 optoPortsCount(void)
{
	if (m_pAppSignalsFile == NULL)
	{
		return 0;
	}
	return 	m_pAppSignalsFile->optoPortsCount;
}

alt_u16 txAppSignalsCount(void)
{
	if (m_pAppSignalsFile == NULL)
	{
		return 0;
	}
	return 	m_pAppSignalsFile->txAppSignalsCount;
}

const struct VduAppSignalParam* const appSignalParam(const char* const appSignalId)
{
	alt_u16 signalIndex = appSignalIndex(appSignalId);
	if (signalIndex == INVALID_INDEX)
	{
		return NULL;
	}
	return appSignalParamByIndex(signalIndex);
}

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

const struct VduAppSignalState* const appSignalState(const char* const appSignalId)
{
	alt_u16 signalIndex = appSignalIndex(appSignalId);
	if (signalIndex == INVALID_INDEX)
	{
		return NULL;
	}
	return appSignalStateByIndex(signalIndex);
}

const struct VduAppSignalState* const appSignalStateByIndex(alt_u16 index)
{
	if ((m_pAppSignalsFile == NULL)
		|| (index >= m_pAppSignalsFile->appSignalsCount))
	{
		return NULL;
	}
	return &m_pSignalState[index];
}

struct VduAppSignalState* const appSignalStateMutableByIndex(alt_u16 index)
{
	if ((m_pAppSignalsFile == NULL)
		|| (index >= m_pAppSignalsFile->appSignalsCount))
	{
		return NULL;
	}
	return &m_pSignalState[index];
}

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


	alt_u64 hash = calcHash64(appSignalId, strlen(appSignalId));

	alt_u16 left = 0u;
	alt_u16 right = m_pAppSignalsFile->appSignalsCount;

	while (left < right)
	{
		const alt_u16 mid = left + (right - left) / 2u;
		const alt_u64 midHash = m_pSignalHashTable[mid].hash;

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
			return m_pSignalHashTable[mid].index;
		}
	}

	return INVALID_INDEX;
}

const char* const appSignalID(alt_u16 index)
{
	const struct VduAppSignalParam* const param = appSignalParamByIndex(index);
	if (param == NULL)
	{
		return NULL;
	}
	return (char*)m_pAppSignalsFile + param->refAppSignalID + sizeof(alt_u16) /*skip string size*/;
}

const char* const customAppSignalID(alt_u16 index)
{
	const struct VduAppSignalParam* const param = appSignalParamByIndex(index);
	if (param == NULL)
	{
		return NULL;
	}
	return (char*)m_pAppSignalsFile + param->refCustomAppSignalID + sizeof(alt_u16) /*skip string size*/;
}

const char* const signalCaption(alt_u16 index)
{
	const struct VduAppSignalParam* const param = appSignalParamByIndex(index);
	if (param == NULL)
	{
		return NULL;
	}
	return (char*)m_pAppSignalsFile + param->refCaption + sizeof(alt_u16) /*skip string size*/;
}

const char* const signalUnit(alt_u16 index)
{
	const struct VduAppSignalParam* const param = appSignalParamByIndex(index);
	if (param == NULL)
	{
		return NULL;
	}
	return (char*)m_pAppSignalsFile + param->refUnit + sizeof(alt_u16) /*skip string size*/;
}

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

const struct VduTxRxAppSignal* const txRxAppSignal(alt_u16 index)
{
	if ((m_pTxRxAppSignals == NULL) ||
			(index >= m_pAppSignalsFile->txAppSignalsCount))
	{
		return NULL;
	}
	return &m_pTxRxAppSignals[index];
}

bool signalAvailableByIndex(alt_u16 signalIndex)
{
	const struct VduAppSignalParam* const param = appSignalParamByIndex(signalIndex);
	if (param == NULL)
	{
		return false;
	}

#ifdef RVDU_SIM // Microsoft Visual Studio-specific
	// Enter critical section (lock)
	mtx_lock(&m_cs);
#endif

	bool res = true;

	if (param->vduSignalInOutType == (alt_u16)RxSignal)
	{
		res = (readEBB16(ADDR_OPTO_VALIDITY) & (1u << param->rxPortIndex)) != 0u;
	}

#ifdef RVDU_SIM // Microsoft Visual Studio-specific
	// Leave critical section (unlock)
	mtx_unlock(&m_cs);
#endif

	return res;
}

bool readSignalValueInt(const char* const appSignalId, alt_32* res)
{
	alt_u16 si = appSignalIndex(appSignalId);
	if (si == INVALID_INDEX)
	{
		return false;
	}
	return readSignalValueIntByIndex(si, res);
}

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

#ifdef RVDU_SIM // Microsoft Visual Studio-specific
	// Enter critical section (lock)
	mtx_lock(&m_cs);
#endif

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
			
			alt_u32 uRes = (((alt_u32)wDataHi << 16u) | wDataLo);
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

#ifdef RVDU_SIM // Microsoft Visual Studio-specific
	// Leave critical section (unlock)
	mtx_unlock(&m_cs);
#endif

	return result;
}

bool readSignalValueFloat(const char* const appSignalId, float* res)
{
	alt_u16 si = appSignalIndex(appSignalId);
	if (si == INVALID_INDEX)
	{
		return false;
	}
	return readSignalValueFloatByIndex(si, res);
}

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

#ifdef RVDU_SIM // Microsoft Visual Studio-specific
	// Enter critical section (lock)
	mtx_lock(&m_cs);
#endif

	if (param->vduSignalType == (alt_u16)AnalogFloat32)
	{
		if ((param->vduSignalInOutType == (alt_u16)RxSignal) || (param->vduSignalInOutType == (alt_u16)Input))
		{
			alt_u16 wDataHi = readEBB16(param->optoAddr.offset);
			alt_u16 wDataLo = readEBB16(param->optoAddr.offset + 1u);

			alt_u32 iRes = (((alt_u32)wDataHi << 16u) | wDataLo);
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

#ifdef RVDU_SIM // Microsoft Visual Studio-specific
	// Leave critical section (unlock)
	mtx_unlock(&m_cs);
#endif

	return result;
}

bool writeSignalValueInt(const char* const appSignalId, alt_32 value)
{
	alt_u16 si = appSignalIndex(appSignalId);
	if (si == INVALID_INDEX)
	{
		return false;
	}
	return writeSignalValueIntByIndex(si, value);
}

bool writeSignalValueIntByIndex(alt_u16 signalIndex, alt_32 value)
{
	const struct VduAppSignalParam* const param = appSignalParamByIndex(signalIndex);
	if (param == NULL)
	{
		return false;
	}

	bool result = false;

#ifdef RVDU_SIM // Microsoft Visual Studio-specific
	// Enter critical section (lock)
	mtx_lock(&m_cs);
#endif

	if ((param->vduSignalInOutType == (alt_u16)Internal) &&
		((param->vduSignalType == (alt_u16)Discrete) || (param->vduSignalType == (alt_u16)AnalogSignedInt32)))
	{
		m_pSignalState[signalIndex].value.intValue = value;
		result = true;
	}
	else if ((param->vduSignalInOutType == (alt_u16)Output) &&
		(param->vduSignalType == (alt_u16)Discrete))
	{
#ifndef RVDU_SIM // Nios-specific
		externalBusBridgeWriteLock();
#endif
			
		alt_u16 outputsState = readEBB16(ADDR_DISCRETE_OUTPUTS_READ);

		// in REAL RVDU State of 6 output discretes are stored in corresponding Input signals.
		// We can control only 4 output discretes with numbers [3..6].
		if (value != 0)
		{
			outputsState = outputsState | (1u << (param->optoAddr.bitNo + ADDR_DISCRETE_OUTPUTS_RW_BITS_DELTA));
		}
		else
		{
			outputsState = outputsState & (~(1u << (param->optoAddr.bitNo + ADDR_DISCRETE_OUTPUTS_RW_BITS_DELTA)));
		}
		writeEBB16(param->optoAddr.offset, outputsState >> ADDR_DISCRETE_OUTPUTS_RW_BITS_DELTA);

#ifdef RVDU_SIM
		// Also, we write them to a READ memory
		{
			writeReadEBB16(ADDR_DISCRETE_OUTPUTS_READ, outputsState);
		}
#endif

		result = true;

#ifndef RVDU_SIM // Nios-specific
		externalBusBridgeWriteUnlock();
#endif
	}
	else
	{
	}

#ifdef RVDU_SIM // Microsoft Visual Studio-specific
	// Leave critical section (unlock)
	mtx_unlock(&m_cs);
#endif

	return result;
}

bool writeSignalValueFloat(const char* const appSignalId, float value)
{
	alt_u16 si = appSignalIndex(appSignalId);
	if (si == INVALID_INDEX)
	{
		return false;
	}
	return writeSignalValueFloatByIndex(si, value);
}

bool writeSignalValueFloatByIndex(alt_u16 signalIndex, float value)
{
	const struct VduAppSignalParam* const param = appSignalParamByIndex(signalIndex);
	if (param == NULL)
	{
		return false;
	}

#ifdef RVDU_SIM // Microsoft Visual Studio-specific
	// Enter critical section (lock)
	mtx_lock(&m_cs);
#endif

	bool result = false;

	if ((param->vduSignalInOutType == (alt_u16)Internal) && 
		(param->vduSignalType == (alt_u16)AnalogFloat32))
	{
		m_pSignalState[signalIndex].value.floatValue = value;
		result = true;
	}

#ifdef RVDU_SIM // Microsoft Visual Studio-specific
	// Leave critical section (unlock)
	mtx_unlock(&m_cs);
#endif

	return result;
}

