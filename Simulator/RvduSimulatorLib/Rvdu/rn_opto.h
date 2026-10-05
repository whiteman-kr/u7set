/*
Module title: Optical Signals Processing
(C) 2026 RPC "Radiy".
All rights reserved.
The information presented here is confidential
and is the intellectual property of PJSC "RPC "Radiy".
Any use of this information is strictly
prohibited without the written consent of PJSC "RPC "Radiy".
Module description: Sends application signals by the optical interface.

-- *************************************************************
-- Version v1.00
-- Date:	13.04.2026.
-- Designed by: Mazyr S.M. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Vintenko B.U.

-- Changes: RADIX-3966, rn_opto initial update

-- *************************************************************
-- Version v1.01
-- Date:	29.04.2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966, https://jira.radiy.com/browse/RADIX-3969
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-3966, rn_opto module refactoring

-- Changes: RADIX-3969, add logging and debugging support
-- Add log errors and messages

-- *************************************************************

-- Version v1.02
-- Date:	09.05.2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-4009
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-4009, adding API support for working with signals by AppSignalIDs

-- *************************************************************

-- Version v1.03
-- Date:	28.07.2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-3966, code is made MISRA-2025 compatible

-- *************************************************************

-- Version v1.04
-- Date:	25.09.2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-4148
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-4148, rn_opto сorrecting errors identified by static analysis testing

-- *************************************************************

*/

#ifndef RN_OPTO_H
#define RN_OPTO_H

#define WORDS_PER_OPTO_CHANNEL 2560u

// This function locks/unlocks the read and write memory and calls a procedure that writes
// application signal values by optical interfaces, works as a wrapper to automatically unlock the memory
//
void optoTxProc(void);

#endif