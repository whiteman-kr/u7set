/*
Module title: Log View.
(C) 2026 RPC "Radiy".
All rights reserved.
The information presented here is confidential
and is the intellectual property of PJSC "RPC 'Radiy".
Any use of this information is strictly
prohibited without the written consent of PJSC "RPC "Radiy".
Module description: Provides log view.

-- *************************************************************
-- Version v1.00
-- Date:	15,04,2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966, https://jira.radiy.com/browse/RADIX-3969
-- Reviewed by: Mazyr S.M.

-- Changes: RADIX-3969, rn_logiew added
-- Add keyboard support
-- Add logScrollPgUp, logScrollPgDown functions

-- Changes: RADIX-3969, add logging and debugging support
-- Add log errors and messages

-- *************************************************************

-- Version v1.01
-- Date:	09,05,2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-4009
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-4009, adding API support for working with signals by AppSignalIDs

-- *************************************************************

-- Version v1.02
-- Date:	28,07,2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-3966, code is made MISRA-2025 compatible

-- *************************************************************
*/

#ifndef RN_LOGVEW_H
#define RN_LOGVEW_H

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
void logSchemaShow(void);
void logSchemaProc(struct PopUpSchemaParams* const schema);

void logScrollUp(void);
void logScrollDown(void);

void logScrollPgUp(void);
void logScrollPgDown(void);

#endif
