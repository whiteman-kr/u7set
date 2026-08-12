/*
Module title: Signal List View.
(C) 2026 RPC "Radiy".
All rights reserved.
The information presented here is confidential
and is the intellectual property of PJSC "RPC 'Radiy".
Any use of this information is strictly
prohibited without the written consent of PJSC "RPC "Radiy".
Module description: Provides signal list view.

-- *************************************************************
-- Version v1.00
-- Date:	15,04,2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966, https://jira.radiy.com/browse/RADIX-3969
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-3969, rn_signalsview added

-- *************************************************************
-- Version v1.01
-- Date:	27,04,2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966, https://jira.radiy.com/browse/RADIX-3969
-- Reviewed by: Nesterenko M.V.

-- Add keyboard support
-- Add signalsScrollPgUp, signalsScrollPgDown functions

-- Changes: RADIX-3969, add logging and debugging support
-- Add log errors and messages

-- *************************************************************

-- Version v1.02
-- Date:	09,05,2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-4009
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-4009, adding API support for working with signals by AppSignalIDs

-- *************************************************************

-- Version v1.03
-- Date:	28,07,2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-3966, code is made MISRA-2025 compatible

-- *************************************************************
*/

#ifndef RN_SIGNALSVIEW_H
#define RN_SIGNALSVIEW_H

// Microsoft Visual Studio-specific
#ifndef RVDU_SIM        
#include <alt_types.h>
#else
#include "msvc_alt_types.h"
#endif
// Microsoft Visual Studio-specific

#include "defs.h" 

struct PopUpSchemaParams;

// Log Schema processing functions
//
void signalsViewShow(void);
void signalsViewProc(struct PopUpSchemaParams* const schema);

void signalsScrollUp(void);
void signalsScrollDown(void);

void signalsScrollPgUp(void);
void signalsScrollPgDown(void);

#endif
