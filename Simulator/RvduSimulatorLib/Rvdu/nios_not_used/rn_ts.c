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
-- Date:	31.03.2026.
-- Designed by: Mazyr S.M. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Vintenko B.U.

-- Changes: RADIX-3966, rn_ts initial update

-- *************************************************************
-- Version v1.01
-- Date:	21.04.2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966, https://jira.radiy.com/browse/RADIX-3969
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-3966, rn_ts module refactoring
-- Change function names: tchscrn_check_event to checkTouchEvent, check_zone to checkTouchZone

-- Changes: RADIX-3969, add logging and debugging support
-- Add log errors and messages

-- *************************************************************

-- Version v1.02
-- Date:	28.07.2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-3966, code is made MISRA-2025 compatible

-- *************************************************************
*/

#include "rn_ts.h"
#include "rn_diag.h"
#include "rn_utils.h"

#include <system.h>
#include <altera_avalon_pio_regs.h>

static bool m_moduleInitialized = false;

// Type of touchscreen
//
#define PENMOUN6000 1
#define DIVA 2
#define NONPROC 3

#define TS_TYPE NONPROC

// Private module data
//
static alt_u16 m_screenWidth = 0;
static alt_u16 m_screenHeight = 0;

static bool m_tchscrnEvent = false;
static alt_u16 m_tchscrnPosX = 0;
static alt_u16 m_tchscrnPosY = 0;
//
// End of Private data


// Initialization of module variables
//
void moduleRnTsInitialize(const alt_u16 screenWidth, const alt_u16 screenHeight)
{
	if (m_moduleInitialized == true)
	{
		RN_ASSERT(false);
		return;
	}

	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	m_moduleInitialized = true;
}

// This function sets the touch event and the module coordinates
//
static void setTouchEvent(const alt_u16 x, const alt_u16 y)
{
	m_tchscrnEvent = true;
	m_tchscrnPosX = x;
	m_tchscrnPosY = y;
}

// This function checks the hardware for touch events
//
static void checkTouchEvent(void)
{
	// Checking for new touchscreen touches
	//
	if (IORD_ALTERA_AVALON_PIO_DATA(DMC6000_READY_BASE) != 0)
	{
		alt_u16 x = 0u;
		alt_u16 y = 0u;

		// Blocking data changes from the driver
		//
		IOWR_ALTERA_AVALON_PIO_DATA(DMC6000_RST_BASE, 1);

#if TS_TYPE == PENMOUN6000
		x = (IORD_ALTERA_AVALON_PIO_DATA(DMC6000_X_BASE) <= 0x66) ? (alt_u16)(0) :
			(IORD_ALTERA_AVALON_PIO_DATA(DMC6000_X_BASE) >= 0x399) ? (alt_u16)(m_screenWidth - 1) :
			(alt_u16)((m_screenWidth - 1) * (IORD_ALTERA_AVALON_PIO_DATA(DMC6000_X_BASE) - 0x66) / (0x399 - 0x66));
		y = (IORD_ALTERA_AVALON_PIO_DATA(DMC6000_Y_BASE) <= 0x60) ? (alt_u16)(0) :
			(IORD_ALTERA_AVALON_PIO_DATA(DMC6000_Y_BASE) >= 0x395) ? (alt_u16)(m_screenHeight - 1) :
			(alt_u16)((m_screenHeight - 1) * (IORD_ALTERA_AVALON_PIO_DATA(DMC6000_Y_BASE) - 0x60) / (0x395 - 0x60));
#elif TS_TYPE == DIVA
		x = (IORD_ALTERA_AVALON_PIO_DATA(DMC6000_X_BASE) <= 0xf70) ?
			(alt_u16)(IORD_ALTERA_AVALON_PIO_DATA(DMC6000_X_BASE) * (m_screenWidth - 1) / 0xf70) :
			(alt_u16)(m_screenWidth - 1);
		y = (IORD_ALTERA_AVALON_PIO_DATA(DMC6000_Y_BASE) <= 0xf70) ?
			(alt_u16)((m_screenHeight - 1) - (IORD_ALTERA_AVALON_PIO_DATA(DMC6000_Y_BASE) * (m_screenHeight - 1) / 0xf70)) :
			(alt_u16)(0);
#elif TS_TYPE == NONPROC
		x = (alt_u16)IORD_ALTERA_AVALON_PIO_DATA(DMC6000_X_BASE);
		y = (alt_u16)IORD_ALTERA_AVALON_PIO_DATA(DMC6000_Y_BASE);
#else
#error "Touchscreen type is not set or unknown!"
#endif

		// Verification of coordinate correctness
		//
		if (x < m_screenWidth)
		{
			if (y < m_screenHeight)
			{
				setTouchEvent(x, y);
			}
		}

		// Permission to update driver data
		//
		if (IORD_ALTERA_AVALON_PIO_DATA(DMC6000_RST_BASE) != 0)
		{
			IOWR_ALTERA_AVALON_PIO_DATA(DMC6000_RST_BASE, 0);
		}
	}
	return;
}

// This function resets the touch event
//
static void resetTouchEvent(void)
{
	m_tchscrnEvent = false;
}

// This function checks the screen touch has been occur inside the provided coordinates
//
bool checkTouchZone(const alt_u16 x, const alt_u16 y, const alt_u16 w, const alt_u16 h)
{
	if (m_moduleInitialized == false)
	{
		RN_ASSERT(false);
		return false;
	}

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

	return result;
}

// This function checks if there is a pending screen click event
//
bool isTouchEvent(void)
{
	if (m_moduleInitialized == false)
	{
		RN_ASSERT(false);
		return false;
	}

	checkTouchEvent();

	bool result = m_tchscrnEvent == true;

	resetTouchEvent();

	return result;
}
