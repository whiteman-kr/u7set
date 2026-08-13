/*
Module title: Message Box Schema Processing
(C) 2026 RPC "Radiy".
All rights reserved.
The information presented here is confidential
and is the intellectual property of PJSC "RPC "Radiy".
Any use of this information is strictly
prohibited without the written consent of PJSC "RPC "Radiy".
Module description: Provides the functionality for displaying confirmation message boxes.

-- *************************************************************
-- Version v1.00
-- Date:	31,03,2026.
-- Designed by: Mazyr S.M. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Vintenko B.U.
-- Changes: RADIX-3966, rn_msgbox initial update

-- *************************************************************
-- Version v1.01
-- Date:	12,04,2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966, https://jira.radiy.com/browse/RADIX-3969
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-3966, rn_msgbox module refactoring
-- Add keyboard support

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

// Microsoft Visual Studio-specific
#ifndef RVDU_SIM

#include <alt_types.h>

#else

#include "msvc_alt_types.h"

#endif
// Microsoft Visual Studio-specific

#include <stdio.h>
#include <string.h>

#include "rn_msgbox.h"
#include "rn_vbuf.h"
#include "rn_prim.h"
#include "rn_ts.h"
#include "rn_utils.h"
#include "rn_schema.h"
#include "rn_fofp.h"
#include "rn_diag.h"
#include "rn_cfg.h"
#include "rn_kbd.h"

#include "defs.h"


// Private module data
//


#define KEY_YES	1u
#define KEY_NO	2u

#define BUTTONS_COUNT 2u
static int m_buttonsCount = 0;
static struct Button m_buttons[BUTTONS_COUNT] = { 0 };

static alt_u16 m_rectIdent = 10u;
static alt_u16 m_rectSpacing = 40u;
static alt_u16 m_minAreaSize = 500u;

static struct Rect m_drawArea = { .x = 390, .y = 210, .w = 500, .h = 140 };
static struct Rect m_textRect = { .x = 400, .y = 230, .w = 480, .h = 50 };

static bool m_textInitialized = false;
//
// End of Private data

static void createButtons(void)
{
	{
		struct Button b = { .x = 630, .y = 300, .w = 120, .h = 40, .code = KEY_YES };

		b.x = (vduConfig()->display0.width / 2u) - b.w - m_rectIdent;

		(void)snprintf(b.text, BUTTON_TEXT_MAX_LEN, "Yes");
		m_buttons[m_buttonsCount] = b;
		m_buttonsCount++;
	}

	{
		struct Button b = { .x = 760, .y = 300, .w = 120, .h = 40, .code = KEY_NO };

		b.x = (vduConfig()->display0.width / 2u) + m_rectIdent;

		(void)snprintf(b.text, BUTTON_TEXT_MAX_LEN, "No");
		m_buttons[m_buttonsCount] = b;
		m_buttonsCount++;
	}
}

static void adjustLayout(const char* const messageText)
{
	char str[STRING_MAX_LEN + 1u] = { 0 };
	(void)strncpy(str, messageText, sizeof(str));
	str[STRING_MAX_LEN] = '\0';

	(void)memset(&m_textRect, 0, sizeof(m_textRect));
	m_textRect.w = m_minAreaSize;

	alt_u16 lines = 1;
	alt_u16 maxWidth = 0;

	char* lineStart = &str[0];

	while ((lineStart != NULL) && (*lineStart != '\0'))
	{
		char* lineEnd = strchr(lineStart, '\n');

		char saved = '\0';

		if (lineEnd != NULL)
		{
			saved = *lineEnd;
			*lineEnd = '\0';
		}

		alt_u16 lineChars = 0;
		alt_u16 textInPx = stringWidthPx(
			lineStart,
			RVDU_SYSTEM_FONT,
			m_textRect.w,
			&lineChars
		);

		if (textInPx > maxWidth)
		{
			maxWidth = textInPx;
		}

		if (lineEnd == NULL)
		{
			break;
		}

		*lineEnd = saved;
		lineStart = lineEnd + 1;
		lines++;
	}

	// Determine the size of the text

	alt_u16 textHeight =
		fontSymbolHeader(RVDU_SYSTEM_FONT, (alt_u32)' ')->height * lines;

	m_textRect.w = maxWidth;
	if (m_textRect.w < m_minAreaSize)
	{
		m_textRect.w = m_minAreaSize;
	}
	m_textRect.h = textHeight;

	// Draw area includes text and buttons

	m_drawArea.w = m_textRect.w + (m_rectIdent * 2u);
	if (m_drawArea.w > vduConfig()->display0.width)
	{
		m_drawArea.w = vduConfig()->display0.width;
	}

	m_drawArea.h = m_textRect.h + m_buttons[1].h + (2u * m_rectIdent) + m_rectSpacing;
	if (m_drawArea.h > vduConfig()->display0.height)
	{
		m_drawArea.h = vduConfig()->display0.height;
	}

	// Adjust draw area to the center

	m_drawArea.x = (vduConfig()->display0.width / 2u) - (m_drawArea.w / 2u);
	m_drawArea.y = (vduConfig()->display0.height / 2u) - (m_drawArea.h / 2u);

	// Adjust buttons position

	m_buttons[0].y = m_drawArea.y + m_textRect.h + m_rectIdent + m_rectSpacing;
	m_buttons[1].y = m_drawArea.y + m_textRect.h + m_rectIdent + m_rectSpacing;

	// Move the text to the top-left position of the area

	m_textRect.x = m_drawArea.x + m_rectIdent;
	m_textRect.y = m_drawArea.y + m_rectIdent;
}

void modeQuestionShow(const struct PopUpSchemaParams* const schema)
{
	if (schema == NULL)
	{
		RN_ASSERT(schema != NULL);
		return;
	}

	alt_u32 backColorRect = 0x888888;
	alt_u32 backColorStr = 0xf8f8f8;

	static const alt_32 blackColor = 0x0;

	// Initialize all buttons
	//
	if (m_buttonsCount == 0)
	{
		createButtons();
	}

	if (m_textInitialized == false)
	{
		adjustLayout(schema->MessageBoxParams.messageText);

		m_textInitialized = true;
	}

	// Main dialog layout
	//
	alt_u32 backColorArea = 0xdcdcdc;
	drawArea(m_drawArea.x, m_drawArea.y, m_drawArea.w, m_drawArea.h, backColorArea);
	drawRect(m_drawArea.x, m_drawArea.y, m_drawArea.w, m_drawArea.h, 1u, backColorRect);

	char outstr[STRING_MAX_LEN + 1u/*for ending zero*/] = {0};
	prepareStr(schema->MessageBoxParams.messageText, (char*)outstr, STRING_MAX_LEN, INVALID_INDEX/*signalIndex*/);

	printStr(m_textRect.x, m_textRect.y, m_textRect.w, m_textRect.h,
			RVDU_SYSTEM_FONT, blackColor, backColorStr, ALIGN_V_CENTER + ALIGN_H_CENTER, (const char* const)outstr) ;

	backColorArea = 0xc0c0c0;
	backColorStr = 0xdcdcdc;

	// Draw all buttons
	//
	for (int i = 0; i < m_buttonsCount; i++)
	{
		drawArea(m_buttons[i].x, m_buttons[i].y, m_buttons[i].w, m_buttons[i].h, backColorArea);

		drawRect(m_buttons[i].x, m_buttons[i].y, m_buttons[i].w, m_buttons[i].h,
			1u, blackColor);

		printStr(m_buttons[i].x, m_buttons[i].y, m_buttons[i].w, m_buttons[i].h,
			RVDU_SYSTEM_FONT, blackColor, backColorStr, ALIGN_V_CENTER + ALIGN_H_CENTER, m_buttons[i].text);
	}

	(void)pushVideoBuffer();

	return;
}

static void checkPressedButton(struct PopUpSchemaParams* const schema)
{
	if (schema == NULL)
	{
		RN_ASSERT(schema != NULL);
		return;
	}

	for (int i = 0; i < m_buttonsCount; i++)
	{
		if (checkTouchZone(m_buttons[i].x, m_buttons[i].y, m_buttons[i].w, m_buttons[i].h) == true)
		{
			m_textInitialized = false;

			if (m_buttons[i].code == KEY_YES)
			{
				schema->retPermission = true;
				schema->boolResult = true;
			}
			else if (m_buttons[i].code == KEY_NO)
			{
				schema->retPermission = true;
				schema->boolResult = false;
			}
			else
			{
				RN_ASSERT(false);
			}
		}
	}
}


void modeQuestionProc(struct PopUpSchemaParams* const schema)
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
	
	if (isPressedKey() == true)
	{
		alt_u16 code = pressedKey();

		switch (code)
		{
		case RK_RETURN:
		{
			schema->retPermission = true;
			schema->boolResult = true;

			m_textInitialized = false;
		}
		break;
		case RK_ESCAPE:
		{
			schema->retPermission = true;

			m_textInitialized = false;
		}
		break;
		default:
		{
			// Other key
		}
		break;
		}
	}

	if (isTouchEvent() == true)
	{
		checkPressedButton(schema);
	}
	return;
}


