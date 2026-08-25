#include <string.h>

#include "rn_vbuf.h"
#include "rn_utils.h"
#include "rn_diag.h"

#include "defs.h"

#include "fat_filelib.h"

#include <stdbool.h>
#include <threads.h>

// Private module data
//
static mtx_t m_cs;

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

	mtx_init(&m_cs, mtx_plain);

	return;
}

void moduleRnVbufUninitialize(void)
{
	mtx_destroy(&m_cs);
	m_moduleInitialized = false;
}

void moduleRnVbufLock(void)
{
	mtx_lock(&m_cs);
}

void moduleRnVbufUnlock(void)
{
	mtx_unlock(&m_cs);
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


	mtx_lock(&m_cs);

	const alt_u64* const pSrc = (alt_u64*)m_pVideoBuffer;
	alt_u64* pDst = (alt_u64*)m_pHdmiBuffer;
	const alt_u32 wholeFrameCopyOperations = (alt_u32)m_screenWidth * (alt_u32)m_screenHeight / 2u;	// Two pixels per operation

	for (alt_u32 i = 0u; i < wholeFrameCopyOperations; i++)
	{
		pDst[i] = pSrc[i];
	}

	mtx_unlock(&m_cs);

	// Sleep for 5 ms
	static struct timespec ts = {.tv_sec = 0, .tv_nsec = 5 * 1000000};
	thrd_sleep(&ts, NULL);

	return true;
}

