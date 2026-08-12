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

#ifndef RN_KBD_H
#define RN_KBD_H

// Microsoft Visual Studio-specific
#ifndef RVDU_SIM        

#include <alt_types.h>

#else

#include "msvc_alt_types.h"

#endif
// Microsoft Visual Studio-specific

#include <stdbool.h>

// Key codes
//
#ifndef RVDU_SIM // Microsoft Visual Studio-specific

#define VK_RETURN		0x5Au
#define VK_ESCAPE		0x76u
#define VK_DELETE		0xDCu

#define VK_NUMPAD1		0x69u
#define VK_NUMPAD2		0x72u
#define VK_NUMPAD3		0x7Au
#define VK_NUMPAD4		0x6Bu
#define VK_NUMPAD5		0x73u
#define VK_NUMPAD6		0x74u
#define VK_NUMPAD7		0x6Cu
#define VK_NUMPAD8		0x75u
#define VK_NUMPAD9		0x7Du
#define VK_NUMPAD0		0x70u
#define VK_DECIMAL		0x71u
#define VK_SUBTRACT		0x7Bu
#define VK_ADD			0x79u

#define VK_UP			0x75u
#define VK_DOWN			0x72u
#define VK_LEFT			0x6Bu
#define VK_RIGHT		0x74u
#define VK_TAB			0x0Du
#define VK_PRIOR		0x7Du
#define VK_NEXT			0x7Au

#define VK_BACKSPACE 	0x66u
#define VK_E			0x24u
#define	VK_TILDA 		0x0Eu

#else

#define VK_BACKSPACE 	'\b'
#define VK_E			'E'
#define	VK_TILDA 		192
#endif


//
// Key codes

// Function that initialize keyboard with ps/2
//
void moduleRnKbdInitialize(void);

#ifdef RVDU_SIM
void moduleRnKbdUninitialize(void);
#endif

// This function sets the touch event.
//
#ifdef RVDU_SIM // Microsoft Visual Studio-specific, exports for simulator only!
void setPressedKey(alt_u16 code);
#endif

// This function checks if there is a pending screen click event.
//
bool isPressedKey(void);

// This function resets the touch event.
//
alt_u16 pressedKey(void);

#endif
