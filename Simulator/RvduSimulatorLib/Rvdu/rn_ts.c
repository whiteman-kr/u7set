/*
Module title: Touchscreen Events Checker.
(C) 2026 RPC "Radiy".
All rights reserved.
The information presented here is confidential
and is the intellectual property of PJSC "RPC "Radiy".
Any use of this information is strictly
prohibited without the written consent of PJSC "RPC "Radiy".
Module description: Detects and processes touch input events from the touchscreen.

-- *************************************************************
-- Version v1.00
-- Date:	31,03,2026.
-- Designed by: Mazyr S.M. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Vintenko B.U.

-- Changes: RADIX-3966, rn_ts initial update

-- *************************************************************
-- Version v1.01
-- Date:	21,04,2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966, https://jira.radiy.com/browse/RADIX-3969
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-3966, rn_ts module refactoring
-- Change function names: tchscrn_check_event to checkTouchEvent, check_zone to checkTouchZone

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

#include "rn_ts.h"
#include "rn_diag.h"
#include "rn_utils.h"

#ifndef RVDU_SIM // Microsoft Visual Studio-specific

#include <system.h>
#include <altera_avalon_pio_regs.h>

#else

#include <threads.h>

static mtx_t m_cs;

#endif // End of Microsoft Visual Studio-specific

static bool m_moduleInitialized = false;

// Type of touchscreen
//
#define PENMOUN6000 1
#define DIVA 2
#define NONPROC 3

#define TS_TYPE NONPROC

// Private module data
//
static bool m_tchscrnEvent = false;
static alt_u16 m_tchscrnPosX = 0;
static alt_u16 m_tchscrnPosY = 0;
//
// End of Private data


void moduleRnTsInitialize(void)
{
	if (m_moduleInitialized == true)
	{
		RN_ASSERT(false);
		return;
	}
	m_moduleInitialized = true;

	// Initialize critical section
#ifdef RVDU_SIM // Microsoft Visual Studio-specific
	mtx_init(&m_cs, mtx_plain);
#endif
}

#ifdef RVDU_SIM // Microsoft Visual Studio-specific
void moduleRnTsUninitialize(void)
{
	m_moduleInitialized = false;
	m_tchscrnEvent = false;
	m_tchscrnPosX = 0u;
	m_tchscrnPosY = 0u;
	mtx_destroy(&m_cs);
}
#endif

#ifdef RVDU_SIM // Microsoft Visual Studio-specific, exports for simulator only!
void setTouchEvent(alt_u16 x, alt_u16 y)
#else
static void setTouchEvent(alt_u16 x, alt_u16 y)
#endif
{
	if (m_moduleInitialized == false)
	{
		RN_ASSERT(false);
		return;
	}

#ifdef RVDU_SIM // Microsoft Visual Studio-specific
	// Enter critical section (lock)
	mtx_lock(&m_cs);
#endif

	m_tchscrnEvent = true;
	m_tchscrnPosX = x;
	m_tchscrnPosY = y;

#ifdef RVDU_SIM // Microsoft Visual Studio-specific
	// Leave critical section (unlock)
	mtx_unlock(&m_cs);
#endif
}

// This function checks the hardware for touch events.
//
static void checkTouchEvent(void)
{
#ifndef RVDU_SIM // Microsoft Visual Studio-specific

	if (IORD_ALTERA_AVALON_PIO_DATA(DMC6000_READY_BASE) != 0u)
	{
		alt_u16 x = 0u;
		alt_u16 y = 0u;

		IOWR_ALTERA_AVALON_PIO_DATA(DMC6000_RST_BASE, 1u);

#ifndef TS_TYPE // Microsoft Visual Studio-specific
#error "Touchscreen type is not set!"
#endif

#if TS_TYPE == PENMOUN6000
		x = ( IORD_ALTERA_AVALON_PIO_DATA(DMC6000_X_BASE) <= 0x66 ) ? (0) :
				( IORD_ALTERA_AVALON_PIO_DATA(DMC6000_X_BASE) >= 0x399 ) ? (1279) :
				( 1279 * (IORD_ALTERA_AVALON_PIO_DATA(DMC6000_X_BASE)-0x66) / (0x399-0x66) );
		y = ( IORD_ALTERA_AVALON_PIO_DATA(DMC6000_Y_BASE) <= 0x60 ) ? (0) :
				( IORD_ALTERA_AVALON_PIO_DATA(DMC6000_Y_BASE) >= 0x395 ) ? (719) :
				( 719 * (IORD_ALTERA_AVALON_PIO_DATA(DMC6000_Y_BASE)-0x60) / (0x395-0x60) );
#endif
#if TS_TYPE == DIVA
		x = ( IORD_ALTERA_AVALON_PIO_DATA(DMC6000_X_BASE) <= 0xf70 ) ?
				( IORD_ALTERA_AVALON_PIO_DATA(DMC6000_X_BASE) * ( scr_w - 1 ) / 0xf70 ) :
				( scr_w - 1 );
		y = ( IORD_ALTERA_AVALON_PIO_DATA(DMC6000_Y_BASE) <= 0xf70 ) ?
				( ( scr_h - 1 ) - (IORD_ALTERA_AVALON_PIO_DATA(DMC6000_Y_BASE) * ( scr_h - 1 ) / 0xf70) ) :
				0;
#endif
#if TS_TYPE == NONPROC
		x = IORD_ALTERA_AVALON_PIO_DATA(DMC6000_X_BASE);
		y = IORD_ALTERA_AVALON_PIO_DATA(DMC6000_Y_BASE);
#endif

		setTouchEvent(x, y);

		if (IORD_ALTERA_AVALON_PIO_DATA(DMC6000_RST_BASE) != 0u)
		{
			IOWR_ALTERA_AVALON_PIO_DATA(DMC6000_RST_BASE, 0u);
		}

		return;
	}
	else
	{
		return;
	}
#else
	return;
#endif
}

// This function resets the touch event.
//
static void resetTouchEvent(void)
{
	if (m_moduleInitialized == false)
	{
		RN_ASSERT(false);
		return;
	}

#ifdef RVDU_SIM // Microsoft Visual Studio-specific
	// Enter critical section (lock)
	mtx_lock(&m_cs);
#endif

	if (m_tchscrnEvent == true)
	{
		m_tchscrnEvent = false;
	}

#ifdef RVDU_SIM // Microsoft Visual Studio-specific
	// Leave critical section (unlock)
	mtx_unlock(&m_cs);
#endif
}

bool checkTouchZone(alt_u16 x, alt_u16 y, alt_u16 w, alt_u16 h)
{
	if (m_moduleInitialized == false)
	{
		RN_ASSERT(false);
		return false;
	}

#ifdef RVDU_SIM // Microsoft Visual Studio-specific
	// Enter critical section (lock)
	mtx_lock(&m_cs);
#endif

	bool result = false;

	if (m_tchscrnPosX >= x)
	{
		if (m_tchscrnPosY >= y)
		{
			if (m_tchscrnPosX < (x + w))
			{
				if (m_tchscrnPosY < (y + h))
				{
					result = true;
				}
			}
		}
	}

#ifdef RVDU_SIM // Microsoft Visual Studio-specific
	// Leave critical section (unlock)
	mtx_unlock(&m_cs);
#endif

	return result;
}

bool isTouchEvent(void)
{
	if (m_moduleInitialized == false)
	{
		RN_ASSERT(false);
		return false;
	}

	checkTouchEvent();

#ifdef RVDU_SIM // Microsoft Visual Studio-specific
	// Enter critical section (lock)
	mtx_lock(&m_cs);
#endif

	bool result = m_tchscrnEvent == true;

#ifdef RVDU_SIM // Microsoft Visual Studio-specific
	// Leave critical section (unlock)
	mtx_unlock(&m_cs);
#endif

	resetTouchEvent();

	return result;
}
