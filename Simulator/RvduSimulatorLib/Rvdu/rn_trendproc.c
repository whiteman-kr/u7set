/*
Module title: Trends Processing
(C) 2026 RPC "Radiy".
All rights reserved.
The information presented here is confidential
and is the intellectual property of PJSC "RPC "Radiy".
Any use of this information is strictly
prohibited without the written consent of PJSC "RPC "Radiy".
Module description: Processes trends data.

-- *************************************************************
-- Version v1.00
-- Date:	01.06.2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-4095, rn_trendproc improve memory allocation control
-- Add memory allocation control
-- *************************************************************

-- Version v1.01
-- Date:	28,07,2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-3966, code is made MISRA-2025 compatible

-- *************************************************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#include "rn_trendproc.h"
#include "rn_trendview.h"
#include "rn_cfg.h"
#include "rn_utils.h"
#include "rn_sifp.h"
#include "rn_diag.h"

#include "defs.h"

// Private module data
//
static alt_u64 m_initTimeSecs = 0;		// Time, in seconds, of starting the program
static alt_u64 m_workTimeSecs = 0;		// Elapsed time, in seconds, from starting the program
static bool m_moduleInitialized = false;

static alt_u16 m_trendSignalDataCount = 0;
static alt_u8* m_trendSignalData = NULL;

//
// End of Private data

static void processTrendData(alt_u16 ti, alt_u64 timeCode);

void moduleRnTrendProcInitialize(void)
{
	if (m_moduleInitialized == true)
	{
		RN_ASSERT(false);
		return;
	}
	m_moduleInitialized = true;

	m_initTimeSecs = currentTimeInSecs();

	return;
}

bool trendIsConsistent(const VduSchemaFileSchemaItemTrend1* const trend)
{
	if ((trend == NULL) || 
		(trend->version != TYPE_TREND_VERSION) || 
		(trend->durationsSecs[0] < TREND_MIN_DURATION) ||
		(trend->durationsSecs[0] > TREND_MAX_DURATION) ||
		(trend->columnCount == 0u))
	{
		return false;
	}

	return true;
}

static bool allocateTrendSignalData(struct VduSchemaFileSchemaItem1* const item, alt_u32 durationSecs, alt_u32 memoryAvailable, alt_u32* bytesUsed)
{
	*bytesUsed = 0;

	for (alt_u16 si = 0; si < item->trend.signalCount; si++)
	{
		// Find a TrendState record with such parameters
		//
		bool trendDataFound = false;

		for (alt_u16 i = 0; i < m_trendSignalDataCount; i++)
		{
			const struct TrendSignalData* tsd = (struct TrendSignalData*)(m_trendSignalData + sizeof(struct TrendSignalData) * i);
			if ((tsd->appSignalIndex == (alt_u16)item->trend.trendSignals[si].appSignalIndex) &&
				(tsd->validityAppSignalIndex == (alt_u16)item->trend.trendSignals[si].validityAppSignalIndex) &&
				(tsd->columnCount == (alt_u16)item->trend.columnCount) &&
				(tsd->durationSecs == durationSecs))
			{
				item->trend.trendSignals[si].trendSignalDataRecordIndex = i;

				trendDataFound = true;
				break;
			}
		}

		if (trendDataFound == false)
		{
			const alt_u32 trendSignalDataSize = (alt_u32)sizeof(struct TrendSignalData);
			if (memoryAvailable < trendSignalDataSize)
			{
				return false;
			}

			// Create a new TrendData
			//
			struct TrendSignalData* tsd = (struct TrendSignalData*)(m_trendSignalData + sizeof(struct TrendSignalData) * m_trendSignalDataCount);

			// Fill the TrendData
			//
			(void)memset(tsd, 0, sizeof(struct TrendSignalData));
			tsd->appSignalIndex = (alt_u16)item->trend.trendSignals[si].appSignalIndex;
			tsd->validityAppSignalIndex = (alt_u16)item->trend.trendSignals[si].validityAppSignalIndex;
			tsd->columnCount = (alt_u16)item->trend.columnCount;
			tsd->durationSecs = durationSecs;

			item->trend.trendSignals[si].trendSignalDataRecordIndex = m_trendSignalDataCount;
			
			m_trendSignalDataCount++;

			// Advance the memory usage
			//
			*bytesUsed += sizeof(struct TrendSignalData);

			memoryAvailable = DEC_MEMORY(memoryAvailable, trendSignalDataSize);
		}
	}

	return true;
}

bool allocateTrendsMemory(alt_u8* const startAddr, alt_u32 memoryAvailable, alt_u32* bytesUsed)
{
	if ((startAddr == NULL) || (bytesUsed == NULL))
	{
		RN_ASSERT(startAddr != NULL);
		RN_ASSERT(bytesUsed != NULL);
		return false;
	}

	*bytesUsed = 0;
	int trendsCount = 0;

	alt_u8* dataAddr = startAddr;

	m_trendSignalData = dataAddr;
	m_trendSignalDataCount = 0;

	alt_u16 count = vduConfig()->schemaCount;
	for (alt_u16 s = 0; s < count; s++)
	{
		struct VduSchemaDescriptor* const schema = vduSchemaByIndex(s);
		if (schema == NULL)
		{
			RN_ASSERT(schema != NULL);
			return false;
		}

		for (alt_u16 it = 0; it < schema->schemaFile->schemaItemCount; it++)
		{
			struct VduSchemaFileSchemaItem1* const item = vduSchemaItem(schema, it);
			if (item->itemType != TYPE_TREND)
			{
				continue;
			}

			trendsCount++;

			if (item->trend.version != TYPE_TREND_VERSION)
			{
				char msg[STRING_MAX_LEN];
				(void)snprintf(msg,
							   STRING_MAX_LEN,
							   "allocateTrendsMemory: unsupported trend version %u, expected %u.",
							   (unsigned int)item->trend.version,
							   (unsigned int)TYPE_TREND_VERSION);
				logError(InternalError, msg);
				return false;
			}
			
			if (trendIsConsistent(&item->trend) == false)
			{
				char msg[STRING_MAX_LEN];
				(void)snprintf(msg,
							   STRING_MAX_LEN,
							   "allocateTrendsMemory: trend object %s is inconsistent.",
							   vduSchemaString(schema, item->label));

				logError(InternalError, msg);
				return false;
			}

			// Create TrendSignalData structures for in-range durations
			//
			for (alt_u16 i = 0; i < TREND_MAX_DURATIONS_COUNT; i++)
			{
				if ((item->trend.durationsSecs[i] < TREND_MIN_DURATION) || (item->trend.durationsSecs[i] > TREND_MAX_DURATION))
				{
					continue;
				}

				alt_u32 trendSignalDataSize = 0u;
				if (allocateTrendSignalData(item, item->trend.durationsSecs[i], memoryAvailable, &trendSignalDataSize) == false)
				{
					char msg[STRING_MAX_LEN];
					(void)snprintf(msg,
								   STRING_MAX_LEN,
								   "allocateTrendsMemory: insufficient memory for trend %s signals.",
								   vduSchemaString(schema, item->label));

					logError(InternalError, msg);
					return false;
				}

				dataAddr += trendSignalDataSize;
				*bytesUsed += trendSignalDataSize;

				memoryAvailable = DEC_MEMORY(memoryAvailable, trendSignalDataSize);
			}

		}
	}

	// Allocate memory in TrendState structures
	//
	for (alt_u16 di = 0; di < m_trendSignalDataCount; di++)
	{
		struct TrendSignalData* tsd = (struct TrendSignalData*)(m_trendSignalData + sizeof(struct TrendSignalData) * di);

		if (memoryAvailable < sizeof(struct TrendColumnData) * tsd->columnCount)
		{
			logError(InternalError, "allocateTrendsMemory: insufficient memory for trends data.");
			return false;
		}

		tsd->columns = (struct TrendColumnData*)dataAddr;
		(void)memset(tsd->columns, 0, sizeof(struct TrendColumnData) * tsd->columnCount);

		dataAddr += sizeof(struct TrendColumnData) * tsd->columnCount;
		*bytesUsed += sizeof(struct TrendColumnData) * tsd->columnCount;
		
		memoryAvailable = DEC_MEMORY(memoryAvailable, sizeof(struct TrendColumnData) * tsd->columnCount);
	}

	char msg[STRING_MAX_LEN];
	(void)snprintf(msg, STRING_MAX_LEN, "allocateTrendsMemory: total %d trends, %u trend signals data structures allocated, %u KBytes used.",
		trendsCount,
		m_trendSignalDataCount,
		(unsigned int)(*bytesUsed) / 1024u);
	logMessage(msg);

	return true;
}

void trendsProc(void)
{
	if (m_moduleInitialized == false)
	{
		RN_ASSERT_MSG(false, "rn_trendproc is not initialized.");
		return;
	}

	if (m_trendSignalData == NULL)
	{
		RN_ASSERT(m_trendSignalData);
		return;
	}

	// Count the number of seconds elapsed from the beginning
	//

	alt_u64 curTimeSecs = currentTimeInSecs();

	m_workTimeSecs = curTimeSecs - m_initTimeSecs;

	externalBusBridgeReadLock();

	// Process trend signal data
	//
	for (alt_u16 i = 0; i < m_trendSignalDataCount; i++)
	{
		processTrendData(i, curTimeSecs);
	}

	externalBusBridgeReadUnlock();

	return;
}

alt_u16 trendSignalDataCount(void)
{
	return m_trendSignalDataCount;
}

const struct TrendSignalData* const trendSignalData(alt_u16 index)
{
	if (m_trendSignalData == NULL) 
	{
		RN_ASSERT(m_trendSignalData);
		return NULL;
	}

	if (index >= m_trendSignalDataCount)
	{
		RN_ASSERT(false);
		return NULL;
	}

	struct TrendSignalData* tsd = (struct TrendSignalData*)(m_trendSignalData + sizeof(struct TrendSignalData) * index);
	return tsd;
}

struct TrendSignalData* const trendSignalDataMutable(alt_u16 index)
{
	if (m_trendSignalData == NULL)
	{
		RN_ASSERT(m_trendSignalData);
		return NULL;
	}

	if (index >= m_trendSignalDataCount)
	{
		RN_ASSERT(false);
		return NULL;
	}

	struct TrendSignalData* tsd = (struct TrendSignalData*)(m_trendSignalData + sizeof(struct TrendSignalData) * index);
	return tsd;
}

static void processTrendData(alt_u16 ti, alt_u64 timeCode)
{
	struct TrendSignalData* const tsd = trendSignalDataMutable(ti);
	if (tsd == NULL)
	{
		RN_ASSERT(tsd != NULL);
		return;
	}

	if ((tsd->columnCount == 0u) || (tsd->durationSecs == 0u))
	{
		RN_ASSERT(tsd->columnCount != 0u);
		RN_ASSERT(tsd->durationSecs != 0u);
		return;
	}

	const alt_u32 trendSecs = (alt_u32)(m_workTimeSecs % tsd->durationSecs);

	const float colsPerSecondF = (float)tsd->columnCount / (float)tsd->durationSecs;	// columns per second

	const float trendColumnF = (float)trendSecs * colsPerSecondF;
	const alt_u16 trendColumn = (alt_u16)trendColumnF;

	tsd->currentColumn = trendColumn;

	tsd->currentTime = currentTime();

	const struct VduAppSignalParam* const asp = appSignalParamByIndex((alt_u16)tsd->appSignalIndex);
	if (asp == NULL)
	{
		RN_ASSERT(asp != NULL);
		return;
	}

	// Get the value and set the validity flag
	//
	VduValue value = { 0 };
	bool valid = signalAvailableByIndex((alt_u16)tsd->appSignalIndex);

	if ((valid == true) && (tsd->validityAppSignalIndex != INVALID_SIGNAL_INDEX))
	{
		// Check the validity signal
		//
		const struct VduAppSignalParam* const vsp = appSignalParamByIndex((alt_u16)tsd->validityAppSignalIndex);
		if (vsp == NULL)
		{
			RN_ASSERT(vsp != NULL);
			return;
		}

		if (vsp->vduSignalType != (alt_u16)Discrete)
		{
			valid = false;
		}
		else
		{
			alt_32 valRes = 0;

			valid &= readSignalValueIntByIndex(tsd->validityAppSignalIndex, &valRes);
			if ((valid == true) && (valRes == 0))
			{
				valid = false;
			}
		}
	}

	// Request the signal value
	//
	switch (asp->vduSignalType)
	{
	case AnalogFloat32:
	{
		valid &= readSignalValueFloatByIndex(tsd->appSignalIndex, &value.data.f32);

		// Check the limits
		//
		if (valid == true)
		{
			if (asp->lowLimit.floatValue < asp->highLimit.floatValue)
			{
				// Low < High
				if ((value.data.f32 < asp->lowLimit.floatValue) || (value.data.f32 > asp->highLimit.floatValue))
				{
					valid = false;
				}
			}
			else
			{
				// Low > High
				if ((value.data.f32 < asp->highLimit.floatValue) || (value.data.f32 > asp->lowLimit.floatValue))
				{
					valid = false;
				}
			}
		} 
	}
	break;
	case AnalogSignedInt32:
	case Discrete:
	{
		valid &= readSignalValueIntByIndex(tsd->appSignalIndex, &value.data.i32);
		
		if (valid == true)
		{
			if (asp->lowLimit.intValue < asp->highLimit.intValue)
			{
				// Low < High
				if ((value.data.i32 < asp->lowLimit.intValue) || (value.data.i32 > asp->highLimit.intValue))
				{
					valid = false;
				}
			}
			else
			{
				// Low > High
				if ((value.data.i32 < asp->highLimit.intValue) || (value.data.i32 > asp->lowLimit.intValue))
				{
					valid = false;
				}
			}
		}
	}
	break;
	default:
	{
	}
	break;
	}

	if (tsd->prevColumn != trendColumn)
	{
		tsd->prevColumn = trendColumn;
		tsd->columns[trendColumn].valid = false;
	}

	if (valid == true)
	{
		// Initialize the next column
		//
		if (tsd->columns[trendColumn].valid == false)
		{
			tsd->columns[trendColumn].minValue = tsd->prevValueValid ? tsd->prevValue : value;
			tsd->columns[trendColumn].maxValue = tsd->prevValueValid ? tsd->prevValue : value;

			tsd->columns[trendColumn].valid = true;
			tsd->columns[trendColumn].timeCodeSecs = timeCode;
		}

		// Write the signal value
		//
		if (asp->vduSignalType == (alt_u16)AnalogFloat32)
		{
			// AnalogFloat32
			//
			if (value.data.f32 < tsd->columns[trendColumn].minValue.data.f32)
			{
				tsd->columns[trendColumn].minValue = value;
			}

			if (value.data.f32 > tsd->columns[trendColumn].maxValue.data.f32)
			{
				tsd->columns[trendColumn].maxValue = value;
			}
		}
		else if ((asp->vduSignalType == (alt_u16)AnalogSignedInt32) || (asp->vduSignalType == (alt_u16)Discrete))
		{
			// AnalogSignedInt32 or Discrete
			//
			if (value.data.i32 < tsd->columns[trendColumn].minValue.data.i32)
			{
				tsd->columns[trendColumn].minValue = value;
			}

			if (value.data.i32 > tsd->columns[trendColumn].maxValue.data.i32)
			{
				tsd->columns[trendColumn].maxValue = value;
			}
		}
		else
		{
		}
	}

	// Fill the gap if one second is contained in several columns
	//
	if (colsPerSecondF > 1.0)
	{
		const float colsPerSecondAdvF = colsPerSecondF + 1.0f;
		const alt_u16 colsPerSecondAdv = (alt_u16)(colsPerSecondAdvF);

		for (alt_u16 i = 1; i < colsPerSecondAdv; i++)
		{
			alt_u16 gc = tsd->currentColumn + i;
			if (gc >= tsd->columnCount)
			{
				gc -= tsd->columnCount;
			}

			tsd->columns[gc].valid = valid;
			tsd->columns[gc].minValue = value;
			tsd->columns[gc].maxValue = value;
			tsd->columns[gc].timeCodeSecs = timeCode;

		}

		tsd->currentColumn += (colsPerSecondAdv - 1u);
		if (tsd->currentColumn >= tsd->columnCount)
		{
			tsd->currentColumn -= tsd->columnCount;
		}
	}

	tsd->prevValue = value;
	tsd->prevValueValid = valid;

	return;
}

