/*
Module title: Diagnostics.

(C) 2026 RPC "Radiy".
All rights reserved.
The information presented here is confidential
and is the intellectual property of PJSC "RPC 'Radiy".
Any use of this information is strictly
prohibited without the written consent of PJSC "RPC "Radiy".

Module description: Provides diagnostics functions.

-- *************************************************************
-- Version v1.00
-- Date:	01,05,2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966, https://jira.radiy.com/browse/RADIX-3969
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-3969, add logging and debugging support
-- Add logError, logWarning, logMessage function to logging all modules
-- Add logMemoryUsage to log memory usage
-- Add errorCounter, warningCounter functions
-- Add LuaGlobalScript error to error code message
-- Add setLogRecordHandler function
-- Add setStartupStage function
-- Add startup stages for all modules
-- Add writeExternalDiagPacket function
-- Add version history of module descriptions
-- Added performance counters support to display FPS and scripts executing time

-- *************************************************************
-- Version v1.01
-- Date:	25,04,2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-4095
-- Reviewed by: Mazyr S.M.

-- Changes: RADIX-4095, rn_diag improve memory allocation control
-- Add memory allocation control
-- *************************************************************

-- Version v1.02
-- Date:	28,07,2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-3966, code is made MISRA-2025 compatible

-- *************************************************************
*/

#include "rn_diag.h"

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#ifndef RVDU_SIM // Microsoft Visual Studio-specific

#include "sys/alt_stdio.h"

#endif

#include "rn_cfg.h"
#include "rn_utils.h"

#include "defs.h"

// Private module data
//
static bool m_moduleInitialized = false;

static alt_u16 m_errorCounters[ErrorCodeCount] = { 0 };
static alt_u16 m_warningCounters[WarningCodeCount] = { 0 };

static alt_u32 m_timerValue[TIMER_COUNT] = { 0 };
static alt_u32 m_timerMaxValue[TIMER_COUNT] = { 0 };

static alt_u16 m_startupStage = 0;

static LogRecord* m_logRecords = NULL;
static alt_u16 m_logRecordsSize = 0;
static alt_u32 m_logCounter = 0;

#define NO_LOG_POS 65535u
static alt_u16 m_logStart = NO_LOG_POS;
static alt_u16 m_logEnd = NO_LOG_POS;
static bool m_logFull = false;

static const char* m_errorCodeMsg[] =
{
	"InternalError",

	"OpenFileError",
	"ReadFileError",
	"FileFormatError",
	"UnexpectedEOF",
	"FileSizeError",
	"UnsupportedVersion",

	"StringTooLong",
	"TooManySymbols",

	"EquipmentKeyWasNotFound",
	"GlobalScriptError",

};

static const char* m_warningCodeMsg[] =
{
	"InternalWarning",

	"SignalReadTypeWrong",
	"SignalWriteTypeWrong",

	"SchemaWasNotFound",
	"SignalWasNotFound",
	"ItemWasNotFound",
	"ImageWasNotFound",

	"WrongImageIndex",
	"WrongImageId",

	"LuaPreDrawScript",
	"LuaClickScript",
	"LuaOnShowScript",

	"DrawError"
};

#ifdef RVDU_SIM // Simulator-specific
LogRecordHandler m_logRecordHandler = NULL;
#endif

//
// End of Private data

bool moduleRnDiagInitialize(alt_u8* logRecordsBuffer, alt_u16 logRecordsSize, alt_u32 memoryAvailable, alt_u32* bytesUsed)
{
	if (m_moduleInitialized == true)
	{
		return false;
	}
	m_moduleInitialized = true;

#ifdef RVDU_SIM // Microsoft Visual Studio-specific
	static_assert((unsigned long long)ErrorCodeCount == (sizeof(m_errorCodeMsg) / sizeof(const char*)), "ErrorCodeCount shall match the number of decsriptions");
	static_assert((unsigned long long)WarningCodeCount == (sizeof(m_warningCodeMsg) / sizeof(const char*)), "WarningCodeCount shall match the number of decsriptions");
#endif

	if (memoryAvailable < (alt_u32)(logRecordsSize * sizeof(LogRecord)))
	{
		return false;
	}

	m_logRecords = (LogRecord*)logRecordsBuffer;
	m_logRecordsSize = logRecordsSize;

	*bytesUsed = logRecordsSize * sizeof(LogRecord);

	return true;
}

alt_u16 logRecordCount(void)
{
	if ((m_logStart == NO_LOG_POS) || (m_logEnd == NO_LOG_POS))
	{
		return 0u;
	}

	return m_logFull ? m_logRecordsSize : (m_logEnd - m_logStart + 1u);
}

const LogRecord* const logRecord(alt_u16 index)
{
	if ((m_logStart == NO_LOG_POS) || (m_logEnd == NO_LOG_POS) || (index >= logRecordCount()))
	{
		return NULL;
	}

	alt_u16 ri = (index + m_logStart) % m_logRecordsSize;
	return &m_logRecords[ri];
}

static void replaceChar(char* str, char find, char replace) 
{
	for (int i = 0; str[i] != '\0'; i++) 
	{
		if (str[i] == find) 
		{
			str[i] = replace;
		}
	}
}

static inline void addLogRecord(enum LogRecordType type, int code, const char* message)
{
	if (m_moduleInitialized == false)
	{
		return;
	}
	if ((m_logRecords == NULL) || (m_logRecordsSize == 0u) || (message == NULL))
	{
		return;
	}

#ifdef RVDU_SIM // Simulator-specific
	if (m_logRecordHandler != NULL)
	{
		m_logRecordHandler(message);
	}
#endif
	
	if (m_logEnd == NO_LOG_POS)
	{
		m_logEnd = 0;
	}
	else
	{
		m_logEnd++;
		if (m_logEnd >= m_logRecordsSize)
		{
			m_logEnd -= m_logRecordsSize;
			m_logFull = true;
		}
	}

	if (m_logStart == NO_LOG_POS)
	{
		m_logStart = 0;
	}
	else
	{
		if (m_logFull == true)
		{
			m_logStart++;
		}
	}
	if (m_logStart >= m_logRecordsSize)
	{
		m_logStart -= m_logRecordsSize;
	}
	
	m_logRecords[m_logEnd].type = type;
	m_logRecords[m_logEnd].code = code;

	(void)memset(m_logRecords[m_logEnd].text, 0, MAX_LOG_RECORD_LENTGH);
	(void)strncpy(m_logRecords[m_logEnd].text, message, MAX_LOG_RECORD_LENTGH);

	replaceChar(m_logRecords[m_logEnd].text, '\n', ' ');
	replaceChar(m_logRecords[m_logEnd].text, '\r', ' ');
}

void logError(enum ErrorCode code, const char* message)
{
	if (message == NULL)
	{
		return;
	}

	if (((int)code < 0) || ((int)code >= (int)ErrorCodeCount))
	{
		return;
	}

	m_errorCounters[code]++;

	static char msg[STRING_MAX_LEN];
	(void)snprintf(msg, STRING_MAX_LEN, "%08u ERR %s: %s", (unsigned int)m_logCounter, m_errorCodeMsg[(int)code % (int)ErrorCodeCount], message);
	m_logCounter++;

	addLogRecord(Error, (int)code, msg);

	alt_printf("%s\n", msg);
}

void logWarning(enum WarningCode code, const char* message)
{
	if (message == NULL)
	{
		return;
	}

	if (((int)code < 0) || ((int)code >= (int)WarningCodeCount))
	{
		return;
	}

	m_warningCounters[code]++;

	static char msg[STRING_MAX_LEN];
	(void)snprintf(msg, STRING_MAX_LEN, "%08u WRN %s: %s", (unsigned int)m_logCounter, m_warningCodeMsg[(int)code % (int)WarningCodeCount], message);
	m_logCounter++;

	addLogRecord(Warning, (int)code, msg);
}

void logMessage(const char* message)
{
	if (message == NULL)
	{
		return;
	}

	static char msg[STRING_MAX_LEN];
	(void)snprintf(msg, STRING_MAX_LEN, "%08u MSG: %s", (unsigned int)m_logCounter, message);
	m_logCounter++;

	addLogRecord(Message, 0, msg);

	alt_printf("%s\n", msg);
}

void logMemoryUsage(const char* const functionName, const alt_u8* const memoryStart, const alt_u8* const memoryEnd, alt_u32 memorySize)
{
	if ((functionName == NULL) ||
		(memoryStart == NULL) ||
		(memoryEnd == NULL) ||
		(memorySize == 0u))
	{
		return;
	}

	ptrdiff_t offset = memoryEnd - memoryStart;
	if (offset < 0)
	{
		return;
	}
	alt_u32 memoryUsed = (alt_u32)offset;

	char msg[STRING_MAX_LEN];
	(void)snprintf(msg, STRING_MAX_LEN, "%s: %u of %u (%.1f percent(s)) kBytes of memory used",
		functionName,
		(unsigned int)(memoryUsed / 1024u),
		(unsigned int)(memorySize / 1024u),
		(float)memoryUsed / (float)memorySize * 100.0f);
	logMessage(msg);
}

alt_u32 logErrorCount(void)
{
	alt_u32 result = 0;
	for (int i = 0; i < (int)ErrorCodeCount; i++)
	{
		result += m_errorCounters[i];
	}

	return result;
}

alt_u32 logWarningCount(void)
{
	alt_u32 result = 0;
	for (int i = 0; i < (int)WarningCodeCount; i++)
	{
		result += m_warningCounters[i];
	}

	return result;
}

void logClearCounters(void)
{
	for (int i = 0; i < (int)ErrorCodeCount; i++)
	{
		m_errorCounters[i] = 0;
	}
	for (int i = 0; i < (int)WarningCodeCount; i++)
	{
		m_warningCounters[i] = 0;
	}
}

alt_u16 errorCounter(enum ErrorCode code)
{
	return m_errorCounters[(int)code % (int)ErrorCodeCount];
}

alt_u16 warningCounter(enum WarningCode code)
{
	return m_warningCounters[(int)code % (int)WarningCodeCount];
}

const char* errorCodeMsg(enum ErrorCode code)
{
	return m_errorCodeMsg[(int)code % (int)ErrorCodeCount];
}

const char* warningCodeMsg(enum WarningCode code)
{
	return m_warningCodeMsg[(int)code % (int)WarningCodeCount];
}

void setStartupStage(alt_u16 value)
{
	m_startupStage = value;

#ifndef RVDU_SIM // Microsoft Visual Studio-specific
	writeEBB16(ADDR_DIAG_STAGE, value);
#endif
}

void setTimerValue(alt_u8 timerIndex, alt_u32 time)
{
	if ((timerIndex == 0u) || (timerIndex >= TIMER_COUNT))
	{
		return;
	}
	m_timerValue[timerIndex] = time;
	
	if (time > m_timerMaxValue[timerIndex])
	{
		m_timerMaxValue[timerIndex] = time;
	}
}

alt_u32 timerValue(alt_u8 timerIndex)
{
	if ((timerIndex == 0u) || (timerIndex >= TIMER_COUNT))
	{
		return 0;
	}
	return m_timerValue[timerIndex];
}

alt_u32 timerMaxValue(alt_u8 timerIndex)
{
	if ((timerIndex == 0u) || (timerIndex >= TIMER_COUNT))
	{
		return 0;
	}
	return m_timerMaxValue[timerIndex];
}

void writeExternalDiagPacket(void)
{
	static_assert(ADDR_DIAG_ERRORS_SIZE >= (alt_u32)ErrorCodeCount, "ADDR_DIAG_ERRORS_SIZE is incorrect");
	static_assert(ADDR_DIAG_WARNINGS_SIZE >= (alt_u32)WarningCodeCount, "ADDR_DIAG_WARNINGS_SIZE is incorrect");

#ifndef RVDU_SIM // Nios-specific
	externalBusBridgeWriteLock();
#endif 

	writeEBB16(ADDR_DIAG_BUS_BASE, m_startupStage);

	writeEBB16(ADDR_DIAG_LOOP_TIMER, (alt_u16)(m_timerValue[TIMER_MAINLOOP] & 0xffffu));
	writeEBB16(ADDR_DIAG_LOOP_TIMERMAX, (alt_u16)m_timerMaxValue[TIMER_MAINLOOP] & 0xffffu);

	writeEBB16(ADDR_DIAG_PREDRAW_TIMER, (alt_u16)m_timerValue[TIMER_PREDRAW] & 0xffffu);
	writeEBB16(ADDR_DIAG_PREDRAW_TIMERMAX, (alt_u16)m_timerMaxValue[TIMER_PREDRAW] & 0xffffu);

	writeEBB16(ADDR_DIAG_ERRORS_FLAGS, (logErrorCount() == 0u) ? 0u : 1u);
	
	for (alt_u16 i = 0; i < (alt_u16)ErrorCodeCount; i++)
	{
		writeEBB16(ADDR_DIAG_ERRORS_BASE + i, errorCounter(i));
	}
	
	for (alt_u16 i = 0; i < (alt_u16)WarningCodeCount; i++)
	{
		writeEBB16(ADDR_DIAG_WARNINGS_BASE + i, warningCounter(i));
	}

#ifndef RVDU_SIM // Nios-specific
	externalBusBridgeWriteUnlock();
#endif 
}

#ifdef RVDU_SIM // Simulator-specific
void setLogRecordHandler(LogRecordHandler handler)
{
	m_logRecordHandler = handler;
}
#endif

