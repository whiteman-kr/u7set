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
-- Date:	31.03.2026.
-- Designed by: Mazyr S.M. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Vintenko B.U.
-- Changes: RADIX-3966, rn_ainput initial update

-- *************************************************************
-- Version v1.01
-- Date:	01.05.2026.
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
-- Date:	02.10.2026.
-- Designed by: Nesterenko M.V. https://jira.radiy.com/browse/RADIX-4116
-- Reviewed by: Vintenko B.U.

-- Changes: RADIX-4161, rn_ainput сorrecting errors identified by static analysis testing

-- *************************************************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>

#include "rn_ainput.h"
#include "rn_diag.h"
#include "rn_fofp.h"
#include "rn_kbd.h"
#include "rn_prim.h"
#include "rn_schema.h"
#include "rn_sifp.h"
#include "rn_ts.h"
#include "rn_utils.h"
#include "rn_vbuf.h"

#include "defs.h"


// Private module data
//
static alt_u16 m_pressedButton = 0u;

static char m_defaultValueStr[STRING_MAX_LEN];

#define MAX_INPUT_LEN 16u

#define KEY_BUTTON_COUNT 15u
#define KEY_BUTTON_IDENT 60u

#define KEY_CHAR_BASE 1u

#define KEY_CHAR_PT 11u
#define KEY_CHAR_MINUS 12u
#define KEY_CHAR_PLUS 13u
#define KEY_CHAR_E 14u

#define KEY_ERASE 15u
#define KEY_DEFAULT 16u
#define KEY_WRITEVALUE 17u
#define KEY_CANCEL 18u
#define KEY_ERROR 19u

#define BUTTONS_COUNT 32u
static alt_u16 m_buttonsCount = 0u;
static struct Button m_buttons[BUTTONS_COUNT] = {0};
static struct Button m_errorButton = {0};

static const alt_u16 m_mainBorderWeight = 1u;
static const alt_u16 m_buttonBorderWeightPressed = 2u;

static const char m_keyButton[KEY_BUTTON_COUNT] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '.', '-', '+', 'E', '<'};
//
// End of Private data

// This function create buttons on schema
//
static void createButtons(void)
{
	{
		alt_u16 keyButtonX = 195u;                                                                      // Position on x-axis for key button
		for (alt_u16 i = 0; i < KEY_BUTTON_COUNT; i++)
		{
			struct Button b = {.x = keyButtonX, .y = 625, .w = 50, .h = 55, .code = KEY_CHAR_BASE + i}; // Main buttons position on schema
			b.text[0] = m_keyButton[i];
			b.text[1] = '\0';

			m_buttons[m_buttonsCount] = b;
			m_buttonsCount++;

			keyButtonX += KEY_BUTTON_IDENT;
		}
	}

	{
		struct Button b = {.x = 350, .y = 375, .w = 170, .h = 40, .code = KEY_DEFAULT};    // Button (Dafault) position on schema
		m_buttons[m_buttonsCount] = b;
		m_buttonsCount++;
	}

	{
		struct Button b = {.x = 680, .y = 375, .w = 120, .h = 40, .code = KEY_WRITEVALUE}; // Button (Write) position on schema
		(void)snprintf(b.text, BUTTON_TEXT_MAX_LEN, "Write");
		m_buttons[m_buttonsCount] = b;
		m_buttonsCount++;
	}

	{
		struct Button b = {.x = 810, .y = 375, .w = 120, .h = 40, .code = KEY_CANCEL}; // Button (Cancel) position on schema
		(void)snprintf(b.text, BUTTON_TEXT_MAX_LEN, "Cancel");
		m_buttons[m_buttonsCount] = b;
		m_buttonsCount++;
	}

	{
		struct Button b = {.x = 580, .y = 530, .w = 120, .h = 40, .code = KEY_ERROR}; // On error button (OK) position on schema
		(void)snprintf(b.text, BUTTON_TEXT_MAX_LEN, "OK");
		m_errorButton = b;
	}
}

// This function open Edit schema dialog.
//
void modeEditShow(struct PopUpSchemaParams* const schema)
{
	static struct Rect m_drawArea = {.x = 340, .y = 135, .w = 600, .h = 290};           // Main area position on schema
	static struct Rect m_drawRect = {.x = 340, .y = 135, .w = 600, .h = 290};           // Main rect position on schema
	static struct Rect m_signalStr = {.x = 350, .y = 145, .w = 120, .h = 25};           // Signal label position on schema

	static struct Rect m_drawRectSignalLimit = {.x = 480, .y = 205, .w = 450, .h = 25}; // Signal value limit rect layout position on schema
	static struct Rect m_drawRectCurrentValue = {.x = 480, .y = 275, .w = 320, .h = 25}; // Signal current value rect layout position on schema

	static struct Rect m_captionStr = {.x = 350, .y = 175, .w = 120, .h = 25};          // Signal caption label position on schema
	static struct Rect m_limitStr = {.x = 350, .y = 205, .w = 120, .h = 25};            // Signal limit label layout position on schema
	static struct Rect m_currentValueStr = {.x = 350, .y = 275, .w = 120, .h = 25}; // Signal current value label layout position on schema

	static struct Rect m_customAppSignalIdStr = {.x = 480, .y = 145, .w = 450, .h = 25}; // Custom signal Id position on schema
	static struct Rect m_signalCaptionStr = {.x = 480, .y = 175, .w = 450, .h = 25};     // Signal caption position on schema
	static struct Rect m_signalUnitStr = {.x = 820, .y = 305, .w = 110, .h = 25};        // Signal unit label position on schema

	static struct Rect m_rangeStr = {.x = 482, .y = 205, .w = 450, .h = 25};             // Signal range position on schema
	static struct Rect m_valueStr = {.x = 480, .y = 275, .w = 320, .h = 25};             // Signal value position on schema

	static struct Rect m_defaultStr = {.x = 350, .y = 375, .w = 170, .h = 40};           // Default signal value position on schema
	static struct Rect m_editTextStr = {.x = 480, .y = 305, .w = 340, .h = 25};          // Edit zone position on schema

	static struct Rect m_drawAreaButtonLayout = {.x = 185, .y = 615, .w = 910, .h = 75}; // Buttons area position on schema
	static struct Rect m_drawRectButtonLayout = {.x = 185, .y = 615, .w = 910, .h = 75}; // Buttons rect position on schema

	static struct Rect m_drawAreaIncorrect = {.x = 470, .y = 460, .w = 340, .h = 120};   // Incorrect area position on schema
	static struct Rect m_drawRectIncorrect = {.x = 470, .y = 460, .w = 340, .h = 120};   // Incorrect rect position on schema

	static struct Rect m_editStrIncorrect = {.x = 475, .y = 465, .w = 320, .h = 25};     // Incorrect text position on schema

	if (schema == NULL)
	{
		RN_ASSERT(schema != NULL);
		return;
	}

	static const alt_u32 whiteColor = 0xf8f8f8;
	static const alt_u32 greyColor = 0x888888;
	static const alt_u32 lightGreyColor = 0xdcdcdc;
	static const alt_u32 darkGreyColor = 0x808080;
	static const alt_u32 silverColor = 0xc0c0c0;

	alt_u32 backColorRect = greyColor;
	alt_u32 backColorStr = whiteColor;

	alt_32 blackColor = 0x000000;

	alt_u32 backColorArea = lightGreyColor;

	drawArea(m_drawArea.x, m_drawArea.y, m_drawArea.w, m_drawArea.h, backColorArea);
	drawRect(m_drawRect.x, m_drawRect.y, m_drawRect.w, m_drawRect.h, m_mainBorderWeight, backColorRect);

	backColorStr = lightGreyColor;

	printStr(m_signalStr.x,
			 m_signalStr.y,
			 m_signalStr.w,
			 m_signalStr.h,
			 RVDU_SYSTEM_FONT,
			 blackColor,
			 backColorStr,
			 ALIGN_V_CENTER + ALIGN_H_LEFT,
			 "Signal");

	printStr(m_captionStr.x,
			 m_captionStr.y,
			 m_captionStr.w,
			 m_captionStr.h,
			 RVDU_SYSTEM_FONT,
			 blackColor,
			 backColorStr,
			 ALIGN_V_CENTER + ALIGN_H_LEFT,
			 "Caption");

	printStr(m_limitStr.x,
			 m_limitStr.y,
			 m_limitStr.w,
			 m_limitStr.h,
			 RVDU_SYSTEM_FONT,
			 blackColor,
			 backColorStr,
			 ALIGN_V_CENTER + ALIGN_H_LEFT,
			 "Limit");

	printStr(m_currentValueStr.x,
			 m_currentValueStr.y,
			 m_currentValueStr.w,
			 m_currentValueStr.h,
			 RVDU_SYSTEM_FONT,
			 blackColor,
			 backColorStr,
			 ALIGN_V_CENTER + ALIGN_H_LEFT,
			 "Current Value");

	drawRect(m_customAppSignalIdStr.x,
			 m_customAppSignalIdStr.y,
			 m_customAppSignalIdStr.w,
			 m_customAppSignalIdStr.h,
			 m_mainBorderWeight,
			 backColorRect);

	static const alt_u16 textIdent = 2u;

	printStr(m_customAppSignalIdStr.x + textIdent,
			 m_customAppSignalIdStr.y,
			 m_customAppSignalIdStr.w,
			 m_customAppSignalIdStr.h,
			 RVDU_SYSTEM_FONT,
			 blackColor,
			 backColorStr,
			 ALIGN_V_CENTER + ALIGN_H_LEFT,
			 customAppSignalID(schema->tuningSignalIndex));

	drawRect(m_signalCaptionStr.x, m_signalCaptionStr.y, m_signalCaptionStr.w, m_signalCaptionStr.h, m_mainBorderWeight, backColorRect);
	printStr(m_signalCaptionStr.x + textIdent,
			 m_signalCaptionStr.y,
			 m_signalCaptionStr.w,
			 m_signalCaptionStr.h,
			 RVDU_SYSTEM_FONT,
			 blackColor,
			 backColorStr,
			 ALIGN_V_CENTER + ALIGN_H_LEFT,
			 signalCaption(schema->tuningSignalIndex));

	// Signal limits layout
	//
	drawRect(m_drawRectSignalLimit.x,
			 m_drawRectSignalLimit.y,
			 m_drawRectSignalLimit.w,
			 m_drawRectSignalLimit.h,
			 m_mainBorderWeight,
			 backColorRect);

	// Current value layout
	//
	drawRect(m_drawRectCurrentValue.x,
			 m_drawRectCurrentValue.y,
			 m_drawRectCurrentValue.w,
			 m_drawRectCurrentValue.h,
			 m_mainBorderWeight,
			 backColorRect);

	// Value layout
	//
	drawRect(m_signalUnitStr.x, m_signalUnitStr.y, m_signalUnitStr.w, m_signalUnitStr.h, m_mainBorderWeight, backColorRect);
	printStr(m_signalUnitStr.x,
			 m_signalUnitStr.y,
			 m_signalUnitStr.w,
			 m_signalUnitStr.h,
			 RVDU_SYSTEM_FONT,
			 blackColor,
			 backColorStr,
			 ALIGN_V_CENTER + ALIGN_H_CENTER,
			 signalUnit(schema->tuningSignalIndex));

	alt_u32 keysColor = darkGreyColor;
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
		popUpSchemaSetTerminate();
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
		popUpSchemaSetTerminate();
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
		popUpSchemaSetTerminate();
		schema->boolResult = false;
		return;
	}

	// Edit
	//
	backColorArea = whiteColor;
	backColorStr = whiteColor;
	drawArea(m_editTextStr.x, m_editTextStr.y, m_editTextStr.w, m_editTextStr.h, backColorArea);
	drawRect(m_editTextStr.x, m_editTextStr.y, m_editTextStr.w, m_editTextStr.h, m_mainBorderWeight, blackColor);
	printStr(m_editTextStr.x,
			 m_editTextStr.y,
			 m_editTextStr.w,
			 m_editTextStr.h,
			 RVDU_SYSTEM_FONT,
			 blackColor,
			 backColorStr,
			 ALIGN_V_CENTER + ALIGN_H_CENTER,
			 (schema->InputBoxData.editText));

	// Draw buttons area
	//
	backColorArea = lightGreyColor;
	backColorRect = greyColor;
	drawArea(m_drawAreaButtonLayout.x, m_drawAreaButtonLayout.y, m_drawAreaButtonLayout.w, m_drawAreaButtonLayout.h, backColorArea);
	drawRect(m_drawRectButtonLayout.x,
			 m_drawRectButtonLayout.y,
			 m_drawRectButtonLayout.w,
			 m_drawRectButtonLayout.h,
			 m_mainBorderWeight,
			 backColorRect);

	// Draw buttons
	//
	backColorStr = silverColor;

	if (schema->InputBoxData.state == ValueOk)
	{
		// Erase incorrect edit area
		//
		backColorArea = whiteColor;
		backColorRect = whiteColor;
		drawArea(m_drawAreaIncorrect.x, m_drawAreaIncorrect.y, m_drawAreaIncorrect.w, m_drawAreaIncorrect.h, backColorArea);
		drawRect(m_drawRectIncorrect.x,
				 m_drawRectIncorrect.y,
				 m_drawRectIncorrect.w,
				 m_drawRectIncorrect.h,
				 m_mainBorderWeight,
				 backColorRect);
	}
	else
	{
		// Draw incorrect value area
		//
		backColorArea = lightGreyColor;
		backColorRect = greyColor;
		backColorStr = lightGreyColor;

		drawArea(m_drawAreaIncorrect.x, m_drawAreaIncorrect.y, m_drawAreaIncorrect.w, m_drawAreaIncorrect.h, backColorArea);
		drawRect(m_drawRectIncorrect.x,
				 m_drawRectIncorrect.y,
				 m_drawRectIncorrect.w,
				 m_drawRectIncorrect.h,
				 m_mainBorderWeight,
				 backColorRect);

		static const alt_u16 incorrectTextIdent = 30u;
		printStr(m_editStrIncorrect.x,
				 m_editStrIncorrect.y,
				 m_editStrIncorrect.w,
				 m_editStrIncorrect.h,
				 RVDU_SYSTEM_FONT,
				 blackColor,
				 backColorStr,
				 ALIGN_V_CENTER + ALIGN_H_CENTER,
				 "ERROR");
		printStr(m_editStrIncorrect.x,
				 m_editStrIncorrect.y + incorrectTextIdent,
				 m_editStrIncorrect.w,
				 m_editStrIncorrect.h,
				 RVDU_SYSTEM_FONT,
				 blackColor,
				 backColorStr,
				 ALIGN_V_CENTER + ALIGN_H_CENTER,
				 (schema->InputBoxData.state == IncorrectValue) ? ("The value is incorrect") :
				 (schema->InputBoxData.state == OutOfRange)     ? ("The value is out of range") :
				 (schema->InputBoxData.state == NoPermission)   ? ("Control permission has been disabled") :
																  ("Internal Exception"));

		backColorArea = silverColor;
		backColorRect = blackColor;
		backColorStr = silverColor;


		drawArea(m_errorButton.x, m_errorButton.y, m_errorButton.w, m_errorButton.h, backColorArea);
		drawRect(m_errorButton.x, m_errorButton.y, m_errorButton.w, m_errorButton.h, m_mainBorderWeight, backColorRect);
		printStr(m_errorButton.x,
				 m_errorButton.y,
				 m_errorButton.w,
				 m_errorButton.h,
				 RVDU_SYSTEM_FONT,
				 blackColor,
				 backColorStr,
				 ALIGN_V_CENTER + ALIGN_H_CENTER,
				 "OK");
	}

	// Initialize all buttons
	//
	if (m_buttonsCount == 0u)
	{
		createButtons();
	}

	// Draw all buttons
	//
	backColorArea = silverColor;
	for (alt_u16 i = 0; i < m_buttonsCount; i++)
	{
		drawArea(m_buttons[i].x, m_buttons[i].y, m_buttons[i].w, m_buttons[i].h, backColorArea);

		drawRect(m_buttons[i].x,
				 m_buttons[i].y,
				 m_buttons[i].w,
				 m_buttons[i].h,
				 (m_pressedButton == m_buttons[i].code) ? (m_buttonBorderWeightPressed) : (m_mainBorderWeight),
				 keysColor);

		printStr(m_buttons[i].x,
				 m_buttons[i].y,
				 m_buttons[i].w,
				 m_buttons[i].h,
				 RVDU_SYSTEM_FONT,
				 keysColor,
				 backColorStr,
				 ALIGN_V_CENTER + ALIGN_H_CENTER,
				 m_buttons[i].text);
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

		(void)snprintf(putStr,
					   STRING_MAX_LEN,
					   float32Format,
					   param->lowLimit.floatValue,
					   param->highLimit.floatValue,
					   signalUnit(schema->tuningSignalIndex));

		printStr(m_rangeStr.x,
				 m_rangeStr.y,
				 m_rangeStr.w,
				 m_rangeStr.h,
				 RVDU_SYSTEM_FONT,
				 blackColor,
				 backColorStr,
				 ALIGN_V_CENTER + ALIGN_H_LEFT,
				 putStr);

		// Value
		//
		(void)snprintf(putStr, STRING_MAX_LEN, float32Len, state->value.floatValue);

		printStr(m_valueStr.x,
				 m_valueStr.y,
				 m_valueStr.w,
				 m_valueStr.h,
				 RVDU_SYSTEM_FONT,
				 blackColor,
				 backColorStr,
				 ALIGN_V_CENTER + ALIGN_H_CENTER,
				 putStr);

		// Default
		//
		(void)snprintf(m_defaultValueStr, STRING_MAX_LEN, float32Len, param->defaultValue.floatValue);
		(void)snprintf(putStr, STRING_MAX_LEN, "Default: %s", m_defaultValueStr);

		printStr(m_defaultStr.x,
				 m_defaultStr.y,
				 m_defaultStr.w,
				 m_defaultStr.h,
				 RVDU_SYSTEM_FONT,
				 keysColor,
				 backColorStr,
				 ALIGN_V_CENTER + ALIGN_H_CENTER,
				 putStr);
	}
	else
	{
		char putStr[STRING_MAX_LEN];

		// Range
		//
		(void)snprintf(putStr,
					   STRING_MAX_LEN,
					   "%d .. %d %s",
					   (int)param->lowLimit.intValue,
					   (int)param->highLimit.intValue,
					   signalUnit(schema->tuningSignalIndex));

		printStr(m_rangeStr.x,
				 m_rangeStr.y,
				 m_rangeStr.w,
				 m_rangeStr.h,
				 RVDU_SYSTEM_FONT,
				 blackColor,
				 backColorStr,
				 ALIGN_V_CENTER + ALIGN_H_LEFT,
				 putStr);

		// Value
		//
		(void)snprintf(putStr, STRING_MAX_LEN, "%d", (int)state->value.intValue);

		printStr(m_valueStr.x,
				 m_valueStr.y,
				 m_valueStr.w,
				 m_valueStr.h,
				 RVDU_SYSTEM_FONT,
				 blackColor,
				 backColorStr,
				 ALIGN_V_CENTER + ALIGN_H_CENTER,
				 putStr);

		// Default
		//
		backColorStr = silverColor;
		(void)snprintf(m_defaultValueStr, STRING_MAX_LEN, "%d", (int)param->defaultValue.intValue);
		(void)snprintf(putStr, STRING_MAX_LEN, "Default: %s", m_defaultValueStr);

		printStr(m_defaultStr.x,
				 m_defaultStr.y,
				 m_defaultStr.w,
				 m_defaultStr.h,
				 RVDU_SYSTEM_FONT,
				 keysColor,
				 backColorStr,
				 ALIGN_V_CENTER + ALIGN_H_CENTER,
				 putStr);
	}

	return;
}

// Function that procces Write button pressed
//
static void onWritePressed(struct PopUpSchemaParams* const schema)
{
	const struct VduAppSignalParam* const param = appSignalParamByIndex(schema->tuningSignalIndex);
	if (param == NULL)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "onWritePressed: signal by index = %d not found", schema->tuningSignalIndex);
		logWarning(SignalWasNotFound, msg);

		// Abort the dialog
		popUpSchemaSetTerminate();
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
		if ((strlen(schema->InputBoxData.editText) == 0u) || (0 != errno) || (*lastChar != '\0'))
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
			popUpSchemaSetTerminate();
			m_pressedButton = 0;

			schema->boolResult = true;

			// Write the signal
			//
			if (writeSignalValueFloatByIndex(schema->tuningSignalIndex, nv) == false)
			{
				char msg[STRING_MAX_LEN];
				(void)snprintf(msg,
							   STRING_MAX_LEN,
							   "onWritePressed: signal by tuningSignalIndex = %d, wrong signal type",
							   schema->tuningSignalIndex);
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
		alt_32 nv = strtol(schema->InputBoxData.editText, &lastChar, 10); // convert a string to a integer with decimal (10-base)
		if ((strlen(schema->InputBoxData.editText) == 0u) || (errno != 0) || (*lastChar != '\0'))
		{
			// Invalid number format
			//
			schema->InputBoxData.state = IncorrectValue;
		}
		else if ((nv < param->lowLimit.intValue) || (nv > param->highLimit.intValue))
		{
			// Number is out of range
			//
			schema->InputBoxData.state = OutOfRange;
		}
		else
		{
			// Close the dialog
			//
			popUpSchemaSetTerminate();
			m_pressedButton = 0;

			schema->boolResult = true;

			// Write the signal
			//
			if (writeSignalValueIntByIndex(schema->tuningSignalIndex, nv) == false)
			{
				char msg[STRING_MAX_LEN];
				(void)snprintf(msg, STRING_MAX_LEN, "onWritePressed: signal by index = %d, wrong signal type", schema->tuningSignalIndex);
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

// Function that updates the Edit value entered
//
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

// Function that procces on error OK button pressed
//
static void onOKInErrorModePressed(struct PopUpSchemaParams* const schema)
{
	// If there is no permission, we can press only "OK" button
	//
	if (schema->InputBoxData.state == NoPermission)
	{
		popUpSchemaSetTerminate();
	}
	else
	{
		schema->InputBoxData.state = ValueOk;
	}
	m_pressedButton = 0;
	return;
}

// Function that procces button pressed
//
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
			popUpSchemaSetTerminate();
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

// This function process all Edit schema dialog event.
//
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
		if (readSignalValueIntByIndex(schema->controlSignalIndex, &permissionValue) == false)
		{
			char msg[STRING_MAX_LEN];
			(void)snprintf(msg, STRING_MAX_LEN, "modeEditProc: signal by controlSignalIndex = %d not found", schema->controlSignalIndex);
			logWarning(SignalWasNotFound, msg);

			// Abort the dialog
			popUpSchemaSetTerminate();
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

		alt_u32 code = pressedKey();

		if (schema->InputBoxData.state == ValueOk)
		{
			switch (code)
			{
			case RK_RETURN:
				m_pressedButton = KEY_WRITEVALUE;
				break;
			case RK_ESCAPE:
				m_pressedButton = KEY_CANCEL;
				break;
			case RK_DELETE:
				m_pressedButton = KEY_ERASE;
				break;
			case RK_1:
			case RK_NUMPAD1:
				m_pressedButton = KEY_CHAR_BASE;
				break;
			case RK_2:
			case RK_NUMPAD2:
				m_pressedButton = KEY_CHAR_BASE + 1u;
				break;
			case RK_3:
			case RK_NUMPAD3:
				m_pressedButton = KEY_CHAR_BASE + 2u;
				break;
			case RK_4:
			case RK_NUMPAD4:
				m_pressedButton = KEY_CHAR_BASE + 3u;
				break;
			case RK_5:
			case RK_NUMPAD5:
				m_pressedButton = KEY_CHAR_BASE + 4u;
				break;
			case RK_6:
			case RK_NUMPAD6:
				m_pressedButton = KEY_CHAR_BASE + 5u;
				break;
			case RK_7:
			case RK_NUMPAD7:
				m_pressedButton = KEY_CHAR_BASE + 6u;
				break;
			case RK_8:
			case RK_NUMPAD8:
				m_pressedButton = KEY_CHAR_BASE + 7u;
				break;
			case RK_9:
			case RK_NUMPAD9:
				m_pressedButton = KEY_CHAR_BASE + 8u;
				break;
			case RK_0:
			case RK_NUMPAD0:
				m_pressedButton = KEY_CHAR_BASE + 9u;
				break;
			case RK_BACKSPACE:
				m_pressedButton = KEY_ERASE;
				break;
			case RK_DECIMAL:
				m_pressedButton = KEY_CHAR_PT;
				break;
			case RK_SUBTRACT:
				m_pressedButton = KEY_CHAR_MINUS;
				break;
			case RK_ADD:
				m_pressedButton = KEY_CHAR_PLUS;
				break;
			case RK_E:
				m_pressedButton = KEY_CHAR_E;
				break;
			default: /*Other key*/
				break;
			}
		}
		else
		{
			if (code == RK_RETURN)
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
					if (checkTouchZone(m_buttons[i].x, m_buttons[i].y, m_buttons[i].w, m_buttons[i].h) == true)
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
				if (checkTouchZone(m_errorButton.x, m_errorButton.y, m_errorButton.w, m_errorButton.h) == true)
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
