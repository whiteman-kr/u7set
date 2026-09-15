/*
Module title: Font Files Processing

(C) 2026 RPC "Radiy".
All rights reserved.
The information presented here is confidential
and is the intellectual property of PJSC "RPC "Radiy".
Any use of this information is strictly
prohibited without the written consent of PJSC "RPC "Radiy".

Module description: Loads, parses, and manages font files for text rendering.

-- *************************************************************
-- Version v1.00
-- Date:	31.03.2026.
-- Designed by: Mazyr S.M. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Vintenko B.U.

-- Changes: RADIX-3966, rn_fofp initial update

-- *************************************************************
-- Version v1.01
-- Date:	06.04.2026.
-- Designed by: Mazyr S.M. https://jira.radiy.com/browse/RADIX-3966, https://jira.radiy.com/browse/RADIX-3969
-- Reviewed by: Vintenko B.U.

-- Changes: RADIX-3966, rn_fofp module refactoring
-- Add fontData, fontSymbolsCount, fontSymbolHeader function
-- Add private module data
-- Change sprintf to snprintf, strcmp to strncmp for MISRA suggestion
-- Add version history of module descriptions

-- Changes: RADIX-3969, add logging and debugging support
-- Add log errors and messages

-- Changes: RADIX-3969, add logging and debugging support
-- Add log errors and messages

-- *************************************************************
-- Version v1.02
-- Date:	25.04.2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-4095
-- Reviewed by: Mazyr S.M.

-- Changes: RADIX-4095, rn_fofp improve memory allocation control
-- Add memory allocation control
-- RADIX-3974, add 64-bit architecture support
-- *************************************************************

-- Version v1.03
-- Date:	28.07.2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-3966, code is made MISRA-2025 compatible

-- *************************************************************

-- Version v1.04
-- Date:	14.09.2026.
-- Designed by: Nesterenko M.V. https://jira.radiy.com/browse/RADIX-4116
-- Reviewed by: Vintenko B.U.

-- Changes: RADIX-4124, rn_fofp сorrecting errors identified by static analysis testing

-- *************************************************************
*/

#ifndef RN_FOFP_H
#define RN_FOFP_H

#include <alt_types.h>

#include <stdbool.h>

#define MAX_FONTS_COUNT 32u

#define RVDU_SYSTEM_FONT 0u	// Shall be small font, used to print logs and signals list

struct VduSymbolHeader
{
	alt_u32 code;
	alt_u16 width;
	alt_u16 height;
	alt_u32 offset;
};

/*
Copies the contents of the all fonts files to memory at the address specified in the parameter.
Returns "true" if the copy is successful.
Returns "false" if an error.
Parameters:
    fontsPath - Path to fonts subfolders (e.g EQUIPMENTID_/Fonts).
    startAddr - 32-bit pointer to a buffer where to store the configuration.
    memoryAvailable - memory, in bytes, available for all font files.
Return Value:
    bytesRead - receives number of actually read bytes. The size of each file is upward-aligned by 4 bytes (to dword).
    Returns "true" if successful, "false" on error.
*/
bool cacheFonts(const char* const fontsPath, alt_u8* const startAddr, alt_u32 memoryAvailable, alt_u32* bytesRead);

//
// Font data access functions
//

// Returns the font data structure for provided fontor NULL if parameters are incorrect
//
const alt_u8* const fontData(alt_u16 fontIndex);

// Returns the font symbol header structure for provided font and character or NULL if parameters are incorrect
//
const struct VduSymbolHeader* const fontSymbolHeader(alt_u16 fontIndex, alt_u32 c);

// Finds a symbol information for a specified Unicode symbol.
/*Returns "1 - 3" if the copy is successful.
Returns "0" if an error.
Parameters:
    fontIndex - index to a font
    pCode - a pointer to a symbol code (may be variable-length)
Return Value:
    currSymbol - receives a pointer to a symbol record.
    Returns number of bytes for this symbol, or 0 on error.
*/
alt_u8 findUtf8Character(const alt_u8 fontIndex, const alt_u8* const pCode, struct VduSymbolHeader* currSymbol);

#endif
