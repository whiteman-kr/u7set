/*
Module title: Analog Value Input Schema Processing
(C) 2026 RPC "Radiy".
All rights reserved.
The information presented here is confidential
and is the intellectual property of PJSC "RPC "Radiy".
Any use of this information is strictly
prohibited without the written consent of PJSC "RPC "Radiy".
Module description: Provides the functionality for entering the analog value.

-- *************************************************************
-- Version v1.00
-- Date:	31,03,2026.
-- Designed by: Mazyr S.M. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Vintenko B.U.
-- Changes: RADIX-3966, rn_ainput initial update

-- *************************************************************
-- Version v1.01
-- Date:	01,05,2026.
-- Designed by: Mazyr S.M. https://jira.radiy.com/browse/RADIX-3966, https://jira.radiy.com/browse/RADIX-3969
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-3966, code refactoring
-- Add functions description
-- Add keyboard support
-- Add checking limits on input string lengths
-- Errors and warnings logging review
-- rn_ainput: add version history of module descriptions

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

-- Version v1.04
-- Date:	10,08,2026.
-- Designed by: Vintenko B.U.
-- Reviewed by: Nesterenko M.V.

-- Changes: 
	RADIX-4112, API functions improvements

-- *************************************************************
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifndef RVDU_SIM // Microsoft Visual Studio-specific

#include "../rvdu_bsp/HAL/inc/sys/alt_errno.h"

#else

#include <windows.h>

#endif // End of Microsoft Visual Studio-specific


#include "rn_ainput.h"
#include "rn_vbuf.h"
#include "rn_prim.h"
#include "rn_ts.h"
#include "rn_kbd.h"
#include "rn_sifp.h"
#include "rn_schema.h"
#include "rn_fofp.h"
#include "rn_diag.h"
#include "rn_utils.h"

#include "defs.h"


// Private module data
//
static alt_u16 m_pressedButton = 0u;

static char m_defaultValueStr[STRING_MAX_LEN];

#define MAX_INPUT_LEN 16u

#define KEY_BUTTON_COUNT 15u
#define KEY_BUTTON_IDENT 60u

//#define CODE_NONE 0
#define KEY_CHAR_1 1u
//#define KEY_CHAR_9 9

#define KEY_CHAR_BASE KEY_CHAR_1

#define KEY_CHAR_PT		11u
#define KEY_CHAR_MINUS	12u
#define KEY_CHAR_PLUS	13u
#define KEY_CHAR_E		14u

#define KEY_ERASE		15u
#define KEY_DEFAULT		16u
#define KEY_WRITEVALUE	17u
#define KEY_CANCEL		18u
#define KEY_ERROR		19u

#define BUTTONS_COUNT 32u
static alt_u16 m_buttonsCount = 0u;
static struct Button m_buttons[BUTTONS_COUNT] = { 0 };
static struct Button m_errorButton = { 0 };
//
// End of Private data

static void createButtons(void)
{
	static const char m_keyButton[KEY_BUTTON_COUNT] = {'1', '2', '3',
												 '4', '5', '6',
												 '7', '8', '9',
												 '0', '.', '-',
												 '+', 'E', '<'};

	{
		alt_u16 keyButtonX = 195;
		for (alt_u16 i = 0; i < KEY_BUTTON_COUNT; i++)
		{
			struct Button b = { .x = keyButtonX, .y = 625, .w = 50, .h = 55, .code = KEY_CHAR_BASE + i };
			b.text[0] = m_keyButton[i];
			b.text[1] = '\0';

			m_buttons[m_buttonsCount] = b;
			m_buttonsCount++;

			keyButtonX += KEY_BUTTON_IDENT;
		}
	}

	{
		struct Button b = { .x = 350, .y = 375, .w = 170, .h = 40, .code = KEY_DEFAULT };
		//(void)snprintf(b.text, BUTTON_TEXT_MAX_LEN, "Default: %s", m_defaultValueStr);
		m_buttons[m_buttonsCount] = b;
		m_buttonsCount++;
	}

	{
		struct Button b = { .x = 680, .y = 375, .w = 120, .h = 40, .code = KEY_WRITEVALUE };
		(void)snprintf(b.text, BUTTON_TEXT_MAX_LEN, "Write");
		m_buttons[m_buttonsCount] = b;
		m_buttonsCount++;
	}

	{
		struct Button b = { .x = 810, .y = 375, .w = 120, .h = 40, .code = KEY_CANCEL};
		(void)snprintf(b.text, BUTTON_TEXT_MAX_LEN, "Cancel");
		m_buttons[m_buttonsCount] = b;
		m_buttonsCount++;
	}

	{
		struct Button b = { .x = 580, .y = 530, .w = 120, .h = 40, .code = KEY_ERROR };
		(void)snprintf(b.text, BUTTON_TEXT_MAX_LEN, "OK");
		m_errorButton = b;
	}
}

void modeEditShow(struct PopUpSchemaParams* const schema) 
{
	static struct Rect m_drawArea = {.x = 340, .y = 135, .w = 600, .h = 290};
	static struct Rect m_drawRect = {.x = 340, .y = 135, .w = 600, .h = 290};
	static struct Rect m_signalStr = {.x = 350, .y = 145, .w = 120, .h = 25};

	static struct Rect m_drawRectSignalID = {.x = 480, .y = 205, .w = 450, .h = 25};
	static struct Rect m_drawRectSignalLimit = {.x = 480, .y = 275, .w = 320, .h = 25};

	static struct Rect m_captionStr = {.x = 350, .y = 175, .w = 120, .h = 25};
	static struct Rect m_limitStr = {.x = 350, .y = 205, .w = 120, .h = 25};
	static struct Rect m_currentValueStr = {.x = 350, .y = 275, .w = 120, .h = 25};

	static struct Rect m_customAppSignalIdStr = {.x = 480, .y = 145, .w = 450, .h = 25};
	static struct Rect m_signalCaptionStr = {.x = 480, .y = 175, .w = 450, .h = 25};
	static struct Rect m_signalUnitStr = {.x = 810, .y = 305, .w = 110, .h = 25};

	static struct Rect m_rangeStr = {.x = 482, .y = 205, .w = 450, .h = 25};
	static struct Rect m_valueStr = {.x = 480, .y = 275, .w = 320, .h = 25};

	static struct Rect m_defaultStr = {.x = 350, .y = 375, .w = 170, .h = 40};	
	static struct Rect m_editTextStr = {.x = 480, .y = 305, .w = 340, .h = 25};

	static struct Rect m_drawAreaButtonLayout = {.x = 185, .y = 615, .w = 910, .h = 75};
	static struct Rect m_drawRectButtonLayout = {.x = 185, .y = 615, .w = 910, .h = 75};

	static struct Rect m_drawAreaIncorrect = {.x = 470, .y = 460, .w = 340, .h = 120};
	static struct Rect m_drawRectIncorrect = {.x = 470, .y = 460, .w = 340, .h = 120};

	static struct Rect m_pinStrIncorrect = {.x = 475, .y = 465, .w = 320, .h = 25};	

	if (schema == NULL)
	{
		RN_ASSERT(schema != NULL);
		return;
	}

	alt_u32 backColorRect = 0x888888;
	alt_u32 backColorStr = 0xf8f8f8;

	static const alt_32 blackColor = 0x0;

	alt_u32 backColorArea = 0xdcdcdc;

	drawArea(m_drawArea.x, m_drawArea.y, m_drawArea.w, m_drawArea.h, backColorArea);
	drawRect(m_drawRect.x, m_drawRect.y, m_drawRect.w, m_drawRect.h, 1u, backColorRect);

	backColorStr = 0xdcdcdc;

	printStr(m_signalStr.x, m_signalStr.y, m_signalStr.w, m_signalStr.h,
		RVDU_SYSTEM_FONT, blackColor, backColorStr, ALIGN_V_CENTER + ALIGN_H_LEFT, "Signal");

	printStr(m_captionStr.x, m_captionStr.y, m_captionStr.w, m_captionStr.h,
		RVDU_SYSTEM_FONT, blackColor, backColorStr, ALIGN_V_CENTER + ALIGN_H_LEFT, "Caption");

	printStr(m_limitStr.x, m_limitStr.y, m_limitStr.w, m_limitStr.h,
		RVDU_SYSTEM_FONT, blackColor, backColorStr, ALIGN_V_CENTER + ALIGN_H_LEFT, "Limit");

	printStr(m_currentValueStr.x, m_currentValueStr.y, m_currentValueStr.w, m_currentValueStr.h,
		RVDU_SYSTEM_FONT, blackColor, backColorStr, ALIGN_V_CENTER + ALIGN_H_LEFT, "Current Value");

	drawRect(m_customAppSignalIdStr.x, m_customAppSignalIdStr.y,
		m_customAppSignalIdStr.w, m_customAppSignalIdStr.h, 1u, backColorRect);

	printStr(m_customAppSignalIdStr.x + 2u /*shift by 2 px*/, m_customAppSignalIdStr.y,
		m_customAppSignalIdStr.w, m_customAppSignalIdStr.h,
		RVDU_SYSTEM_FONT, blackColor, backColorStr, ALIGN_V_CENTER + ALIGN_H_LEFT, customAppSignalID(schema->tuningSignalIndex));

	drawRect(m_signalCaptionStr.x, m_signalCaptionStr.y, m_signalCaptionStr.w, m_signalCaptionStr.h, 1u, backColorRect);
	printStr(m_signalCaptionStr.x + 2u /*shift by 2 px*/, m_signalCaptionStr.y, m_signalCaptionStr.w, m_signalCaptionStr.h,
		RVDU_SYSTEM_FONT, blackColor, backColorStr, ALIGN_V_CENTER + ALIGN_H_LEFT, signalCaption(schema->tuningSignalIndex));

	// Signal layout
	//
	drawRect(m_drawRectSignalID.x, m_drawRectSignalID.y, m_drawRectSignalID.w, m_drawRectSignalID.h, 1u, backColorRect);

	// Limits layout
	//
	drawRect(m_drawRectSignalLimit.x, m_drawRectSignalLimit.y, m_drawRectSignalLimit.w, m_drawRectSignalLimit.h, 1u,
		backColorRect);

	// Value layout
	//
	drawRect(m_signalUnitStr.x, m_signalUnitStr.y, m_signalUnitStr.w, m_signalUnitStr.h, 1u, backColorRect);
	printStr(m_signalUnitStr.x, m_signalUnitStr.y, m_signalUnitStr.w, m_signalUnitStr.h,
		RVDU_SYSTEM_FONT, blackColor, backColorStr, ALIGN_V_CENTER + ALIGN_H_CENTER, signalUnit(schema->tuningSignalIndex));

	alt_u32 keysColor = 0x808080;
	if (schema->InputBoxData.state == ValueOk)
	{
		keysColor = blackColor;
	}

	const struct VduAppSignalParam* const param = appSignalParamByIndex(schema->tuningSignalIndex);
	if (param == NULL)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "modeEditShow: signal by index = %d not found.", schema->tuningSignalIndex);
		logWarning(SignalWasNotFound, msg);

		// Abort the dialog
		schema->retPermission = true;
		schema->boolResult = false;
		return;
	}

	const struct VduAppSignalState* const state = appSignalStateByIndex(schema->tuningSignalIndex);
	if (state == NULL)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "modeEditShow: signal by index = %d not found.", schema->tuningSignalIndex);
		logWarning(SignalWasNotFound, msg);

		// Abort the dialog
		schema->retPermission = true;
		schema->boolResult = false;
		return;
	}

	// This schema processes ONLY internal signals
	//
	if (param->vduSignalInOutType != (alt_u16)Internal)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "modeEditShow: tunable signal must be Internal type");
		logWarning(SignalWriteTypeWrong, msg);

		// Abort the dialog
		schema->retPermission = true;
		schema->boolResult = false;
		return;
	}

	// Edit
	//
	backColorArea = 0xf8f8f8;
	backColorStr = 0xf8f8f8;
	drawArea(m_editTextStr.x, m_editTextStr.y, m_editTextStr.w, m_editTextStr.h, backColorArea);
	drawRect(m_editTextStr.x, m_editTextStr.y, m_editTextStr.w, m_editTextStr.h, 1u, blackColor);
	printStr(m_editTextStr.x, m_editTextStr.y, m_editTextStr.w, m_editTextStr.h,
			RVDU_SYSTEM_FONT, blackColor, backColorStr, ALIGN_V_CENTER + ALIGN_H_CENTER, (schema->InputBoxData.editText));

	// Draw buttons area
	//
	backColorArea = 0xdcdcdc;
	backColorRect = 0x888888;
	drawArea(m_drawAreaButtonLayout.x, m_drawAreaButtonLayout.y, m_drawAreaButtonLayout.w, m_drawAreaButtonLayout.h, backColorArea);
	drawRect(m_drawRectButtonLayout.x, m_drawRectButtonLayout.y, m_drawRectButtonLayout.w, m_drawRectButtonLayout.h,
		1u, backColorRect);
	
	// Draw buttons
	//
	backColorStr = 0xc0c0c0;

	if (schema->InputBoxData.state == ValueOk)
	{
		// Erase incorrect pin area
		//
		backColorArea = 0xf8f8f8;
		backColorRect = 0xf8f8f8;
		drawArea(m_drawAreaIncorrect.x, m_drawAreaIncorrect.y, m_drawAreaIncorrect.w, m_drawAreaIncorrect.h, backColorArea);
		drawRect(m_drawRectIncorrect.x, m_drawRectIncorrect.y, m_drawRectIncorrect.w, m_drawRectIncorrect.h, 1u, backColorRect);
	}
	else
	{
		// Draw incorrect value area
		//
		backColorArea = 0xdcdcdc;
		backColorRect = 0x888888;
		backColorStr = 0xdcdcdc;

		drawArea(m_drawAreaIncorrect.x, m_drawAreaIncorrect.y, m_drawAreaIncorrect.w, m_drawAreaIncorrect.h, backColorArea);
		drawRect(m_drawRectIncorrect.x, m_drawRectIncorrect.y, m_drawRectIncorrect.w, m_drawRectIncorrect.h, 1u, backColorRect);


		printStr(m_pinStrIncorrect.x, m_pinStrIncorrect.y, m_pinStrIncorrect.w, m_pinStrIncorrect.h,
				RVDU_SYSTEM_FONT, blackColor, backColorStr, ALIGN_V_CENTER + ALIGN_H_CENTER, "ERROR" );
		printStr(m_pinStrIncorrect.x, m_pinStrIncorrect.y + 30u /*shift by 30 px*/, m_pinStrIncorrect.w, m_pinStrIncorrect.h,
				RVDU_SYSTEM_FONT, blackColor, backColorStr, ALIGN_V_CENTER + ALIGN_H_CENTER,
				( schema->InputBoxData.state == IncorrectValue)?("The value is incorrect"):
				( schema->InputBoxData.state == OutOfRange)?("The value is out of range"):
				( schema->InputBoxData.state == NoPermission)?("Control permission has been disabled"):
				("Internal Exception") );
		
		backColorArea = 0xc0c0c0;
		backColorRect = 0x000000;
		backColorStr = 0xc0c0c0;


		drawArea(m_errorButton.x, m_errorButton.y, m_errorButton.w, m_errorButton.h, backColorArea);
		drawRect(m_errorButton.x, m_errorButton.y, m_errorButton.w, m_errorButton.h, 1u, backColorRect);
		printStr(m_errorButton.x, m_errorButton.y, m_errorButton.w, m_errorButton.h,
				RVDU_SYSTEM_FONT, blackColor, backColorStr, ALIGN_V_CENTER + ALIGN_H_CENTER, "OK" );
	}

	// Initialize all buttons
	//
	if (m_buttonsCount == 0u)
	{
		createButtons();
	}

	// Draw all buttons
	//
	backColorArea = 0xc0c0c0;
	for (alt_u16 i = 0; i < m_buttonsCount; i++)
	{
		drawArea(m_buttons[i].x, m_buttons[i].y, m_buttons[i].w, m_buttons[i].h, backColorArea);

		drawRect(m_buttons[i].x, m_buttons[i].y, m_buttons[i].w, m_buttons[i].h,
			(m_pressedButton == m_buttons[i].code) ? (2u) : (1u), keysColor);

		printStr(m_buttons[i].x, m_buttons[i].y, m_buttons[i].w, m_buttons[i].h,
			RVDU_SYSTEM_FONT, keysColor, backColorStr, ALIGN_V_CENTER + ALIGN_H_CENTER, m_buttons[i].text);
	}

	// Draw dynamic text
//
	if (param->vduSignalType == (alt_u16)AnalogFloat32)
	{
		char putStr[STRING_MAX_LEN];

		// Range
		//
#define FORMAT_STR_MAX_LENGTH 32u
		char float32Len[FORMAT_STR_MAX_LENGTH];
		(void)snprintf(float32Len, FORMAT_STR_MAX_LENGTH, "%%.%df", param->decimalPlaces);

		char float32Format[STRING_MAX_LEN];
		(void)snprintf(float32Format, STRING_MAX_LEN, "%s .. %s %%s", float32Len, float32Len);

		(void)snprintf(putStr, STRING_MAX_LEN, float32Format,
			param->lowLimit.floatValue,
			param->highLimit.floatValue,
			signalUnit(schema->tuningSignalIndex));

		printStr(m_rangeStr.x, m_rangeStr.y, m_rangeStr.w, m_rangeStr.h,
			RVDU_SYSTEM_FONT, blackColor, backColorStr, ALIGN_V_CENTER + ALIGN_H_LEFT, putStr);

		// Value
		//
		(void)snprintf(putStr, STRING_MAX_LEN, float32Len, state->value.floatValue);

		printStr(m_valueStr.x, m_valueStr.y, m_valueStr.w, m_valueStr.h,
			RVDU_SYSTEM_FONT, blackColor, backColorStr, ALIGN_V_CENTER + ALIGN_H_CENTER, putStr);

		// Default
		//
		(void)snprintf(m_defaultValueStr, STRING_MAX_LEN, float32Len, param->defaultValue.floatValue);
		(void)snprintf(putStr, STRING_MAX_LEN, "Default: %s", m_defaultValueStr);

		printStr(m_defaultStr.x, m_defaultStr.y, m_defaultStr.w, m_defaultStr.h,
			RVDU_SYSTEM_FONT, keysColor, backColorStr, ALIGN_V_CENTER + ALIGN_H_CENTER, putStr);
	}
	else
	{
		char putStr[STRING_MAX_LEN];

		// Range
		//
		(void)snprintf(putStr, STRING_MAX_LEN, "%d .. %d %s",
			(int)param->lowLimit.intValue,
			(int)param->highLimit.intValue,
			signalUnit(schema->tuningSignalIndex));

		printStr(m_rangeStr.x, m_rangeStr.y, m_rangeStr.w, m_rangeStr.h,
			RVDU_SYSTEM_FONT, blackColor, backColorStr, ALIGN_V_CENTER + ALIGN_H_LEFT, putStr);

		// Value
		//
		(void)snprintf(putStr, STRING_MAX_LEN, "%d", (int)state->value.intValue);

		printStr(m_valueStr.x, m_valueStr.y, m_valueStr.w, m_valueStr.h,
			RVDU_SYSTEM_FONT, blackColor, backColorStr, ALIGN_V_CENTER + ALIGN_H_CENTER, putStr);

		// Default
		//
		backColorStr = 0xc0c0c0;
		(void)snprintf(m_defaultValueStr, STRING_MAX_LEN, "%d", (int)param->defaultValue.intValue);
		(void)snprintf(putStr, STRING_MAX_LEN, "Default: %s", m_defaultValueStr);

		printStr(m_defaultStr.x, m_defaultStr.y, m_defaultStr.w, m_defaultStr.h,
			RVDU_SYSTEM_FONT, keysColor, backColorStr, ALIGN_V_CENTER + ALIGN_H_CENTER, putStr);
	}

	(void)pushVideoBuffer();

	return;
}

static void onWritePressed(struct PopUpSchemaParams* const schema)
{
	const struct VduAppSignalParam* const param = appSignalParamByIndex(schema->tuningSignalIndex);
	if (param == NULL)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "onWritePressed: signal by index = %d not found", schema->tuningSignalIndex);
		logWarning(SignalWasNotFound, msg);

		// Abort the dialog
		schema->retPermission = true;
		schema->boolResult = false;
		return;
	}

	// Write operation for analog float32 signal
	//
	if (param->vduSignalType == (alt_u16)AnalogFloat32)
	{
		errno = 0u;
		char* lastChar = NULL;
		float nv = strtof(schema->InputBoxData.editText, &lastChar);
		if ((strlen(schema->InputBoxData.editText) == 0u) ||
			(0 != errno) ||
			(*lastChar != '\0'))
		{
			// Number is invalid
			//
			schema->InputBoxData.state = IncorrectValue;
		}
		else if ((nv < param->lowLimit.floatValue) || (nv > param->highLimit.floatValue))
		{
			// Number is out of range
			//
			schema->InputBoxData.state = OutOfRange;
		}
		else
		{
			// Close the dialog
			//
			schema->retPermission = true;
			m_pressedButton = 0;

			schema->boolResult = true;

			// Write the signal
			//
			if (writeSignalValueFloatByIndex(schema->tuningSignalIndex, nv) == false)
			{
				char msg[STRING_MAX_LEN];
				(void)snprintf(msg, STRING_MAX_LEN, "modeEditProc: signal by tuningSignalIndex = %d, wrong signal type", schema->tuningSignalIndex);
				logWarning(SignalWriteTypeWrong, msg);

				schema->boolResult = false;
			}
			else
			{
				schema->InputBoxData.editText[0] = '\0';
			}

			return;
		}
	}
	// Write operation for discrete / analog signed integer signal
	//
	else
	{
		char* lastChar = NULL;
		errno = 0u;
		alt_32 nv = strtol(schema->InputBoxData.editText, &lastChar, 10);
		if ((strlen(schema->InputBoxData.editText) == 0u) ||
			(errno != 0) ||
			(*lastChar != '\0'))
		{
			// Invalid number format
			//
			schema->InputBoxData.state = IncorrectValue;
		}
		else if (
			(nv < param->lowLimit.intValue) ||
			(nv > param->highLimit.intValue))
		{
			// Number is out of range
			//
			schema->InputBoxData.state = OutOfRange;
		}
		else
		{
			// Close the dialog
			//
			schema->retPermission = true;
			m_pressedButton = 0;

			schema->boolResult = true;

			// Write the signal
			//
			if (writeSignalValueIntByIndex(schema->tuningSignalIndex, nv) == false)
			{
				char msg[STRING_MAX_LEN];
				(void)snprintf(msg, STRING_MAX_LEN, "modeEditProc: signal by index = %d, wrong signal type", schema->tuningSignalIndex);
				logWarning(SignalWriteTypeWrong, msg);

				schema->boolResult = false;
			}
			else
			{
				schema->InputBoxData.editText[0] = '\0';
			}

			return;
		}
	}
}


static void updateValue(struct PopUpSchemaParams* const schema)
{
	char* dest = (char*)(schema->InputBoxData.editText);

	// Make pointer to last symbol
	//
	while (*dest != 0)
	{
		dest++;
	}

	if (m_pressedButton == KEY_ERASE)
	{
		if (strlen(schema->InputBoxData.editText) != 0u)
		{
			dest--;
			*dest = '\0';
		}
		return;
	}
	else
	{
		// Check keys "0..9, E, ., -, +"
		//
		if (strlen(schema->InputBoxData.editText) < MAX_INPUT_LEN)
		{
			*dest = m_buttons[m_pressedButton - KEY_CHAR_BASE].text[0];
			dest++;
		}

		*dest = '\0';
	}
}

static void onOKInErrorModePressed(struct PopUpSchemaParams* const schema)
{
	// If there is no permission, we can press only "OK" button
	//
	if (schema->InputBoxData.state == NoPermission)
	{
		schema->retPermission = true;
	}
	else
	{
		schema->InputBoxData.state = ValueOk;
	}
	m_pressedButton = 0;
	return;
}

static void onButtonPressed(struct PopUpSchemaParams* const schema)
{
	switch (m_pressedButton)
	{
	case KEY_DEFAULT:
	{
		(void)strncpy(schema->InputBoxData.editText, m_defaultValueStr, STRING_MAX_LEN);
		m_pressedButton = KEY_DEFAULT;
	}
	break;
	case KEY_WRITEVALUE:
	{
		onWritePressed(schema);
	}
	break;
	case KEY_ERROR:
	{
		onOKInErrorModePressed(schema);
	}
	break;
	case KEY_CANCEL:
	{
		schema->InputBoxData.editText[0] = '\0';
		schema->boolResult = false;
		schema->retPermission = true;
		m_pressedButton = 0;
	}
	break;
	default:
	{
		// cases KEY0 .. KEY_ERASE
		updateValue(schema);
	}
	break;
	}
}

void modeEditProc(struct PopUpSchemaParams* const schema)
{
	if (schema == NULL)
	{
		RN_ASSERT(schema != NULL);
		return;
	}

	if (m_buttonsCount == 0u)
	{
		RN_ASSERT(m_buttonsCount != 0u);
		return;
	}

	// Check the permission
	//
	{
		alt_32 permissionValue = 0;
		if (readSignalValueIntByIndex(schema->controlSignalIndex , &permissionValue) == false)
		{
			char msg[STRING_MAX_LEN];
			(void)snprintf(msg, STRING_MAX_LEN, "modeEditProc: signal by controlSignalIndex = %d not found", schema->controlSignalIndex);
			logWarning(SignalWasNotFound, msg);

			// Abort the dialog
			schema->retPermission = true;
			schema->boolResult = false;
			return;
		}

		if (permissionValue == 0)
		{
			schema->InputBoxData.state = NoPermission;
		}
		else
		{
			if (schema->InputBoxData.state == NoPermission)
			{
				schema->InputBoxData.state = ValueOk;
			}
		}
	}

	bool hasEvents = false;

	if (isPressedKey() == true)
	{
		hasEvents = true;

		alt_u16 code = pressedKey();

		if (schema->PinBoxData.state == PinCodeOk)
		{
			switch (code)
			{
			case VK_RETURN: m_pressedButton = KEY_WRITEVALUE;	break;
			case VK_ESCAPE:	m_pressedButton = KEY_CANCEL;	break;
			case VK_DELETE:	m_pressedButton = KEY_ERASE;	break;
			case '1': case VK_NUMPAD1: m_pressedButton = KEY_CHAR_BASE;		break;
			case '2': case VK_NUMPAD2: m_pressedButton = KEY_CHAR_BASE + 1u;	break;
			case '3': case VK_NUMPAD3: m_pressedButton = KEY_CHAR_BASE + 2u;	break;
			case '4': case VK_NUMPAD4: m_pressedButton = KEY_CHAR_BASE + 3u; break;
			case '5': case VK_NUMPAD5: m_pressedButton = KEY_CHAR_BASE + 4u;	break;
			case '6': case VK_NUMPAD6: m_pressedButton = KEY_CHAR_BASE + 5u;	break;
			case '7': case VK_NUMPAD7: m_pressedButton = KEY_CHAR_BASE + 6u;	break;
			case '8': case VK_NUMPAD8: m_pressedButton = KEY_CHAR_BASE + 7u;	break;
			case '9': case VK_NUMPAD9: m_pressedButton = KEY_CHAR_BASE + 8u;	break;
			case '0': case VK_NUMPAD0: m_pressedButton = KEY_CHAR_BASE + 9u;	break;
			case VK_BACKSPACE: m_pressedButton = KEY_ERASE;			break;
			case VK_DECIMAL: m_pressedButton = KEY_CHAR_PT;			break;
			case VK_SUBTRACT: m_pressedButton = KEY_CHAR_MINUS;			break;
			case VK_ADD: m_pressedButton = KEY_CHAR_PLUS;			break;
			case VK_E: m_pressedButton = KEY_CHAR_E;			break;
			default:	/*Other key*/	break;
			}
		}
		else
		{
			if (code == (alt_u8)VK_RETURN)
			{
				m_pressedButton = KEY_ERROR;
			}
		}
	}

	if (hasEvents == false)
	{
		if (isTouchEvent() == true)
		{
			hasEvents = true;

			if (schema->InputBoxData.state == ValueOk)
			{
				// Check keyboard buttons
				//
				for (alt_u16 i = 0; i < m_buttonsCount; i++)
				{
					if (checkTouchZone(m_buttons[i].x, m_buttons[i].y, m_buttons[i].w, m_buttons[i].h) == true) // Key "1"
					{
						m_pressedButton = KEY_CHAR_BASE + i;
						break;
					}
				}
			}
			else
			{
				// Check only error button
				//
				if (checkTouchZone(m_errorButton.x, m_errorButton.y, m_errorButton.w, m_errorButton.h) == true) // Key "1"
				{
					m_pressedButton = m_errorButton.code;
				}
			}
		}
	}

	if (hasEvents == true)
	{
		if (m_pressedButton != 0u)
		{
			onButtonPressed(schema);
		}
	}
	else
	{
		// No click or pressed key - remove the pressed button
		//
		m_pressedButton = 0;
	}

	return;
}
