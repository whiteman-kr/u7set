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

#include <stdio.h>

#include "rn_diag.h"
#include "rn_fofp.h"
#include "rn_cfg.h"
#include "rn_utils.h"

#include "defs.h"

#include "fat_filelib.h"

// Private module data
//
static alt_u8* m_pFontData[MAX_FONTS_COUNT] = { 0 };
static alt_u16 m_symbolsCount[MAX_FONTS_COUNT] = { 0 };	// Symbols count for each font
//
// End of Private data

// Cache specified font file
//
static bool cacheFontFile(const char* fileName, alt_u16 fontIndex, alt_u8* const fontStartAddr, alt_u32 maxFontSize, alt_u32* bytesRead)
{
	char msg[STRING_MAX_LEN];

	void* f = fl_fopen(fileName, "rb");
	if (f == NULL)
	{
		(void)snprintf(msg, STRING_MAX_LEN, "cacheFontFile: font file %s, file open error.", fileName);
		logError(OpenFileError, msg);
		return false;
	}

	bool readOk = true;

	// Check file size
	//
	alt_u32 size = fileSize(f);

	if ((size == WRONG_FILE_SIZE) ||
		(size > maxFontSize) ||
		(size < sizeof(struct VduSymbolHeader)))
	{
		(void)snprintf(msg, STRING_MAX_LEN, "cacheFontFile: file has wrong size (%d), max size is %d bytes.",
				(int)size,
				(int)maxFontSize);
		logError(FileSizeError, msg);

		readOk = false;
	}

	// Read file data
	//
	if (readOk == true)
	{
		if (freadBigData(fontStartAddr, size, f) == false)
		{
			(void)snprintf(msg, STRING_MAX_LEN, "cacheFontFile: file %s read error.", fileName);
			logError(ReadFileError, msg);

			readOk = false;
		}
	}

	// Calculate number of symbols in the file. Divide the offset of first symbol to size of the symbol header, 
	// and the rounded result will give a number of symbols.
	//
	if (readOk == true)
	{
		const struct VduSymbolHeader* const symbolHeader = (const struct VduSymbolHeader* const)fontStartAddr;

		const alt_u32 symbolsCount = symbolHeader->offset / sizeof(struct VduSymbolHeader);
		static const alt_u32 maxSymbolsCount = 65535u;

		if (symbolsCount > maxSymbolsCount)
		{
			readOk = false;

			(void)snprintf(msg, STRING_MAX_LEN, "cacheFontFile: font file %s contains more than %u symbols.", fileName, (unsigned int)symbolsCount);
			logError(TooManySymbols, msg);
		}
		else
		{
			m_symbolsCount[fontIndex] = (alt_u16)symbolsCount;
		}
	}

	(void)fl_fclose(f);

	if (readOk == true)
	{
		*bytesRead = size;
		return true;
	}

	return readOk;
}

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
bool cacheFonts(const char* const fontsPath, alt_u8* const startAddr, alt_u32 memoryAvailable, alt_u32* bytesRead)
{
	if ((fontsPath == NULL) || (startAddr == NULL) || (bytesRead == NULL) || (memoryAvailable == 0u))
	{
		RN_ASSERT(false);
		return false;
	}

	alt_u16 fontCount = vduConfig()->fontCount;
	if ((fontCount == 0u) || (fontCount > MAX_FONTS_COUNT))
	{
		RN_ASSERT(false);
		return false;
	}

	alt_u8* fontStartAddr = startAddr;
	*bytesRead = 0u;

	for (alt_u16 i = 0u; i < fontCount; i++)
	{
		char fileName[FILE_NAME_MAX_LEN] = { 0 };
		(void)snprintf(fileName, FILE_NAME_MAX_LEN, "%s/%d/font.vdf", fontsPath, i);

		alt_u32 fontBytesRead = 0;
		bool result = cacheFontFile(fileName, i, fontStartAddr, memoryAvailable, &fontBytesRead);
		if (result == false)
		{
			return false;
		}

		m_pFontData[i] = fontStartAddr;

		// Number of bytes read from the file is upward-rounded to dword-size,
		// because all fonts data shall be aligned by dword
		//
		fontBytesRead = alignAddressToDword(fontBytesRead);

		fontStartAddr += fontBytesRead;
		*bytesRead += fontBytesRead;

		memoryAvailable = DEC_MEMORY(memoryAvailable, fontBytesRead);
	}
	return true;
}

// Returns the font symbols count
//
static alt_u16 fontSymbolsCount(alt_u16 fontIndex)
{
	if (fontIndex >= MAX_FONTS_COUNT)
	{
		RN_ASSERT(false);
		return 0u;
	}
	return m_symbolsCount[fontIndex];
}

// Returns the font data structure for provided fontor NULL if parameters are incorrect
//
const alt_u8* const fontData(alt_u16 fontIndex)
{
	if (fontIndex >= MAX_FONTS_COUNT)
	{
		RN_ASSERT(false);
		return NULL;
	}

	return (alt_u8*)(m_pFontData[fontIndex]);
}

// Returns the font symbol header structure for provided font and character or NULL if parameters are incorrect
//
const struct VduSymbolHeader* const fontSymbolHeader(alt_u16 fontIndex, alt_u32 c)
{
	if (fontIndex >= MAX_FONTS_COUNT)
	{
		RN_ASSERT(false);
		return NULL;
	}

	if (c >= fontSymbolsCount(fontIndex))
	{
		RN_ASSERT(false);
		return NULL;
	}

	return (const struct VduSymbolHeader* const)(m_pFontData[fontIndex] + (c * sizeof(struct VduSymbolHeader)));
}

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
alt_u8 findUtf8Character(const alt_u8 fontIndex, const alt_u8* const pCode, struct VduSymbolHeader* currSymbol)
{
	static const alt_u32 DEFAULT_CHAR = ((alt_u32)0x7Fu - (alt_u32)' ');

	// Control character
	//
	if (*pCode < ' ')
	{
		const struct VduSymbolHeader* const defaultHeader = fontSymbolHeader(fontIndex, DEFAULT_CHAR);
		if (defaultHeader == NULL)
		{
			RN_ASSERT(defaultHeader != NULL);
			return 0u;
		}
		*currSymbol = *defaultHeader;

		return 1u;
	}

	// ASCII character
	//
	static const alt_u8 maxLatinCharacter = 127u;
	static const alt_u8 minLatinCharacter = 32u;

	if (*pCode <= maxLatinCharacter)
	{
		const alt_u32 correctedCode = ((alt_u32)*pCode) - ((alt_u32)' ');

		const struct VduSymbolHeader* const header = fontSymbolHeader(fontIndex, correctedCode);
		if (header == NULL)
		{
			RN_ASSERT(header != NULL);
			return 0u;
		}
		*currSymbol = *header;
		return 1u;
	}

	// Unicode characters
	//
	const struct VduSymbolHeader* const defaultHeader = fontSymbolHeader(fontIndex, DEFAULT_CHAR);
	if (defaultHeader == NULL)
	{
		RN_ASSERT(defaultHeader != NULL);
		return 0u;
	}
	*currSymbol = *defaultHeader;

	alt_u8 utfBytes = 0u;
	alt_u32 utf8code = 0u;

	if ((*pCode & 0b11100000u) == 0b11000000u)
	{
		// First byte of a 2-byte code unit sequence
		//
		utf8code = (alt_u32)*pCode + ((alt_u32)*(pCode + 1u) << 8u); // 2-bytes code to find 
		utfBytes = 2u;
	}
	else if ((*pCode & 0b11110000u) == 0b11100000u)
	{
		// First byte of a 3-byte code unit sequence
		//
		utf8code = (alt_u32)*pCode + ((alt_u32) *(pCode + 1) << 8u) + ((alt_u32) *(pCode + 2u) << 16u); // 3-bytes code to find 
		utfBytes = 3u;
	}
	else if ((*pCode & 0b11111000u) == 0b11110000u)
	{
		// First byte of a 4-byte code unit sequence
		//
		utf8code = (alt_u32)*pCode + ((alt_u32) *(pCode + 1) << 8u) + ((alt_u32) *(pCode + 2u) << 16u) + ((alt_u32)*(pCode + 3u) << 24u); // 4-bytes code to find
		utfBytes = 4u;
	}
	else
	{
		// Incorrect UTF code
		//
		return 0u;
	}

	alt_u32 symbolsQuantity = fontSymbolsCount(fontIndex);
	for (alt_u32 i = (maxLatinCharacter - minLatinCharacter); i < symbolsQuantity; i++) // loop through all non-latin characters
	{
		const struct VduSymbolHeader* const header = fontSymbolHeader(fontIndex, i);
		if (header == NULL)
		{
			RN_ASSERT(header != NULL);
			return 0u;
		}

		if (header->code == utf8code)
		{
			*currSymbol = *header;
			break;
		}
	}

	return utfBytes;
}
