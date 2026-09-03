/*
Module title: Utilities.
(C) 2026 RPC "Radiy".
All rights reserved.
The information presented here is confidential
and is the intellectual property of PJSC "RPC "Radiy".
Any use of this information is strictly
prohibited without the written consent of PJSC "RPC "Radiy".
Module description: Provides support functions for other modules (hash calculating, etc.).

-- *************************************************************
-- Version v1.00
-- Date:	31,03,2026.
-- Designed by: Mazyr S.M. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Vintenko B.U.

-- Changes: RADIX-3966, rn_utils initial update

-- *************************************************************
-- Version v1.01
-- Date:	19,04,2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966, https://jira.radiy.com/browse/RADIX-3969
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-3966, rn_utils module refactoring
-- Add hwBaseTo8ptr function
-- Add freadBigData function
-- Mode function prepareStr to rn_prim
-- Add reverseBytes64, calcCrc64 functions
-- Add hardware registers access functions
-- Add timers control functions

-- Changes: RADIX-3969, add logging and debugging support
-- Add log errors and messages

-- *************************************************************

-- Version v1.02
-- Date:	09,05,2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-4009
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-4009, adding API support for working with signals by AppSignalIDs
-- RADIX-3973: add real time providing functions

-- *************************************************************

-- Version v1.03
-- Date:	28,07,2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-3966, code is made MISRA-2025 compatible

-- *************************************************************
*/

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>

#include "rn_utils.h"
#include "rn_diag.h"

#include "fat_filelib.h"

#include <alt_errno.h>

static bool m_moduleInitialized = false;

#include <threads.h>
static mtx_t m_signalsReadMutex;
//bool m_ebbReadLocked = false;

static mtx_t m_signalsWriteMutex;
//bool m_ebbWriteLocked = false;

void moduleRnUtilsInitialize(void)
{
	if (m_moduleInitialized == true)
	{
		RN_ASSERT(false);
		return;
	}
	m_moduleInitialized = true;

	// Initialize critical section
	mtx_init(&m_signalsReadMutex, mtx_plain);
	mtx_init(&m_signalsWriteMutex, mtx_plain);
}

void moduleRnUtilsUninitialize(void)
{
	mtx_destroy(&m_signalsReadMutex);
	mtx_destroy(&m_signalsWriteMutex);

	m_moduleInitialized = false;
}

void do_assert(bool condition)
{
	assert(condition);
}

void writeComparsionID(const alt_u8 vduConfigID[])
{
	if (vduConfigID == NULL)
	{
		RN_ASSERT(vduConfigID != NULL);
		return;
	}

	writeEBB16(ADDR_COMPARSION_ID_WORD0, vduConfigID[6] | ((alt_u16)vduConfigID[7] << 8));
	writeEBB16(ADDR_COMPARSION_ID_WORD1, vduConfigID[4] | ((alt_u16)vduConfigID[5] << 8));
	writeEBB16(ADDR_COMPARSION_ID_WORD2, vduConfigID[2] | ((alt_u16)vduConfigID[3] << 8));
	writeEBB16(ADDR_COMPARSION_ID_WORD3, vduConfigID[0] | ((alt_u16)vduConfigID[1] << 8));

	(void)printf("writeComparsionID: id = 0x");
	for (int i = 0; i < 8; i++)
	{
		(void)printf("%02x", vduConfigID[i]);
	}
	(void)printf("\n");
}

// Lock the input buffers (RX-optical and input signals)
//
void externalBusBridgeReadLock(void)
{
	/*if (m_ebbReadLocked == true)
	{
		RN_ASSERT(false);
	}
	m_ebbReadLocked = true;*/
	mtx_lock(&m_signalsReadMutex);
}

// Unlock the input buffers (RX-optical and input signals)
//
void externalBusBridgeReadUnlock(void)
{
	/*if (m_ebbReadLocked == false)
	{
		RN_ASSERT(false);
	}
	m_ebbReadLocked = false;*/
	mtx_unlock(&m_signalsReadMutex);
}

// Lock the output buffers (TX-optical and output signals)
//
void externalBusBridgeWriteLock(void)
{
	/*if (m_ebbWriteLocked == true)
	{
		RN_ASSERT(false);
	}
	m_ebbWriteLocked = true;*/
	mtx_lock(&m_signalsWriteMutex);
}

// Unlock the output buffers (TX-optical and output signals)
//
void externalBusBridgeWriteUnlock(void)
{
	/*if (m_ebbWriteLocked == false)
	{
		RN_ASSERT(false);
	}
	m_ebbWriteLocked = false;*/
	mtx_unlock(&m_signalsWriteMutex);
}

void invertRs485SignalPin(void)
{
}

static struct timespec m_counter[TIMER_COUNT] = { 0 };
static alt_u32  m_tickCount[TIMER_COUNT] = { 0 };

void resetBaseMeasuringTimer(void)
{
}
// Start measuring timer
//
void startMeasuringTimer(alt_u8 timerIndex)
{
	if ((timerIndex == 0u) || (timerIndex >= TIMER_COUNT))
	{
		return;
	}

	(void)timespec_get(&m_counter[timerIndex], TIME_UTC);
}

// Stop measuring timer
//
void stopMeasuringTimer(alt_u8 timerIndex)
{
	if ((timerIndex == 0u) || (timerIndex >= TIMER_COUNT))
	{
		return;
	}

	struct timespec ts;
	(void)timespec_get(&ts, TIME_UTC);

	long long seconds = ts.tv_sec - m_counter[timerIndex].tv_sec;
	long long nanoseconds = ts.tv_nsec - m_counter[timerIndex].tv_nsec;

	alt_u32 elapsed = (alt_u32)((seconds * 1000) + (nanoseconds / 1000000));
	m_tickCount[timerIndex] = (alt_u32)elapsed;
}

alt_u32 measuringTimerValue(alt_u8 timerIndex)
{
	if ((timerIndex == 0u) || (timerIndex >= TIMER_COUNT))
	{
		return 0;
	}

	return m_tickCount[timerIndex];
}

LocalTime currentTime(void)
{
	LocalTime tm = { 0 };

	// Get current time with nanosecond precision
	struct timespec ts;
	(void)timespec_get(&ts, TIME_UTC);

	struct tm* t = localtime(&ts.tv_sec);

	tm.wYear = (alt_u16)(t->tm_year + 1900u);
	tm.wMonth = (alt_u16)(t->tm_mon + 1u);
	tm.wDay = (alt_u16)(t->tm_mday);

	tm.wHour = (alt_u16)(t->tm_hour);
	tm.wMinute = (alt_u16)(t->tm_min);
	tm.wSecond = (alt_u16)(t->tm_sec);
	tm.wMilliseconds = (alt_u16)(ts.tv_nsec / 1000000u);

	return tm;
}

alt_u16 currentSecs(void)
{
	struct timespec ts;
	(void)timespec_get(&ts, TIME_UTC);

	struct tm* t = localtime(&ts.tv_sec);
	return (alt_u16)t->tm_sec;
}

alt_u16 currentMSecs(void)
{
	struct timespec ts;
	(void)timespec_get(&ts, TIME_UTC);
	return (alt_u16)(ts.tv_nsec / 1000000u);
}
