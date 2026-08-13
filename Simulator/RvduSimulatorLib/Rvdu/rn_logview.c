/*
Module title: Log View.
(C) 2026 RPC "Radiy".
All rights reserved.
The information presented here is confidential
and is the intellectual property of PJSC "RPC 'Radiy".
Any use of this information is strictly
prohibited without the written consent of PJSC "RPC "Radiy".
Module description: Provides log view.

-- *************************************************************
-- Version v1.00
-- Date:	15,04,2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966, https://jira.radiy.com/browse/RADIX-3969
-- Reviewed by: Mazyr S.M.

-- Changes: RADIX-3969, rn_logiew added
-- Add keyboard support
-- Add logScrollPgUp, logScrollPgDown functions

-- Changes: RADIX-3969, add logging and debugging support
-- Add log errors and messages

 *************************************************************

-- Version v1.01
-- Date:	09,05,2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-4009
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-4009, adding API support for working with signals by AppSignalIDs

-- *************************************************************

-- Version v1.02
-- Date:	28,07,2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-3966, code is made MISRA-2025 compatible

-- *************************************************************
*/

#include <string.h>
#include <stdio.h>
#include <stdint.h>

#ifndef RVDU_SIM // Microsoft Visual Studio-specific

#include "sys/alt_stdio.h"

#endif

#include "rn_logview.h"
#include "rn_diag.h"
#include "rn_cfg.h"
#include "rn_fofp.h"
#include "rn_prim.h"
#include "rn_vbuf.h"
#include "rn_ts.h"
#include "rn_schema.h"
#include "rn_utils.h"
#include "rn_kbd.h"

#include "defs.h"

// Private module data
//
static alt_u16 m_logScrollPos = 0;

static struct Button m_closeButton;
static struct Button m_upButton;
static struct Button m_downButton;
static struct Button m_clearButton;

static alt_u16 m_controlAreaHeight = 0;

//
// End of Private data

static struct Button createButton(alt_u16 code, alt_u16 x, alt_u16 y, alt_u16 w, alt_u16 h)
{
	struct Button result = { 0 };
	result.code = code;
	result.x = x;
	result.y = y;
	result.w = w;
	result.h = h;
	return result;
}

static void drawControlArea(alt_u16 screenWidth, alt_u16 screenHeight, alt_u16 fontHeight, alt_u16 firstRecordIndex, alt_u16 lastRecordIndex)
{
	static const alt_u32 blackColor = 0xff000000u;
	static const alt_u32 whiteColor = 0xffffffffu;

	drawLine(0u, screenHeight - m_controlAreaHeight, screenWidth - 1u, screenHeight - m_controlAreaHeight, 1u, whiteColor);

	static const alt_u16 buttonWidth = 80u;
	static const alt_u16 buttonHeight = 40u;
	static const alt_u16 controlsIdent = 20u;

	alt_u16 x = screenWidth - controlsIdent - buttonWidth;
	alt_u16 y = screenHeight - controlsIdent - buttonHeight;

	m_closeButton = createButton(0, x, y, buttonWidth, buttonHeight);

	y -= buttonHeight + controlsIdent;
	m_clearButton = createButton(0, x, y, buttonWidth, buttonHeight);
	
	y = screenHeight - controlsIdent - buttonHeight;

	x -= buttonWidth + controlsIdent;
	m_downButton = createButton(0, x, y, buttonWidth, buttonHeight);

	y -= buttonHeight + controlsIdent;
	m_upButton = createButton(0, x, y, buttonWidth, buttonHeight);

	drawRect(m_upButton.x, m_upButton.y, m_upButton.w, m_upButton.h, 1u, whiteColor);
	printStr(m_upButton.x, m_upButton.y, m_upButton.w, m_upButton.h, 
		RVDU_SYSTEM_FONT, whiteColor, blackColor, ALIGN_H_CENTER, "Page Up");

	drawRect(m_downButton.x, m_downButton.y, m_downButton.w, m_downButton.h, 1u, whiteColor);
	printStr(m_downButton.x, m_downButton.y, m_downButton.w, m_downButton.h,
		RVDU_SYSTEM_FONT, whiteColor, blackColor, ALIGN_H_CENTER, "Page Down");

	drawRect(m_clearButton.x, m_clearButton.y, m_clearButton.w, m_clearButton.h, 1u, whiteColor);
	printStr(m_clearButton.x, m_clearButton.y, m_clearButton.w, m_clearButton.h,
		RVDU_SYSTEM_FONT, whiteColor, blackColor, ALIGN_H_CENTER, "Clear\nErrors");

	drawRect(m_closeButton.x, m_closeButton.y, m_closeButton.w, m_closeButton.h, 1u, whiteColor);
	printStr(m_closeButton.x, m_closeButton.y, m_closeButton.w, m_closeButton.h,
		RVDU_SYSTEM_FONT, whiteColor, blackColor, ALIGN_H_CENTER, "Close");

	char msg[STRING_MAX_LEN];

	x = 0;
	y = screenHeight - (fontHeight * 3u);
	(void)snprintf(msg, STRING_MAX_LEN, "Records count: %d, displayed: %d - %d", logRecordCount(), firstRecordIndex, lastRecordIndex);
	printStr(x, y, screenWidth - x - 1u, fontHeight,
		RVDU_SYSTEM_FONT,
		whiteColor,
		blackColor,
		ALIGN_H_LEFT, msg);
	
	y += fontHeight;

	(void)snprintf(msg, STRING_MAX_LEN, "Errors: %u, Warnings: %u", (unsigned int)logErrorCount(), (unsigned int)logWarningCount());
	printStr(x, y, screenWidth - x - 1u, fontHeight,
		RVDU_SYSTEM_FONT,
		whiteColor,
		blackColor,
		ALIGN_H_LEFT, msg);

	y = screenHeight - m_controlAreaHeight + fontHeight;

	alt_u16 textTopMargin = m_downButton.x - controlsIdent;

	// Print errors
	//
	if (logErrorCount() > 0u)
	{
		(void)snprintf(msg, STRING_MAX_LEN, "Errors: ");
		for (int i = 0; i < (int)ErrorCodeCount; i++)
		{
			if (errorCounter(i) != 0u)
			{
				static char codeMsg[STRING_MAX_LEN];
				(void)snprintf(codeMsg, STRING_MAX_LEN, "%s = %d ", errorCodeMsg(i), errorCounter(i));

				alt_u16 charsCount = 0;
				alt_u16 msgWidth = stringWidthPx(msg, RVDU_SYSTEM_FONT, textTopMargin - x - 1u, &charsCount);
				alt_u16 codeMsgWidth = stringWidthPx(codeMsg, RVDU_SYSTEM_FONT, textTopMargin - x - 1u, &charsCount);

				if ((msgWidth + codeMsgWidth) >= (textTopMargin - x - (alt_u16)1u))
				{
					// The length of message is not enough to store more text - put line break
					//
					printStr(x, y, textTopMargin - x - 1u, screenHeight - y - 1u,
						RVDU_SYSTEM_FONT,
						whiteColor,
						blackColor,
						ALIGN_H_LEFT | ALIGN_V_TOP, msg);

					y += fontHeight;

					// Reset the string
					//
					msg[0] = '\0';
				}
				
				(void)strncat(msg, (char*)codeMsg, STRING_MAX_LEN - strlen(msg));
			}
		}
		printStr(x, y, textTopMargin - x - 1u, screenHeight - y - 1u,
			RVDU_SYSTEM_FONT,
			whiteColor,
			blackColor,
			ALIGN_H_LEFT | ALIGN_V_TOP, msg);
	}
	else
	{
		printStr(x, y, screenWidth - x - 1u, fontHeight,
			RVDU_SYSTEM_FONT,
			whiteColor,
			blackColor,
			ALIGN_H_LEFT, "Errors: none");
	}
	y += fontHeight;

	// Print wanings
	//
	if (logWarningCount() > 0u)
	{
		(void)snprintf(msg, STRING_MAX_LEN, "Warnings: ");
		for (int i = 0; i < (int)WarningCodeCount; i++)
		{
			if (warningCounter(i) != 0u)
			{
				static char codeMsg[STRING_MAX_LEN];
				(void)snprintf(codeMsg, STRING_MAX_LEN, "%s = %d ", warningCodeMsg(i), warningCounter(i));

				alt_u16 charsCount = 0;
				alt_u16 msgWidth = stringWidthPx(msg, RVDU_SYSTEM_FONT, textTopMargin - x - 1u, &charsCount);
				alt_u16 codeMsgWidth = stringWidthPx(codeMsg, RVDU_SYSTEM_FONT, textTopMargin - x - 1u, &charsCount);

				if ((msgWidth + codeMsgWidth) >= (textTopMargin - x - (alt_u16)1u))
				{
					// The length of message is not enough to store more text - put line break
					//
					printStr(x, y, textTopMargin - x - 1u, screenHeight - y - 1u,
						RVDU_SYSTEM_FONT,
						whiteColor,
						blackColor,
						ALIGN_H_LEFT | ALIGN_V_TOP, msg);

					y += fontHeight;

					// Reset the string
					//
					msg[0] = '\0';
				}

				(void)strncat(msg, (char*)codeMsg, STRING_MAX_LEN - strlen(msg));
			}
		}
		printStr(x, y, textTopMargin - x - 1u, screenHeight - y - 1u,
			RVDU_SYSTEM_FONT,
			whiteColor,
			blackColor,
			ALIGN_H_LEFT | ALIGN_V_TOP, msg);
	}
	else
	{
		printStr(x, y, screenWidth - x - 1u, fontHeight,
			RVDU_SYSTEM_FONT,
			whiteColor,
			blackColor,
			ALIGN_H_LEFT, "Warnings: none");
	}

	x = 0;
	y = screenHeight - fontHeight - 1u;
	(void)snprintf(msg, STRING_MAX_LEN, "Build: %s, %s", __DATE__, __TIME__);
	printStr(x, y, screenWidth - x - 1u, fontHeight,
		RVDU_SYSTEM_FONT,
		whiteColor,
		blackColor,
		ALIGN_H_CENTER, msg);

	return;
}

void logSchemaShow(void)
{
	static const alt_u32 blackColor = 0xff000000u;
	static const alt_u32 whiteColor = 0xffffffffu;
	static const alt_u32 redColor = 0xffff0000u;
	static const alt_u32 yellowColor = 0xffffff00u;

	if (writeVideoBuffer(blackColor) == false)
	{
		RN_ASSERT(false);
		return;
	}

	m_controlAreaHeight = vduConfig()->display0.height / 5u;

	const struct VduSymbolHeader* fontHeader = fontSymbolHeader(RVDU_SYSTEM_FONT, (alt_u32)' ');
	if ((fontHeader == NULL) || (fontHeader->height == 0u))
	{
		RN_ASSERT(fontHeader != NULL);
		return;
	}
	alt_u16 fontHeight = fontHeader->height;

	alt_u16 logAreaWidth = vduConfig()->display0.width;
	alt_u16 logAreaHeight = vduConfig()->display0.height - m_controlAreaHeight;

	alt_u16 recCount = logRecordCount();
	if (recCount == 0u)
	{
		printStr(0u, 0u, vduConfig()->display0.width - 1u, fontHeight,
			RVDU_SYSTEM_FONT,
			whiteColor,
			blackColor,
			ALIGN_H_LEFT, "Log is empty.");
	}
	
	alt_u16 maxVisibleRecords = (logAreaHeight - fontHeight) / fontHeight;

	alt_u16 lastRecordIndex = recCount;
	if (lastRecordIndex >= (m_logScrollPos + 1u))
	{
		lastRecordIndex -= (m_logScrollPos + 1u);
	}
	else
	{
		lastRecordIndex = 0;
	}

	alt_u16 firstRecordIndex = lastRecordIndex;
	if (firstRecordIndex >= maxVisibleRecords)
	{
		firstRecordIndex -= maxVisibleRecords;
	}
	else
	{
		firstRecordIndex = 0;
	}

	drawControlArea(vduConfig()->display0.width, vduConfig()->display0.height, fontHeight, firstRecordIndex, lastRecordIndex);

	// Output records
	//
	alt_u16 x = 0;
	alt_u16 y = 0;
	for (alt_u16 i = firstRecordIndex; i <= lastRecordIndex; i++)
	{
		const LogRecord* const rec = logRecord(i);

		alt_u32 color = whiteColor;
		if (rec->type == Error)
		{
			color = redColor;
		}
		else
		{
			if (rec->type == Warning)
			{
				color = yellowColor;
			}
		}

		alt_u16 charsCount = 0;
		alt_u16 msgWidth = stringWidthPx(rec->text, RVDU_SYSTEM_FONT, logAreaWidth - x - 1u, &charsCount);

		if (msgWidth >= (logAreaWidth - x - 1u))
		{
			char msgShort[STRING_MAX_LEN] = { 0 };
			(void)strncpy(msgShort, rec->text, charsCount);
			msgShort[charsCount - 1u] = '.';
			msgShort[charsCount - 2u] = '.';
			msgShort[charsCount - 3u] = '.';

			printStr(x, y, logAreaWidth - x - 1u, fontHeight,
				RVDU_SYSTEM_FONT,
				color,
				blackColor,
				ALIGN_H_LEFT, msgShort);
		}
		else
		{
			printStr(x, y, logAreaWidth - x - 1u, fontHeight,
				RVDU_SYSTEM_FONT,
				color,
				blackColor,
				ALIGN_H_LEFT, rec->text);
		}

		y += fontHeight;

		if (y >= logAreaHeight)
		{
			break;
		}
	}

	(void)pushVideoBuffer();

	return;
}

void logSchemaProc(struct PopUpSchemaParams* const schema)
{
	if (schema == NULL)
	{
		RN_ASSERT(schema != NULL);
		return;
	}

	if (isPressedKey() == true)
	{
		alt_u16 code = pressedKey();

		switch (code)
		{
		case RK_ESCAPE:
		{
			schema->retPermission = true;
			return;
		}
		case RK_UP:
		{
			logScrollUp();
		}
		break;
		case RK_DOWN:
		{
			logScrollDown();
		}
		break;
		case RK_PRIOR:
		{
			logScrollPgUp();
		}
		break;
		case RK_NEXT:
		{
			logScrollPgDown();
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
		if (checkTouchZone(m_closeButton.x, m_closeButton.y, m_closeButton.w, m_closeButton.h))
		{
			schema->retPermission = true;
		}
		else if (checkTouchZone(m_clearButton.x, m_clearButton.y, m_clearButton.w, m_clearButton.h))
		{
			logClearCounters();
		}
		else if (checkTouchZone(m_upButton.x, m_upButton.y, m_upButton.w, m_upButton.h))
		{
			logScrollPgUp();
		}
		else if (checkTouchZone(m_downButton.x, m_downButton.y, m_downButton.w, m_downButton.h)) 
		{
			logScrollPgDown();
		}
		else
		{
			;	// Click outside keys - no need any actions
		}
	}
	return;
}

void logScrollUp(void)
{
	const struct VduSymbolHeader* fontHeader = fontSymbolHeader(RVDU_SYSTEM_FONT, (alt_u32)' ');
	if ((fontHeader == NULL) || (fontHeader->height == 0u))
	{
		RN_ASSERT(fontHeader != NULL);
		return;
	}
	alt_u16 fontHeight = fontHeader->height;

	alt_u16 screenHeight = vduConfig()->display0.height - m_controlAreaHeight;

	alt_u16 maxVisibleRecords = ((screenHeight - fontHeight) / fontHeight) + 1u;

	if (m_logScrollPos < logRecordCount() - maxVisibleRecords)
	{
		m_logScrollPos++;
	}
}

void logScrollDown(void)
{
	if (m_logScrollPos > 0u)
	{
		m_logScrollPos--;
	}
}

void logScrollPgUp(void)
{
	const struct VduSymbolHeader* fontHeader = fontSymbolHeader(RVDU_SYSTEM_FONT, (alt_u32)' ');
	if ((fontHeader == NULL) || (fontHeader->height == 0u))
	{
		RN_ASSERT(fontHeader != NULL);
		return;
	}
	alt_u16 fontHeight = fontHeader->height;

	alt_u16 screenHeight = vduConfig()->display0.height - m_controlAreaHeight;

	alt_u16 maxVisibleRecords = ((screenHeight - fontHeight) / fontHeight) + 1u;

	m_logScrollPos += maxVisibleRecords;

	if (m_logScrollPos > logRecordCount() - maxVisibleRecords)
	{
		m_logScrollPos = logRecordCount() - maxVisibleRecords;
	}

}

void logScrollPgDown(void)
{
	const struct VduSymbolHeader* fontHeader = fontSymbolHeader(RVDU_SYSTEM_FONT, (alt_u32)' ');
	if ((fontHeader == NULL) || (fontHeader->height == 0u))
	{
		RN_ASSERT(fontHeader != NULL);
		return;
	}
	alt_u16 fontHeight = fontHeader->height;

	alt_u16 screenHeight = vduConfig()->display0.height - m_controlAreaHeight;

	alt_u16 maxVisibleRecords = ((screenHeight - fontHeight) / fontHeight) + 1u;

	if (m_logScrollPos > maxVisibleRecords)
	{
		m_logScrollPos -= maxVisibleRecords;
	}
	else
	{
		m_logScrollPos = 0u;
	}
}
