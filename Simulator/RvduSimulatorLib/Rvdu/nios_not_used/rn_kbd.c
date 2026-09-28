/*
Module title: Keyboard Events Checker.
(C) 2026 RPC "Radiy".
All rights reserved.
The information presented here is confidential
and is the intellectual property of PJSC "RPC "Radiy".
Any use of this information is strictly
prohibited without the written consent of PJSC "RPC "Radiy".
Module description: Detects and processes key press events from the keyboard.

-- *************************************************************
-- Version v1.00
-- Date:	20,04,2026.
-- Designed by: Mazyr S.M. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Vintenko B.U.

-- Changes: RADIX-3966, rn_kbd initial update

-- *************************************************************
-- Version v1.01
-- Date:	01,05,2026.
-- Designed by: Mazyr S.M. https://jira.radiy.com/browse/RADIX-3966, https://jira.radiy.com/browse/RADIX-3969
-- Reviewed by: Vintenko B.U.

-- Changes: RADIX-3966, rn_kbd module refactoring

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

#include "rn_kbd.h"
#include "rn_diag.h"
#include "rn_utils.h"

#include <system.h>
#include <altera_up_avalon_ps2_regs.h>
#include <altera_up_avalon_ps2.h>
#include <altera_up_ps2_keyboard.h>
#include "sys/alt_stdio.h"

// Private module data
//
static alt_up_ps2_dev* m_ps2 = NULL;

static bool m_moduleInitialized = false;
static bool m_keyboardConnected = false;
static alt_u16 m_code = 0xffu;
static bool m_keyPressed = false;
//
// End of Private data

void moduleRnKbdInitialize(void)
{
	if (m_moduleInitialized == true)
	{
		RN_ASSERT(false);
		return;
	}
	m_moduleInitialized = true;

	m_ps2 = alt_up_ps2_open_dev("/dev/ps2_0");
	if (m_ps2 == NULL)
	{
		logError(InternalError, "PS/2 failure.");
		return;
	}

	alt_up_ps2_clear_fifo(m_ps2);

	logMessage("PS/2 device is connected.");
	m_keyboardConnected = true;

	/*
	if (m_ps2->device_type == PS2_KEYBOARD)
	{
		logMessage("PS/2 Keyboard connected.");
		m_keyboardConnected = true;
	}
	else if (m_ps2->device_type == PS2_MOUSE)
	{
		logMessage("PS/2 Mouse connected.");
	}
	else if (m_ps2->device_type == PS2_UNKNOWN)
	{
		logWarning(InternalWarning, "PS/2 Unknown device is connected.");
	}
	else
	{
		logMessage("PS/2 Device is not connected.");
	}*/
}

static void setPressedKey(alt_u16 code)
{
	m_code = code;
	m_keyPressed = true;
}

static alt_u16 scan_kb(void)
{
	if (m_ps2 == NULL)
	{
		return 0xff;
	}

	static alt_u16 last_kb_code = 0x5Au;
	last_kb_code &= 0xffu;

	alt_u32 kb_code = IORD_ALT_UP_PS2_PORT_DATA_REG(m_ps2->base);
	if ((kb_code & 0xffff0000u) > 0x20000u)
	{
		do
		{
			kb_code = IORD_ALT_UP_PS2_PORT_DATA_REG(m_ps2->base);
		} while ((kb_code & 0xffff0000u) > 0x20000u);
	}

	if ((kb_code & 0xffff00ffu) == (0x20000u | 0xf0u))
	{
		last_kb_code = IORD_ALT_UP_PS2_PORT_DATA_REG(m_ps2->base);
	}

	return last_kb_code;
}

// This function checks the hardware for touch events.
//
static bool checkPressedKey(void)
{
	alt_u16 code = scan_kb();
	if (m_code != code)
	{
		setPressedKey(code);
		return true;
	}
	else
	{
		return false;
	}
}

bool isPressedKey(void)
{
	if (m_moduleInitialized == false)
	{
		RN_ASSERT_MSG(false, "rn_kbd is not initialized.");
		return false;
	}

	if (m_keyboardConnected == false)
	{
		return false;
	}

	if (checkPressedKey() == false)
	{
		return false;
	}

	bool result = m_keyPressed == true;
	m_keyPressed = false;

	return result;
}

alt_u16 pressedKey(void)
{
	if (m_moduleInitialized == false)
	{
		RN_ASSERT_MSG(false, "rn_kbd is not initialized.");
		return 0xffu;
	}

	if (m_keyboardConnected == false)
	{
		return 0xffu;
	}

	alt_u16 result = m_code;

	return result;
}
