/*
Module title: Schemas Processing.
(C) 2026 RPC "Radiy".
All rights reserved.
The information presented here is confidential
and is the intellectual property of PJSC "RPC "Radiy".
Any use of this information is strictly
prohibited without the written consent of PJSC "RPC "Radiy".
Module description: Provides the main functionality for displaying RVDU schemas.

-- *************************************************************
-- Version v1.00
-- Date:	31.03.2026.
-- Designed by: Mazyr S.M. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Vintenko B.U.

-- Changes: RADIX-3966, rn_schema initial update

-- *************************************************************

-- Version v1.01
-- Date:	13.04.2026.
-- Designed by: Mazyr S.M. https://jira.radiy.com/browse/RADIX-3966, https://jira.radiy.com/browse/RADIX-3969
-- Reviewed by: Vintenko B.U.

-- Changes: RADIX-3966, rn_schema module refactoring
-- Change logic of schemaProc function
-- Add GlobalScript support
-- Add log error for schemaProc and logScriptError
-- Optimization logic of schemaProc function
-- Add keyboard support
-- Added performance counters support to display FPS and scripts executing time
-- And schemas transparency control
-- By default, schema's background is not cleared before redraw, this saves time. But if Lua functons that move items are called, this
schema is redrawed fully.
-- Now, predrawScripts and clickScripts are compiled to bytecode on startup.

-- Changes: RADIX-3969, add logging and debugging support
-- Add log errors and messages

-- *************************************************************

-- Version v1.02
-- Date:	19.06.2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-3966, schema switching improvement: it is scheduled and actually changed at the next schemaProc loop to prevent scripts
data corruption
-- RADIX-3973: add real-time blink phase functions

-- *************************************************************

-- Version v1.03
-- Date:	28.07.2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-3966, code is made MISRA-2025 compatible

-- *************************************************************

-- Version v1.04
-- Date:	06.08.2026.
-- Designed by: Vintenko B.U.
-- Reviewed by: Nesterenko M.V.

-- Changes:
RADIX-4110, red frame on error is displayed on both errors and warnings counter change
RADIX-4111, schema item labels are displayed in the log on errors
RADIX-4112, OnShowScript and PreDrawScript on schemas are processed

-- *************************************************************

-- Version v1.05
-- Date:	29.09.2026.
-- Designed by: Vintenko B.U.
-- Reviewed by: Nesterenko M.V.

-- Changes:
RADIX-4149, Redraw only non-static items when schema or items size do not change

-- *************************************************************
*/

#include <string.h>
#include <stdbool.h>

#include <lualib.h>
#include <lauxlib.h>

#include "rn_cfg.h"
#include "rn_schema.h"
#include "rn_diag.h"
#include "rn_vbuf.h"
#include "rn_prim.h"
#include "rn_ts.h"
#include "rn_utils.h"
#include "rn_scfp.h"
#include "rn_opto.h"
#include "rn_lapi.h"
#include "rn_ainput.h"
#include "rn_msgbox.h"
#include "rn_pin.h"
#include "rn_logview.h"
#include "rn_signalsview.h"
#include "rn_trendview.h"
#include "rn_trendproc.h"
#include "rn_kbd.h"

#include <alt_types.h>

#include "defs.h"

#define SCRIPT_OK 0u
#define SCRIPT_ERROR 1u

// Private module data
//
static bool m_terminateFlag = false;

static struct VduSchemaFileSchemaItem1* m_scriptRunSchemaItem = NULL;
static bool m_fullRedrawFlag = false;
static alt_u16 m_blinkPhase = 0;
//
// End of Private data

void popUpSchemaResetTerminate(void);

bool popUpSchemaTerminate(void);

static void logScriptWarning(lua_State* const luaState, enum WarningCode code, const char* const objectName, const char* const label)
{
	static char msg[STRING_MAX_LEN * 4u];

	const char* err = lua_tostring(luaState, -1);

	lua_pop(luaState, 1); // pop original error + traceback

	if ((objectName != NULL) && (strlen(objectName) != 0u))
	{
		(void)snprintf(msg, STRING_MAX_LEN * 4u, "%s, ObjectName = '%s': %s.", label, objectName, err);
	}
	else
	{
		(void)snprintf(msg, STRING_MAX_LEN * 4u, "%s: %s.", label, err);
	}

	logWarning(code, msg);
}

static void processSchemaKeys(void)
{
	if (isPressedKey() == false)
	{
		return;
	}

	alt_u16 key = pressedKey();

	if ((key == RK_LEFT) || (key == RK_RIGHT))
	{
		// Switch to previous/Next schema
		//

		// Get the index of current schema
		//
		bool currentSchemaFound = false;
		alt_u16 currentSchemaIndex = 0u;

		const struct VduConfigFile1* const cfg = vduConfig();
		if ((cfg == NULL) || (cfg->schemaCount == 0u))
		{
			return;
		}

		const struct VduSchemaDescriptor* const currentSchema = vduCurrentSchema();
		if (currentSchema == NULL)
		{
			RN_ASSERT(currentSchema != NULL);
			return;
		}

		for (alt_u16 i = 0; i < cfg->schemaCount; i++)
		{
			const struct VduSchemaDescriptor* const schema = vduSchemaByIndex(i);
			if (schema == NULL)
			{
				continue;
			}

			if (currentSchema->schemaFile->schemaProperties.schemaId == schema->schemaFile->schemaProperties.schemaId)
			{
				currentSchemaFound = true;
				currentSchemaIndex = i;
				break;
			}
		}

		if (currentSchemaFound == false)
		{
			RN_ASSERT(false);
			return;
		}

		// Get new schema index
		//
		if (key == RK_LEFT)
		{
			if (currentSchemaIndex == 0u)
			{
				currentSchemaIndex = cfg->schemaCount - 1u;
			}
			else
			{
				currentSchemaIndex--;
			}
		}
		else
		{
			currentSchemaIndex++;
			if (currentSchemaIndex >= cfg->schemaCount)
			{
				currentSchemaIndex -= cfg->schemaCount;
			}
		}

		// Switch to the new schema
		//
		const struct VduSchemaDescriptor* newSchema = vduSchemaByIndex(currentSchemaIndex);
		if (newSchema == NULL)
		{
			return;
		}

		(void)vduScheduleSchemaChange(vduSchemaString(newSchema, newSchema->schemaFile->schemaProperties.schemaId));

		return;
	}

	struct PopUpSchemaParams schema = { 0 };

	switch (key)
	{
	case RK_TILDA:
	{
		popUpSchema(SCENE_LOG, &schema);
	}
	break;
	case RK_TAB:
	{
		popUpSchema(SCENE_SIGNALS, &schema);
	}
	break;
	default:
	{
		return;
	}
	}
	return;
}

static inline bool runScript(lua_State* rvduLuaState, struct VduSchemaFileSchemaItem1* item, 
	alt_u32 scriptRef, alt_u32 scriptRefByteCode, 
	enum WarningCode warningCode,
	const char* scriptName)
{
	alt_u16 scriptLength = vduCurrentSchemaStringLength(scriptRef);
	if (scriptLength == 0u)
	{
		return true;
	}

	m_scriptRunSchemaItem = item;

	alt_u32 prevWarningCount = logWarningCount();

	static bool m_useByteCode = true;

	// Use LUA bytecode
	//
	if (m_useByteCode == true)
	{
		alt_u32 byteCodeLen = vduCurrentSchemaScriptLength(scriptRefByteCode);
		const char* const bytecode = vduCurrentSchemaScript(scriptRefByteCode);

		// Call the bytecode
		//
		int status = luaL_loadbuffer(rvduLuaState, bytecode, byteCodeLen, NULL);

		if (status != LUA_OK)
		{
			if (item != NULL)
			{
				logScriptWarning(rvduLuaState, warningCode, vduCurrentSchemaString(item->objectName), vduCurrentSchemaString(item->label));
			}
			else
			{
				logScriptWarning(rvduLuaState, warningCode, NULL, scriptName);
			}

			if ((bytecode != NULL) && (byteCodeLen > 4u))
			{
				alt_u8 bytecodeVersion = (alt_u8)bytecode[4];

				static char msg[STRING_MAX_LEN];
				(void)snprintf(msg, STRING_MAX_LEN, "Scripts bytecode compatibility error, using strings instead. Used Lua version is %s, bytecode version code is %x.",
					(const char*)LUA_RELEASE,
					bytecodeVersion);
				logWarning(warningCode, msg);
			}

			// Switch to using strings
			//
			m_useByteCode = false;
		}

		// Call the bytecode
		//
		if (m_useByteCode == true)
		{
			status = lua_pcall(rvduLuaState, 0, LUA_MULTRET, 0);

			if (status != LUA_OK)
			{
				if (item != NULL)
				{
					logScriptWarning(rvduLuaState, warningCode, vduCurrentSchemaString(item->objectName), vduCurrentSchemaString(item->label));
				}
				else
				{
					logScriptWarning(rvduLuaState, warningCode, NULL, "SchemaScript");
				}
				return false;
			}
		}
	}

	// Use LUA strings
	//
	if (m_useByteCode == false)
	{
		if (luaL_dostring(rvduLuaState, vduCurrentSchemaString(scriptRef)) != LUA_OK)
		{
			if (item != NULL)
			{
				logScriptWarning(rvduLuaState, warningCode, vduCurrentSchemaString(item->objectName), vduCurrentSchemaString(item->label));
			}
			else
			{
				logScriptWarning(rvduLuaState, warningCode, NULL, "SchemaScript");
			}
			return false;
		}
	}

	if (logWarningCount() > prevWarningCount)
	{
		return false;
	}

	return true;
}

// Perform drawing only if blink phase or second has been changed
//
bool calculateSchemaDrawingPhase(void)
{
	bool drawingPhase = true;

	// Calculate the blink phase
	//
	static alt_u16 m_prevBlinkPhase = 65535u;
	static alt_u16 m_prevSecs = 65535u;

	m_blinkPhase = currentMSecs() / 250u;
	alt_u16 m_secs = currentSecs();

	if (m_prevBlinkPhase == m_blinkPhase)
	{
		if (m_prevSecs == m_secs)
		{
			drawingPhase = false;
		}
	}

	m_prevBlinkPhase = m_blinkPhase;
	m_prevSecs = m_secs;

	return drawingPhase;
}

void schemaProc(lua_State* rvduLuaState, bool drawingPhase)
{
	if (rvduLuaState == NULL)
	{
		RN_ASSERT(rvduLuaState != NULL);
		return;
	}

	// Process schema switching by keyboard
	//
	processSchemaKeys();

	// If there is a schema switch pending - switch it
	//
	const struct VduSchemaDescriptor* const scheduledSchema = vduGetScheduledSchema();
	if (scheduledSchema != NULL)
	{
		(void)vduSetCurrentSchema(vduSchemaString(scheduledSchema, scheduledSchema->schemaFile->schemaProperties.schemaId));
	}

	// Take a pointer to current schema
	//
	const struct VduSchemaDescriptor* const pSchema = vduCurrentSchema();
	if (pSchema == NULL)
	{
		RN_ASSERT(pSchema != NULL);
		return;
	}

	// If schema change, redraw it including static items, and schedule onShowScript executing
	//
	bool drawStatic = false;
	bool runOnShowScript = false;
	
	static const struct VduSchemaDescriptor* prevSchema = NULL;
	if (prevSchema != pSchema)
	{
		// Schema has been changed
		//
		prevSchema = pSchema;
	
		drawingPhase = true;        // Draw the schema immediately
		drawStatic = true;			// Draw all static items
		runOnShowScript = true;		// Execute the onShowScript
	}

	// If popup schema has just terminated or full redraw is set, set redraw static items flag
	//
	if ((popUpSchemaTerminate() == true) || (m_fullRedrawFlag == true))
	{
		drawingPhase = true;		// Draw the schema immediately
		drawStatic = true;			// Draw all static items
	}

	popUpSchemaResetTerminate();
	m_fullRedrawFlag = false;

	// Erase the screen
	//
	if ((drawingPhase == true) && (drawStatic == true))
	{
		if (writeVideoBuffer(pSchema->schemaFile->schemaProperties.backgroundColor) == false)
		{
			RN_ASSERT(false);
			return;
		}
	}

	// Lock the input buffers (optical and input signals) to prevent signals data change
	// while rendering the schema
	//
	externalBusBridgeReadLock();

	// First pass - reset script errors and process clickScripts
	//
	bool touchEvent = isTouchEvent();

	for (alt_u16 itemIndex = 0u; itemIndex < pSchema->schemaFile->schemaItemCount; itemIndex++)
	{
		struct VduSchemaFileSchemaItem1* const item = vduCurrentSchemaItem(itemIndex);
		if (item == NULL)
		{
			RN_ASSERT(item != NULL);
			continue;
		}

		item->scriptResult = SCRIPT_OK;	// Reset script error flag on each item

		if ((touchEvent == true) && (item->acceptClick == true))
		{
			alt_u16 scriptLength = vduCurrentSchemaStringLength(item->clickScript);

			if (scriptLength != 0u)
			{
				if (checkTouchZone(item->rect.left, item->rect.top, item->rect.width, item->rect.height) == true)
				{
					if (runScript(rvduLuaState, item, item->clickScript, item->clickScriptBytecode, LuaClickScript, "ClickScript") == false)
					{
						item->scriptResult = SCRIPT_ERROR;
					}
				}
			}

			if (m_terminateFlag == true)
			{
				lua_settop(rvduLuaState, 0);

				m_terminateFlag = false;
				return;
			}
		}
	}

	if (drawingPhase == true)
	{
		// Second pass - preDrawScripts processing
		//
		startMeasuringTimer(TIMER_PREDRAW);

		// Schema OnShowScript
		//
		if (runOnShowScript == true)
		{
			alt_u16 scriptLength = vduCurrentSchemaStringLength(pSchema->schemaFile->schemaProperties.onShowScript);

			if (scriptLength != 0u)
			{
				(void)runScript(rvduLuaState,
								NULL,
								pSchema->schemaFile->schemaProperties.onShowScript,
								pSchema->schemaFile->schemaProperties.onShowScriptBytecode,
								LuaPreDrawScript,
								"SchemaOnShowScript");
			}
		}

		// Schema PredrawScript
		//
		{
			alt_u16 scriptLength = vduCurrentSchemaStringLength(pSchema->schemaFile->schemaProperties.preDrawScript);

			if (scriptLength != 0u)
			{
				(void)runScript(rvduLuaState,
								NULL,
								pSchema->schemaFile->schemaProperties.preDrawScript,
								pSchema->schemaFile->schemaProperties.preDrawScriptBytecode,
								LuaPreDrawScript,
								"SchemaPredrawScript");
			}
		}

		if (m_terminateFlag == true)
		{
			stopMeasuringTimer(TIMER_PREDRAW);

			lua_settop(rvduLuaState, 0);

			m_terminateFlag = false;
			return;
		}

		// Items PredrawScripts
		//		
		for (alt_u16 itemIndex = 0u; itemIndex < pSchema->schemaFile->schemaItemCount; itemIndex++)
		{
			struct VduSchemaFileSchemaItem1* const item = vduCurrentSchemaItem(itemIndex);
			if (item == NULL)
			{
				RN_ASSERT(item != NULL);
				continue;
			}

			if ((item->isStatic == true) && (drawStatic == false))
			{
				continue;
			}

			alt_u16 scriptLength = vduCurrentSchemaStringLength(item->preDrawScript);

			if (scriptLength != 0u)
			{
				if (runScript(rvduLuaState,
					item,
					item->preDrawScript,
					item->preDrawScriptBytecode,
					LuaPreDrawScript,
					"PredrawScript") == false)
				{
					item->scriptResult = SCRIPT_ERROR;
				}
			}

			if (m_terminateFlag == true)
			{
				stopMeasuringTimer(TIMER_PREDRAW);

				lua_settop(rvduLuaState, 0);

				m_terminateFlag = false;
				return;
			}
		}

		stopMeasuringTimer(TIMER_PREDRAW);
	
		// Third pass - items rendering
		//
		for (alt_u16 itemIndex = 0u; itemIndex < pSchema->schemaFile->schemaItemCount; itemIndex++)
		{
			struct VduSchemaFileSchemaItem1* const item = vduCurrentSchemaItem(itemIndex);
			if (item == NULL)
			{
				RN_ASSERT(item != NULL);
				continue;
			}

			if ((item->isStatic == true) && (drawStatic == false))
			{
				continue;
			}

			alt_u32 prevErrorCount = logErrorCount();
			alt_u32 prevWarningCount = logWarningCount();

			switch (item->itemType)
			{
			case TYPE_LINE:
			{
				drawItemLine(item);
			}
			break;
			case TYPE_RECT:
			{
				drawItemRect(item);
			}
			break;
			case TYPE_VALUE:
			{
				drawItemValue(item);
			}
			break;
			case TYPE_IMAGE:
			case TYPE_IMAGEVALUE:
			{
				drawImage(item);
			}
			break;
			case TYPE_TREND:
			{
				drawTrend(&item->trend);
			}
			break;
			default:
			{
				// Unknown item - does not require any action
				//
			}
			break;
			}

			if ((logErrorCount() > prevErrorCount) || (logWarningCount() > prevWarningCount))
			{
				item->scriptResult = SCRIPT_ERROR;
			}
		}

		// Fouth pass - displaying errors
		//
		bool blink = blinkPhase();

		for (alt_u16 itemIndex = 0u; itemIndex < pSchema->schemaFile->schemaItemCount; itemIndex++)
		{
			const struct VduSchemaFileSchemaItem1* const item = vduCurrentSchemaItem(itemIndex);
			if (item == NULL)
			{
				RN_ASSERT(item != NULL);
				continue;
			}

			if (item->scriptResult == SCRIPT_ERROR)
			{
				drawErrorRect(item->rect.left, item->rect.top, item->rect.width, item->rect.height, 1u,
					blink ? 0xff0000u : 0xffffffu);
			}
		}
	}

	// Unlock the input buffers (allow optical and input signals to be modified)
	//
	externalBusBridgeReadUnlock();

	lua_settop(rvduLuaState, 0);

	if (drawingPhase == true)
	{
		(void)pushVideoBuffer();
	}

	return;
}

// Performs processing of pop-up schemas: messageBox, pinCode, etc.
//
void popUpSchema(alt_u8 type, struct PopUpSchemaParams* const schema)
{
	//
	// Here the external bus shall be UNLOCKED (externalBusBridgeReadUnlock was called in schemaProc())
	//

	// Stop measuring timer
	//
	stopMeasuringTimer(TIMER_MAINLOOP);

	// Erase the screen
	//
	alt_u32 backColorArea = 0xf8f8f8;
	if (writeVideoBuffer(backColorArea) == false)
	{
		RN_ASSERT(false);
		return;
	}

	while (popUpSchemaTerminate() == false)
	{
		startMeasuringTimer(TIMER_MAINLOOP);

		// Process trend data
		//
		trendsProc();

		// Process pop-up schema
		//
		switch (type)
		{
		case SCENE_EDIT_MODE:
		{
			externalBusBridgeReadLock();

			modeEditShow(schema);

			externalBusBridgeReadUnlock();

			(void)pushVideoBuffer();

			externalBusBridgeReadLock();

			modeEditProc(schema);

			externalBusBridgeReadUnlock();
		}
		break;
		case SCENE_PINCODE_MODE:
		{
			modePinShow(schema);
			
			(void)pushVideoBuffer();
			
			modePinProc(schema);
		}
		break;
		case SCENE_QUESTION_MODE:
		{
			externalBusBridgeReadLock();

			modeQuestionShow(schema);

			externalBusBridgeReadUnlock();

			(void)pushVideoBuffer();

			modeQuestionProc(schema);
		}
		break;
		case SCENE_LOG:
		{
			logSchemaShow();
			
			(void)pushVideoBuffer();
			
			logSchemaProc(schema);
		}
		break;
		case SCENE_SIGNALS:
		{
			externalBusBridgeReadLock();

			signalsViewShow();

			externalBusBridgeReadUnlock();

			(void)pushVideoBuffer();

			signalsViewProc(schema);
		}
		break;
		default:
		{
		}
		break;
		}

		// Write optical TX signals
		//
		optoTxProc();

		// Update time measuring timer
		//
		stopMeasuringTimer(TIMER_MAINLOOP);
		setTimerValue(TIMER_MAINLOOP, measuringTimerValue(TIMER_MAINLOOP));

		// Write diagnostics
		//
		writeExternalDiagPacket();
	}

	// Start measuring timer
	//
	startMeasuringTimer(TIMER_MAINLOOP);
}

void schemaSetTerminate(void)
{
	externalBusBridgeReadUnlock();

	m_terminateFlag = true;
}

struct VduSchemaFileSchemaItem1* const scriptRunSchemaItem(void)
{
	return m_scriptRunSchemaItem;
}

void schemaSetFullRedraw(void) 
{ 
	m_fullRedrawFlag = true; 
}

bool blinkPhase(void)
{
	return ((m_blinkPhase & 1u) == 1u) ? true : false;
}
