/*
Module title: Utilities Platform.
(C) 2026 RPC "Radiy".
All rights reserved.
The information presented here is confidential
and is the intellectual property of PJSC "RPC "Radiy".
Any use of this information is strictly
prohibited without the written consent of PJSC "RPC "Radiy".
Module description: Provides platform-specific support functions for other modules (time reading, etc.).

-- *************************************************************
-- Version v1.00
-- Date:	24,08,2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Nesterenko M. V.

-- Changes: RADIX-3966, rn_utils initial update

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

#include <errno.h>

#include "../rvdu_bsp/system.h"
#include <altera_avalon_performance_counter.h>


void do_assert(bool condition)
{
}

void writeComparsionID(const alt_u8 vduConfigID[])
{
	if (vduConfigID == NULL)
	{
		RN_ASSERT(vduConfigID != NULL);
		return;
	}


	externalBusBridgeWriteLock();

	writeEBB16(ADDR_COMPARSION_ID_WORD0, vduConfigID[6] | ((alt_u16)vduConfigID[7] << 8));
	writeEBB16(ADDR_COMPARSION_ID_WORD1, vduConfigID[4] | ((alt_u16)vduConfigID[5] << 8));
	writeEBB16(ADDR_COMPARSION_ID_WORD2, vduConfigID[2] | ((alt_u16)vduConfigID[3] << 8));
	writeEBB16(ADDR_COMPARSION_ID_WORD3, vduConfigID[0] | ((alt_u16)vduConfigID[1] << 8));

	__builtin_stwio(hwBaseToU32ptr(EBB1_COMP_ID_RDY_BASE), 1);

	externalBusBridgeWriteUnlock();
}

// Lock the input buffers (RX-optical and input signals)
//
void externalBusBridgeReadLock(void)
{
	__builtin_stwio(hwBaseToU32ptr(EBB1_RD_ACTIVE_N_BASE), 0);
}

// Unlock the input buffers (RX-optical and input signals)
//
void externalBusBridgeReadUnlock(void)
{
	__builtin_stwio(hwBaseToU32ptr(EBB1_RD_ACTIVE_N_BASE), 1);
}

// Lock the output buffers (TX-optical and output signals)
//
void externalBusBridgeWriteLock(void)
{
	__builtin_stwio(hwBaseToU32ptr(EBB1_WR_ACTIVE_N_BASE), 0);
}

// Unlock the output buffers (TX-optical and output signals)
//
void externalBusBridgeWriteUnlock(void)
{
	__builtin_stwio(hwBaseToU32ptr(EBB1_WR_ACTIVE_N_BASE), 1);
}

void invertRs485SignalPin(void)
{
	__builtin_stwio((void*)hwBaseToU32ptr((alt_u32)PIO_0_RS485_DE_BASE),
		~((unsigned int)__builtin_ldwio((void*)hwBaseToU32ptr((alt_u32)PIO_0_RS485_DE_BASE))));
}

void resetBaseMeasuringTimer(void)
{
	PERF_RESET (PERFORMANCE_COUNTER_0_BASE);
	PERF_START_MEASURING (PERFORMANCE_COUNTER_0_BASE);	
}
// Start measuring timer
//
void startMeasuringTimer(alt_u8 timerIndex)
{
	if ((timerIndex == 0u) || (timerIndex >= TIMER_COUNT))
	{
		return;
	}
	PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, timerIndex);
}

// Stop measuring timer
//
void stopMeasuringTimer(alt_u8 timerIndex)
{
	if ((timerIndex == 0u) || (timerIndex >= TIMER_COUNT))
	{
		return;
	}
	PERF_END(PERFORMANCE_COUNTER_0_BASE, timerIndex);
}

alt_u32 measuringTimerValue(alt_u8 timerIndex)
{
	if ((timerIndex == 0u) || (timerIndex >= TIMER_COUNT))
	{
		return 0;
	}
	static alt_u32 CPU_FREQ_MS = ALT_CPU_FREQ / 1000;
	alt_u32 t = (alt_u32)((alt_u64)perf_get_section_time((void*)PERFORMANCE_COUNTER_0_BASE, (int)timerIndex) / (alt_u64)CPU_FREQ_MS);
	return t;
}

LocalTime currentTime(void)
{
	LocalTime tm = { 0 };

	tm.wYear = readEBB16(ADDR_RTU_YEAR);
	tm.wMonth = readEBB16(ADDR_RTU_MONTH);
	tm.wDay = readEBB16(ADDR_RTU_DAY);

	tm.wHour = readEBB16(ADDR_RTU_HOUR);
	tm.wMinute = readEBB16(ADDR_RTU_MINUTE);
	tm.wSecond = readEBB16(ADDR_RTU_SECOND);
	tm.wMilliseconds = readEBB16(ADDR_RTU_MILLISECOND);

	return tm;
}

alt_u16 currentSecs(void)
{
	return readEBB16(ADDR_RTU_SECOND);
}

alt_u16 currentMSecs(void)
{
	return readEBB16(ADDR_RTU_MILLISECOND);
}


#ifndef ALT_STDOUT_PRESENT
void alt_printf(const char* fmt, ...)
{
	return;
}
#endif // ALT_STDOUT_PRESENT


