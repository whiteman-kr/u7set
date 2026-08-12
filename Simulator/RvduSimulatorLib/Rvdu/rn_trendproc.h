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

#ifndef RN_TRENDPROC_H
#define RN_TRENDPROC_H

// Microsoft Visual Studio-specific
#ifndef RVDU_SIM

#include <alt_types.h>

#else

#include "msvc_alt_types.h"

#endif
// Microsoft Visual Studio-specific

#include <stdbool.h>

#include "defs.h"

#include "rn_scfp.h"
#include "rn_utils.h"

// State

struct TrendColumnData
{
    bool valid;
    VduValue minValue;
    VduValue maxValue;
    alt_u64 timeCodeSecs;
};

// This structure holds information about trend signals with same parameters
// It can be shared by multiple trend schema items
//
struct TrendSignalData
{
    alt_u16 appSignalIndex;
    alt_u16 validityAppSignalIndex; // 0xFFFFFFFF if no validity signal is used
    alt_u32 durationSecs;
    alt_u16 columnCount;

    alt_u16 reserved0;

    alt_u16 currentColumn;
    alt_u16 prevColumn;
    
    bool prevValueValid;
    VduValue prevValue;

    LocalTime currentTime;

    struct TrendColumnData* columns;
};

#define TREND_DEFAULT_IDENT 5u
#define TREND_MIN_DURATION 30u
#define TREND_MAX_DURATION 259200u //72 * 60 * 60

/*
Function of initalizing trends processing module.
Parameters:
*/
void moduleRnTrendProcInitialize(void);

// Trends processing functions.
//

/*
Function checks if trend object is valid and has correct parameters (version, columns count)
Parameters:
    trend  - a pointer to a trend.
Return value:
    returns true if object is consistent, otherwise returns false.
*/
bool trendIsConsistent(const VduSchemaFileSchemaItemTrend1* const trend);

/*
Function allocates memory to store trend data for each trend signal with all durations.
Trend data structure is created for each unique tuple [appSignalIndex, validityAppSignalIndex, durationSecs, columnCount].
Parameters:
    startAddr - 32-bit pointer to a buffer where to store the trend data.
    memoryAvailable - memory available for trend data.
Return Value:
    bytesUsed - receives number of actually allocated bytes. The size of each file is upward-aligned by 4 bytes (to dword).
    Returns "true" if successful, "false" on error.
*/
bool allocateTrendsMemory(alt_u8* const startAddr, alt_u32 memoryAvailable, alt_u32* bytesUsed);

/*
Function requests trend signal values, validity signal values and writes them to trend column data structures.
Each column of the trend stores validity flag, minimum and maximum signal values.
If the signal value is out of range, it is treated as non-valid.
*/
void trendsProc(void);

/*
Function returns number of unique trend data structures.
*/
alt_u16 trendSignalDataCount(void);

/*
Function returns the constant trend data structure by index or NULL if index is incorrect.
*/
const struct TrendSignalData* const trendSignalData(alt_u16 index);

/*
Function returns the mutable trend data structure by index or NULL if index is incorrect.
*/
struct TrendSignalData* const trendSignalDataMutable(alt_u16 index);

#endif
