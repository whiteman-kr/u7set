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

#ifndef RN_SCHEMA_H
#define RN_SCHEMA_H

#include <stdbool.h>
#include <lua.h>

#include <alt_types.h>

#include "defs.h"

#define SCENE_CURR_SH 0u
#define SCENE_GOD_MODE 1u
#define SCENE_CALIB_MODE 2u
#define SCENE_EDIT_MODE 3u
#define SCENE_PINCODE_MODE 4u
#define SCENE_QUESTION_MODE 5u
#define SCENE_LOG 6u
#define SCENE_SIGNALS 7u

enum InputState
{
	ValueOk = 0,
	IncorrectValue,
	OutOfRange,
	NoPermission
}; 

enum PinState
{
	PinCodeOk = 0,
	PinCodeEmpty,
	PinCodeWrong,
	PinNoPermission
};

struct PopUpSchemaParams
{
	bool boolResult;

	alt_u16 tuningSignalIndex;
	alt_u16 controlSignalIndex;

	union
	{
		struct
		{
			char messageText[STRING_MAX_LEN + 1u/*for ending zero*/];
		} MessageBoxParams;

		struct
		{
			enum InputState state;
			char editText[STRING_MAX_LEN];
		} InputBoxData;

		struct
		{

			enum PinState state;
			char editText[STRING_MAX_LEN];
			int tryCount;
		} PinBoxData;
	};
};

// Calculates if it is time to redraw a schema when 250 ms has been elapsed since last redraw
// Returns true if schema has to be redrawn, otherwise returns false
bool calculateSchemaDrawingPhase(void);

/*
Performs sequential rendering of all schema elements and executes their scripts.
Upon successful completion, it copies the frame to video memory. Upon failure,
it terminates the operation.
If drawingPhase parameter is false, no graphics is redrawn
*/
void schemaProc(lua_State* rvduLuaState, bool drawingPhase);

// Switches to processing a pop-up schema with specified type
//
void popUpSchema(alt_u8 type, struct PopUpSchemaParams* const schema);

// Sets the flag for stopping the processing of the schema.
//
void schemaSetTerminate(void);

// Sets the flag for stopping the processing of the popup schema.
//
void popUpSchemaSetTerminate(void);

// Returns the address of the schema element whose script is currently being processed.
//
struct VduSchemaFileSchemaItem1* const scriptRunSchemaItem(void);

// Sets the flag for full schema redraw on the next loop
//
void schemaSetFullRedraw(void);

// Returns blink phase: 0 or 1
//
bool blinkPhase(void);

#endif
