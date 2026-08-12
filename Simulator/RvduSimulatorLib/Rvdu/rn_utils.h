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

#ifndef RN_UTILS_H
#define RN_UTILS_H

#include <stdint.h>
#include <stdbool.h>

// Microsoft Visual Studio-specific
#ifdef RVDU_SIM

#include "msvc_alt_types.h"

#else

#include <alt_types.h>
#include <system.h>
#include <stddef.h>

#define TO_EXTERNAL_BUS_BRIDGE_1_BASE_READ TO_EXTERNAL_BUS_BRIDGE_1_BASE
#define TO_EXTERNAL_BUS_BRIDGE_1_BASE_WRITE TO_EXTERNAL_BUS_BRIDGE_1_BASE

#endif


#define WRONG_FILE_SIZE (alt_u32)-1

typedef struct LocalTime {
    alt_u16 wYear;
    alt_u16 wMonth;
    alt_u16 wDay;
    alt_u16 wHour;
    alt_u16 wMinute;
    alt_u16 wSecond;
    alt_u16 wMilliseconds;
} LocalTime;


// Logging assert function
//
void rnAssertImpl(bool condition, const char* expr, const char* file, int line, const char* msg);

#define RN_ASSERT(expr) rnAssertImpl((expr), #expr, __FILE__, __LINE__, NULL)
#define RN_ASSERT_MSG(expr, msg) rnAssertImpl((expr), #expr, __FILE__, __LINE__, msg)

// Hash calculating functions

/*
FNV1A_Jesteress hash function for 32-bit hash.
Parameters:
    data - pointer to a buffer with bytes to process.
    bytesCount - size, in bytes, of the buffer.
Return Value:
    Returns the calculated hash.
*/
static inline alt_u32 calcHash32(const alt_u8* data, const alt_u8 bytesCount)
{
    const alt_u8* dataPtr = data;
    const alt_u32 fnvPrime = 16777619U;          // FNV prime number
    const alt_u32 offsetBasis = 2166136261U;     // FNV start value
    alt_u32 hash = offsetBasis;
    alt_u8 copyBytesCount = bytesCount;

    while (copyBytesCount > 0U)
    {
        hash ^= *dataPtr;
        dataPtr++;
        hash *= fnvPrime;
        hash ^= (hash >> 1);
        copyBytesCount--;
    }
    // Final mixing
    //
    hash ^= (hash << 7);
    hash ^= (hash >> 3);
    hash ^= (hash << 17);
    return hash;
}

static inline alt_u64 calcHash64(const char* data, size_t byteSize)
{
    alt_u64 hash = 0;
    const char* prt = data;

    for (size_t i = 0u; i < byteSize; i++)
    {
        hash += (hash << 5) + *prt;
        prt++;
    }

    return hash;
}

// Upward align the input address to a 4-byte boundary (32-bit / DWORD alignment)
//
static inline alt_u32 alignAddressToDword(const alt_u32 address)
{
    if ((address & 0b11u) != 0u)
    {
        return (address & 0xfffffffcu) + 0b100u;
    }

    return address;
}

/*
The function converts a 32-bit address into a pointer to an unsigned 32-bit
integer to provide access to the processor's peripherals. Because the standard
prohibits converting integers into a pointer, it is used with a directive.
Parameters:
    base - 32-bit address from processor address space.
Return Value:
    Returns pointer to address from input parameter.
*/
inline alt_u32* hwBaseToU32ptr(const alt_u32 base)
{
    return (alt_u32*)(uintptr_t)base;
}

// This functions writes comparsion ID to the RVDU memory
//
void writeComparsionID(const alt_u8 vduConfigID[]);

// This function returns file size in bytes. File shall be opened.
// Sets the file pointer to the beginning of the fils
// On error, returns WRONG_FILE_SIZE
//
alt_u32 fileSize(void* f);

// This function reads data of size more than 512 bytes (one sector) from file,
// splitting the operation to multiple reads
//
bool freadBigData(alt_u8* buffer, alt_u32 size, void* file);

// External bus specific addresses
//
#define ADDR_PIN_HI 20496
#define ADDR_PIN_LO 20497

#define ADDR_COMPARSION_ID_WORD0 20481u
#define ADDR_COMPARSION_ID_WORD1 20482u
#define ADDR_COMPARSION_ID_WORD2 20483u
#define ADDR_COMPARSION_ID_WORD3 20484u

#define DISCRETE_OUTPUT_COUNT 4
#define ADDR_DISCRETE_OUTPUTS_READ 20483u   // Offset, in bytes, of reading the discrete outputs state
#define ADDR_DISCRETE_OUTPUTS_WRITE 20480u  // Offset, in bytes, of writing the discrete outputs state
#define ADDR_DISCRETE_OUTPUTS_RW_BITS_DELTA 2u  // Delta, in bits, between writing and reading address of discrete outputs

#define ADDR_OPTO_VALIDITY 20491u

#define ADDR_RTU_YEAR           20484u
#define ADDR_RTU_MONTH          20485u
#define ADDR_RTU_DAY            20486u

#define ADDR_RTU_HOUR           20487u
#define ADDR_RTU_MINUTE         20488u
#define ADDR_RTU_SECOND         20489u
#define ADDR_RTU_MILLISECOND    20490u

//
// External bus specific addresses

// Lock the output buffers (TX-optical and output signals)
//
static inline void externalBusBridgeWriteLock(void)
{
#ifndef RVDU_SIM // Nios-specific
    __builtin_stwio(hwBaseToU32ptr(EBB1_WR_ACTIVE_N_BASE), 0);
#endif
}

// Unlock the output buffers (TX-optical and output signals)
//
static inline void externalBusBridgeWriteUnlock(void)
{
#ifndef RVDU_SIM // Nios-specific
    __builtin_stwio(hwBaseToU32ptr(EBB1_WR_ACTIVE_N_BASE), 1);
#endif 
}

#ifdef RVDU_SIM // Simulator-specific
extern bool m_ebbLocked;
#endif


// Lock the input buffers (RX-optical and input signals)
//
static inline void externalBusBridgeReadLock(void)
{
#ifndef RVDU_SIM // Nios-specific
    __builtin_stwio(hwBaseToU32ptr(EBB1_RD_ACTIVE_N_BASE), 0);
#else
    if (m_ebbLocked == true)
    {
        RN_ASSERT(false);
    }
    m_ebbLocked = true;
#endif
}

// Unlock the input buffers (RX-optical and input signals)
//
static inline void externalBusBridgeReadUnlock(void)
{
#ifndef RVDU_SIM // Nios-specific
    __builtin_stwio(hwBaseToU32ptr(EBB1_RD_ACTIVE_N_BASE), 1);
#else
    if (m_ebbLocked == false)
    {
        RN_ASSERT(false);
    }
    m_ebbLocked = false;
#endif
}

// Invert the output PIN that allows to control schema processing time (RS485-pin is used as free pin)
//
static inline void resetRs485SignalPin(void)
{
#ifndef RVDU_SIM // Nios-specific
    __builtin_stwio((void*)hwBaseToU32ptr((alt_u32)PIO_0_RS485_DE_BASE),
    		~((unsigned int)__builtin_ldwio((void*)hwBaseToU32ptr((alt_u32)PIO_0_RS485_DE_BASE))));
#endif
}

// Read the 16-bit value from the external bus bridge
//
static inline alt_u16 readEBB16(alt_u16 wOffset)
{
    const alt_u16* p16 = (alt_u16*)TO_EXTERNAL_BUS_BRIDGE_1_BASE_READ;
    return *(p16 + wOffset);
}

// Write the 16-bit value to the external bus bridge
//
void writeEBB16(alt_u16 wOffset, alt_u16 data);

#ifdef RVDU_SIM // Simulator-specific
// Write the 16-bit value to the external bus bridge (read area)
//
void writeReadEBB16(alt_u16 wOffset, alt_u16 data);
#endif

// Reverse byte order (e.g. big endian to little endian)
//
alt_u64 reverseBytes64(alt_u64 val);

// Calculate CRC-64
//
alt_u64 calcCrc64(const alt_u8* src, alt_u32 size);

// Measuring timer control functions
//
#define TIMER_COUNT		8u	// indexes 0..7, 0 is reserved

void resetBaseMeasuringTimer(void);

void startMeasuringTimer(alt_u8 timerIndex);

void stopMeasuringTimer(alt_u8 timerIndex);

alt_u32 measuringTimerValue(alt_u8 timerIndex);

//
// Time processing functions
//

// Returns current date and time
//
LocalTime currentTime(void);

// Returns current seconds value
//
alt_u16 currentSecs(void);

// Returns current milliseconds value
//
alt_u16 currentMSecs(void);

// Returns number of seconds passed since 01.01.1970 00:00:00
//
alt_u64 currentTimeInSecs(void);

// Returns the time structure from the time code
//
LocalTime localTime(alt_u64 timeCode);
    
// Overriding the function for systems without std-out device
//
#ifndef RVDU_SIM        

#ifndef ALT_STDOUT_PRESENT
void alt_printf(const char* fmt, ... );
#endif // ALT_STDOUT_PRESENT

#endif // RVDU_SIM        

#endif  // RN_UTILS_H
