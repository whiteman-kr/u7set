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

#include <errno.h>

void do_assert(bool condition);

void rnAssertImpl(bool condition, const char* expr, const char* file, int line, const char* msg)
{
	if (!condition)
	{
		char buf[STRING_MAX_LEN];
		if (msg != NULL)
		{
			(void)snprintf(buf, STRING_MAX_LEN, "Assertion failed: %s (%s:%d), %s", expr, file, line, msg);
		}
		else
		{
			(void)snprintf(buf, STRING_MAX_LEN, "Assertion failed: %s (%s:%d)", expr, file, line);
		}
		logError(InternalError, buf);

		do_assert(false);
	}
}

alt_u32 fileSize(void* f)
{
	if (f == NULL)
	{
		return WRONG_FILE_SIZE;
	}

	if (fl_fseek(f, 0L, SEEK_END) != 0)
	{
		return WRONG_FILE_SIZE;
	}

	errno = 0;
	long pos = fl_ftell(f);
	if (pos < 0)
	{
		return WRONG_FILE_SIZE;
	}

	alt_u32 size = (alt_u32)pos;

	if (fl_fseek(f, 0L, SEEK_SET) != 0)
	{
		return WRONG_FILE_SIZE;
	}

	return size;
}

bool freadBigData(alt_u8* buffer, alt_u32 size, void* file)
{
	if (size == 0u)
	{
		return true;
	}

	if ((buffer == NULL) || (file == NULL))
	{
		RN_ASSERT(buffer != NULL);
		RN_ASSERT(file != NULL);
		return false;
	}

	alt_u8* chunk = buffer;
	alt_u32 bytesToRead = size;

	while(bytesToRead > 0u)
	{
		size_t chunkSize = (bytesToRead > 512u) ? 512u : bytesToRead;

		size_t read = (size_t)fl_fread((void*)chunk, 1, (int)chunkSize, file);
		if (read != chunkSize)
		{
			return false;
		}

		chunk += chunkSize;
		bytesToRead -= (alt_u32)chunkSize;
	}

	return true;
}

alt_u64 reverseBytes64(alt_u64 val)
{
	val = ((val << 8) & 0xFF00FF00FF00FF00ULL) | ((val >> 8) & 0x00FF00FF00FF00FFULL);
	val = ((val << 16) & 0xFFFF0000FFFF0000ULL) | ((val >> 16) & 0x0000FFFF0000FFFFULL);
	return (val << 32) | (val >> 32);
}

alt_u64 calcCrc64(const alt_u8* src, alt_u32 size)
{
	const alt_u8* p = src;
	alt_u64 crc = 0xFFFFFFFFFFFFFFFFu;

	for (alt_u32 i = 0u; i < size; i++)
	{
		crc ^= (alt_u64)*p << 56;
		p++;

		for (alt_u8 j = 0; j < 8u; j++)
		{
			crc = ((crc & 0x8000000000000000ULL) != 0ULL) ? ((crc << 1ULL) ^ 0x000000000000001BULL) : (crc << 1ULL);
		}
	}

	return crc;
}

static bool isLeapYear(alt_u16 year)
{
	return (((year % 4u) == 0u) && ((year % 100u) != 0u)) || ((year % 400u) == 0u);
}

static alt_u16 leapYearsBefore(alt_u16 year)
{
	// Number of leap years before January 1 of 'year'
	year--;
	return (year / 4u) - (year / 100u) + (year / 400u);
}

static const alt_u16 daysBeforeMonth[2][12] =
{
	// Normal year
	{0,31,59,90,120,151,181,212,243,273,304,334},

	// Leap year
	{0,31,60,91,121,152,182,213,244,274,305,335}
};

static alt_u64 timeToTimestamp(const LocalTime* t)
{
	alt_u16 year = t->wYear;

	alt_u64 days =
		(((alt_u64)year - 1970u) * 365u) +
		((alt_u64)leapYearsBefore(year) - (alt_u64)leapYearsBefore(1970u));

	days += daysBeforeMonth[(isLeapYear(year) == false) ? 0 : 1][t->wMonth];
	days += t->wDay;
	days--;

	return (days * 86400ULL) +
		((alt_u64)t->wHour * 3600ULL) +
		((alt_u64)t->wMinute * 60ULL) +
		(alt_u64)t->wSecond;
}

alt_u64 currentTimeInSecs(void)
{
	LocalTime st = currentTime();
	st.wMonth--;           // (0 = January)

	time_t timestamp = timeToTimestamp(&st);
	return timestamp;
}

LocalTime localTime(alt_u64 timeCode)
{
	LocalTime result = { 0 };

	int64_t t = (int64_t)timeCode;

	// Days since epoch
	int64_t days = t / 86400;
	int64_t rem = t % 86400;

	result.wHour = (alt_u16)((alt_u64)rem / 3600u);
	rem %= 3600;

	result.wMinute = (alt_u16)((alt_u64)rem / 60u);
	result.wSecond = (alt_u16)((alt_u64)rem % 60u);

	//------------------------------------------------------------
	// Determine year
	//------------------------------------------------------------

	alt_u16 year = 1970u;

	while (true)
	{
		int64_t yearDays = (isLeapYear(year) == true) ? 366 : 365;

		if (days >= yearDays)
		{
			days -= yearDays;
			year++;
		}
		else if (days < 0)
		{
			year--;
			days += isLeapYear(year) ? 366 : 365;
		}
		else
		{
			break;
		}
	}

	result.wYear = year;

	//------------------------------------------------------------
	// Month
	//------------------------------------------------------------

	int leap = (isLeapYear(year) == true) ? 1 : 0;

	int month = 0;

	for (month = 11; month > 0; --month)
	{
		if (days >= (int64_t)daysBeforeMonth[leap][month])
		{
			break;
		}
	}

	result.wMonth = (alt_u16)month;

	int64_t wDay64 = days - (int64_t)daysBeforeMonth[leap][month] + 1LL;
	result.wDay = (alt_u16)(wDay64);

	return result;
}

void writeEBB16(alt_u16 wOffset, alt_u16 data)
{
	volatile alt_u16* p16 = (alt_u16*)TO_EXTERNAL_BUS_BRIDGE_1_BASE_WRITE + wOffset;
	*p16 = data;
	return;
}


