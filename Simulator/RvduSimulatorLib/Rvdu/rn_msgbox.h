/*
Module title: Message Box Schema Processing
(C) 2026 RPC "Radiy".
All rights reserved.
The information presented here is confidential
and is the intellectual property of PJSC "RPC "Radiy".
Any use of this information is strictly
prohibited without the written consent of PJSC "RPC "Radiy".
Module description: Provides the functionality for displaying confirmation message boxes.

-- *************************************************************
-- Version v1.00
-- Date:	31,03,2026.
-- Designed by: Mazyr S.M. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Vintenko B.U.

-- Changes: RADIX-3966, rn_msgbox initial update

-- *************************************************************
-- Version v1.01
-- Date:	12,04,2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966, https://jira.radiy.com/browse/RADIX-3969
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-3966, rn_msgbox module refactoring
-- Add keyboard support

-- Changes: RADIX-3969, add logging and debugging support
-- Add log errors and messages

-- *************************************************************

-- Version v1.02
-- Date:	28,07,2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Nesterenko M.

-- Changes: RADIX-3966, code is made MISRA-2025 compatible

-- *************************************************************
*/

#ifndef RN_MSGBOX_H
#define RN_MSGBOX_H

#include <alt_types.h>

struct PopUpSchemaParams;

// This function open displaying confirmation message boxes diaglog.
//
void modeQuestionShow(const struct PopUpSchemaParams* const schema);

// This function process all displaying confirmation message boxes diaglog event.
//
void modeQuestionProc(struct PopUpSchemaParams* const schema);

#endif
