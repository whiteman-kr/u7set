/*
Module title: LUA API
(C) 2026 RPC "Radiy".
All rights reserved.
The information presented here is confidential
and is the intellectual property of PJSC "RPC "Radiy".
Any use of this information is strictly
prohibited without the written consent of PJSC "RPC "Radiy".
Module description: Provides data exchange with the Lua environment.

-- *************************************************************
-- Version v1.00
-- Date:	31,03,2026.
-- Designed by: Mazyr S.M. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Vintenko B.U.

-- Changes: RADIX-3966, rn_lapi initial update

-- *************************************************************
-- Version v1.01
-- Date:	01,05,2026.
-- Designed by: Vintenko B.U.
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-3966, rn_lapi module refactoring
-- Added GlobalScript support
-- Add logShow, logErrorCount, logWarningCount, logClearCounters functions
-- Add signalAvailableByIndex function
-- Add timerValue, timerMaxValue functions
-- Add vary screen resolution support

-- Changes: RADIX-3969, add logging and debugging support
-- Add log errors and messages

-- *************************************************************

-- Version v1.02
-- Date:	09,05,2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-4009
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-4009, adding API support for working with signals by AppSignalIDs
-- RADIX-3969, add API support for logging functions
-- RADIX-3966, schema switching improvement: it is scheduled and actually changed at the next schemaProc loop to prevent scripts data corruption
-- RADIX-3973: add schema items checking and blink phase functions

-- *************************************************************

-- Version v1.03
-- Date:	28,07,2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-3966, code is made MISRA-2025 compatible

-- *************************************************************

-- Version v1.04
-- Date:	06.08.2026.
-- Designed by: Vintenko B.U.
-- Reviewed by: Nesterenko M.V.

-- Changes: 
RADIX-4106, add functions: signalIdByIndex, signalCaptionByIndex, signalUnitsByIndex, signalId, signalCaption, signalUnits 
RADIX-4108, add deviceEquipmentId and deviceCaption functions
RADIX-4109, add itemRectSetText function

-- *************************************************************
*/


#ifndef RN_LAPI_H
#define RN_LAPI_H

// Microsoft Visual Studio-specific
#ifndef RVDU_SIM        

#include <alt_types.h>

#else

#include "msvc_alt_types.h"

#endif
// Microsoft Visual Studio-specific

#include <lua.h>

#include <stdbool.h>

struct lua_State;

lua_State* moduleRnLapiInitialize(alt_u16 screenWidth, alt_u16 screenHeight);

bool compileGlobalScript(lua_State* luaState, const char* globalScript);

#endif
