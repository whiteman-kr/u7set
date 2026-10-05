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

#include "fat_filelib.h"

// Private module data
//
static bool m_moduleInitialized = false;

static alt_u32* m_pVideoBuffer = NULL;
static alt_u32* m_pHdmiBuffer = NULL;

static alt_u16 m_screenWidth = 0u;
static alt_u16 m_screenHeight = 0u;
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

	writeVideoBuffer(0x0u);	// Fill color with black

	return;
}

void moduleRnVbufUninitialize(void)
{
	m_moduleInitialized = false;
}

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

	const alt_u64* const pSrc = (alt_u64*)m_pVideoBuffer;
	alt_u64* pDst = (alt_u64*)m_pHdmiBuffer;
	const alt_u32 wholeFrameCopyOperations = (alt_u32)m_screenWidth * (alt_u32)m_screenHeight / 2u;	// Two pixels per operation

	for (alt_u32 i = 0u; i < wholeFrameCopyOperations; i++)
	{
		pDst[i] = pSrc[i];
	}

	// Cyclic switching of video memory pages 0-1-2
	//
	__builtin_stwio((hwBaseToU32ptr(FRAME_SW_BASE)),
		(__builtin_ldwio(hwBaseToU32ptr(FRAME_SW_BASE)) < 2) ?
		(__builtin_ldwio(hwBaseToU32ptr(FRAME_SW_BASE)) + 1) :
		(0));

	return true;
}

