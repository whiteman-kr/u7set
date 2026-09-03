/*
Module title: PIN Code Schema Processing
(C) 2026 RPC "Radiy".
All rights reserved.
The information presented here is confidential
and is the intellectual property of PJSC "RPC "Radiy".
Any use of this information is strictly
prohibited without the written consent of PJSC "RPC "Radiy".
Module description: Provides the functionality for PIN code dialog.

-- *************************************************************
-- Version v1.00
-- Date:	31,03,2026.
-- Designed by: Mazyr S.M. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Vintenko B.U.

-- Changes: RADIX-3966, rn_pin initial update

-- *************************************************************
-- Version v1.01
-- Date:	22,04,2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966, https://jira.radiy.com/browse/RADIX-3969
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-3966, rn_pin module refactoring
-- Add keyboard support

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

#include <string.h>
#include <stdio.h>

#include "rn_pin.h"
#include "rn_vbuf.h"
#include "rn_prim.h"
#include "rn_ts.h"
#include "rn_utils.h"
#include "rn_sifp.h"
#include "rn_schema.h"
#include "rn_fofp.h"
#include "rn_diag.h"
#include "rn_kbd.h"

#include <alt_types.h>

#include "defs.h"

#define MAX_PIN_LEN 16u

// Private module data
//
static alt_u16 m_pressedButton = 0;

#define KEY_BUTTON_COUNT 11u
#define KEY_BUTTON_IDENT 60u

#define CODE_NONE		0u
#define KEY_CHAR_BASE	1u
#define KEY_ERASE		11u
#define KEY_OK			12u
#define KEY_CANCEL		13u
#define KEY_ERROR		14u


#define BUTTONS_COUNT 32
static int m_buttonsCount = 0;
static struct Button m_buttons[BUTTONS_COUNT] = { 0 };
static struct Button m_errorButton = { 0 };
//
// End of Private data

static void createButtons(void)
{
	static const char m_keyButton[KEY_BUTTON_COUNT] = { '1', '2', '3',
	   '4', '5', '6',
	   '7', '8', '9',
	   '0',  '<'
	};

	{
		alt_u16 keyButtonX = 315; // Start keyboard "1"
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
		struct Button b = { .x = 580, .y = 310, .w = 120, .h = 40, .code = KEY_OK };
		(void)snprintf(b.text, BUTTON_TEXT_MAX_LEN, "OK");
		m_buttons[m_buttonsCount] = b;
		m_buttonsCount++;
	}

	{
		struct Button b = { .x = 710, .y = 310, .w = 120, .h = 40, .code = KEY_CANCEL };
		(void)snprintf(b.text, BUTTON_TEXT_MAX_LEN, "Cancel");
		m_buttons[m_buttonsCount] = b;
		m_buttonsCount++;
	}

	{
		struct Button b = { .x = 580, .y = 495, .w = 120, .h = 40, .code = KEY_ERROR };
		(void)snprintf(b.text, BUTTON_TEXT_MAX_LEN, "OK");
		m_errorButton = b;
	}
}

static alt_u32 pinCodeHash(void)
{
	return (((alt_u32)readEBB16(ADDR_PIN_HI)) << 16) | readEBB16(ADDR_PIN_LO);
}

void modePinShow(const struct PopUpSchemaParams* const schema)
{
	if (schema == NULL)
	{
		RN_ASSERT(schema != NULL);
		return;
	}

	alt_u32 backColor = 0xf8f8f8;
	alt_u32 backColorRect = 0x888888;
	alt_u32 backColorStr = 0xf8f8f8;

	static const alt_32 blackColor = 0x0;


	static struct Rect m_drawAreaMain = { .x = 440, .y = 200, .w = 400, .h = 160 };
	static struct Rect m_drawRectMain = { .x = 440, .y = 200, .w = 400, .h = 160 };
	static struct Rect m_pinStr = { .x = 450, .y = 235, .w = 100, .h = 25 };

	static struct Rect m_drawAreaKeyBoard = { .x = 305, .y = 615, .w = 670, .h = 75 };
	static struct Rect m_drawRectKeyBoard = { .x = 305, .y = 615, .w = 670, .h = 75 };

	static struct Rect m_drawAreaEditZone = { .x = 450, .y = 265, .w = 380, .h = 25 };
	static struct Rect m_drawRectEditZone = { .x = 450, .y = 265, .w = 380, .h = 25 };
	static struct Rect m_pinStrEditZone = { .x = 450, .y = 265, .w = 380, .h = 25 };

	static struct Rect m_drawAreaIncorrect = { .x = 470, .y = 430, .w = 340, .h = 115 };
	static struct Rect m_drawRectIncorrect = { .x = 470, .y = 430, .w = 340, .h = 115 };
	static struct Rect m_pinStrIncorrect = { .x = 480, .y = 450, .w = 320, .h = 25 };

	// Main background
	//
	alt_u32 backColorArea = 0xdcdcdc;
	drawArea(m_drawAreaMain.x, m_drawAreaMain.y, m_drawAreaMain.w, m_drawAreaMain.h, backColorArea);
	drawRect(m_drawRectMain.x, m_drawRectMain.y, m_drawRectMain.w, m_drawRectMain.h, 1u, backColorRect);
	printStr(m_pinStr.x, m_pinStr.y, m_pinStr.w, m_pinStr.h,
			RVDU_SYSTEM_FONT, blackColor, backColor, ALIGN_V_CENTER + ALIGN_H_LEFT, "PIN Code:" );

	// Keyboard background
	//
	drawArea(m_drawAreaKeyBoard.x, m_drawAreaKeyBoard.y, m_drawAreaKeyBoard.w, m_drawAreaKeyBoard.h, backColorArea);
	drawRect(m_drawRectKeyBoard.x, m_drawRectKeyBoard.y, m_drawRectKeyBoard.w, m_drawRectKeyBoard.h, 1u, backColorRect);

	char putStr[STRING_MAX_LEN];

	// Edit zone
	backColorArea = 0xf8f8f8;
	drawArea(m_drawAreaEditZone.x, m_drawAreaEditZone.y, m_drawAreaEditZone.w, m_drawAreaEditZone.h, backColorArea);
	drawRect(m_drawRectEditZone.x, m_drawRectEditZone.y, m_drawRectEditZone.w, m_drawRectEditZone.h, 1u, blackColor);
	(void)strncpy(putStr, (schema->PinBoxData.editText), STRING_MAX_LEN);
	for (alt_16 i = (alt_16)strlen((schema->PinBoxData.editText)) -2 ; i >= 0; i--)
	{
		putStr[i] = '*';
	}
	printStr(m_pinStrEditZone.x, m_pinStrEditZone.y, m_pinStrEditZone.w, m_pinStrEditZone.h,
			RVDU_SYSTEM_FONT, blackColor, backColorStr, ALIGN_V_CENTER + ALIGN_H_CENTER, putStr );

	if (schema->PinBoxData.state == PinCodeOk)
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
		// Draw incorrect pin area
		//
		backColorArea = 0xdcdcdc;
		backColorRect = 0x888888;

		
		drawArea(m_drawAreaIncorrect.x, m_drawAreaIncorrect.y, m_drawAreaIncorrect.w, m_drawAreaIncorrect.h, backColorArea);
		drawRect(m_drawRectIncorrect.x, m_drawRectIncorrect.y, m_drawRectIncorrect.w, m_drawRectIncorrect.h, 1u, backColorRect);


		switch (schema->PinBoxData.state)
		{
		case PinCodeEmpty:
		{
			printStr(m_pinStrIncorrect.x, m_pinStrIncorrect.y, m_pinStrIncorrect.w, m_pinStrIncorrect.h,
				RVDU_SYSTEM_FONT, blackColor, backColor, ALIGN_V_CENTER + ALIGN_H_CENTER,
				"PIN Code can not be empty");
		}
		break;
		case PinCodeWrong:
		{
			if (schema->PinBoxData.tryCount < 2)
			{
				printStr(m_pinStrIncorrect.x, m_pinStrIncorrect.y, m_pinStrIncorrect.w, m_pinStrIncorrect.h,
					RVDU_SYSTEM_FONT, blackColor, backColor, ALIGN_V_CENTER + ALIGN_H_CENTER,
					"Wrong PIN Code! Please try again");
			}
			else
			{
				printStr(m_pinStrIncorrect.x, m_pinStrIncorrect.y, m_pinStrIncorrect.w, m_pinStrIncorrect.h,
					RVDU_SYSTEM_FONT, blackColor, backColor, ALIGN_V_CENTER + ALIGN_H_CENTER,
					"Wrong PIN Code!");
			}
		}
		break;
		case PinNoPermission:
		{
			printStr(m_pinStrIncorrect.x, m_pinStrIncorrect.y, m_pinStrIncorrect.w, m_pinStrIncorrect.h,
				RVDU_SYSTEM_FONT, blackColor, backColor, ALIGN_V_CENTER + ALIGN_H_CENTER,
				"Control permission has been disabled");
		}
		break;
		default:
		{
			RN_ASSERT(false);
		}
		break;
		}

		// Draw incorrect pin button
		//
		backColorArea = 0xc0c0c0;
		backColorRect = blackColor;
		
		drawArea(m_errorButton.x, m_errorButton.y, m_errorButton.w, m_errorButton.h, backColorArea);
		drawRect(m_errorButton.x, m_errorButton.y, m_errorButton.w, m_errorButton.h, 1u, backColorRect);
		printStr(m_errorButton.x, m_errorButton.y, m_errorButton.w, m_errorButton.h,
				RVDU_SYSTEM_FONT, blackColor, backColor, ALIGN_V_CENTER + ALIGN_H_CENTER, "OK" );
	}

	alt_u32 keysColor = 0x505050;
	if (schema->PinBoxData.state == PinCodeOk)
	{
		keysColor = blackColor;
	}

	// Initialize all buttons
	//
	if (m_buttonsCount == 0)
	{
		createButtons();
	}

	// Draw all buttons
	//

	backColorArea = 0xc0c0c0;
	for (int i = 0; i < m_buttonsCount; i++)
	{
		drawArea(m_buttons[i].x, m_buttons[i].y, m_buttons[i].w, m_buttons[i].h, backColorArea);

		drawRect(m_buttons[i].x, m_buttons[i].y, m_buttons[i].w, m_buttons[i].h,
			(m_pressedButton == m_buttons[i].code) ? (2u) : (1u), keysColor);

		printStr(m_buttons[i].x, m_buttons[i].y, m_buttons[i].w, m_buttons[i].h,
			RVDU_SYSTEM_FONT, keysColor, backColorStr, ALIGN_V_CENTER + ALIGN_H_CENTER, m_buttons[i].text);
	}

	return;

}

static void onOKPressed(struct PopUpSchemaParams* const schema)
{
	alt_u32 enteredPinHash = calcHash32((alt_u8*)(schema->PinBoxData.editText), 
		(alt_u8)strlen(schema->PinBoxData.editText));

	if (strlen(schema->PinBoxData.editText) == 0u)
	{
		schema->PinBoxData.state = PinCodeEmpty;
	}
	else if (pinCodeHash() == enteredPinHash)
	{
		schema->boolResult = true;
		popUpSchemaSetTerminate();
		schema->PinBoxData.editText[0] = '\0';
		schema->PinBoxData.tryCount = 0;
		schema->PinBoxData.state = PinCodeOk;
	}
	else
	{
		schema->PinBoxData.state = PinCodeWrong;
	}

	m_pressedButton = 0;
	return;
}

static void onOKInErrorModePressed(struct PopUpSchemaParams* const schema)
{
	if (schema->PinBoxData.state == PinNoPermission)
	{
		popUpSchemaSetTerminate();
		schema->boolResult = false;
		(schema->PinBoxData.editText)[0] = '\0';
	}
	else
	{
		schema->PinBoxData.state = PinCodeOk;
		schema->PinBoxData.tryCount++;
		(schema->PinBoxData.editText)[0] = '\0';
		
		if (schema->PinBoxData.tryCount >= 3)
		{
			popUpSchemaSetTerminate();
			schema->boolResult = false;
		}
	}
	m_pressedButton = 0;
	return;
}

static void updatePin(struct PopUpSchemaParams* const schema)
{
	char* dest = (char*)(schema->PinBoxData.editText);

	// Make pointer to last symbol
	//
	while (*dest != 0)
	{
		dest++;
	}

	if (m_pressedButton == KEY_ERASE)
	{
		if (strlen(schema->PinBoxData.editText) != 0u)
		{
			dest--;
			*dest = '\0';
		}
		return;
	}
	else
	{
		if (strlen(schema->PinBoxData.editText) < MAX_PIN_LEN)
		{
			// Check keys "0..9"
			//
			*dest = m_buttons[m_pressedButton - KEY_CHAR_BASE].text[0];
			dest++;

			*dest = '\0';
		}
	}
}

static void onButtonPressed(struct PopUpSchemaParams* const schema)
{
	switch (m_pressedButton)
	{
	case KEY_OK:
	{
		onOKPressed(schema);
	}
	break;
	case KEY_ERROR:
	{
		onOKInErrorModePressed(schema);
	}
	break;
	case KEY_CANCEL:
	{
		schema->boolResult = false; 
		popUpSchemaSetTerminate();
		schema->PinBoxData.editText[0] = '\0'; 
		schema->PinBoxData.tryCount = 0;
		schema->PinBoxData.state = PinCodeOk;
		m_pressedButton = 0;
	}
	break;
	default:
	{
		// cases KEY0 .. KEY_ERASE
		updatePin(schema);
	}
	break;
	}
}

void modePinProc(struct PopUpSchemaParams* const schema)
{
	if (schema == NULL)
	{
		RN_ASSERT(schema != NULL);
		return;
	}

	if (m_buttonsCount == 0)
	{
		RN_ASSERT(m_buttonsCount != 0);
		return;
	}
	
	{
		alt_32 permissionValue = 0;
		if (readSignalValueIntByIndex(schema->controlSignalIndex, &permissionValue) == false)
		{
			char msg[STRING_MAX_LEN];
			(void)snprintf(msg, STRING_MAX_LEN, "modePinProc: signal by controlSignalIndex = %d not found", schema->controlSignalIndex);
			logWarning(SignalWasNotFound, msg);

			// Abort the dialog
			popUpSchemaSetTerminate();
			schema->boolResult = false;
			return;
		}

		if (permissionValue == 0)
		{
			schema->PinBoxData.state = PinNoPermission;
		}
		else
		{
			if (schema->PinBoxData.state == PinNoPermission)
			{
				schema->PinBoxData.state = PinCodeOk;
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
			case RK_RETURN: m_pressedButton = KEY_OK;		break;
			case RK_ESCAPE:	m_pressedButton = KEY_CANCEL;	break;
			case RK_DELETE:	m_pressedButton = KEY_ERASE;	break;
			case '1': case RK_NUMPAD1: m_pressedButton = KEY_CHAR_BASE;		break;
			case '2': case RK_NUMPAD2: m_pressedButton = KEY_CHAR_BASE + 1u;	break;
			case '3': case RK_NUMPAD3: m_pressedButton = KEY_CHAR_BASE + 2u;	break;
			case '4': case RK_NUMPAD4: m_pressedButton = KEY_CHAR_BASE + 3u; break;
			case '5': case RK_NUMPAD5: m_pressedButton = KEY_CHAR_BASE + 4u;	break;
			case '6': case RK_NUMPAD6: m_pressedButton = KEY_CHAR_BASE + 5u;	break;
			case '7': case RK_NUMPAD7: m_pressedButton = KEY_CHAR_BASE + 6u;	break;
			case '8': case RK_NUMPAD8: m_pressedButton = KEY_CHAR_BASE + 7u;	break;
			case '9': case RK_NUMPAD9: m_pressedButton = KEY_CHAR_BASE + 8u;	break;
			case '0': case RK_NUMPAD0: m_pressedButton = KEY_CHAR_BASE + 9u;	break;
			case RK_BACKSPACE: m_pressedButton = KEY_ERASE;			break;
			default:
			{
				// Other key
			}
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

			if (schema->PinBoxData.state == PinCodeOk)
			{
				// Check keyboard buttons
				//
				for (int i = 0; i < m_buttonsCount; i++)
				{
					if (checkTouchZone(m_buttons[i].x, m_buttons[i].y, m_buttons[i].w, m_buttons[i].h) == true) // Key "1"
					{
						m_pressedButton = m_buttons[i].code;
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

	if(hasEvents == true)
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



