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
-- Date:	20.04.2026.
-- Designed by: Mazyr S.M. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Vintenko B.U.

-- Changes: RADIX-3966, rn_kbd initial update

-- *************************************************************
-- Version v1.01
-- Date:	01.05.2026.
-- Designed by: Mazyr S.M. https://jira.radiy.com/browse/RADIX-3966, https://jira.radiy.com/browse/RADIX-3969
-- Reviewed by: Vintenko B.U.

-- Changes: RADIX-3966, rn_kbd module refactoring

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

#ifndef RN_KBD_H
#define RN_KBD_H

#include <alt_types.h>

#include "keycodes.h"

#include <stdbool.h>

// Function that initialize keyboard with ps/2.
//
void moduleRnKbdInitialize(void);

// This function checks if there is a pending key event and resets it.
//
bool isPressedKey(void);

// This function returns the key event code.
//
alt_u32 pressedKey(void);

#endif
