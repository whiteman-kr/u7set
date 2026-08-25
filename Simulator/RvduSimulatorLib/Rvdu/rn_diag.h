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

#ifndef RN_DIAG_H
#define RN_DIAG_H

#include <alt_types.h>

#include "defs.h" 

#include <stdbool.h>

enum ErrorCode
{
	InternalError,

	OpenFileError,
	ReadFileError,
	FileFormatError,
	UnexpectedEOF,
	FileSizeError,
	UnsupportedVersion,

	StringTooLong,
	TooManySymbols,

	EquipmentKeyWasNotFound,

	GlobalScriptError,

	ErrorCodeCount
};

enum WarningCode
{
	InternalWarning,

	SignalReadTypeWrong,
	SignalWriteTypeWrong,

	SchemaWasNotFound,
	SignalWasNotFound,
	ItemWasNotFound,
	ImageWasNotFound,

	WrongImageIndex,
	WrongImageId,

	LuaPreDrawScript,
	LuaClickScript,

	DrawError,

	WarningCodeCount
}; 

// Diagnostics addresses
//
#define ADDR_DIAG_BUS_BASE  20485u
#define ADDR_DIAG_BUS_SIZE  50u

#define ADDR_DIAG_STAGE         	ADDR_DIAG_BUS_BASE      // RN_Startup_Stage

#define ADDR_DIAG_LOOP_TIMER    	ADDR_DIAG_BUS_BASE + 5u  // RN_Schema_Counter
#define ADDR_DIAG_LOOP_TIMERMAX     ADDR_DIAG_BUS_BASE + 6u  // RN_Max_Schema_Counter

#define ADDR_DIAG_PREDRAW_TIMER    	ADDR_DIAG_BUS_BASE + 7u  // RN_Predraw_Counter
#define ADDR_DIAG_PREDRAW_TIMERMAX  ADDR_DIAG_BUS_BASE + 8u  // RN_Max_Predraw_Counter

#define ADDR_DIAG_ERRORS_FLAGS  	ADDR_DIAG_BUS_BASE + 12u // Error flags (0 - RN_Fatal_error, T1)

#define ADDR_DIAG_ERRORS_BASE   	ADDR_DIAG_BUS_BASE + 13u // Errors counters
#define ADDR_DIAG_ERRORS_SIZE   	19u

#define ADDR_DIAG_WARNINGS_BASE 	ADDR_DIAG_BUS_BASE + 32u // Warnings counters
#define ADDR_DIAG_WARNINGS_SIZE 	20u

#define MAX_LOG_RECORD_LENTGH 200u

enum LogRecordType
{
	Error, 
	Warning,
	Message
};

typedef struct
{
	enum LogRecordType type;
	int code;
	char text[MAX_LOG_RECORD_LENTGH + 1u/*for null-terminate*/];
} LogRecord;

// Startup stages
//
#define STAGE_MEM_INIT	0
#define STAGE_MEM_ERR	1

#define STAGE_TS_INIT	2
#define STAGE_KBD_INIT	4
#define STAGE_LOG_INIT	6

#define STAGE_SD_INIT	8
#define STAGE_SD_ERR	10
#define STAGE_SD_FS_ERR 12

#define STAGE_FAT_INIT	14
#define STAGE_FAT_ERR	16

#define STAGE_EQUIPMENT_INIT	18
#define STAGE_EQUIPMENT_ERR		20

#define STAGE_SHOWLOGO	22

#define STAGE_CFG_READ	24
#define STAGE_CFG_ERR	26
#define STAGE_SCREEN_ERR	27

#define STAGE_GLOBALSCRIPT_INIT	28
#define STAGE_GLOBALSCRIPT_ERR	30

#define STAGE_LUA_INIT	32
#define STAGE_LUA_INIT_ERR	33

#define STAGE_GLOBALSCRIPT_COMPILE	34
#define STAGE_GLOBALSCRIPT_COMPILE_ERR 36

#define STAGE_SIGNALS_INIT	38
#define STAGE_SIGNALS_ERR	40

#define STAGE_FONTS_INIT	42
#define STAGE_FONTS_ERR		44

#define STAGE_SCHEMAS_INIT	46
#define STAGE_SCHEMAS_ERR	48

#define STAGE_SET_DEFAULT_SCHEMA	50
#define STAGE_SET_DEFAULT_SCHEMA_ERR 52

#define STAGE_RUNNING	255
//
// Startup stages

// Timers
//
#define TIMER_MAINLOOP 	1u
#define TIMER_PREDRAW	2u

#define DEC_MEMORY(memoryAvailable, size) ((memoryAvailable) > (size) ? (memoryAvailable) - (size) : 0u)

/*
Function of initalizing memory for the log.
Parameters:
	logRecordsBuffer - pointer to log buffer.
	logRecordsSize - max log records size.
	memoryAvailable - available memory in bytes
Returns:
	bytesUsed - a value that receives the used memory size
	Returns true on success, false on memory allocation error

*/
bool moduleRnDiagInitialize(alt_u8* logRecordsBuffer, alt_u16 logRecordsSize, alt_u32 memoryAvailable, alt_u32* bytesUsed);

// Logging functions
//
alt_u16 logRecordCount(void);
const LogRecord* const logRecord(alt_u16 index);

void logError(enum ErrorCode code, const char* message);
void logWarning(enum WarningCode code, const char* message);
void logMessage(const char* message);
void logMemoryUsage(const char* const functionName, const alt_u8* const memoryStart, const alt_u8* const memoryEnd, alt_u32 memorySize);

// Error counters functions
//
alt_u32 logErrorCount(void);
alt_u32 logWarningCount(void);
void logClearCounters(void);

alt_u16 errorCounter(enum ErrorCode code);
alt_u16 warningCounter(enum WarningCode code);

const char* errorCodeMsg(enum ErrorCode code);
const char* warningCodeMsg(enum WarningCode code);

// Function that set startup stage code RVDU log, shall be called once before setLogRecordHandler
//
void setStartupStage(alt_u16 value);

// Functions that sets and gets the timer value in milliseconds.
//
void setTimerValue(alt_u8 timerIndex, alt_u32 time);

alt_u32 timerValue(alt_u8 timerIndex);

alt_u32 timerMaxValue(alt_u8 timerIndex);

// Function that writes the diagnostics data to external diagnostics.
//
void writeExternalDiagPacket(void);

// Function that can be write log messages into log file
//
typedef void (*LogRecordHandler)(int, const char*);
void setLogRecordHandler(LogRecordHandler handler);

#endif
