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

-- Version v1.05
-- Date:	10,08,2026.
-- Designed by: Vintenko B.U.
-- Reviewed by: Nesterenko M.V.

-- Changes:
	RADIX-4112, API functions improvements

-- *************************************************************
*/

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#ifndef RVDU_SIM // Microsoft Visual Studio-specific

#include "../rvdu_bsp/HAL/inc/sys/alt_errno.h"

#include "../rvdu_bsp/system.h"

#endif // Microsoft Visual Studio-specific

#include "rn_lapi.h"
#include "rn_scfp.h"
#include "rn_cfg.h"
#include "rn_sifp.h"
#include "rn_schema.h"
#include "rn_diag.h"
#include "rn_ts.h"
#include "rn_utils.h"
#include "rn_trendproc.h"

#include "defs.h"

#include <lualib.h>
#include <lauxlib.h>

// Private module data
//
static alt_u16 m_screenWidth = 0u;
static alt_u16 m_screenHeight = 0u;
//
// End of Private data


//*************************************
// GLOBAL
//*************************************

static int deviceEquipmentId(lua_State* L);
static int deviceCaption(lua_State* L);

//*************************************
// SCHEMAS
//*************************************

static int schemaId(lua_State* L);
static int schemaExists(lua_State* L);
static int schemaCaption(lua_State* L);
static int schemaCaptionById(lua_State* L);
static int setSchema(lua_State* L);

static int luaBlinkPhase(lua_State* L);

//*************************************
// Common - AppSignals
//*************************************

static int signalExistsByIndex(lua_State* L);
static int signalIsAvailableByIndex(lua_State* L);
static int luaSignalIdByIndex(lua_State* L);
static int luaSignalCaptionByIndex(lua_State* L);
static int luaSignalUnitsByIndex(lua_State* L);
static int isDiscreteByIndex(lua_State* L);
static int isAnalogByIndex(lua_State* L);
static int isTuningByIndex(lua_State* L);
static int signalTypeByIndex(lua_State* L);
static int signalValueByIndexInt(lua_State* L);
static int signalValueByIndexFloat(lua_State* L);

//*************************************
// Common - AppSignals by appSignalId
//*************************************

static int signalExists(lua_State* L);
static int signalIsAvailable(lua_State* L);
static int luaSignalId(lua_State* L);
static int luaSignalCaption(lua_State* L);
static int luaSignalUnits(lua_State* L);
static int isDiscrete(lua_State* L);
static int isAnalog(lua_State* L);
static int isTuning(lua_State* L);
static int signalType(lua_State* L);
static int signalValueInt(lua_State* L);
static int signalValueFloat(lua_State* L);


//*************************************
// Properties - AppSignals
//*************************************

static int signalTuningDefaultValueByIndexInt(lua_State* L);
static int signalTuningDefaultValueByIndexFloat(lua_State* L);
static int signalLowEngUnitsByIndexInt(lua_State* L);
static int signalLowEngUnitsByIndexFloat(lua_State* L);
static int signalHighEngUnitsByIndexInt(lua_State* L);
static int signalHighEngUnitsByIndexFloat(lua_State* L);
static int decimalPlacesByIndex(lua_State* L);

//*************************************
// Properties - AppSignals by appSignalId
//*************************************

static int signalTuningDefaultValueInt(lua_State* L);
static int signalTuningDefaultValueFloat(lua_State* L);
static int signalLowEngUnitsInt(lua_State* L);
static int signalLowEngUnitsFloat(lua_State* L);
static int signalHighEngUnitsInt(lua_State* L);
static int signalHighEngUnitsFloat(lua_State* L);
static int decimalPlaces(lua_State* L);

//*************************************
// Control - AppSignals
//*************************************

static int writeValueByIndexInt(lua_State* L);
static int writeValueByIndexFloat(lua_State* L);
static int copyValueByIndex(lua_State* L);

static int writeValueInt(lua_State* L);
static int writeValueFloat(lua_State* L);
static int copyValue(lua_State* L);

static int itemSignalValueInt(lua_State* L);
static int itemSignalValueFloat(lua_State* L);

//*************************************
// SchemaItem - Common
//*************************************

static int itemIsValid(lua_State* L);
static int currentItem(lua_State* L);
static int itemByObjectName(lua_State* L);
static int itemType(lua_State* L);
static int itemObjectName(lua_State* L);
static int itemSignalCount(lua_State* L);
static int itemAppSignalIds(lua_State* L);
static int signalGlobalIndex(lua_State* L);
static int itemAcceptClick(lua_State* L);
static int itemSetAcceptClick(lua_State* L);
static int itemSetLeft(lua_State* L);
static int itemSetTop(lua_State* L);
static int itemSetWidth(lua_State* L);
static int itemSetHeight(lua_State* L);
static int itemLeft(lua_State* L);
static int itemTop(lua_State* L);
static int itemWidth(lua_State* L);
static int itemHeight(lua_State* L);

//*************************************
// SchemaItemVduLine
//*************************************

static int itemLineWeight(lua_State* L);
static int itemLineSetWeight(lua_State* L);
static int itemLineColor(lua_State* L);
static int itemLineSetLineColor(lua_State* L);

//*************************************
// SchemaItemVduRect
//*************************************

static int itemRectWeight(lua_State* L);
static int itemRectSetWeight(lua_State* L);
static int itemRectFill(lua_State* L);
static int itemRectSetFill(lua_State* L);
static int itemRectDrawRect(lua_State* L);
static int itemRectSetDrawRect(lua_State* L);
static int itemRectLineColor(lua_State* L);
static int itemRectSetLineColor(lua_State* L);
static int itemRectFillColor(lua_State* L);
static int itemRectSetFillColor(lua_State* L);
static int itemRectTextColor(lua_State* L);
static int itemRectSetTextColor(lua_State* L);
static int itemRectText(lua_State* L);
static int itemRectSetText(lua_State* L);

//*************************************
// SchemaItemVduValue
//*************************************

static int itemValueWeight(lua_State* L);
static int itemValueSetWeight(lua_State* L);
static int itemValueDrawRect(lua_State* L);
static int itemValueSetDrawRect(lua_State* L);
static int itemValueLineColor(lua_State* L);
static int itemValueSetLineColor(lua_State* L);
static int itemValueFillColor(lua_State* L);
static int itemValueSetFillColor(lua_State* L);
static int itemValueTextColor(lua_State* L);
static int itemValueSetTextColor(lua_State* L);
static int itemValueDecimalPlaces(lua_State* L);
static int itemValueSetDecimalPlaces(lua_State* L);
static int itemValueText(lua_State* L);
static int itemValueSetText(lua_State* L);

//*************************************
// SchemaItemImageValue
//*************************************

static int itemImageValueCurrentImageID(lua_State* L);
static int itemImageValueImageExists(lua_State* L);
static int itemImageValueSetCurrentImageID(lua_State* L);

//*************************************
// ADDITIONAL
//*************************************

static int editMode(lua_State* L);
static int editModeByIndex(lua_State* L);

static int checkPinCode(lua_State* L);
static int checkPinCodeByIndex(lua_State* L);

static int questionMessageBox(lua_State* L);

//*************************************
// LOG
//*************************************

static int luaLogShow(lua_State* L);
static int luaLogErrorCount(lua_State* L);
static int luaLogWarningCount(lua_State* L);
static int luaLogClearCounters(lua_State* L);

static int luaLogWriteError(lua_State* L);
static int luaLogWriteWarning(lua_State* L);
static int luaLogWriteMessage(lua_State* L);

//*************************************
// TIMER
//*************************************

static int luaTimerValue(lua_State* L);
static int luaTimerMaxValue(lua_State* L);

//*************************************
// SIGNALS
//*************************************
static int luaSignalsShow(lua_State* L);

//*************************************
// Trends
//*************************************
static int luaSetTrendDuration(lua_State* L);
static int luaSetTrendViewMode(lua_State* L);
static int luaSetTrendScaleType(lua_State* L);

static int luaSetTrendSignalHighViewLimit(lua_State* L);
static int luaSetTrendSignalLowViewLimit(lua_State* L);
static int luaSetTrendSignalLineWidth(lua_State* L);
static int luaSetTrendSignalLineColor(lua_State* L);
static int luaSetTrendSignalValueFormat(lua_State* L);
static int luaSetTrendSignalDecimalPlaces(lua_State* L);
static int luaSetTrendSignalVisible(lua_State* L);

//*************************************
// End of forward-declarations
//*************************************

//*************************************
// Pointers and strings checking macros
//*************************************

static bool schemaItemPtrIsValid(const struct VduSchemaFileSchemaItem1* const schemaItem)
{
	if (schemaItem == NULL)
	{
		return false;
	}

	if ((schemaItem->itemType != TYPE_LINE) &&
		(schemaItem->itemType != TYPE_RECT) &&
		(schemaItem->itemType != TYPE_VALUE) &&
		(schemaItem->itemType != TYPE_IMAGE) &&
		(schemaItem->itemType != TYPE_IMAGEVALUE) &&
		(schemaItem->itemType != TYPE_TREND)
		)
	{
		return false;
	}

	return true;
}

#define SCHEMA_ITEM_PTR_IS_VALID(expr) schemaItemPtrIsValid((expr))

static bool safeStrCopy(char* dst, size_t dstSize, const char* src)
{
	if ((dst == NULL) || (dstSize == 0u))
	{
		return false;
	}

	if (src == NULL)
	{
		dst[0] = '\0';
		return  false;
	}

	(void)strncpy(dst, src, dstSize);
	dst[dstSize - 1u] = '\0';

	return true;
}

#define SAFE_STR_COPY(dst, size, src) \
    safeStrCopy((dst), (size), (src))

//*************************************
// End of pointers and strings checking macros
//*************************************

static lua_State* scriptApiRegistration(void)
{
	lua_State* rvduLuaState = luaL_newstate();
	if (rvduLuaState == NULL)
	{
		RN_ASSERT(rvduLuaState != NULL);
		return NULL;
	}

	lua_register(rvduLuaState, "deviceEquipmentId", deviceEquipmentId);
	lua_register(rvduLuaState, "deviceCaption", deviceCaption);

	lua_register(rvduLuaState, "schemaId", schemaId);
	lua_register(rvduLuaState, "schemaExists", schemaExists);
	lua_register(rvduLuaState, "schemaCaption", schemaCaption);
	lua_register(rvduLuaState, "schemaCaptionById", schemaCaptionById);
	lua_register(rvduLuaState, "setSchema", setSchema);
	lua_register(rvduLuaState, "blinkPhase", luaBlinkPhase);

	//*************************************
	// Common - AppSignals
	//*************************************

	lua_register(rvduLuaState, "signalExistByIndex", signalExistsByIndex);
	lua_register(rvduLuaState, "signalExistsByIndex", signalExistsByIndex);
	lua_register(rvduLuaState, "signalIdByIndex", luaSignalIdByIndex);
	lua_register(rvduLuaState, "signalCaptionByIndex", luaSignalCaptionByIndex);
	lua_register(rvduLuaState, "signalUnitsByIndex", luaSignalUnitsByIndex);
	lua_register(rvduLuaState, "signalAvailableByIndex", signalIsAvailableByIndex);
	lua_register(rvduLuaState, "isDiscreteByIndex", isDiscreteByIndex);
	lua_register(rvduLuaState, "isAnalogByIndex", isAnalogByIndex);
	lua_register(rvduLuaState, "isTuningByIndex", isTuningByIndex);
	lua_register(rvduLuaState, "signalTypeByIndex", signalTypeByIndex);
	lua_register(rvduLuaState, "signalValueByIndexInt", signalValueByIndexInt);
	lua_register(rvduLuaState, "signalValueByIndexFloat", signalValueByIndexFloat);

	//*************************************
	// Common - AppSignals by appSignalId
	//*************************************

	lua_register(rvduLuaState, "signalExist", signalExists);
	lua_register(rvduLuaState, "signalExists", signalExists);
	lua_register(rvduLuaState, "signalId", luaSignalId);
	lua_register(rvduLuaState, "signalCaption", luaSignalCaption);
	lua_register(rvduLuaState, "signalUnits", luaSignalUnits);
	lua_register(rvduLuaState, "signalAvailable", signalIsAvailable);
	lua_register(rvduLuaState, "isDiscrete", isDiscrete);
	lua_register(rvduLuaState, "isAnalog", isAnalog);
	lua_register(rvduLuaState, "isTuning", isTuning);
	lua_register(rvduLuaState, "signalType", signalType);
	lua_register(rvduLuaState, "signalValueInt", signalValueInt);
	lua_register(rvduLuaState, "signalValueFloat", signalValueFloat);

	//*************************************
	// Properties - AppSignals
	//*************************************

	lua_register(rvduLuaState, "signalTuningDefaultValueByIndexInt", signalTuningDefaultValueByIndexInt);
	lua_register(rvduLuaState, "signalTuningDefaultValueByIndexFloat", signalTuningDefaultValueByIndexFloat);

	lua_register(rvduLuaState, "signalLowEngUnitsByIndexInt", signalLowEngUnitsByIndexInt);
	lua_register(rvduLuaState, "signalLowEngUnitsByIndexFloat", signalLowEngUnitsByIndexFloat);
	lua_register(rvduLuaState, "signalTuningLowBoundByIndexInt", signalLowEngUnitsByIndexInt);
	lua_register(rvduLuaState, "signalTuningLowBoundByIndexFloat", signalLowEngUnitsByIndexFloat);

	lua_register(rvduLuaState, "signalHighEngUnitsByIndexInt", signalHighEngUnitsByIndexInt);
	lua_register(rvduLuaState, "signalHighEngUnitsByIndexFloat", signalHighEngUnitsByIndexFloat);
	lua_register(rvduLuaState, "signalTuningHighBoundByIndexInt", signalHighEngUnitsByIndexInt);
	lua_register(rvduLuaState, "signalTuningHighBoundByIndexFloat", signalHighEngUnitsByIndexFloat);

	lua_register(rvduLuaState, "decimalPlacesByIndex", decimalPlacesByIndex);

	//*************************************
	// Properties - AppSignals by appSignalId
	//*************************************

	lua_register(rvduLuaState, "signalTuningDefaultValueInt", signalTuningDefaultValueInt);
	lua_register(rvduLuaState, "signalTuningDefaultValueFloat", signalTuningDefaultValueFloat);

	lua_register(rvduLuaState, "signalLowEngUnitsInt", signalLowEngUnitsInt);
	lua_register(rvduLuaState, "signalLowEngUnitsFloat", signalLowEngUnitsFloat);
	lua_register(rvduLuaState, "signalTuningLowBoundInt", signalLowEngUnitsInt);
	lua_register(rvduLuaState, "signalTuningLowBoundFloat", signalLowEngUnitsFloat);

	lua_register(rvduLuaState, "signalHighEngUnitsInt", signalHighEngUnitsInt);
	lua_register(rvduLuaState, "signalHighEngUnitsFloat", signalHighEngUnitsFloat);
	lua_register(rvduLuaState, "signalTuningHighBoundInt", signalHighEngUnitsInt);
	lua_register(rvduLuaState, "signalTuningHighBoundFloat", signalHighEngUnitsFloat);

	lua_register(rvduLuaState, "decimalPlaces", decimalPlaces);

	//*************************************
	// Control - AppSignals
	//*************************************

	lua_register(rvduLuaState, "writeValueByIndexInt", writeValueByIndexInt);
	lua_register(rvduLuaState, "writeValueByIndexFloat", writeValueByIndexFloat);
	lua_register(rvduLuaState, "copyValueByIndex", copyValueByIndex);

	lua_register(rvduLuaState, "writeValueInt", writeValueInt);
	lua_register(rvduLuaState, "writeValueFloat", writeValueFloat);
	lua_register(rvduLuaState, "copyValue", copyValue);

	//

	lua_register(rvduLuaState, "itemSignalValueInt", itemSignalValueInt);
	lua_register(rvduLuaState, "itemSignalValueFloat", itemSignalValueFloat);

	//*************************************
	// SchemaItem - Common
	//*************************************

	lua_register(rvduLuaState, "itemIsValid", itemIsValid);
	lua_register(rvduLuaState, "currentItem", currentItem);
	lua_register(rvduLuaState, "itemByObjectName", itemByObjectName);
	lua_register(rvduLuaState, "itemType", itemType);
	lua_register(rvduLuaState, "itemObjectName", itemObjectName);
	lua_register(rvduLuaState, "itemSignalCount", itemSignalCount);
	lua_register(rvduLuaState, "itemAppSignalIds", itemAppSignalIds);
	lua_register(rvduLuaState, "signalGlobalIndex", signalGlobalIndex);
	lua_register(rvduLuaState, "itemAcceptClick", itemAcceptClick);
	lua_register(rvduLuaState, "itemSetAcceptClick", itemSetAcceptClick);

	lua_register(rvduLuaState, "itemSetLeft", itemSetLeft);
	lua_register(rvduLuaState, "itemSetTop", itemSetTop);
	lua_register(rvduLuaState, "itemSetWidth", itemSetWidth);
	lua_register(rvduLuaState, "itemSetHeight", itemSetHeight);
	lua_register(rvduLuaState, "itemLeft", itemLeft);
	lua_register(rvduLuaState, "itemTop", itemTop);
	lua_register(rvduLuaState, "itemWidth", itemWidth);
	lua_register(rvduLuaState, "itemHeight", itemHeight);

	//*************************************
	// SchemaItemVduLine
	//*************************************

	lua_register(rvduLuaState, "itemLineWeight", itemLineWeight);
	lua_register(rvduLuaState, "itemLineSetWeight", itemLineSetWeight);
	lua_register(rvduLuaState, "itemLineColor", itemLineColor);
	lua_register(rvduLuaState, "itemLineSetLineColor", itemLineSetLineColor);

	//*************************************
	// SchemaItemVduRect
	//*************************************

	lua_register(rvduLuaState, "itemRectWeight", itemRectWeight);
	lua_register(rvduLuaState, "itemRectSetWeight", itemRectSetWeight);
	lua_register(rvduLuaState, "itemRectFill", itemRectFill);
	lua_register(rvduLuaState, "itemRectSetFill", itemRectSetFill);
	lua_register(rvduLuaState, "itemRectDrawRect", itemRectDrawRect);
	lua_register(rvduLuaState, "itemRectSetDrawRect", itemRectSetDrawRect);
	lua_register(rvduLuaState, "itemRectLineColor", itemRectLineColor);
	lua_register(rvduLuaState, "itemRectSetLineColor", itemRectSetLineColor);
	lua_register(rvduLuaState, "itemRectFillColor", itemRectFillColor);
	lua_register(rvduLuaState, "itemRectSetFillColor", itemRectSetFillColor);
	lua_register(rvduLuaState, "itemRectTextColor", itemRectTextColor);
	lua_register(rvduLuaState, "itemRectSetTextColor", itemRectSetTextColor);
	lua_register(rvduLuaState, "itemRectText", itemRectText);
	lua_register(rvduLuaState, "itemRectSetText", itemRectSetText);

	//*************************************
	// SchemaItemVduValue
	//*************************************

	lua_register(rvduLuaState, "itemValueWeight", itemValueWeight);
	lua_register(rvduLuaState, "itemValueSetWeight", itemValueSetWeight);
	lua_register(rvduLuaState, "itemValueDrawRect", itemValueDrawRect);
	lua_register(rvduLuaState, "itemValueSetDrawRect", itemValueSetDrawRect);
	lua_register(rvduLuaState, "itemValueLineColor", itemValueLineColor);
	lua_register(rvduLuaState, "itemValueSetLineColor", itemValueSetLineColor);
	lua_register(rvduLuaState, "itemValueFillColor", itemValueFillColor);
	lua_register(rvduLuaState, "itemValueSetFillColor", itemValueSetFillColor);
	lua_register(rvduLuaState, "itemValueTextColor", itemValueTextColor);
	lua_register(rvduLuaState, "itemValueSetTextColor", itemValueSetTextColor);
	lua_register(rvduLuaState, "itemValueDecimalPlaces", itemValueDecimalPlaces);
	lua_register(rvduLuaState, "itemValueSetDecimalPlaces", itemValueSetDecimalPlaces);
	lua_register(rvduLuaState, "itemValueText", itemValueText);
	lua_register(rvduLuaState, "itemValueSetText", itemValueSetText);

	//*************************************
	// SchemaItemImageValue
	//*************************************

	lua_register(rvduLuaState, "itemImageValueCurrentImageID", itemImageValueCurrentImageID);
	lua_register(rvduLuaState, "itemImageValueImageExists", itemImageValueImageExists);
	lua_register(rvduLuaState, "itemImageValueSetCurrentImageID", itemImageValueSetCurrentImageID);

	//*************************************
	// ADDITIONAL
	//*************************************

	lua_register(rvduLuaState, "enterValueScreen", editMode);
	lua_register(rvduLuaState, "editMode", editModeByIndex);	// obsolete
	lua_register(rvduLuaState, "enterValueScreenByIndex", editModeByIndex);

	lua_register(rvduLuaState, "checkPinCode", checkPinCode);
	lua_register(rvduLuaState, "checkPinCodeByIndex", checkPinCodeByIndex);

	lua_register(rvduLuaState, "questionMessageBox", questionMessageBox);

	lua_register(rvduLuaState, "logShow", luaLogShow);
	lua_register(rvduLuaState, "logErrorCount", luaLogErrorCount);
	lua_register(rvduLuaState, "logWarningCount", luaLogWarningCount);
	lua_register(rvduLuaState, "logClearCounters", luaLogClearCounters);

	lua_register(rvduLuaState, "logWriteError", luaLogWriteError);
	lua_register(rvduLuaState, "logWriteWarning", luaLogWriteWarning);
	lua_register(rvduLuaState, "logWriteMessage", luaLogWriteMessage);

	lua_register(rvduLuaState, "timerValue", luaTimerValue);
	lua_register(rvduLuaState, "timerMaxValue", luaTimerMaxValue);

	lua_register(rvduLuaState, "signalsShow", luaSignalsShow);
	
	// Trends

	lua_register(rvduLuaState, "setTrendDuration", luaSetTrendDuration);
	lua_register(rvduLuaState, "setTrendViewMode", luaSetTrendViewMode);
	lua_register(rvduLuaState, "setTrendScaleType", luaSetTrendScaleType);

	lua_register(rvduLuaState, "setTrendSignalHighViewLimit", luaSetTrendSignalHighViewLimit);
	lua_register(rvduLuaState, "setTrendSignalLowViewLimit", luaSetTrendSignalLowViewLimit);
	lua_register(rvduLuaState, "setTrendSignalLineWidth", luaSetTrendSignalLineWidth);
	lua_register(rvduLuaState, "setTrendSignalLineColor", luaSetTrendSignalLineColor);
	lua_register(rvduLuaState, "setTrendSignalValueFormat", luaSetTrendSignalValueFormat);
	lua_register(rvduLuaState, "setTrendSignalDecimalPlaces", luaSetTrendSignalDecimalPlaces);
	lua_register(rvduLuaState, "setTrendSignalVisible", luaSetTrendSignalVisible);

	luaL_openlibs(rvduLuaState);

	return rvduLuaState;
}

lua_State* moduleRnLapiInitialize(alt_u16 screenWidth, alt_u16 screenHeight)
{
	static bool m_moduleInitialized = false;
	if (m_moduleInitialized == true)
	{
		RN_ASSERT(false);
		return NULL;
	}
	m_moduleInitialized = true;

	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	return scriptApiRegistration();
}

static int writer(lua_State* L,	const void* p, size_t sz, void* ud)
{
	(void)L;
	(void)p;
	size_t* total = (size_t*)ud;
	*total += sz;
	return 0;
}

bool compileGlobalScript(lua_State* luaState, const char* globalScript)
{
	char msg[STRING_MAX_LEN];

	if (luaState == NULL) 
	{
		return false;
	}
	
	if (globalScript == NULL)
	{
		return false;
	}

	if (strlen(globalScript) == 0u)
	{
		return true;
	}

	if (luaL_loadstring(luaState, globalScript) != LUA_OK)
	{
		(void)snprintf(msg, STRING_MAX_LEN, "compileGlobalScript, compile error: %s", lua_tostring(luaState, -1));
		logError(GlobalScriptError, msg);
		
		lua_pop(luaState, 1);
		return false;
	}

	int total = 0;
	if (lua_dump(luaState, writer, &total, 0) != LUA_OK)
	{
		(void)snprintf(msg, STRING_MAX_LEN, "compileGlobalScript, dump error: %s", lua_tostring(luaState, -1));
		logError(GlobalScriptError, msg);

		lua_pop(luaState, 1);
		return false;
	}
	(void)snprintf(msg, STRING_MAX_LEN, "compileGlobalScript: code size is %d bytes.", total);
	logMessage(msg);

	// execute compiled chunk
	//
	if (lua_pcall(luaState, 0, 0, 0) != LUA_OK)
	{
		(void)snprintf(msg, STRING_MAX_LEN, "compileGlobalScript, execution error: %s", lua_tostring(luaState, -1));
		logError(GlobalScriptError, msg);

		lua_pop(luaState, 1);
		return false;
	}
	
	return true;
}

static int deviceEquipmentId(lua_State* L)
{
	const struct VduConfigFile1* const config = vduConfig();
	if (config == NULL)
	{
		RN_ASSERT(config != NULL);
		lua_pushnil(L);
	}
	else
	{
		(void)lua_pushstring(L, vduConfigString(vduConfig()->equipmentId));
	}

	return 1;
}

static int deviceCaption(lua_State* L)
{
	const struct VduConfigFile1* const config = vduConfig();
	if (config == NULL)
	{
		RN_ASSERT(config != NULL);
		lua_pushnil(L);
	}
	else
	{
		(void)lua_pushstring(L, vduConfigString(vduConfig()->caption));
	}

	return 1;
}

//*************************************
// SCHEMAS
//*************************************

static int schemaId(lua_State* L)
{
	const struct VduSchemaFile* const schema = vduCurrentSchema();
	if (schema == NULL)
	{
		RN_ASSERT(schema != NULL);
		lua_pushnil(L);
	}
	else
	{
		(void)lua_pushstring(L, vduCurrentSchemaString(schema->schemaProperties.schemaId));
	}

	return 1;
}

static int schemaExists(lua_State* L)
{
	char id[STRING_MAX_LEN] = { 0 };
	if (SAFE_STR_COPY(id, STRING_MAX_LEN, lua_tostring(L, 1)) == false)
	{
		logWarning(SchemaWasNotFound, "LUA schemaExists: schemaId = NULL.");
		lua_pushboolean(L, false);
		return 1;
	}

	if (vduSchemaByID(id) == NULL)
	{
		lua_pushboolean(L, false);
	}
	else
	{
		lua_pushboolean(L, true);
	}

	return 1;
}

static int schemaCaption(lua_State* L)
{
	const struct VduSchemaFile* const schema = vduCurrentSchema();
	if (schema == NULL)
	{
		RN_ASSERT(schema != NULL);
		lua_pushnil(L);
	}
	else
	{
		(void)lua_pushstring(L, vduCurrentSchemaString(schema->schemaProperties.caption));
	}

	return 1;
}

static int schemaCaptionById(lua_State* L)
{
	char id[STRING_MAX_LEN] = { 0 };
	if (SAFE_STR_COPY(id, STRING_MAX_LEN, lua_tostring(L, 1)) == false)
	{
		logWarning(SchemaWasNotFound, "LUA schemaCaptionById: schemaId = NULL.");
		lua_pushnil(L);
		return 1;
	}

	alt_u16 count = vduConfig()->schemaCount;
	for (alt_u16 i = 0; i < count; i++)
	{
		const struct VduConfigSchema1* const pSchema = vduConfigSchema(i);
		if (pSchema == NULL)
		{
			RN_ASSERT(pSchema != NULL);
			break;
		}

		if (strncmp(id, vduConfigString(pSchema->schemaId), STRING_MAX_LEN) == 0)
		{
			(void)lua_pushstring(L, vduConfigString(pSchema->caption));
			return 1;
		}
	}

	lua_pushnil(L);
	return 1;
}

static int setSchema(lua_State* L)
{
	char id[STRING_MAX_LEN] = { 0 };
	if (SAFE_STR_COPY(id, STRING_MAX_LEN, lua_tostring(L, 1)) == false)
	{
		logWarning(SchemaWasNotFound, "LUA setSchema: schemaId = NULL.");
		return 0;
	}

	(void)vduScheduleSchemaChange(id);
	
	schemaSetTerminate();

	return 0;
}

static int luaBlinkPhase(lua_State* L)
{
	lua_pushboolean(L, blinkPhase());
	return 1;
}

//*************************************
// Common - AppSignals
//*************************************

static int signalExistsByIndex( lua_State* L )
{
	alt_u16 signalIndex = luaL_checkinteger ( L, 1 );
	bool res = false;

	if (signalIndex < appSignalsCount())
	{
		res = true;
	}

	lua_pushboolean( L, res );
	return 1;
}

static int signalIsAvailableByIndex(lua_State* L)
{
	alt_u16 signalIndex = luaL_checkinteger(L, 1);

	if (signalIndex < appSignalsCount())
	{
		lua_pushboolean(L, (signalAvailableByIndex(signalIndex) == true) ? 1 : 0);
	}
	else
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "LUA signalIsAvailableByIndex: signal with index %u not found.", (unsigned int)signalIndex);
		logWarning(SignalWasNotFound, msg);

		lua_pushboolean(L, false);
	}
	
	return 1;
}

static int luaSignalIdByIndex(lua_State* L)
{
	alt_u16 signalIndex = luaL_checkinteger(L, 1);

	const struct VduAppSignalParam* const param = appSignalParamByIndex(signalIndex);
	if (param == NULL)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "LUA signalIdByIndex: signal with index %u not found.", (unsigned int)signalIndex);
		logWarning(SignalWasNotFound, msg);
		lua_pushnil(L);
	}
	else
	{
		(void)lua_pushstring(L, customAppSignalID(signalIndex));
	}
	return 1;
}

static int luaSignalCaptionByIndex(lua_State* L)
{
	alt_u16 signalIndex = luaL_checkinteger(L, 1);

	const struct VduAppSignalParam* const param = appSignalParamByIndex(signalIndex);
	if (param == NULL)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "LUA signalCaptionByIndex: signal with index %u not found.", (unsigned int)signalIndex);
		logWarning(SignalWasNotFound, msg);
		lua_pushnil(L);
	}
	else
	{
		(void)lua_pushstring(L, signalCaption(signalIndex));
	}
	return 1;
}

static int luaSignalUnitsByIndex(lua_State* L)
{
	alt_u16 signalIndex = luaL_checkinteger(L, 1);

	const struct VduAppSignalParam* const param = appSignalParamByIndex(signalIndex);
	if (param == NULL)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "LUA signalUnitsByIndex: signal with index %u not found.", (unsigned int)signalIndex);
		logWarning(SignalWasNotFound, msg);
		lua_pushnil(L);
	}
	else
	{
		(void)lua_pushstring(L, signalUnit(signalIndex));
	}
	return 1;
}

static int isDiscreteByIndex( lua_State* L )
{
	alt_u16 signalIndex = luaL_checkinteger ( L, 1 );

	bool res = false;

	const struct VduAppSignalParam* const param = appSignalParamByIndex(signalIndex);
	if (param == NULL)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "LUA isDiscreteByIndex: signal with index %u not found.", (unsigned int)signalIndex);
		logWarning(SignalWasNotFound, msg);
	}
	else
	{
		if ( param->vduSignalType == (alt_u16)Discrete )
		{
			res = true;
		}
	}

	lua_pushboolean( L, (res == true) ? 1 : 0);
	return 1;
}

static int isAnalogByIndex( lua_State* L )
{
	alt_u16 signalIndex = luaL_checkinteger ( L, 1 );

	bool res = false;

	const struct VduAppSignalParam* const param = appSignalParamByIndex(signalIndex);
	if (param == NULL)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "LUA isAnalogByIndex: signal with index %u not found.", (unsigned int)signalIndex);
		logWarning(SignalWasNotFound, msg);
	}
	else
	{
		if ((param->vduSignalType == (alt_u16)AnalogFloat32) || (param->vduSignalType == (alt_u16)AnalogSignedInt32))
		{
			res = true;
		}
	}

	lua_pushboolean( L, res );
	return 1;
}

static int isTuningByIndex( lua_State* L )
{
	alt_u16 signalIndex = luaL_checkinteger ( L, 1 );

	bool res = false;

	const struct VduAppSignalParam* const param = appSignalParamByIndex(signalIndex);
	if (param == NULL)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "LUA isTuningByIndex: signal with index %u not found.", (unsigned int)signalIndex);
		logWarning(SignalWasNotFound, msg);
	}
	else
	{
		if (param->props.bits.tunable == true)
		{
			res = true;	// signal is tuning
		}
	}

	lua_pushboolean(L, res);
	return 1;
}

static int signalTypeByIndex( lua_State* L )
{
	alt_u16 signalIndex = luaL_checkinteger ( L, 1 );

	const struct VduAppSignalParam* const param = appSignalParamByIndex(signalIndex);
	if (param == NULL)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "LUA signalTypeByIndex: signal with index %u not found.", (unsigned int)signalIndex);
		logWarning(SignalWasNotFound, msg);

		lua_pushnil(L);
	}
	else
	{
		alt_u32 res = param->vduSignalType;
		lua_pushinteger(L, res);
	}
	
	return 1;
}

static int signalValueByIndexInt( lua_State* L )
{
	alt_32 res = 0;
	alt_u16 signalIndex = luaL_checkinteger(L, 1);

	if (readSignalValueIntByIndex(signalIndex, &res) == false)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "LUA signalValueByIndexInt: can't get value of signal with index %u.", (unsigned int)signalIndex);
		logWarning(SignalReadTypeWrong, msg);

		lua_pushnil(L);
	}
	else
	{
		lua_pushinteger(L, res);
	}
	
	return 1;
}

static int signalValueByIndexFloat( lua_State* L )
{
	float res = 0.0f;

	alt_u16 signalIndex = luaL_checkinteger(L, 1);

	if (readSignalValueFloatByIndex(signalIndex, &res) == false)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "LUA signalValueByIndexFloat: can't get value of signal with index %u.", (unsigned int)signalIndex);
		logWarning(SignalReadTypeWrong, msg);

		lua_pushnil(L);
	}
	else
	{
		lua_pushnumber(L, res);
	}

	return 1;
}

//*************************************
// Common - AppSignals by appSignalId
//*************************************

static int signalExists(lua_State* L)
{
	char appSignalId[STRING_MAX_LEN] = { 0 };
	if (SAFE_STR_COPY(appSignalId, STRING_MAX_LEN, lua_tostring(L, 1)) == false)
	{
		logWarning(SignalWasNotFound, "LUA signalExists: appSignalId = NULL.");
		lua_pushboolean(L, false);
		return 1;
	}

	alt_u16 signalIndex = appSignalIndex(appSignalId);
	bool res = false;

	if (signalIndex < appSignalsCount())
	{
		res = true;
	}

	lua_pushboolean(L, res);
	return 1;
}

static int signalIsAvailable(lua_State* L)
{
	char appSignalId[STRING_MAX_LEN] = { 0 };
	if (SAFE_STR_COPY(appSignalId, STRING_MAX_LEN, lua_tostring(L, 1)) == false)
	{
		logWarning(SignalWasNotFound, "LUA signalIsAvailable: appSignalId = NULL.");
		lua_pushboolean(L, false);
		return 1;
	}

	alt_u16 signalIndex = appSignalIndex(appSignalId);

	if (signalIndex < appSignalsCount())
	{
		lua_pushboolean(L, signalAvailableByIndex(signalIndex));
	}
	else
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "LUA signalIsValid: signal %s not found.", appSignalId);
		logWarning(SignalWasNotFound, msg);

		lua_pushboolean(L, false);
	}

	return 1;
}

static int luaSignalId(lua_State* L)
{
	char appSignalId[STRING_MAX_LEN] = { 0 };
	if (SAFE_STR_COPY(appSignalId, STRING_MAX_LEN, lua_tostring(L, 1)) == false)
	{
		logWarning(SignalWasNotFound, "LUA signalId: appSignalId = NULL.");
		lua_pushnil(L);
		return 1;
	}

	alt_u16 signalIndex = appSignalIndex(appSignalId);

	const struct VduAppSignalParam* const param = appSignalParamByIndex(signalIndex);
	if (param == NULL)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "LUA signalId: signal with index %u not found.", (unsigned int)signalIndex);
		logWarning(SignalWasNotFound, msg);
		lua_pushnil(L);
	}
	else
	{
		(void)lua_pushstring(L, customAppSignalID(signalIndex));
	}
	return 1;
}

static int luaSignalCaption(lua_State* L)
{
	char appSignalId[STRING_MAX_LEN] = { 0 };
	if (SAFE_STR_COPY(appSignalId, STRING_MAX_LEN, lua_tostring(L, 1)) == false)
	{
		logWarning(SignalWasNotFound, "LUA signalCaption: appSignalId = NULL.");
		lua_pushnil(L);
		return 1;
	}

	alt_u16 signalIndex = appSignalIndex(appSignalId);

	const struct VduAppSignalParam* const param = appSignalParamByIndex(signalIndex);
	if (param == NULL)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "LUA signalCaption: signal with index %u not found.", (unsigned int)signalIndex);
		logWarning(SignalWasNotFound, msg);
		lua_pushnil(L);
	}
	else
	{
		(void)lua_pushstring(L, signalCaption(signalIndex));
	}
	return 1;
}

static int luaSignalUnits(lua_State* L)
{
	char appSignalId[STRING_MAX_LEN] = { 0 };
	if (SAFE_STR_COPY(appSignalId, STRING_MAX_LEN, lua_tostring(L, 1)) == false)
	{
		logWarning(SignalWasNotFound, "LUA signalUnits: appSignalId = NULL.");
		lua_pushnil(L);
		return 1;
	}

	alt_u16 signalIndex = appSignalIndex(appSignalId);

	const struct VduAppSignalParam* const param = appSignalParamByIndex(signalIndex);
	if (param == NULL)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "LUA signalUnits: signal with index %u not found.", (unsigned int)signalIndex);
		logWarning(SignalWasNotFound, msg);
		lua_pushnil(L);
	}
	else
	{
		const char* units = signalUnit(signalIndex);
		if ((strlen(units) == 1u) && (units[0] == '%'))
		{
			(void)lua_pushstring(L, "%%");
		}
		else
		{
			(void)lua_pushstring(L, units);
		}
	}
	return 1;
}

static int isDiscrete(lua_State* L)
{
	char appSignalId[STRING_MAX_LEN] = { 0 };
	if (SAFE_STR_COPY(appSignalId, STRING_MAX_LEN, lua_tostring(L, 1)) == false)
	{
		logWarning(SignalWasNotFound, "LUA isDiscrete: appSignalId = NULL.");
		lua_pushboolean(L, false);
		return 1;
	}

	alt_u16 signalIndex = appSignalIndex(appSignalId);

	bool res = false;

	const struct VduAppSignalParam* const param = appSignalParamByIndex(signalIndex);
	if (param == NULL)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "LUA isDiscrete: signal %s not found.", appSignalId);
		logWarning(SignalWasNotFound, msg);

		lua_pushboolean(L, false);
		return 1;
	}
	else
	{
		if (param->vduSignalType == (alt_u16)Discrete)
		{
			res = true;
		}
	}

	lua_pushboolean(L, res);
	return 1;
}

static int isAnalog(lua_State* L)
{
	char appSignalId[STRING_MAX_LEN] = { 0 };
	if (SAFE_STR_COPY(appSignalId, STRING_MAX_LEN, lua_tostring(L, 1)) == false)
	{
		logWarning(SignalWasNotFound, "LUA isAnalog: appSignalId = NULL.");
		lua_pushboolean(L, false);
		return 1;
	}

	alt_u16 signalIndex = appSignalIndex(appSignalId);

	bool res = false;

	const struct VduAppSignalParam* const param = appSignalParamByIndex(signalIndex);
	if (param == NULL)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "LUA isAnalog: signal %s not found.", appSignalId);
		logWarning(SignalWasNotFound, msg);

		lua_pushboolean(L, false);
		return 1;
	}
	else
	{
		if ((param->vduSignalType == (alt_u16)AnalogFloat32) || (param->vduSignalType == (alt_u16)AnalogSignedInt32))
		{
			res = true;
		}
	}

	lua_pushboolean(L, res);
	return 1;
}

static int isTuning(lua_State* L)
{
	char appSignalId[STRING_MAX_LEN] = { 0 };
	if (SAFE_STR_COPY(appSignalId, STRING_MAX_LEN, lua_tostring(L, 1)) == false)
	{
		logWarning(SignalWasNotFound, "LUA isTuning: appSignalId = NULL.");
		lua_pushboolean(L, false);
		return 1;
	}

	alt_u16 signalIndex = appSignalIndex(appSignalId);

	bool res = false;

	const struct VduAppSignalParam* const param = appSignalParamByIndex(signalIndex);
	if (param == NULL)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "LUA isTuning: signal %s not found.", appSignalId);
		logWarning(SignalWasNotFound, msg);

		lua_pushboolean(L, false);
		return 1;
	}
	else
	{
		if (param->props.bits.tunable == true)
		{
			res = true;	// signal is tuning
		}
	}

	lua_pushboolean(L, res);
	return 1;
}

static int signalType(lua_State* L)
{
	char appSignalId[STRING_MAX_LEN] = { 0 };
	if (SAFE_STR_COPY(appSignalId, STRING_MAX_LEN, lua_tostring(L, 1)) == false)
	{
		logWarning(SignalWasNotFound, "LUA signalType: appSignalId = NULL.");
		lua_pushnil(L);
		return 1;
	}

	alt_u16 signalIndex = appSignalIndex(appSignalId);

	const struct VduAppSignalParam* const param = appSignalParamByIndex(signalIndex);
	if (param == NULL)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "LUA signalType: signal %s not found.", appSignalId);
		logWarning(SignalWasNotFound, msg);

		lua_pushnil(L);
	}
	else
	{
		alt_u32 res = param->vduSignalType;
		lua_pushinteger(L, res);
	}

	return 1;
}

static int signalValueInt(lua_State* L)
{
	char appSignalId[STRING_MAX_LEN] = { 0 };
	if (SAFE_STR_COPY(appSignalId, STRING_MAX_LEN, lua_tostring(L, 1)) == false)
	{
		logWarning(SignalWasNotFound, "LUA signalValueInt: appSignalId = NULL.");
		lua_pushnil(L);
		return 1;
	}

	alt_u16 signalIndex = appSignalIndex(appSignalId);

	char msg[STRING_MAX_LEN];

	alt_32 res = 0;
	if (signalIndex == INVALID_INDEX)
	{
		(void)snprintf(msg, STRING_MAX_LEN, "LUA signalValueInt: signal %s is not found.", appSignalId);
		logWarning(SignalReadTypeWrong, msg);

		lua_pushnil(L);
	}
	else
	{
		if (readSignalValueIntByIndex(signalIndex, &res) == false)
		{
			(void)snprintf(msg, STRING_MAX_LEN, "LUA signalValueInt: can't get value of signal %s.", appSignalId);
			logWarning(SignalReadTypeWrong, msg);

			lua_pushnil(L);
		}
		else
		{
			lua_pushinteger(L, res);
		}
	}

	return 1;
}

static int signalValueFloat(lua_State* L)
{
	char appSignalId[STRING_MAX_LEN] = { 0 };
	if (SAFE_STR_COPY(appSignalId, STRING_MAX_LEN, lua_tostring(L, 1)) == false)
	{
		logWarning(SignalWasNotFound, "LUA signalValueFloat: appSignalId = NULL.");
		lua_pushnil(L);
		return 1;
	}

	alt_u16 signalIndex = appSignalIndex(appSignalId);

	char msg[STRING_MAX_LEN];
	float res = 0.0f;

	if (signalIndex == INVALID_INDEX)
	{
		(void)snprintf(msg, STRING_MAX_LEN, "LUA signalValueFloat: signal %s is not found.", appSignalId);
		logWarning(SignalReadTypeWrong, msg);

		lua_pushnil(L);
	}
	else
	{
		if (readSignalValueFloatByIndex(signalIndex, &res) == false)
		{
			(void)snprintf(msg, STRING_MAX_LEN, "LUA signalValueFloat: can't get value of signal %s.", appSignalId);
			logWarning(SignalReadTypeWrong, msg);

			lua_pushnil(L);
		}
		else
		{
			lua_pushnumber(L, res);
		}
	}

	return 1;
}

//*************************************
// Properties - AppSignals
//*************************************

static int signalTuningDefaultValueByIndexInt( lua_State* L )
{
	alt_u16 signalIndex = luaL_checkinteger ( L, 1 );

	alt_32 res = 0;

	const struct VduAppSignalParam* const param = appSignalParamByIndex(signalIndex);
	if (param == NULL)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "LUA signalTuningDefaultValueByIndexInt: signal with index %u not found.", (unsigned int)signalIndex);
		logWarning(SignalWasNotFound, msg);

		lua_pushnil(L);
		return 1;
	}
	else
	{
		res = param->defaultValue.intValue;
	}

	lua_pushinteger(L, res);
	return 1;
}

static int signalTuningDefaultValueByIndexFloat( lua_State* L )
{
	alt_u16 signalIndex = luaL_checkinteger ( L, 1 );

	float res = 0.0f;

	const struct VduAppSignalParam* const param = appSignalParamByIndex(signalIndex);
	if (param == NULL)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "LUA signalTuningDefaultValueByIndexFloat: signal with index %u not found.", (unsigned int)signalIndex);
		logWarning(SignalWasNotFound, msg);

		lua_pushnil(L);
		return 1;
	}
	else
	{
		res = param->defaultValue.floatValue;
	}

	lua_pushnumber(L, res);
	return 1;
}

static int signalLowEngUnitsByIndexInt( lua_State* L )
{
	alt_u16 signalIndex = luaL_checkinteger ( L, 1 );
	alt_32 res = 0;

	const struct VduAppSignalParam* const param = appSignalParamByIndex(signalIndex);
	if (param == NULL)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "LUA signalLowEngUnitsByIndexInt: signal with index %u not found.", (unsigned int)signalIndex);
		logWarning(SignalWasNotFound, msg);

		lua_pushnil(L);
		return 1;
	}
	else
	{
		res = param->lowLimit.intValue;
	}

	lua_pushinteger(L, res);
	return 1;
}

static int signalLowEngUnitsByIndexFloat( lua_State* L )
{
	alt_u16 signalIndex = luaL_checkinteger ( L, 1 );
	float res = 0.0f;

	const struct VduAppSignalParam* const param = appSignalParamByIndex(signalIndex);
	if (param == NULL)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "LUA signalLowEngUnitsByIndexFloat: signal with index %u not found.", (unsigned int)signalIndex);
		logWarning(SignalWasNotFound, msg);

		lua_pushnil(L);
		return 1;
	}
	else
	{
		res = param->lowLimit.floatValue;
	}

	lua_pushnumber(L, res);
	return 1;
}

static int signalHighEngUnitsByIndexInt( lua_State* L )
{
	alt_u16 signalIndex = luaL_checkinteger ( L, 1 );
	alt_32 res = 0;

	const struct VduAppSignalParam* const param = appSignalParamByIndex(signalIndex);
	if (param == NULL)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "LUA signalHighEngUnitsByIndexInt: signal with index %u not found.", (unsigned int)signalIndex);
		logWarning(SignalWasNotFound, msg);

		lua_pushnil(L);
		return 1;
	}
	else
	{
		res = param->highLimit.intValue;
	}

	lua_pushinteger( L, res );
	return 1;
}

static int signalHighEngUnitsByIndexFloat( lua_State* L )
{
	alt_u16 signalIndex = luaL_checkinteger ( L, 1 );
	float res = 0.0f;

	const struct VduAppSignalParam* const param = appSignalParamByIndex(signalIndex);
	if (param == NULL)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "LUA signalHighEngUnitsByIndexFloat: signal with index %u not found.", (unsigned int)signalIndex);
		logWarning(SignalWasNotFound, msg);

		lua_pushnil(L);
		return 1;
	}
	else
	{
		res = param->highLimit.floatValue;
	}

	lua_pushnumber( L, res );
	return 1;

}

static int decimalPlacesByIndex( lua_State* L )
{
	alt_u16 signalIndex = luaL_checkinteger ( L, 1 );
	alt_u16 res = 0u;

	const struct VduAppSignalParam* const param = appSignalParamByIndex(signalIndex);
	if (param == NULL)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "LUA decimalPlacesByIndex: signal with index %u not found.", (unsigned int)signalIndex);
		logWarning(SignalWasNotFound, msg);

		lua_pushnil(L);
		return 1;
	}
	else
	{
		res = param->decimalPlaces;
	}

	lua_pushinteger(L, res);
	return 1;
}

//**************************** *********
// Properties - AppSignals by Index
//*************************************

static int signalTuningDefaultValueInt(lua_State* L)
{
	char appSignalId[STRING_MAX_LEN] = { 0 };
	if (SAFE_STR_COPY(appSignalId, STRING_MAX_LEN, lua_tostring(L, 1)) == false)
	{
		logWarning(SignalWasNotFound, "LUA signalTuningDefaultValueInt: appSignalId = NULL.");
		lua_pushnil(L);
		return 1;
	}

	alt_u16 signalIndex = appSignalIndex(appSignalId);

	alt_32 res = 0;

	const struct VduAppSignalParam* const param = appSignalParamByIndex(signalIndex);
	if (param == NULL)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "LUA signalTuningDefaultValueInt: signal %s not found.", appSignalId);
		logWarning(SignalWasNotFound, msg);

		lua_pushnil(L);
		return 1;
	}
	else
	{
		res = param->defaultValue.intValue;
	}

	lua_pushinteger(L, res);
	return 1;
}

static int signalTuningDefaultValueFloat(lua_State* L)
{
	char appSignalId[STRING_MAX_LEN] = { 0 };
	if (SAFE_STR_COPY(appSignalId, STRING_MAX_LEN, lua_tostring(L, 1)) == false)
	{
		logWarning(SignalWasNotFound, "LUA signalTuningDefaultValueFloat: appSignalId = NULL.");
		lua_pushnil(L);
		return 1;
	}

	alt_u16 signalIndex = appSignalIndex(appSignalId);

	float res = 0.0f;

	const struct VduAppSignalParam* const param = appSignalParamByIndex(signalIndex);
	if (param == NULL)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "LUA signalTuningDefaultValueFloat: signal %s not found.", appSignalId);
		logWarning(SignalWasNotFound, msg);

		lua_pushnil(L);
		return 1;
	}
	else
	{
		res = param->defaultValue.floatValue;
	}

	lua_pushnumber(L, res);
	return 1;
}

static int signalLowEngUnitsInt(lua_State* L)
{
	char appSignalId[STRING_MAX_LEN] = { 0 };
	if (SAFE_STR_COPY(appSignalId, STRING_MAX_LEN, lua_tostring(L, 1)) == false)
	{
		logWarning(SignalWasNotFound, "LUA signalLowEngUnitInt: appSignalId = NULL.");
		lua_pushnil(L);
		return 1;
	}

	alt_u16 signalIndex = appSignalIndex(appSignalId);

	alt_32 res = 0;

	const struct VduAppSignalParam* const param = appSignalParamByIndex(signalIndex);
	if (param == NULL)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "LUA signalLowEngUnitInt: signal %s not found.", appSignalId);
		logWarning(SignalWasNotFound, msg);

		lua_pushnil(L);
		return 1;
	}
	else
	{
		res = param->lowLimit.intValue;
	}

	lua_pushinteger(L, res);
	return 1;
}

static int signalLowEngUnitsFloat(lua_State* L)
{
	char appSignalId[STRING_MAX_LEN] = { 0 };
	if (SAFE_STR_COPY(appSignalId, STRING_MAX_LEN, lua_tostring(L, 1)) == false)
	{
		logWarning(SignalWasNotFound, "LUA signalLowEngUnitsFloat: appSignalId = NULL.");
		lua_pushnil(L);
		return 1;
	}

	alt_u16 signalIndex = appSignalIndex(appSignalId);

	float res = 0.0f;

	const struct VduAppSignalParam* const param = appSignalParamByIndex(signalIndex);
	if (param == NULL)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "LUA signalLowEngUnitsFloat: signal %s not found.", appSignalId);
		logWarning(SignalWasNotFound, msg);

		lua_pushnil(L);
		return 1;
	}
	else
	{
		res = param->lowLimit.floatValue;
	}

	lua_pushnumber(L, res);
	return 1;
}

static int signalHighEngUnitsInt(lua_State* L)
{
	char appSignalId[STRING_MAX_LEN] = { 0 };
	if (SAFE_STR_COPY(appSignalId, STRING_MAX_LEN, lua_tostring(L, 1)) == false)
	{
		logWarning(SignalWasNotFound, "LUA signalHighEngUnitsInt: appSignalId = NULL.");
		lua_pushnil(L);
		return 1;
	}

	alt_u16 signalIndex = appSignalIndex(appSignalId);

	alt_32 res = 0;

	const struct VduAppSignalParam* const param = appSignalParamByIndex(signalIndex);
	if (param == NULL)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "LUA signalHighEngUnitsInt: signal %s not found.", appSignalId);
		logWarning(SignalWasNotFound, msg);

		lua_pushnil(L);
		return 1;
	}
	else
	{
		res = param->highLimit.intValue;
	}

	lua_pushinteger(L, res);
	return 1;
}

static int signalHighEngUnitsFloat(lua_State* L)
{
	char appSignalId[STRING_MAX_LEN] = { 0 };
	if (SAFE_STR_COPY(appSignalId, STRING_MAX_LEN, lua_tostring(L, 1)) == false)
	{
		logWarning(SignalWasNotFound, "LUA signalHighEngUnitsFloat: appSignalId = NULL.");
		lua_pushnil(L);
		return 1;
	}

	alt_u16 signalIndex = appSignalIndex(appSignalId);

	float res = 0.0f;

	const struct VduAppSignalParam* const param = appSignalParamByIndex(signalIndex);
	if (param == NULL)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "LUA signalHighEngUnitsFloat: signal %s not found.", appSignalId);
		logWarning(SignalWasNotFound, msg);

		lua_pushnil(L);
		return 1;
	}
	else
	{
		res = param->highLimit.floatValue;
	}

	lua_pushnumber(L, res);
	return 1;

}

static int decimalPlaces(lua_State* L)
{
	char appSignalId[STRING_MAX_LEN] = { 0 };
	if (SAFE_STR_COPY(appSignalId, STRING_MAX_LEN, lua_tostring(L, 1)) == false)
	{
		logWarning(SignalWasNotFound, "LUA decimalPlaces: appSignalId = NULL.");
		lua_pushnil(L);
		return 1;
	}

	alt_u16 signalIndex = appSignalIndex(appSignalId);

	alt_u16 res = 0;

	const struct VduAppSignalParam* const param = appSignalParamByIndex(signalIndex);
	if (param == NULL)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "LUA decimalPlaces: signal %s not found.", appSignalId);
		logWarning(SignalWasNotFound, msg);

		lua_pushnil(L);
		return 1;
	}
	else
	{
		res = param->decimalPlaces;
	}

	lua_pushinteger(L, res);
	return 1;
}

//*************************************
// Control - AppSignals
//*************************************

static int writeValueByIndexInt( lua_State* L )
{
	bool res = true;

	alt_u16 signalIndex = luaL_checkinteger(L, 1);
	alt_32 newValue = luaL_checkinteger(L, 2);
	if (writeSignalValueIntByIndex(signalIndex, newValue) == false)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "LUA writeValueByIndexInt: can't get value of signal with index %u.", (unsigned int)signalIndex);
		logWarning(SignalWriteTypeWrong, msg);

		res = false;
	}

	lua_pushboolean(L, res);
	return 1;
}

static int writeValueByIndexFloat( lua_State* L )
{
	bool res = true;

	alt_u16 signalIndex = luaL_checkinteger(L, 1);
	float newValue = lua_tonumber(L, 2);

	if (writeSignalValueFloatByIndex(signalIndex, newValue) == false)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "LUA writeValueByIndexFloat: can't get value of signal with index %u.", (unsigned int)signalIndex);
		logWarning(SignalWriteTypeWrong, msg);

		res = false;
	}

	lua_pushboolean(L, res);
	return 1;
}

static void copyValueProc(lua_State* L, alt_u16 signalIndexSrc, alt_u16 signalIndexDst)
{
	bool res = true;

	const struct VduAppSignalParam* const paramSrc = appSignalParamByIndex(signalIndexSrc);
	const struct VduAppSignalParam* const paramDest = appSignalParamByIndex(signalIndexDst);
	if (paramSrc == NULL)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "LUA copyValueProc: source signal was not found, index = %u).", (unsigned int)signalIndexSrc);
		logWarning(SignalWasNotFound, msg);

		res = false;
		lua_pushboolean(L, res);
		return;
	}

	if (paramDest == NULL)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "LUA copyValueProc: destination signal was not found, index = %u).", (unsigned int)signalIndexDst);
		logWarning(SignalWasNotFound, msg);

		res = false;
		lua_pushboolean(L, res);
		return;
	}

	if (((paramSrc->vduSignalType == (alt_u16)Discrete) || (paramSrc->vduSignalType == (alt_u16)AnalogSignedInt32)) &&
		((paramDest->vduSignalType == (alt_u16)Discrete) || (paramDest->vduSignalType == (alt_u16)AnalogSignedInt32)))
	{
		// Copy discrete or integer value
		//
		alt_32 value = 0;
		if (readSignalValueIntByIndex(signalIndexSrc, &value) == false)
		{
			char msg[STRING_MAX_LEN];
			(void)snprintf(msg, STRING_MAX_LEN, "LUA copyValueProc: signalValueIntByIndex failed.");
			logWarning(SignalReadTypeWrong, msg);

			res = false;
		}
		else
		{
			if (writeSignalValueIntByIndex(signalIndexDst, value) == false)
			{
				char msg[STRING_MAX_LEN];
				(void)snprintf(msg, STRING_MAX_LEN, "LUA copyValueProc: writeSignalValueIntByIndex failed.");
				logWarning(SignalWriteTypeWrong, msg);

				res = false;
			}
		}

	}
	else if ((paramSrc->vduSignalType == (alt_u16)AnalogFloat32) && (paramDest->vduSignalType == (alt_u16)AnalogFloat32))
	{
		// Copy float value
		//
		float value = 0.0f;
		if (readSignalValueFloatByIndex(signalIndexSrc, &value) == false)
		{
			char msg[STRING_MAX_LEN];
			(void)snprintf(msg, STRING_MAX_LEN, "LUA copyValueProc: signalValueFloatByIndex failed.");
			logWarning(SignalReadTypeWrong, msg);

			res = false;
		}
		else
		{
			if (writeSignalValueFloatByIndex(signalIndexDst, value) == false)
			{
				char msg[STRING_MAX_LEN];
				(void)snprintf(msg, STRING_MAX_LEN, "LUA copyValueProc: writeSignalValueFloatByIndex failed.");
				logWarning(SignalWriteTypeWrong, msg);

				res = false;
			}
		}
	}
	else
	{
		res = false;
	}

	lua_pushboolean(L, res);
	return;
}

static int copyValueByIndex(lua_State* L)
{
	alt_u16 signalIndexSrc = luaL_checkinteger(L, 1);
	alt_u16 signalIndexDst = luaL_checkinteger(L, 2);

	copyValueProc(L, signalIndexSrc, signalIndexDst);
	return 1;
}

static int writeValueInt(lua_State* L)
{
	char appSignalId[STRING_MAX_LEN] = { 0 };
	if (SAFE_STR_COPY(appSignalId, STRING_MAX_LEN, lua_tostring(L, 1)) == false)
	{
		logWarning(SignalWasNotFound, "LUA writeValueInt: appSignalId = NULL.");
		lua_pushboolean(L, false);

		return 1;
	}

	alt_u16 signalIndex = appSignalIndex(appSignalId);

	char msg[STRING_MAX_LEN];
	bool res = true;

	if (signalIndex == INVALID_INDEX)
	{
		(void)snprintf(msg, STRING_MAX_LEN, "LUA writeValueInt: signal %s is not found.", appSignalId);
		logWarning(SignalReadTypeWrong, msg);

		res = false;
	}
	else
	{
		alt_32 newValue = luaL_checkinteger(L, 2);
		if (writeSignalValueIntByIndex(signalIndex, newValue) == false)
		{
			(void)snprintf(msg, STRING_MAX_LEN, "LUA writeValueInt: can't write value of signal %s.", appSignalId);
			logWarning(SignalWriteTypeWrong, msg);

			res = false;
		}
	}

	lua_pushboolean(L, res);
	return 1;
}

static int writeValueFloat(lua_State* L)
{
	char appSignalId[STRING_MAX_LEN] = { 0 };
	if (SAFE_STR_COPY(appSignalId, STRING_MAX_LEN, lua_tostring(L, 1)) == false)
	{
		logWarning(SignalWasNotFound, "LUA writeValueFloat: appSignalId = NULL.");
		lua_pushboolean(L, false);

		return 1;
	}

	alt_u16 signalIndex = appSignalIndex(appSignalId);

	char msg[STRING_MAX_LEN];
	bool res = true;

	if (signalIndex == INVALID_INDEX)
	{
		(void)snprintf(msg, STRING_MAX_LEN, "LUA writeValueFloat: signal %s is not found.", appSignalId);
		logWarning(SignalReadTypeWrong, msg);

		res = false;
	}
	else
	{
		float newValue = lua_tonumber(L, 2);

		if (writeSignalValueFloatByIndex(signalIndex, newValue) == false)
		{
			(void)snprintf(msg, STRING_MAX_LEN, "LUA writeValueFloat: can't write value of signal %s.", appSignalId);
			logWarning(SignalWriteTypeWrong, msg);

			res = false;
		}
	}

	lua_pushboolean(L, res);
	return 1;
}

static int copyValue(lua_State* L)
{
	char appSignalIdSrc[STRING_MAX_LEN] = { 0 };
	if (SAFE_STR_COPY(appSignalIdSrc, STRING_MAX_LEN, lua_tostring(L, 1)) == false)
	{
		logWarning(SignalWasNotFound, "LUA copyValue: appSignalIdSrc = NULL.");
		lua_pushboolean(L, false);
		return 1;
	}

	char appSignalIdDst[STRING_MAX_LEN] = { 0 };
	if (SAFE_STR_COPY(appSignalIdDst, STRING_MAX_LEN, lua_tostring(L, 2)) == false)
	{
		logWarning(SignalWasNotFound, "LUA copyValue: appSignalIdDst = NULL.");
		lua_pushboolean(L, false);
		return 1;
	}

	alt_u16 signalIndexSrc = appSignalIndex(appSignalIdSrc);
	alt_u16 signalIndexDst = appSignalIndex(appSignalIdDst);

	copyValueProc(L, signalIndexSrc, signalIndexDst);
	return 1;
}

//*************************************
// ScheemaItem - Common
//*************************************

static int itemIsValid(lua_State* L)
{
	const struct VduSchemaFileSchemaItem1* const schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == false)
	{
		lua_pushboolean(L, false);
	}
	else
	{
		lua_pushboolean(L, true);
	}

	return 1;
}

static int currentItem(lua_State* L)
{
	struct VduSchemaFileSchemaItem1* const schemaItem = scriptRunSchemaItem();
	if (schemaItem == NULL)
	{
		RN_ASSERT(schemaItem != NULL);
		lua_pushnil(L);
	}
	else
	{
		lua_pushlightuserdata(L, schemaItem);
	}
	
	return 1;
}

static int itemByObjectName(lua_State* L)
{
	const struct VduSchemaFile* const schemaFile = vduCurrentSchema();
	if (schemaFile == NULL)
	{
		RN_ASSERT(schemaFile != NULL);

		lua_pushnil(L);
		return 1;
	}

	bool res = false;

	const char* objectName = luaL_checkstring (L, 1);
	size_t msgLen = strlen(objectName);

	for (alt_u16 i = 0; i < schemaFile->schemaItemCount; i++)
	{
		struct VduSchemaFileSchemaItem1* const item = vduCurrentSchemaItem(i);
		if (item == NULL)
		{
			RN_ASSERT(item != NULL);
			continue;
		}

		if (msgLen == vduCurrentSchemaStringLength(item->objectName))
		{
			if (strcmp(objectName, vduCurrentSchemaString(item->objectName)) == 0)
			{
				lua_pushlightuserdata(L, item);

				res = true;
				break;
			}
		}
	}

	if (res == false)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "LUA itemByObjectName: item with objectName %s was not found.", objectName);
		logWarning(ItemWasNotFound, msg);

		lua_pushnil(L);
	}

	return 1;
}

static int itemType(lua_State* L)
{
	const struct VduSchemaFileSchemaItem1* const schemaItem = (const struct VduSchemaFileSchemaItem1* const)lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == false)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "LUA itemType: invalid item was supplied.");
		logWarning(ItemWasNotFound, msg);

		lua_pushnil(L);
	}
	else
	{
		lua_pushinteger(L, schemaItem->itemType);
	}

	return 1;
}

static int itemObjectName(lua_State* L)
{
	const struct VduSchemaFileSchemaItem1* const schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == false)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "LUA itemObjectName: invalid item was supplied.");
		logWarning(ItemWasNotFound, msg);

		lua_pushnil(L);
	}
	else
	{
		(void)lua_pushstring(L, vduCurrentSchemaString(schemaItem->objectName));
	}

	return 1;
}

static int itemSignalCount(lua_State* L)
{
	const struct VduSchemaFileSchemaItem1* schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == false)
	{
		logWarning(ItemWasNotFound, "LUA itemSignalCount: invalid item was supplied.");
		lua_pushnil(L);
	}
	else
	{
		alt_u32 res = 0u;

		if (schemaItem->itemType == TYPE_VALUE)
		{
			res = schemaItem->value.appSignalCount;
		}
		else if (schemaItem->itemType == TYPE_IMAGEVALUE)
		{
			res = schemaItem->imageValue.appSignalCount;
		}
		else
		{
			logWarning(ItemWasNotFound, "LUA itemSignalCount: invalid item type.");
		}

		lua_pushinteger(L, res);
	}

	return 1;
}

static int itemAppSignalIds(lua_State* L)
{
	const struct VduSchemaFileSchemaItem1* schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == false)
	{
		logWarning(ItemWasNotFound, "LUA itemSignalCount: invalid item was supplied.");
		lua_pushnil(L);
	}
	else
	{
		if (schemaItem->itemType == TYPE_VALUE)
		{
			lua_createtable(L, schemaItem->value.appSignalCount, 0);

			for (alt_u32 i = 0; i < schemaItem->value.appSignalCount; i++)
			{
				alt_u16 index = schemaItemValueAppSignalIndex(&schemaItem->value, (alt_u16)i);
				if (index == INVALID_INDEX)
				{
					(void)lua_pushstring(L, "");
				}
				else
				{
					(void)lua_pushstring(L, appSignalID(index));
				}

				lua_rawseti(L, -2, (lua_Integer)i + 1 /*1-based index*/);
			}

			return 1;
		}
		
		if (schemaItem->itemType == TYPE_IMAGEVALUE)
		{
			lua_createtable(L, schemaItem->value.appSignalCount, 0);

			for (alt_u32 i = 0; i < schemaItem->imageValue.appSignalCount; i++)
			{
				alt_u16 index = schemaItemImageValueAppSignalIndex(&schemaItem->imageValue, (alt_u16)i);
				if (index == INVALID_INDEX)
				{
					(void)lua_pushstring(L, "");
				}
				else
				{
					(void)lua_pushstring(L, appSignalID(index));
				}

				lua_rawseti(L, -2, (lua_Integer)i + 1 /*1-based index*/);
			}

			return 1;
		}
	}

	logWarning(ItemWasNotFound, "LUA itemAppSignalIds: invalid item type.");
	lua_pushnil(L);
	return 1;
}

// Helper function
//
static alt_u16 signalGlobalIndexProc(const struct VduSchemaFileSchemaItem1* const item, alt_u16 index)
{
	if (SCHEMA_ITEM_PTR_IS_VALID(item) == false)
	{
		logWarning(ItemWasNotFound, "LUA signalGlobalIndexProc: invalid item was supplied.");
		return INVALID_INDEX;
	}
	
	if (item->itemType == TYPE_VALUE)
	{
		return schemaItemValueAppSignalIndex(&item->value, index);
	}
	else if (item->itemType == TYPE_IMAGEVALUE)
	{
		return schemaItemImageValueAppSignalIndex(&item->imageValue, index);
	}
	else
	{
	}
	
	return INVALID_INDEX;
}

static int signalGlobalIndex(lua_State* L)
{
	alt_u16 localIndex = luaL_checkinteger(L, 2);
	alt_u16 signalIndex = signalGlobalIndexProc((void*)lua_touserdata(L, 1), localIndex);
	if (signalIndex == INVALID_INDEX)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "LUA signalGlobalIndex: global index by local index = %u not found.", (unsigned int)localIndex);
		logWarning(SignalReadTypeWrong, msg);

		lua_pushinteger(L, (int) - 1);
	}
	else
	{
		lua_pushinteger(L, signalIndex);
	}
	return 1;
}

static int itemSignalValueInt(lua_State* L)
{
	const struct VduSchemaFileSchemaItem1* schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == false)
	{
		logWarning(ItemWasNotFound, "LUA itemSignalValueInt: invalid item was supplied.");
		lua_pushnil(L);
		return 1;
	}

	alt_u16 signalIndex = signalGlobalIndexProc(schemaItem, (alt_u16)luaL_checkinteger(L, 2));
	if (signalIndex == INVALID_INDEX)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "LUA signalGlobalIndex: global index by local index = %u not found.", (unsigned int)signalIndex);
		logWarning(SignalReadTypeWrong, msg);

		lua_pushnil(L);
		return 1;
	}

	alt_32 res = 0;
	if (readSignalValueIntByIndex(signalIndex, &res) == false)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "LUA itemSignalValueInt: can't get value of signal with index %u.", (unsigned int)signalIndex);
		logWarning(SignalReadTypeWrong, msg);

		lua_pushnil(L);
	}
	else
	{
		lua_pushinteger(L, res);
	}

	return 1;
}

static int itemSignalValueFloat(lua_State* L)
{
	const struct VduSchemaFileSchemaItem1* schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == false)
	{
		logWarning(ItemWasNotFound, "LUA itemSignalValueInt: invalid item was supplied.");
		lua_pushnil(L);
		return 1;
	}

	alt_u16 signalIndex = signalGlobalIndexProc(schemaItem, (alt_u16)luaL_checkinteger(L, 2));
	if (signalIndex == INVALID_INDEX)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "LUA signalGlobalIndex: global index by local index = %u not found.", (unsigned int)signalIndex);
		logWarning(SignalReadTypeWrong, msg);

		lua_pushnil(L);
		return 1;
	}

	float res = 0.0f;
	if (readSignalValueFloatByIndex(signalIndex, &res) == false)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "LUA itemSignalValueFloat: can't get value of signal with index %u.", (unsigned int)signalIndex);
		logWarning(SignalReadTypeWrong, msg);

		lua_pushnil(L);
	}
	else
	{
		lua_pushnumber(L, res);
	}

	return 1;
}

static int itemAcceptClick(lua_State* L)
{
	const struct VduSchemaFileSchemaItem1* schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == false)
	{
		logWarning(ItemWasNotFound, "LUA itemAcceptClick: invalid item was supplied.");
		lua_pushboolean(L, false);
	}
	else
	{
		lua_pushboolean(L, schemaItem->acceptClick);
	}

	return 1;
}

static int itemSetAcceptClick(lua_State* L )
{
	struct VduSchemaFileSchemaItem1* schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == false)
	{
		logWarning(ItemWasNotFound, "LUA itemSetAcceptClick: invalid item was supplied.");
	}
	else
	{
		schemaItem->acceptClick = lua_toboolean(L, 2);
	}

	return 0;
}

static int itemSetLeft(lua_State* L)
{
	bool invalidParams = true;

	struct VduSchemaFileSchemaItem1* schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == true)
	{
		alt_u16 left = luaL_checkinteger(L, 2);

		if  (left < m_screenWidth)
		{
			if (schemaItem->itemType == TYPE_RECT)
			{
				invalidParams = false;
				schemaItem->rect.left = left;
			}
			else if (schemaItem->itemType == TYPE_VALUE)
			{
				invalidParams = false;
				schemaItem->value.left = left;
			}
			else if (schemaItem->itemType == TYPE_IMAGE)
			{
				invalidParams = false;
				schemaItem->image.left = left;
			}
			else if (schemaItem->itemType == TYPE_IMAGEVALUE)
			{
				invalidParams = false;
				schemaItem->imageValue.left = left;
			}
			else
			{
				/* No action required */
			}
		}
	}

	if (invalidParams == true)
	{
		logWarning(ItemWasNotFound, "LUA itemSetLeft: invalid item was supplied.");
	}

	return 0;
}

static int itemSetTop(lua_State* L)
{
	bool invalidParams = true;

	struct VduSchemaFileSchemaItem1* schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == true)
	{
		alt_u16 top = luaL_checkinteger(L, 2);

		if  (top < m_screenHeight)
		{
			if (schemaItem->itemType == TYPE_RECT)
			{
				invalidParams = false;
				schemaItem->rect.top = top;
			}
			else if (schemaItem->itemType == TYPE_VALUE)
			{
				invalidParams = false;
				schemaItem->value.top = top;
			}
			else if (schemaItem->itemType == TYPE_IMAGE)
			{
				invalidParams = false;
				schemaItem->image.top = top;
			}
			else if (schemaItem->itemType == TYPE_IMAGEVALUE)
			{
				invalidParams = false;
				schemaItem->imageValue.top = top;
			}
			else
			{
				/* No action required */
			}
		}
	}

	if (invalidParams == true)
	{
		logWarning(ItemWasNotFound, "LUA itemSetTop: invalid item was supplied.");
	}

	return 0;
}

static int itemSetWidth(lua_State* L)
{
	bool invalidParams = true;

	struct VduSchemaFileSchemaItem1* schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == true)
	{
		alt_u16 width = luaL_checkinteger(L, 2);

		if  (width <= m_screenWidth)
		{
			if (schemaItem->itemType == TYPE_RECT)
			{
				invalidParams = false;
				schemaItem->rect.width = width;
			}
			else if (schemaItem->itemType == TYPE_VALUE)
			{
				invalidParams = false;
				schemaItem->value.width = width;
			}
			else if (schemaItem->itemType == TYPE_IMAGE)
			{
				invalidParams = false;
				schemaItem->image.width = width;
			}
			else if (schemaItem->itemType == TYPE_IMAGEVALUE)
			{
				invalidParams = false;
				schemaItem->imageValue.width = width;
			}
			else
			{
				/* No action required */
			}
		}
	}

	if (invalidParams == true)
	{
		logWarning(ItemWasNotFound, "LUA itemSetWidth: invalid item was supplied.");
	}

	return 0;
}

static int itemSetHeight(lua_State* L)
{
	bool invalidParams = true;

	struct VduSchemaFileSchemaItem1* schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == true)
	{
		alt_u16 height  = luaL_checkinteger(L, 2);

		if  (height <= m_screenHeight)
		{
			if (schemaItem->itemType == TYPE_RECT)
			{
				invalidParams = false;
				schemaItem->rect.height  = height;
			}
			else if (schemaItem->itemType == TYPE_VALUE)
			{
				invalidParams = false;
				schemaItem->value.height  = height;
			}
			else if (schemaItem->itemType == TYPE_IMAGE)
			{
				invalidParams = false;
				schemaItem->image.height  = height;
			}
			else if (schemaItem->itemType == TYPE_IMAGEVALUE)
			{
				invalidParams = false;
				schemaItem->imageValue.height  = height;
			}
			else
			{
				/* No action required */
			}
		}
	}

	if (invalidParams == true)
	{
		logWarning(ItemWasNotFound, "LUA itemSetHeight: invalid item was supplied.");
	}

	return 0;
}

static int itemLeft(lua_State* L)
{
	alt_u16 res = 0u;
	bool invalidParams = true;

	const struct VduSchemaFileSchemaItem1* schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == true)
	{
		if (schemaItem->itemType == TYPE_RECT)
		{
			invalidParams = false;
			res = schemaItem->rect.left;
		}
		else if (schemaItem->itemType == TYPE_VALUE)
		{
			invalidParams = false;
			res = schemaItem->value.left;
		}
		else if (schemaItem->itemType == TYPE_IMAGE)
		{
			invalidParams = false;
			res = schemaItem->image.left;
		}
		else if (schemaItem->itemType == TYPE_IMAGEVALUE)
		{
			invalidParams = false;
			res = schemaItem->imageValue.left;
		}
		else
		{
			/* No action required */
		}
	}

	if (invalidParams == true)
	{
		logWarning(ItemWasNotFound, "LUA itemLeft: invalid item was supplied.");

		lua_pushnil(L);
	}
	else
	{
		lua_pushinteger(L, res);
	}

	
	return 1;
}

static int itemTop(lua_State* L)
{
	alt_u16 res = 0u;
	bool invalidParams = true;

	const struct VduSchemaFileSchemaItem1* schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == true)
	{
		if (schemaItem->itemType == TYPE_RECT)
		{
			invalidParams = false;
			res = schemaItem->rect.top;
		}
		else if (schemaItem->itemType == TYPE_VALUE)
		{
			invalidParams = false;
			res = schemaItem->value.top;
		}
		else if (schemaItem->itemType == TYPE_IMAGE)
		{
			invalidParams = false;
			res = schemaItem->image.top;
		}
		else if (schemaItem->itemType == TYPE_IMAGEVALUE)
		{
			invalidParams = false;
			res = schemaItem->imageValue.top;
		}
		else
		{
			/* No action required */
		}
	}

	if (invalidParams == true)
	{
		logWarning(ItemWasNotFound, "LUA itemTop: invalid item was supplied.");
		lua_pushnil(L);
	}
	else
	{
		lua_pushinteger(L, res);
	}

	return 1;
}

static int itemWidth(lua_State* L)
{
	alt_u16 res = 0u;
	bool invalidParams = true;

	const struct VduSchemaFileSchemaItem1* schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == true)
	{
		if (schemaItem->itemType == TYPE_RECT)
		{
			invalidParams = false;
			res = schemaItem->rect.width;
		}
		else if (schemaItem->itemType == TYPE_VALUE)
		{
			invalidParams = false;
			res = schemaItem->value.width;
		}
		else if (schemaItem->itemType == TYPE_IMAGE)
		{
			invalidParams = false;
			res = schemaItem->image.width;
		}
		else if (schemaItem->itemType == TYPE_IMAGEVALUE)
		{
			invalidParams = false;
			res = schemaItem->imageValue.width;
		}
		else
		{
			/* No action required */
		}
	}

	if (invalidParams == true)
	{
		logWarning(ItemWasNotFound, "LUA itemWidth: invalid item was supplied.");
		lua_pushnil(L);
	}
	else
	{
		lua_pushinteger(L, res);
	}

	return 1;
}

static int itemHeight(lua_State* L)
{
	alt_u16 res = 0u;
	bool invalidParams = true;

	const struct VduSchemaFileSchemaItem1* schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == true)
	{
		if (schemaItem->itemType == TYPE_RECT)
		{
			invalidParams = false;
			res = schemaItem->rect.height;
		}
		else if (schemaItem->itemType == TYPE_VALUE)
		{
			invalidParams = false;
			res = schemaItem->value.height;
		}
		else if (schemaItem->itemType == TYPE_IMAGE)
		{
			invalidParams = false;
			res = schemaItem->image.height;
		}
		else if (schemaItem->itemType == TYPE_IMAGEVALUE)
		{
			invalidParams = false;
			res = schemaItem->imageValue.height;
		}
		else
		{
			/* No action required */
		}
	}

	if (invalidParams == true)
	{
		logWarning(ItemWasNotFound, "LUA itemHeight: invalid item was supplied.");
		lua_pushnil(L);
	}
	else
	{
		lua_pushinteger(L, res);
	}

	return 1;
}

//*************************************
// SchemaItemVduLine
//*************************************

static int itemLineWeight(lua_State* L)
{
	alt_u16 res = 0u;
	bool invalidParams = true;

	const struct VduSchemaFileSchemaItem1* schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == true)
	{
		if (schemaItem->itemType == TYPE_LINE)
		{
			invalidParams = false;
			res = schemaItem->line.weight;
		}
	}

	if (invalidParams == true)
	{
		logWarning(ItemWasNotFound, "LUA itemLineWeight: invalid item was supplied.");
		lua_pushnil(L);
	}
	else
	{
		lua_pushinteger(L, res);
	}

	return 1;
}

static int itemLineSetWeight(lua_State* L)
{
	bool invalidParams = true;

	struct VduSchemaFileSchemaItem1* schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == true)
	{
		if (schemaItem->itemType == TYPE_LINE)
		{
			invalidParams = false;
			schemaItem->line.weight = luaL_checkinteger(L, 2);
			
			if (schemaItem->line.weight > 16u)
			{
				logWarning(InternalWarning, "LUA itemLineSetWeight: line width was limited to 16.");
				schemaItem->line.weight = 16u;
			}

		}
	}

	if (invalidParams == true)
	{
		logWarning(ItemWasNotFound, "LUA itemLineSetWeight: invalid item was supplied.");
	}

	return 0;
}

static int itemLineColor(lua_State* L)
{
	alt_u32 res = 0u;
	bool invalidParams = true;

	const struct VduSchemaFileSchemaItem1* schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == true)
	{
		if (schemaItem->itemType == TYPE_LINE)
		{
			invalidParams = false;
			res = schemaItem->line.color;
		}
	}

	if (invalidParams == true)
	{
		logWarning(ItemWasNotFound, "LUA itemLineColor: invalid item was supplied.");
		lua_pushnil(L);
	}
	else
	{
		lua_pushinteger(L, res);
	}

	return 1;
}

static int itemLineSetLineColor(lua_State* L)
{
	bool invalidParams = true;

	struct VduSchemaFileSchemaItem1* schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == true)
	{
		if (schemaItem->itemType == TYPE_LINE)
		{
			invalidParams = false;
			schemaItem->line.color = luaL_checkinteger(L, 2);
		}
	}

	if (invalidParams == true)
	{
		logWarning(ItemWasNotFound, "LUA itemLineSetLineColor: invalid item was supplied.");
	}

	return 0;
}

//*************************************
// SchemaItemVduRect
//*************************************

static int itemRectWeight(lua_State* L)
{
	alt_u16 res = 0u;
	bool invalidParams = true;

	const struct VduSchemaFileSchemaItem1* schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == true)
	{
		if (schemaItem->itemType == TYPE_RECT)
		{
			invalidParams = false;
			res = schemaItem->rect.weight;
		}
	}

	if (invalidParams == true)
	{
		logWarning(ItemWasNotFound, "LUA itemRectWeight: invalid item was supplied.");
		lua_pushnil(L);
	}
	else
	{
		lua_pushinteger(L, res);
	}
	return 1;
}

static int itemRectSetWeight(lua_State* L)
{
	bool invalidParams = true;

	struct VduSchemaFileSchemaItem1* schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == true)
	{
		if (schemaItem->itemType == TYPE_RECT)
		{
			invalidParams = false;
			schemaItem->rect.weight = luaL_checkinteger(L, 2);

			if (schemaItem->rect.weight > 16u)
			{
				logWarning(InternalWarning, "LUA itemRectSetWeight: line width was limited to 16.");
				schemaItem->rect.weight = 16u;
			}
		}
	}

	if (invalidParams == true)
	{
		logWarning(ItemWasNotFound, "LUA itemRectSetWeight: invalid item was supplied.");
	}

	return 0;
}

static int itemRectFill(lua_State* L)
{
	bool res = false;
	bool invalidParams = true;

	const struct VduSchemaFileSchemaItem1* schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == true)
	{
		if (schemaItem->itemType == TYPE_RECT)
		{
			invalidParams = false;
			res = schemaItem->rect.fill;
		}
	}

	if (invalidParams == true)
	{
		logWarning(ItemWasNotFound, "LUA itemRectFill: invalid item was supplied.");
		lua_pushboolean(L, false);
	}
	else
	{
		lua_pushboolean(L, res);
	}
	return 1;
}

static int itemRectSetFill(lua_State* L)
{
	bool invalidParams = true;

	struct VduSchemaFileSchemaItem1* schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == true)
	{
		if (schemaItem->itemType == TYPE_RECT)
		{
			invalidParams = false;
			schemaItem->rect.fill = lua_toboolean(L, 2);
		}
	}

	if (invalidParams == true)
	{
		logWarning(ItemWasNotFound, "LUA itemRectSetFill: invalid item was supplied.");
	}

	return 0;
}

static int itemRectDrawRect(lua_State* L)
{
	bool res = true;
	bool invalidParams = true;

	const struct VduSchemaFileSchemaItem1* schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == true)
	{
		if (schemaItem->itemType == TYPE_RECT)
		{
			invalidParams = false;
			res = schemaItem->rect.drawRect;
		}
	}

	if (invalidParams == true)
	{
		logWarning(ItemWasNotFound, "LUA itemRectDrawRect: invalid item was supplied.");
		lua_pushboolean(L, false);
	}
	else
	{
		lua_pushboolean(L, res);
	}
	return 1;
}

static int itemRectSetDrawRect(lua_State* L)
{
	bool invalidParams = true;

	struct VduSchemaFileSchemaItem1* schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == true)
	{
		if (schemaItem->itemType == TYPE_RECT)
		{
			invalidParams = false;
			schemaItem->rect.drawRect = lua_toboolean(L, 2);
		}
	}

	if (invalidParams == true)
	{
		logWarning(ItemWasNotFound, "LUA itemRectSetDrawRect: invalid item was supplied.");
	}

	return 0;
}

static int itemRectLineColor(lua_State* L)
{
	alt_u32 res = 0u;
	bool invalidParams = true;

	const struct VduSchemaFileSchemaItem1* schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == true)
	{
		if (schemaItem->itemType == TYPE_RECT)
		{
			invalidParams = false;
			res = schemaItem->rect.lineColor;
		}
	}

	if (invalidParams == true)
	{
		logWarning(ItemWasNotFound, "LUA itemRectLineColor: invalid item was supplied.");
		lua_pushnil(L);
	}
	else
	{
		lua_pushinteger(L, res);
	}
	return 1;
}

static int itemRectSetLineColor(lua_State* L)
{
	bool invalidParams = true;

	struct VduSchemaFileSchemaItem1* schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == true)
	{
		if (schemaItem->itemType == TYPE_RECT)
		{
			invalidParams = false;
			schemaItem->rect.lineColor = luaL_checkinteger(L, 2);
		}
	}

	if (invalidParams == true)
	{
		logWarning(ItemWasNotFound, "LUA itemRectSetLineColor: invalid item was supplied.");
	}

	return 0;
}

static int itemRectFillColor(lua_State* L)
{
	alt_u32 res = 0u;
	bool invalidParams = true;

	const struct VduSchemaFileSchemaItem1* schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == true)
	{
		if (schemaItem->itemType == TYPE_RECT)
		{
			invalidParams = false;
			res = schemaItem->rect.fillColor;
		}
	}

	if (invalidParams == true)
	{
		logWarning(ItemWasNotFound, "LUA itemRectFillColor: invalid item was supplied.");
		lua_pushnil(L);
	}
	else
	{
		lua_pushinteger(L, res);
	}
	return 1;
}

static int itemRectSetFillColor(lua_State* L)
{
	bool invalidParams = true;

	struct VduSchemaFileSchemaItem1* schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == true)
	{
		if (schemaItem->itemType == TYPE_RECT)
		{
			invalidParams = false;
			schemaItem->rect.fillColor &= 0xff000000u;
			schemaItem->rect.fillColor |= luaL_checkinteger(L, 2) & 0x00ffffffu;
		}
	}

	if (invalidParams == true)
	{
		logWarning(ItemWasNotFound, "LUA itemRectSetFillColor: invalid item was supplied.");
	}

	return 0;
}

static int itemRectTextColor(lua_State* L)
{
	alt_u32 res = 0u;
	bool invalidParams = true;

	const struct VduSchemaFileSchemaItem1* schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == true)
	{
		if (schemaItem->itemType == TYPE_RECT)
		{
			invalidParams = false;
			res = schemaItem->rect.textColor;
		}
	}

	if (invalidParams == true)
	{
		logWarning(ItemWasNotFound, "LUA itemRectTextColor: invalid item was supplied.");
		lua_pushnil(L);
	}
	else
	{
		lua_pushinteger(L, res);
	}
	return 1;
}

static int itemRectSetTextColor(lua_State* L)
{
	bool invalidParams = true;

	struct VduSchemaFileSchemaItem1* schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == true)
	{
		if (schemaItem->itemType == TYPE_RECT)
		{
			invalidParams = false;
			schemaItem->rect.textColor = luaL_checkinteger(L, 2);
		}
	}

	if (invalidParams == true)
	{
		logWarning(ItemWasNotFound, "LUA itemRectSetTextColor: invalid item was supplied.");
	}

	return 0;
}

static int itemRectText(lua_State* L)
{
	bool invalidParams = true;

	const struct VduSchemaFileSchemaItem1* schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == true)
	{
		if (schemaItem->itemType == TYPE_RECT)
		{
			invalidParams = false;
		}
	}

	if (invalidParams == true)
	{
		logWarning(ItemWasNotFound, "LUA itemRectText: invalid item was supplied.");
		lua_pushnil(L);
	}
	else
	{
		(void)lua_pushstring(L, schemaItem->rect.text);
	}

	return 1;
}

static int itemRectSetText(lua_State* L)
{
	bool invalidParams = true;

	struct VduSchemaFileSchemaItem1* schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == true)
	{
		if (schemaItem->itemType == TYPE_RECT)
		{
			invalidParams = false;
			(void)strncpy(schemaItem->rect.text, luaL_checkstring(L, 2), sizeof(schemaItem->rect.text));
		}
	}

	if (invalidParams == true)
	{
		logWarning(ItemWasNotFound, "LUA itemRectSetText: invalid item was supplied.");
	}

	return 0;
}

//*************************************
// SchemaItemVduValue
//*************************************

static int itemValueWeight(lua_State* L)
{
	alt_u16 res = 0u;
	bool invalidParams = true;

	const struct VduSchemaFileSchemaItem1* schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == true)
	{
		if (schemaItem->itemType == TYPE_VALUE)
		{
			invalidParams = false;
			res = schemaItem->value.weight;
		}
	}

	if (invalidParams == true)
	{
		logWarning(ItemWasNotFound, "LUA itemValueWeight: invalid item was supplied.");
		lua_pushnil(L);
	}
	else
	{
		lua_pushinteger(L, res);
	}
	return 1;
}

static int itemValueSetWeight(lua_State* L)
{
	bool invalidParams = true;

	struct VduSchemaFileSchemaItem1* schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == true)
	{
		if (schemaItem->itemType == TYPE_VALUE)
		{
			invalidParams = false;
			schemaItem->value.weight = luaL_checkinteger(L, 2);

			if (schemaItem->value.weight > 16u)
			{
				logWarning(InternalWarning, "LUA itemValueSetWeight: line width was limited to 16.");
				schemaItem->value.weight = 16u;
			}
		}
	}

	if (invalidParams == true)
	{
		logWarning(ItemWasNotFound, "LUA itemValueSetWeight: invalid item was supplied.");
	}

	return 0;
}

static int itemValueDrawRect(lua_State* L)
{
	bool res = false;
	bool invalidParams = true;

	const struct VduSchemaFileSchemaItem1* schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == true)
	{
		if (schemaItem->itemType == TYPE_VALUE)
		{
			invalidParams = false;
			res = schemaItem->value.drawRect;
		}
	}

	if (invalidParams == true)
	{
		logWarning(ItemWasNotFound, "LUA itemValueDrawRect: invalid item was supplied.");
		lua_pushnil(L);
	}
	else
	{
		lua_pushboolean(L, res);
	}
	return 1;
}

static int itemValueSetDrawRect(lua_State* L)
{
	bool invalidParams = true;

	struct VduSchemaFileSchemaItem1* schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == true)
	{
		if (schemaItem->itemType == TYPE_VALUE)
		{
			invalidParams = false;
			schemaItem->value.drawRect = lua_toboolean(L, 2);
		}
	}

	if (invalidParams == true)
	{
		logWarning(ItemWasNotFound, "LUA itemValueSetDrawRect: invalid item was supplied.");
	}

	return 0;
}

static int itemValueLineColor(lua_State* L)
{
	alt_u32 res = 0u;
	bool invalidParams = true;

	const struct VduSchemaFileSchemaItem1* schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == true)
	{
		if (schemaItem->itemType == TYPE_VALUE)
		{
			invalidParams = false;
			res = schemaItem->value.lineColor;
		}
	}

	if (invalidParams == true)
	{
		logWarning(ItemWasNotFound, "LUA itemValueLineColor: invalid item was supplied.");
		lua_pushnil(L);
	}
	else
	{
		lua_pushinteger(L, res);
	}
	return 1;
}

static int itemValueSetLineColor(lua_State* L)
{
	bool invalidParams = true;

	struct VduSchemaFileSchemaItem1* schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == true)
	{
		if (schemaItem->itemType == TYPE_VALUE)
		{
			invalidParams = false;
			schemaItem->value.lineColor = luaL_checkinteger(L, 2);
		}
	}

	if (invalidParams == true)
	{
		logWarning(ItemWasNotFound, "LUA itemValueSetLineColor: invalid item was supplied.");
	}

	return 0;
}

static int itemValueFillColor(lua_State* L)
{
	alt_u32 res = 0u;
	bool invalidParams = true;

	const struct VduSchemaFileSchemaItem1* schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == true)
	{
		if (schemaItem->itemType == TYPE_VALUE)
		{
			invalidParams = false;
			res = schemaItem->value.fillColor;
		}
	}

	if (invalidParams == true)
	{
		logWarning(ItemWasNotFound, "LUA itemValueFillColor: invalid item was supplied.");
		lua_pushnil(L);
	}
	else
	{
		lua_pushinteger(L, res);
	}
	return 1;
}

static int itemValueSetFillColor(lua_State* L)
{
	bool invalidParams = true;

	struct VduSchemaFileSchemaItem1* schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == true)
	{
		if (schemaItem->itemType == TYPE_VALUE)
		{
			invalidParams = false;
			schemaItem->value.fillColor &= 0xff000000u;
			schemaItem->value.fillColor |= luaL_checkinteger(L, 2) & 0x00ffffffu;
		}
	}

	if (invalidParams == true)
	{
		logWarning(ItemWasNotFound, "LUA itemValueSetFillColor: invalid item was supplied.");
	}

	return 0;
}

static int itemValueTextColor(lua_State* L)
{
	alt_u32 res = 0u;
	bool invalidParams = true;

	const struct VduSchemaFileSchemaItem1* schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == true)
	{
		if (schemaItem->itemType == TYPE_VALUE)
		{
			invalidParams = false;
			res = schemaItem->value.textColor;
		}
	}

	if (invalidParams == true)
	{
		logWarning(ItemWasNotFound, "LUA itemValueTextColor: invalid item was supplied.");
		lua_pushnil(L);
	}
	else
	{
		lua_pushinteger(L, res);
	}
	return 1;
}

static int itemValueSetTextColor(lua_State* L)
{
	bool invalidParams = true;

	struct VduSchemaFileSchemaItem1* schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == true)
	{
		if (schemaItem->itemType == TYPE_VALUE)
		{
			invalidParams = false;
			schemaItem->value.textColor = luaL_checkinteger(L, 2);
		}
	}

	if (invalidParams == true)
	{
		logWarning(ItemWasNotFound, "LUA itemValueSetTextColor: invalid item was supplied.");
	}

	return 0;
}

static int itemValueDecimalPlaces(lua_State* L)
{
	alt_u16 res = 0u;
	bool invalidParams = true;

	const struct VduSchemaFileSchemaItem1* schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == true)
	{
		if (schemaItem->itemType == TYPE_VALUE)
		{
			invalidParams = false;
			res = schemaItem->value.decimalPlaces;
		}
	}

	if (invalidParams == true)
	{
		logWarning(ItemWasNotFound, "LUA itemValueDecimalPlaces: invalid item was supplied.");
		lua_pushnil(L);
	}
	else
	{
		lua_pushinteger(L, res);
	}
	return 1;
}

static int itemValueSetDecimalPlaces(lua_State* L)
{
	bool invalidParams = true;

	struct VduSchemaFileSchemaItem1* schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == true)
	{
		if (schemaItem->itemType == TYPE_VALUE)
		{
			invalidParams = false;

			alt_16 places = luaL_checkinteger(L, 2);
			if (places < -1)
			{
				logWarning(InternalWarning, "LUA itemValueSetDecimalPlaces: line width was limited to -1.");
				places = -1;
			}
			if (places > 32)
			{
				logWarning(InternalWarning, "LUA itemValueSetDecimalPlaces: line width was limited to 32.");
				places = 32;
			}

			schemaItem->value.decimalPlaces = (alt_u16)places;
		}
	}

	if (invalidParams == true)
	{
		logWarning(ItemWasNotFound, "LUA itemValueSetDecimalPlaces: invalid item was supplied.");
	}

	return 0;
}

static int itemValueText(lua_State* L)
{
	bool invalidParams = true;

	const struct VduSchemaFileSchemaItem1* schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == true)
	{
		if (schemaItem->itemType == TYPE_VALUE)
		{
			invalidParams = false;
		}
	}

	if (invalidParams == true)
	{
		logWarning(ItemWasNotFound, "LUA itemValueText: invalid item was supplied.");
		lua_pushnil(L);
	}
	else
	{
		(void)lua_pushstring(L, schemaItem->value.text);
	}

	return 1;
}

static int itemValueSetText(lua_State* L)
{
	bool invalidParams = true;

	struct VduSchemaFileSchemaItem1* schemaItem = lua_touserdata(L, 1);
	if (SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == true)
	{
		if (schemaItem->itemType == TYPE_VALUE)
		{
			invalidParams = false;
			(void)strncpy(schemaItem->value.text, luaL_checkstring(L, 2), sizeof(schemaItem->value.text));
		}
	}

	if (invalidParams == true)
	{
		logWarning(ItemWasNotFound, "LUA itemValueSetText: invalid item was supplied.");
	}

	return 0;
}

//*************************************
// SchemaItemImageValue
//*************************************

static int itemImageValueCurrentImageID( lua_State* L )
{
	const struct VduSchemaFileSchemaItem1* const schemaItem = lua_touserdata(L, 1);
	if ((SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == false) || 
		(schemaItem->itemType != TYPE_IMAGEVALUE))
	{
		logWarning(ItemWasNotFound, "LUA itemImageValueCurrentImageID: invalid item was supplied.");
		lua_pushnil(L);
	}
	else
	{
		if (schemaItem->imageValue.currentImage >= schemaItem->imageValue.imageCount)
		{
			char msg[STRING_MAX_LEN];
			(void)snprintf(msg, STRING_MAX_LEN, "LUA itemImageValueCurrentImageID: invalid image index %u was supplied, imageCount = %u.",
				schemaItem->imageValue.currentImage,
				schemaItem->imageValue.imageCount);
			logWarning(WrongImageIndex, msg);

			lua_pushnil(L);
		}
		else
		{
			(void)lua_pushstring(L, vduCurrentSchemaString(schemaItem->imageValue.images[schemaItem->imageValue.currentImage].imageId));
		}
	}

	return 1;
}

static int itemImageValueImageExists(lua_State* L)
{
	const struct VduSchemaFileSchemaItem1* const schemaItem = lua_touserdata(L, 1);
	if ((SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == false) ||
		(schemaItem->itemType != TYPE_IMAGEVALUE))
	{
		logWarning(ItemWasNotFound, "LUA itemImageValueImageExists: invalid item was supplied.");
		lua_pushboolean(L, false);
	}
	else
	{
		const char* imageId = NULL;
		if (lua_isnil(L, 2) == 0) 
		{
			imageId = luaL_checkstring(L, 2);
		}
		
		if (imageId == NULL)
		{
			logWarning(WrongImageId, "LUA itemImageValueImageExists: invalid image id was supplied.");
			lua_pushboolean(L, false);
		}
		else
		{
			bool imageFound = false;
			for (alt_u16 i = 0; i < schemaItem->imageValue.imageCount; i++)
			{
				if (strcmp(imageId, vduCurrentSchemaString(schemaItem->imageValue.images[i].imageId)) == 0)
				{
					imageFound = true;
					break;
				}
			}
			lua_pushboolean(L, imageFound);
		}
	}

	return 1;
}

static int itemImageValueSetCurrentImageID( lua_State* L )
{
	struct VduSchemaFileSchemaItem1* const schemaItem = (struct VduSchemaFileSchemaItem1* const)lua_touserdata(L, 1);
	if ((SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == false) ||
		(schemaItem->itemType != TYPE_IMAGEVALUE))
	{
		logWarning(ItemWasNotFound, "LUA itemImageValueSetCurrentImageID: invalid item was supplied.");
	}
	else
	{
		const char* imageId = luaL_checkstring(L, 2);
		if (imageId == NULL)
		{
			logWarning(WrongImageId, "LUA itemImageValueSetCurrentImageID: invalid image id was supplied.");
		}
		else
		{
			bool imageFound = false;
			for (alt_u16 i = 0; i < schemaItem->imageValue.imageCount; i++)
			{
				if (strcmp(imageId, vduCurrentSchemaString(schemaItem->imageValue.images[i].imageId)) == 0)
				{
					schemaItem->imageValue.currentImage = i;
					imageFound = true;
					break;
				}
			}
			if (imageFound == false)
			{
				char msg[STRING_MAX_LEN];
				(void)snprintf(msg, STRING_MAX_LEN, "LUA itemImageValueSetCurrentImageID: image id %s was not found.", imageId);
				logWarning(WrongImageId, msg);
			}
		}
	}

	return 0;
}

static int editMode(lua_State* L)
{
	char msg[STRING_MAX_LEN];
	struct PopUpSchemaParams schema = { 0 };

	char appSignalIdTuning[STRING_MAX_LEN] = { 0 };
	if (SAFE_STR_COPY(appSignalIdTuning, STRING_MAX_LEN, lua_tostring(L, 1)) == false)
	{
		logWarning(SignalWasNotFound, "LUA editMode: appSignalIdTuning = NULL.");
		lua_pushboolean(L, false);
		return 1;
	}

	char appSignalIdControl[STRING_MAX_LEN] = { 0 };
	if (SAFE_STR_COPY(appSignalIdControl, STRING_MAX_LEN, lua_tostring(L, 2)) == false)
	{
		logWarning(SignalWasNotFound, "LUA enterValueScreen: appSignalIdControl = NULL.");
		lua_pushboolean(L, false);
		return 1;
	}

	schema.tuningSignalIndex = appSignalIndex(appSignalIdTuning);
	schema.controlSignalIndex = appSignalIndex(appSignalIdControl);

	{
		const struct VduAppSignalParam* const param = appSignalParamByIndex(schema.tuningSignalIndex);
		if (param == NULL)
		{
			(void)snprintf(msg, STRING_MAX_LEN, "LUA enterValueScreen: tuning signal %s is not found.", appSignalIdTuning);
			logWarning(SignalReadTypeWrong, msg);
			lua_pushboolean(L, false);
			return 1;
		}

		if (param->props.bits.tunable == false)
		{
			(void)snprintf(msg, STRING_MAX_LEN, "LUA enterValueScreen: tuning signal %s is not tunable.", appSignalIdTuning);
			logWarning(SignalReadTypeWrong, msg);
			lua_pushboolean(L, false);
			return 1;
		}

		if (param->vduSignalInOutType != (alt_u16)Internal)
		{
			(void)snprintf(msg, STRING_MAX_LEN, "LUA enterValueScreen: tuning signal %s is not Internal.", appSignalIdTuning);
			logWarning(SignalReadTypeWrong, msg);
			lua_pushboolean(L, false);
			return 1;
		}
	}

	{
		const struct VduAppSignalParam* const param = appSignalParamByIndex(schema.controlSignalIndex);
		if (param == NULL)
		{
			(void)snprintf(msg, STRING_MAX_LEN, "LUA enterValueScreen: permission signal %s is not found.", appSignalIdControl);
			logWarning(SignalReadTypeWrong, msg);

			lua_pushboolean(L, false);
			return 1;
		}

		if (param->vduSignalType != (alt_u16)Discrete)
		{
			(void)snprintf(msg, STRING_MAX_LEN, "LUA enterValueScreen: permission signal %s is not discrete.", appSignalIdControl);
			logWarning(SignalReadTypeWrong, msg);
			lua_pushboolean(L, false);
			return 1;
		}
	}

	// Terminate drawing the current schema
	//
	schemaSetTerminate();

	// Show the tuning value input schema
	//
	popUpSchema(SCENE_EDIT_MODE, &schema);

	lua_pushboolean(L, schema.boolResult);
	return 1;
}

static int editModeByIndex(lua_State* L)
{
	char msg[STRING_MAX_LEN];
	struct PopUpSchemaParams schema = { 0 };

	schema.tuningSignalIndex = luaL_checkinteger(L, 1);
	schema.controlSignalIndex = luaL_checkinteger(L, 2);

	{
		const struct VduAppSignalParam* const param = appSignalParamByIndex(schema.tuningSignalIndex);
		if (param == NULL)
		{
			(void)snprintf(msg, STRING_MAX_LEN, "LUA enterValueScreenByIndex: tuning signal with index %u is not found.", schema.tuningSignalIndex);
			logWarning(SignalReadTypeWrong, msg);
			lua_pushboolean(L, false);
			return 1;
		}

		if (param->props.bits.tunable == false)
		{
			(void)snprintf(msg, STRING_MAX_LEN, "LUA enterValueScreenByIndex: tuning signal with index %u is not tunable.", schema.tuningSignalIndex);
			logWarning(SignalReadTypeWrong, msg);
			lua_pushboolean(L, false);
			return 1;
		}

		if (param->vduSignalInOutType != (alt_u16)Internal)
		{
			(void)snprintf(msg, STRING_MAX_LEN, "LUA enterValueScreenByIndex: tuning signal with index %u is not Internal.", schema.tuningSignalIndex);
			logWarning(SignalReadTypeWrong, msg);
			lua_pushboolean(L, false);
			return 1;
		}
	}

	{
		const struct VduAppSignalParam* const param = appSignalParamByIndex(schema.controlSignalIndex);
		if (param == NULL)
		{
			(void)snprintf(msg, STRING_MAX_LEN, "LUA enterValueScreenByIndex: permission signal with index %u is not found.", schema.controlSignalIndex);
			logWarning(SignalReadTypeWrong, msg);

			lua_pushboolean(L, false);
			return 1;
		}

		if (param->vduSignalType != (alt_u16)Discrete)
		{
			(void)snprintf(msg, STRING_MAX_LEN, "LUA enterValueScreenByIndex: permission signal with index %u is not discrete.", schema.controlSignalIndex);
			logWarning(SignalReadTypeWrong, msg);
			lua_pushboolean(L, false);
			return 1;
		}
	}

	// Terminate drawing the current schema
	//
	schemaSetTerminate();

	// Show the tuning value input schema
	//
	popUpSchema(SCENE_EDIT_MODE, &schema);

	lua_pushboolean(L, schema.boolResult);
	return 1;
}

static int checkPinCode(lua_State* L)
{
	char msg[STRING_MAX_LEN];
	struct PopUpSchemaParams schema = { 0 };

	char appSignalIdControl[STRING_MAX_LEN] = { 0 };
	if (SAFE_STR_COPY(appSignalIdControl, STRING_MAX_LEN, lua_tostring(L, 1)) == false)
	{
		logWarning(SignalWasNotFound, "LUA checkPinCode: appSignalIdControl = NULL.");
		lua_pushboolean(L, false);
		return 1;
	}

	schema.controlSignalIndex = appSignalIndex(appSignalIdControl);

	{
		const struct VduAppSignalParam* const param = appSignalParamByIndex(schema.controlSignalIndex);
		if (param == NULL)
		{
			(void)snprintf(msg, STRING_MAX_LEN, "LUA checkPinCode: permission signal with index %u is not found.", schema.controlSignalIndex);
			logWarning(SignalReadTypeWrong, msg);

			lua_pushboolean(L, false);
			return 1;
		}

		if (param->vduSignalType != (alt_u16)Discrete)
		{
			(void)snprintf(msg, STRING_MAX_LEN, "LUA checkPinCode: permission signal with index %u is not discrete.", schema.controlSignalIndex);
			logWarning(SignalReadTypeWrong, msg);
			lua_pushboolean(L, false);
			return 1;
		}
	}

	// Terminate drawing the current schema
	//
	schemaSetTerminate();

	// Show the PIN code schema
	//
	popUpSchema(SCENE_PINCODE_MODE, &schema);

	lua_pushboolean(L, schema.boolResult);
	return 1;
}

static int checkPinCodeByIndex(lua_State* L)
{
	char msg[STRING_MAX_LEN];
	struct PopUpSchemaParams schema = { 0 };
	
	schema.controlSignalIndex = luaL_checkinteger(L, 1);

	{
		const struct VduAppSignalParam* const param = appSignalParamByIndex(schema.controlSignalIndex);
		if (param == NULL)
		{
			(void)snprintf(msg, STRING_MAX_LEN, "LUA checkPinCodeByIndex: permission signal with index %u is not found.", schema.controlSignalIndex);
			logWarning(SignalReadTypeWrong, msg);

			lua_pushboolean(L, false);
			return 1;
		}

		if (param->vduSignalType != (alt_u16)Discrete)
		{
			(void)snprintf(msg, STRING_MAX_LEN, "LUA checkPinCodeByIndex: permission signal with index %u is not discrete.", schema.controlSignalIndex);
			logWarning(SignalReadTypeWrong, msg);
			lua_pushboolean(L, false);
			return 1;
		}
	}

	// Terminate drawing the current schema
	//
	schemaSetTerminate();

	// Show the PIN code schema
	//
	popUpSchema(SCENE_PINCODE_MODE, &schema);

	lua_pushboolean(L, schema.boolResult);
	return 1;
}

static int questionMessageBox(lua_State* L)
{
	struct PopUpSchemaParams schema = { 0 };
	(void)strncpy(schema.MessageBoxParams.messageText, luaL_checkstring(L, 1), STRING_MAX_LEN);

	// Terminate drawing the current schema
	//
	schemaSetTerminate();

	// Show the Question schema
	//
	popUpSchema(SCENE_QUESTION_MODE, &schema);

	lua_pushboolean(L, schema.boolResult);
	return 1;
}

static int luaLogShow(lua_State* L)
{
	(void)L;

	struct PopUpSchemaParams schema = { 0 };
	
	// Terminate drawing the current schema
	//
	schemaSetTerminate();

	// Show the tuning value input schema
	//
	popUpSchema(SCENE_LOG, &schema);

	return 0;
}

static int luaLogErrorCount(lua_State* L)
{
	lua_pushinteger(L, logErrorCount());
	return 1;
}

static int luaLogWarningCount(lua_State* L)
{
	lua_pushinteger(L, logWarningCount());
	return 1;
}

static int luaLogClearCounters(lua_State* L)
{
	(void)L;
	logClearCounters();
	return 0;
}

static int luaLogWriteError(lua_State* L)
{
	char msg[STRING_MAX_LEN] = { 0 };
	if (SAFE_STR_COPY(msg, STRING_MAX_LEN, lua_tostring(L, 1)) == false)
	{
		logWarning(InternalError, "LUA luaLogWriteError: msg = NULL.");
		return 0;
	}

	logError(InternalError, msg);

	return 0;
}
static int luaLogWriteWarning(lua_State* L)
{
	char msg[STRING_MAX_LEN] = { 0 };
	if (SAFE_STR_COPY(msg, STRING_MAX_LEN, lua_tostring(L, 1)) == false)
	{
		logWarning(InternalError, "LUA luaLogWriteWarning: msg = NULL.");
		return 0;
	}

	logWarning(InternalWarning, msg);
	
	return 0;
}

static int luaLogWriteMessage(lua_State* L)
{
	char msg[STRING_MAX_LEN] = { 0 };
	if (SAFE_STR_COPY(msg, STRING_MAX_LEN, lua_tostring(L, 1)) == false)
	{
		logWarning(InternalError, "LUA luaLogWriteMessage: msg = NULL.");
		return 0;
	}

	logMessage(msg);

	return 0;
}

static int luaTimerValue(lua_State* L)
{
	alt_u8 timerIndex = luaL_checkinteger(L, 1);
	if ((timerIndex != TIMER_MAINLOOP) && (timerIndex != TIMER_PREDRAW))
	{
		logWarning(InternalWarning, "LUA luaTimerValue: invalid timer index was supplied.");
		lua_pushnil(L);
	}
	else
	{
		lua_pushinteger(L, timerValue(timerIndex));
	}
	return 1;
}

static int luaTimerMaxValue(lua_State* L)
{
	alt_u8 timerIndex = luaL_checkinteger(L, 1);
	if ((timerIndex != TIMER_MAINLOOP) && (timerIndex != TIMER_PREDRAW))
	{
		logWarning(InternalWarning, "LUA luaTimerMaxValue: invalid timer index was supplied.");
		lua_pushnil(L);
	}
	else
	{
		lua_pushinteger(L, timerMaxValue(timerIndex));
	}
	return 1;
}

static int luaSignalsShow(lua_State* L)
{
	(void)L;
	struct PopUpSchemaParams schema = { 0 };

	// Terminate drawing the current schema
	//
	schemaSetTerminate();

	// Show the tuning value input schema
	//
	popUpSchema(SCENE_SIGNALS, &schema);

	return 0;
}

static int luaSetTrendDuration(lua_State* L)
{
	struct VduSchemaFileSchemaItem1* const schemaItem = (struct VduSchemaFileSchemaItem1* const)lua_touserdata(L, 1);
	if ((SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == false) ||
		(schemaItem->itemType != TYPE_TREND))
	{
		logWarning(ItemWasNotFound, "LUA luaSetTrendDuration: invalid item was supplied.");
		return 0;
	}

	alt_u32 duration = luaL_checkinteger(L, 2);

	// Find a TrendState record with such parameters
	//
	for (alt_u16 si = 0u; si < schemaItem->trend.signalCount; si++)
	{
		bool trendDataFound = false;

		alt_u16 dataCount = trendSignalDataCount();
		for (alt_u16 di = 0u; di < dataCount; di++)
		{
			const struct TrendSignalData* const tsd = trendSignalData(di);

			if ((tsd->appSignalIndex == schemaItem->trend.trendSignals[si].appSignalIndex) &&
				(tsd->validityAppSignalIndex == schemaItem->trend.trendSignals[si].validityAppSignalIndex) &&
				(tsd->durationSecs == duration) &&
				(tsd->columnCount == schemaItem->trend.columnCount))
			{
				schemaItem->trend.trendSignals[si].trendSignalDataRecordIndex = di;

				trendDataFound = true;
				break;
			}
		}

		if (trendDataFound == false)
		{
			char msg[STRING_MAX_LEN];
			(void)snprintf(msg, STRING_MAX_LEN, "LUA luaSetTrendDuration: can't set duration %d for signal with index %d.", (int)duration, (int)si);
			logWarning(InternalWarning, msg);
		}
	}

	return 0;
}

static int luaSetTrendViewMode(lua_State* L)
{
	struct VduSchemaFileSchemaItem1* const schemaItem = (struct VduSchemaFileSchemaItem1* const)lua_touserdata(L, 1);
	if ((SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == false) ||
		(schemaItem->itemType != TYPE_TREND))
	{
		logWarning(ItemWasNotFound, "LUA luaSetTrendViewMode: invalid item was supplied.");
		return 0;
	}

	alt_u16 viewMode = luaL_checkinteger(L, 2);
	if ((viewMode != (alt_u16)Separate) && (viewMode != (alt_u16)Overlapped))
	{
		logWarning(InternalWarning, "LUA luaSetTrendViewMode: invalid viewMode was supplied.");
		return 0;
	}

	schemaItem->trend.viewMode = viewMode;
	return 0;
}

static int luaSetTrendScaleType(lua_State* L)
{
	struct VduSchemaFileSchemaItem1* const schemaItem = (struct VduSchemaFileSchemaItem1* const)lua_touserdata(L, 1);
	if ((SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == false) ||
		(schemaItem->itemType != TYPE_TREND))
	{
		logWarning(ItemWasNotFound, "LUA luaSetTrendScaleType: invalid item was supplied.");
		return 0;
	}

	alt_u16 scaleType = luaL_checkinteger(L, 2);
	if ((scaleType != (alt_u16)Linear) && (scaleType != (alt_u16)Logarithmic))
	{
		logWarning(InternalWarning, "LUA luaSetTrendScaleType: invalid scaleType was supplied.");
		return 0;
	}
	
	schemaItem->trend.scaleType = scaleType;
	return 0;
}

static int luaSetTrendSignalHighViewLimit(lua_State* L)
{
	struct VduSchemaFileSchemaItem1* const schemaItem = (struct VduSchemaFileSchemaItem1* const)lua_touserdata(L, 1);
	if ((SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == false) ||
		(schemaItem->itemType != TYPE_TREND))
	{
		logWarning(ItemWasNotFound, "LUA luaSetTrendSignalHighViewLimit: invalid item was supplied.");
		return 0;
	}

	alt_u16 si = luaL_checkinteger(L, 2);
	if (si >= schemaItem->trend.signalCount)
	{
		logWarning(SignalWasNotFound, "LUA luaSetTrendSignalHighViewLimit: invalid signal index was supplied.");
		return 0;
	}

	switch (schemaItem->trend.trendSignals[si].highViewLimit.type)
	{
	case f32:		schemaItem->trend.trendSignals[si].highViewLimit.data.f32 = lua_tonumber(L, 3);		break;
	case i32:		schemaItem->trend.trendSignals[si].highViewLimit.data.i32 = luaL_checkinteger(L, 3);	break;
	default:		RN_ASSERT(false);	break;
	}
	
	return 0;
}

static int luaSetTrendSignalLowViewLimit(lua_State* L) 
{
	struct VduSchemaFileSchemaItem1* const schemaItem = (struct VduSchemaFileSchemaItem1* const)lua_touserdata(L, 1);
	if ((SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == false) ||
		(schemaItem->itemType != TYPE_TREND))
	{
		logWarning(ItemWasNotFound, "LUA luaSetTrendSignalLowViewLimit: invalid item was supplied.");
		return 0;
	}

	alt_u16 si = luaL_checkinteger(L, 2);
	if (si >= schemaItem->trend.signalCount)
	{
		logWarning(SignalWasNotFound, "LUA luaSetTrendSignalLowViewLimit: invalid signal index was supplied.");
		return 0;
	}


	switch (schemaItem->trend.trendSignals[si].lowViewLimit.type)
	{
	case f32:		schemaItem->trend.trendSignals[si].lowViewLimit.data.f32 = lua_tonumber(L, 3);		break;
	case i32:		schemaItem->trend.trendSignals[si].lowViewLimit.data.i32 = luaL_checkinteger(L, 3);		break;
	default:		RN_ASSERT(false);	break;
	}

	return 0;
}

static int luaSetTrendSignalLineWidth(lua_State* L) 
{
	struct VduSchemaFileSchemaItem1* const schemaItem = (struct VduSchemaFileSchemaItem1* const)lua_touserdata(L, 1);
	if ((SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == false) ||
		(schemaItem->itemType != TYPE_TREND))
	{
		logWarning(ItemWasNotFound, "LUA luaSetTrendSignalLineWidth: invalid item was supplied.");
		return 0;
	}

	alt_u16 si = luaL_checkinteger(L, 2);
	if (si >= schemaItem->trend.signalCount)
	{
		logWarning(SignalWasNotFound, "LUA luaSetTrendSignalLineWidth: invalid signal index was supplied.");
		return 0;
	}


	alt_u16 lineWidth = luaL_checkinteger(L, 3);
	if (lineWidth > 6u)
	{
		logWarning(InternalWarning, "LUA luaSetTrendSignalLineWidth: line width was limited to 6.");
		lineWidth = 6u;
	}

	schemaItem->trend.trendSignals[si].lineWeight = lineWidth;
	return 0;
}

static int luaSetTrendSignalLineColor(lua_State* L) 
{
	struct VduSchemaFileSchemaItem1* const schemaItem = (struct VduSchemaFileSchemaItem1* const)lua_touserdata(L, 1);
	if ((SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == false) ||
		(schemaItem->itemType != TYPE_TREND))
	{
		logWarning(ItemWasNotFound, "LUA luaSetTrendSignalLineColor: invalid item was supplied.");
		return 0;
	}

	alt_u16 si = luaL_checkinteger(L, 2);
	if (si >= schemaItem->trend.signalCount)
	{
		logWarning(SignalWasNotFound, "LUA luaSetTrendSignalLineColor: invalid signal index was supplied.");
		return 0;
	}


	schemaItem->trend.trendSignals[si].color = luaL_checkinteger(L, 3);
	return 0;
}

static int luaSetTrendSignalValueFormat(lua_State* L) 
{
	struct VduSchemaFileSchemaItem1* const schemaItem = (struct VduSchemaFileSchemaItem1* const)lua_touserdata(L, 1);
	if ((SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == false) ||
		(schemaItem->itemType != TYPE_TREND))
	{
		logWarning(ItemWasNotFound, "LUA luaSetTrendSignalValueFormat: invalid item was supplied.");
		return 0;
	}

	alt_u16 si = luaL_checkinteger(L, 2);
	if (si >= schemaItem->trend.signalCount)
	{
		logWarning(SignalWasNotFound, "LUA luaSetTrendSignalValueFormat: invalid signal index was supplied.");
		return 0;
	}


	alt_u16 valueFormat = luaL_checkinteger(L, 3);
	if ((valueFormat != (alt_u16)Auto) && (valueFormat != (alt_u16)Decimal) && (valueFormat != (alt_u16)Exponential))
	{
		logWarning(InternalWarning, "LUA luaSetTrendSignalValueFormat: invalid valueFormat was supplied.");
		return 0;
	}

	schemaItem->trend.trendSignals[si].valueFormat = valueFormat;
	return 0;
}

static int luaSetTrendSignalDecimalPlaces(lua_State* L) 
{
	struct VduSchemaFileSchemaItem1* const schemaItem = (struct VduSchemaFileSchemaItem1* const)lua_touserdata(L, 1);
	if ((SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == false) ||
		(schemaItem->itemType != TYPE_TREND))
	{
		logWarning(ItemWasNotFound, "LUA luaSetTrendSignalDecimalPlaces: invalid item was supplied.");
		return 0;
	}

	alt_u16 si = luaL_checkinteger(L, 2);
	if (si >= schemaItem->trend.signalCount)
	{
		logWarning(SignalWasNotFound, "LUA luaSetTrendSignalDecimalPlaces: invalid signal index was supplied.");
		return 0;
	}


	alt_16 places = luaL_checkinteger(L, 3);
	if (places < -1)
	{
		logWarning(InternalWarning, "LUA luaSetTrendSignalDecimalPlaces: line width was limited to -1.");
		places = -1;
	}
	if (places > 16)
	{
		logWarning(InternalWarning, "LUA luaSetTrendSignalDecimalPlaces: line width was limited to 16.");
		places = 16;
	}

	schemaItem->trend.trendSignals[si].decimalPlaces = places;
	return 0;
}

static int luaSetTrendSignalVisible(lua_State* L) 
{
	struct VduSchemaFileSchemaItem1* const schemaItem = (struct VduSchemaFileSchemaItem1* const)lua_touserdata(L, 1);
	if ((SCHEMA_ITEM_PTR_IS_VALID(schemaItem) == false) ||
		(schemaItem->itemType != TYPE_TREND))
	{
		logWarning(ItemWasNotFound, "LUA luaSetTrendSignalVisible: invalid item was supplied.");
		return 0;
	}

	alt_u16 si = luaL_checkinteger(L, 2);
	if (si >= schemaItem->trend.signalCount)
	{
		logWarning(SignalWasNotFound, "LUA luaSetTrendSignalVisible: invalid signal index was supplied.");
		return 0;
	}


	schemaItem->trend.trendSignals[si].flags.bits.visible = lua_toboolean(L, 3);
	return 0;
}
