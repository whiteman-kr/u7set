/*
Module title: Description File Processing.

(C) 2026 RPC "Radiy".
All rights reserved.
The information presented here is confidential
and is the intellectual property of PJSC "RPC "Radiy".
Any use of this information is strictly
prohibited without the written consent of PJSC "RPC "Radiy".

Module description: Reads and interprets description files that define the list of 
available RVDUs in the subsystem, Equipment identifiers, Configuration numbers and Jumpers configuration.

-- *************************************************************
-- Version v1.00
-- Date:	31,03,2026.
-- Designed by: Mazyr S.M. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Vintenko B.U.
-- Changes: RADIX-3966, rn_desc initial update

-- *************************************************************
-- Version v1.01
-- Date:	01,05,2026.
-- Designed by: Mazyr S.M. https://jira.radiy.com/browse/RADIX-3966, https://jira.radiy.com/browse/RADIX-3969
-- Reviewed by: Vintenko B.U.

-- Changes: RADIX-3966, rn_desc module refactoring
-- Change sprintf to snprintf, strcmp to strncmp for MISRA suggestion
-- Add version history to module descriptions

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

#ifndef RN_DESC_H
#define RN_DESC_H

// Microsoft Visual Studio-specific
#ifndef RVDU_SIM        
#include <alt_types.h>
#else
#include "msvc_alt_types.h"
#endif
// Microsoft Visual Studio-specific

#include <stdbool.h>

/*
Finds a record in description file that has the same key set by jumpers as passed by thee quipmentKey parameter.
Records the device identifier to the address specified by the equipmentId parameter.
Parameters:
    equipmentKey - a hardware-set key of the device (jumpers)
    descriptionFile - description file name
    equipmentId - pointer to a buffer where to store the identifier
    bufferSize - size, in bytes, of the buffer.
Return Value:
    Returns "true" if successful, "false" on error.
*/
bool findEquipmentId(alt_u16 equipmentKey, const char* const descriptionFile, char* const equipmentId, alt_u16 bufferSize);

#endif