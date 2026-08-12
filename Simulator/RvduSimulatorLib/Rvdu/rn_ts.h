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

#ifndef RN_TS_H
#define RN_TS_H

// Microsoft Visual Studio-specific
#ifndef RVDU_SIM        

#include <alt_types.h>

#else

#include "msvc_alt_types.h"

#endif
// Microsoft Visual Studio-specific

#include <stdbool.h>

void moduleRnTsInitialize(void);

#ifdef RVDU_SIM // Microsoft Visual Studio-specific, exports for simulator only!
void moduleRnTsUninitialize(void);
#endif

// This function sets the touch event.
//
#ifdef RVDU_SIM // Microsoft Visual Studio-specific, exports for simulator only!
void setTouchEvent(alt_u16 x, alt_u16 y);
#endif

// This function checks if there is a pending screen click event.
//
bool isTouchEvent(void);

// This function checks the screen touch has been occur inside the provided coordinates.
//
bool checkTouchZone(alt_u16 x, alt_u16 y, alt_u16 w, alt_u16 h);

#endif
