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
-- Version v1.03
-- Date:	29.09.2026.
-- Designed by: Mazur S.M. https://jira.radiy.com/browse/RADIX-4119
-- Reviewed by: Vintenko B.U.

-- Changes: Adding comments; removing magic numbers

-- *************************************************************
*/

#ifndef RN_TS_H
#define RN_TS_H

#include <alt_types.h>

#include <stdbool.h>

// Initialization of module variables
//
void moduleRnTsInitialize(const alt_u16 screenWidth, const alt_u16 screenHeight);

// This function checks if there is a pending screen click event
//
bool isTouchEvent(void);

// This function checks the screen touch has been occur inside the provided coordinates
//
bool checkTouchZone(const alt_u16 x, const alt_u16 y, const alt_u16 zoneWidth, const alt_u16 zoneHeight);

#endif
