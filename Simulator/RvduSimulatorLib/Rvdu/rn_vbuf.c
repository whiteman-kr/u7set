/*
Module title: Video Buffer Operation.
(C) 2026 RPC "Radiy".
All rights reserved.
The information presented here is confidential
and is the intellectual property of PJSC "RPC "Radiy".
Any use of this information is strictly
prohibited without the written consent of PJSC "RPC "Radiy".
Module description: Manages video buffers for rendering and display output.

-- *************************************************************
-- Version v1.00
-- Date:	31,03,2026.
-- Designed by: Mazyr S.M. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Vintenko B.U.

-- Changes: RADIX-3966, rn_vbuf initial update

-- *************************************************************
-- Version v1.01
-- Date:	09,04,2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966, https://jira.radiy.com/browse/RADIX-3969
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-3966, rn_vbuf module refactoring
-- Add log error for showLogo function
-- Add vary screen resolution support

-- Changes: RADIX-3969, add logging and debugging support
-- Add log errors and messages

-- *************************************************************

-- Version v1.02
-- Date:	28,07,2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-3966, code is made MISRA-2025 compatible

-- *************************************************************
*/

#include <string.h>

#include "rn_vbuf.h"
#include "rn_utils.h"
#include "rn_diag.h"

#include "defs.h"

#ifndef RVDU_SIM // Microsoft Visual Studio-specific

#include "../rvdu_bsp/system.h"
#include "../rvdu_bsp/drivers/inc/altera_avalon_pio_regs.h"
#include "../rvdu_bsp/HAL/inc/sys/alt_stdio.h"
#include "../rvdu_bsp/HAL/inc/io.h"
#include "../fat_io_lib/fat_filelib.h"
#include "nios_ii_built_in.h"

#else

#include <stdbool.h>

static mtx_t m_cs;

#endif // End of Microsoft Visual Studio-specific

// Private module data
//
static bool m_moduleInitialized = false;

static alt_u32* m_pVideoBuffer = NULL;
static alt_u32* m_pHdmiBuffer = NULL;

static alt_u16 m_screenWidth = 0;
static alt_u16 m_screenHeight = 0;
//
// End of Private data

void moduleRnVbufInitialize(alt_u32* const pVideoBuffer, alt_u32* const pHdmiBuffer, alt_u16 screenWidth, alt_u16 screenHeight)
{
	if (m_moduleInitialized == true)
	{
		RN_ASSERT(false);
		return;
	}
	m_moduleInitialized = true;

	m_pVideoBuffer = pVideoBuffer;
	m_pHdmiBuffer = pHdmiBuffer;

	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

#ifdef RVDU_SIM // Microsoft Visual Studio-specific
	// Initialize critical section
	mtx_init(&m_cs, mtx_plain);
#endif

	return;
}

#ifdef RVDU_SIM // Microsoft Visual Studio-specific

void moduleRnVbufUninitialize(void)
{
	mtx_destroy(&m_cs);
	m_moduleInitialized = false;
}

mtx_t* rnVbufMutex(void)
{
	RN_ASSERT(m_moduleInitialized == true);
	return &m_cs;
}

#endif

bool writeVideoBuffer(const alt_u32 backgroundColor)
{
	if (m_moduleInitialized == false)
	{
		RN_ASSERT_MSG(false, "writeVideoBuffer: module is not initialized, moduleRnVbufInitialize shall be executed!.");
		return false;
	}
	
	const alt_u64 doublePixelsColor = ((alt_u64)backgroundColor << 32) | backgroundColor;
	
	alt_u64* pSource = (alt_u64*)m_pVideoBuffer;
	const alt_u32 wholeFrameCopyOperations = (alt_u32)m_screenWidth * (alt_u32)m_screenHeight / 2u;	// Two pixels per operation

	for (alt_u32 i = wholeFrameCopyOperations; i > 0u; i--)
	{
		*pSource = doublePixelsColor;
		pSource++;
	}
	
	return true;
}

bool pushVideoBuffer(void)
{
	if (m_moduleInitialized == false)
	{
		RN_ASSERT_MSG(false, "writeVideoBuffer: module is not initialized, moduleRnVbufInitialize shall be executed!.");
		return false;
	}


#ifdef RVDU_SIM // Microsoft Visual Studio-specific, exports for simulator only!
	// Enter critical section (lock)
	mtx_lock(&m_cs);
#endif

	const alt_u64* const pSrc = (alt_u64*)m_pVideoBuffer;
	alt_u64* pDst = (alt_u64*)m_pHdmiBuffer;
	const alt_u32 wholeFrameCopyOperations = (alt_u32)m_screenWidth * (alt_u32)m_screenHeight / 2u;	// Two pixels per operation

	for (alt_u32 i = 0u; i < wholeFrameCopyOperations; i++)
	{
		pDst[i] = pSrc[i];
	}

#ifdef RVDU_SIM // Microsoft Visual Studio-specific, exports for simulator only!
	// Leave critical section (unlock)
	mtx_unlock(&m_cs);
#endif

	// Microsoft Visual Studio-specific
#ifndef RVDU_SIM   
	// Cyclic switching of video memory pages 0-1-2
	//
	__builtin_stwio((hwBaseToU32ptr(FRAME_SW_BASE)),
		(__builtin_ldwio(hwBaseToU32ptr(FRAME_SW_BASE)) < 2) ?
		(__builtin_ldwio(hwBaseToU32ptr(FRAME_SW_BASE)) + 1) :
		(0));
#endif
	return true;
}

bool showLogo(const char* logoFile)
{
	if (logoFile == NULL)
	{
		RN_ASSERT(logoFile != NULL);
		return false;
	}

	if (m_moduleInitialized == false)
	{
		RN_ASSERT_MSG(false, "showLogo: module is not initialized, moduleRnVbufInitialize shall be executed!.");
		return false;
	}

#define MAX_LOGO_ROW_SIZE (((MAX_SCREEN_WIDTH * 3u) + 3u) & (~3u))

	void* file = fl_fopen(logoFile, "rb");
	if (file == NULL)
	{
		logError(OpenFileError, "showLogo: file open error.");
		return false;
	}

	bool readOk = true;

	// Read the BMP header
	//
	struct BMPFileHeader bmpHeader = {0};
	struct BMPInfoHeader bmpInfo = { 0 };

	if (freadBigData((alt_u8*)&bmpHeader, sizeof(struct BMPFileHeader), file) == false)
	{
		logError(ReadFileError, "showLogo: file header read error.");
		readOk = false;
	}

	if (readOk == true)
	{
		if (freadBigData((alt_u8*)&bmpInfo, sizeof(struct BMPInfoHeader), file) == false)
		{
			logError(ReadFileError, "showLogo: file info read error.");
			readOk = false;
		}
	}

	// Check the BMP header
	//
	if (readOk == true)
	{
		if ((bmpHeader.bfType != 0x4d42u) || // BM
			(bmpInfo.biSize != 40u) ||
			(bmpInfo.biPlanes != 1u) ||
			(bmpInfo.biBitCount != 24u) ||
			(bmpInfo.biCompression != 0u))
		{
			logError(FileFormatError, "showLogo: file format error.");
			readOk = false;
		}
	}

	if (readOk == true)
	{
		if (((alt_u16)bmpInfo.biWidth != m_screenWidth) ||
			((alt_u16)bmpInfo.biHeight != m_screenHeight))
		{
			logError(FileFormatError, "showLogo: image size error.");
			readOk = false;
		}
	}

	// Read the BMP data
	//
	if (readOk == true)
	{
		static alt_u8 row[MAX_LOGO_ROW_SIZE] = { 0 };

		const alt_u16 rowSize = ((m_screenWidth * 3u) + 3u) & (~3u);

		for (alt_u16 y = (m_screenHeight - 1u); y != 0u; y--)
		{
			if (freadBigData(row, rowSize, file) == false)
			{
				logError(UnexpectedEOF, "showLogo: file read error.");
				
				readOk = false;
				break;
			}

			const alt_u8* rowPtr = row;

			for (alt_u16 x = 0; x < m_screenWidth; x++)
			{
				alt_u8 b = *rowPtr;
				rowPtr++;
				alt_u8 g = *rowPtr;
				rowPtr++;
				alt_u8 r = *rowPtr;
				rowPtr++;

				alt_u32 data = 0xff000000u | ((alt_u32)r << 16u) | ((alt_u32)g << 8u) | (alt_u32)b;
				*(m_pVideoBuffer + (y * m_screenWidth) + x) = data;
			}
		}

		if (readOk == true)
		{
			readOk &= pushVideoBuffer();
		}
	}

	(void)fl_fclose(file);

	return readOk;
}
