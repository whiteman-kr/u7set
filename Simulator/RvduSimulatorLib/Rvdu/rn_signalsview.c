/*
Module title: Signal List View.
(C) 2026 RPC "Radiy".
All rights reserved.
The information presented here is confidential
and is the intellectual property of PJSC "RPC 'Radiy".
Any use of this information is strictly
prohibited without the written consent of PJSC "RPC "Radiy".
Module description: Provides signal list view.

-- *************************************************************
-- Version v1.00
-- Date:	15,04,2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966, https://jira.radiy.com/browse/RADIX-3969
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-3969, rn_signalsview added

-- *************************************************************
-- Version v1.01
-- Date:	27,04,2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966, https://jira.radiy.com/browse/RADIX-3969
-- Reviewed by: Nesterenko M.V.

-- Add keyboard support
-- Add signalsScrollPgUp, signalsScrollPgDown functions

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
#include <stdint.h>

#include "rn_signalsview.h"
#include "rn_diag.h"
#include "rn_cfg.h"
#include "rn_fofp.h"
#include "rn_prim.h"
#include "rn_vbuf.h"
#include "rn_ts.h"
#include "rn_schema.h"
#include "rn_sifp.h"
#include "rn_utils.h"
#include "rn_kbd.h"

#include "defs.h"

// Private module data
//
static alt_u16 m_scrollPos = 0u;

static struct Button m_closeButton;
static struct Button m_upButton;
static struct Button m_downButton;

static alt_u16 m_controlAreaHeight = 0u;

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

	x -= buttonWidth + controlsIdent;
	m_downButton = createButton(0, x, y, buttonWidth, buttonHeight);

	y -= buttonHeight + controlsIdent;
	m_upButton = createButton(0, x, y, buttonWidth, buttonHeight);

	drawRect(m_upButton.x, m_upButton.y, m_upButton.w, m_upButton.h, 1u, whiteColor);
	printStr(m_upButton.x, m_upButton.y, m_upButton.w, m_upButton.h, 
		RVDU_SYSTEM_FONT, whiteColor, blackColor, ALIGN_H_CENTER, "Page Up");

	drawRect(m_downButton.x, m_downButton.y, m_downButton.w, m_downButton.h, 1, whiteColor);
	printStr(m_downButton.x, m_downButton.y, m_downButton.w, m_downButton.h,
		RVDU_SYSTEM_FONT, whiteColor, blackColor, ALIGN_H_CENTER, "Page Down");

	drawRect(m_closeButton.x, m_closeButton.y, m_closeButton.w, m_closeButton.h, 1, whiteColor);
	printStr(m_closeButton.x, m_closeButton.y, m_closeButton.w, m_closeButton.h,
		RVDU_SYSTEM_FONT, whiteColor, blackColor, ALIGN_H_CENTER, "Close");

	char msg[STRING_MAX_LEN];

	x = 0u;
	y = screenHeight - (fontHeight * 2u);
	(void)snprintf(msg, STRING_MAX_LEN, "Signals count: %d, displayed: %d - %d", appSignalsCount(), firstRecordIndex, lastRecordIndex);
	printStr(x, y, screenWidth - x - 1u, fontHeight,
		RVDU_SYSTEM_FONT,
		whiteColor,
		blackColor,
		ALIGN_H_LEFT, msg);
	
	return;
}

void signalsViewShow(void)
{
	static const alt_u32 blackColor = 0xff000000u;
	static const alt_u32 grayColor = 0xffc0c0c0u;
	static const alt_u32 whiteColor = 0xffffffffu;

	if (writeVideoBuffer(blackColor) == false)
	{
		RN_ASSERT(false);
		return;
	}

	m_controlAreaHeight = (alt_u16)(vduConfig()->display0.height / 5u);

	const struct VduSymbolHeader* fontHeader = fontSymbolHeader(RVDU_SYSTEM_FONT, (alt_u32)' ');
	if ((fontHeader == NULL) || (fontHeader->height == 0u))
	{
		RN_ASSERT(fontHeader != NULL);
		return;
	}
	alt_u16 fontHeight = fontHeader->height;

	alt_u16 signalsAreaWidth = vduConfig()->display0.width;
	alt_u16 signalsAreaHeight = vduConfig()->display0.height - m_controlAreaHeight;

	alt_u16 recCount = appSignalsCount();
	if (recCount == 0u)
	{
		printStr(0u, 0u, vduConfig()->display0.width - 1u, fontHeight,
			RVDU_SYSTEM_FONT,
			whiteColor,
			blackColor,
			ALIGN_H_LEFT, "No signals visible");
	}
	
	alt_u16 maxVisibleSignals = ((signalsAreaHeight - fontHeight) / fontHeight) + 1u;

	alt_u16 signalsCount = appSignalsCount();
	
	alt_u16 firstIndex = m_scrollPos;

	alt_u16 lastIndex = maxVisibleSignals + m_scrollPos - 1u;

	if (lastIndex >= signalsCount)
	{
		lastIndex = signalsCount - 1u;
	}

	drawControlArea(vduConfig()->display0.width, vduConfig()->display0.height, fontHeight, firstIndex, lastIndex);

	static alt_u16 colWitdh[] = { 60, 400, 950, 990, 1050, 1150 };

#define MAX_STRID_LEN 35u
#define MAX_CAPTION_LEN 60u

	// Output records
	//

	alt_u16 y = 0;
	for (alt_u16 i = firstIndex; i <= lastIndex; i++)
	{
		if (i >= signalsCount)
		{
			RN_ASSERT(false);
			break;
		}

		alt_u16 x = 0;

		int col = 0;

		char valueStr[STRING_MAX_LEN] = { 0 };

		alt_u32 color = ((i & 1u) == 1u) ? whiteColor : grayColor;

		(void)snprintf(valueStr, STRING_MAX_LEN, "%04d", (int)i);
		printStr(x, y, signalsAreaWidth - x, fontHeight, RVDU_SYSTEM_FONT, color, blackColor, ALIGN_H_LEFT, valueStr);
		x = colWitdh[col];
		col++;

		(void)snprintf(valueStr, STRING_MAX_LEN, "%s", customAppSignalID(i));
		if (strlen(valueStr) > MAX_STRID_LEN)
		{
			valueStr[MAX_STRID_LEN - 3u] = '.';
			valueStr[MAX_STRID_LEN - 2u] = '.';
			valueStr[MAX_STRID_LEN - 1u] = '.';
			valueStr[MAX_STRID_LEN] = '\0';
		}
		printStr(x, y, signalsAreaWidth - x, fontHeight, RVDU_SYSTEM_FONT, color, blackColor, ALIGN_H_LEFT, valueStr);
		x = colWitdh[col];
		col++;

		(void)snprintf(valueStr, STRING_MAX_LEN, "%s", signalCaption(i));
		if (strlen(valueStr) > MAX_CAPTION_LEN)
		{
			valueStr[MAX_CAPTION_LEN - 3u] = '.';
			valueStr[MAX_CAPTION_LEN - 2u] = '.';
			valueStr[MAX_CAPTION_LEN - 1u] = '.';
			valueStr[MAX_CAPTION_LEN] = '\0';
		}
		printStr(x, y, signalsAreaWidth - x, fontHeight, RVDU_SYSTEM_FONT, color, blackColor, ALIGN_H_LEFT, valueStr);
		x = colWitdh[col];
		col++;

		{
			const struct VduAppSignalParam* const asp = appSignalParamByIndex(i);
			if (asp == NULL)
			{
				return;
			}

			switch (asp->vduSignalType)
			{
			case AnalogFloat32:
				printStr(x, y, signalsAreaWidth - x, fontHeight, RVDU_SYSTEM_FONT, color, blackColor, ALIGN_H_LEFT, "FP");
				break;
			case AnalogSignedInt32:
				printStr(x, y, signalsAreaWidth - x, fontHeight, RVDU_SYSTEM_FONT, color, blackColor, ALIGN_H_LEFT, "SI");
				break;
			case Discrete:
				printStr(x, y, signalsAreaWidth - x, fontHeight, RVDU_SYSTEM_FONT, color, blackColor, ALIGN_H_LEFT, "D");
				break;
			default:
				{
				}
				break;
			}
			x = colWitdh[col];
			col++;

			switch (asp->vduSignalInOutType)
			{
			case Internal:
				printStr(x, y, signalsAreaWidth - x, fontHeight, RVDU_SYSTEM_FONT, color, blackColor, ALIGN_H_LEFT, "Internal");
				break;
			case Input:
				printStr(x, y, signalsAreaWidth - x, fontHeight, RVDU_SYSTEM_FONT, color, blackColor, ALIGN_H_LEFT, "Input");
				break;
			case Output:
				printStr(x, y, signalsAreaWidth - x, fontHeight, RVDU_SYSTEM_FONT, color, blackColor, ALIGN_H_LEFT, "Output");
				break;
			case RxSignal:
				printStr(x, y, signalsAreaWidth - x, fontHeight, RVDU_SYSTEM_FONT, color, blackColor, ALIGN_H_LEFT, "Rx");
				break;
			default:
				{
				}
				break;
			}
			x = colWitdh[col];
			col++;

			if (asp->vduSignalInOutType == (alt_u16)Output)
			{
				// Output signals' values can't be read

				(void)snprintf(valueStr, STRING_MAX_LEN, "-");
			}
			else
			{
				// Signal can be non-valid
				
				if (signalAvailableByIndex(i) == false)
				{
					(void)snprintf(valueStr, STRING_MAX_LEN, "???");
				}
				else
				{
					if (asp->vduSignalType == (alt_u16)AnalogFloat32)
					{
						// VduSignalType AnalogFloat32
						//
#define FORMAT_STR_MAX_LENGTH 32u
						char convertFormat[FORMAT_STR_MAX_LENGTH];
						(void)snprintf(convertFormat, FORMAT_STR_MAX_LENGTH, "%%.%df", asp->decimalPlaces);

						float res = 0.0f;
						if (readSignalValueFloatByIndex(i, &res) == false)
						{
							(void)snprintf(valueStr, STRING_MAX_LEN, "???");
						}
						else
						{
							(void)snprintf(valueStr, STRING_MAX_LEN, convertFormat, res);
						}
					}
					else
					{
						// VduSignalType discrete or integer
						//
						alt_32 res = 0;
						if (readSignalValueIntByIndex(i, &res) == false)
						{
							(void)snprintf(valueStr, STRING_MAX_LEN, "???");
						}
						else
						{
							(void)snprintf(valueStr, STRING_MAX_LEN, "%d", (int)res);
						}
					}
				}
			}

			printStr(x, y, signalsAreaWidth - x, fontHeight, RVDU_SYSTEM_FONT, color, blackColor, ALIGN_H_LEFT, valueStr);
			x = colWitdh[col];
		}

		printStr(x, y, signalsAreaWidth - x, fontHeight, RVDU_SYSTEM_FONT, color, blackColor, ALIGN_H_LEFT, signalUnit(i));

		y += fontHeight;

		if (y >= signalsAreaHeight)
		{
			break;
		}
	}

	return;
}

void signalsViewProc(struct PopUpSchemaParams* const schema)
{
	if (schema == NULL)
	{
		RN_ASSERT(schema != NULL);
		return;
	}

	if (isPressedKey() == true)
	{
		alt_u32 code = pressedKey();

		switch (code)
		{
		case RK_ESCAPE:
		{
			popUpSchemaSetTerminate();
			return;
		}
		case RK_UP:
		{
			signalsScrollUp();
		}
		break;
		case RK_DOWN:
		{
			signalsScrollDown();
		}
		break;
		case RK_PRIOR:
		{
			signalsScrollPgUp();
		}
		break;
		case RK_NEXT:
		{
			signalsScrollPgDown();
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
			popUpSchemaSetTerminate();
		}
		else if (checkTouchZone(m_upButton.x, m_upButton.y, m_upButton.w, m_upButton.h))
		{
			signalsScrollPgUp();
		}
		else if (checkTouchZone(m_downButton.x, m_downButton.y, m_downButton.w, m_downButton.h)) 
		{
			signalsScrollPgDown();
		}
		else
		{
			;	// Click outside keys - no need any actions
		}
	}
	return;
}

void signalsScrollUp(void)
{
	if (m_scrollPos > 0u)
	{
		m_scrollPos--;
	}
}

void signalsScrollDown(void)
{
	const struct VduSymbolHeader* fontHeader = fontSymbolHeader(RVDU_SYSTEM_FONT, (alt_u32)' ');
	if ((fontHeader == NULL) || (fontHeader->height == 0u))
	{
		RN_ASSERT((fontHeader != NULL) && (fontHeader->height != 0u));
		return;
	}
	alt_u16 fontHeight = fontHeader->height;

	alt_u16 screenHeight = vduConfig()->display0.height - m_controlAreaHeight;

	alt_u16 maxVisibleSignals = ((screenHeight - fontHeight) / fontHeight) + 1u;

	alt_u16 signalsCount = appSignalsCount();

	if (signalsCount <= maxVisibleSignals)
	{
		m_scrollPos = 0;
	}
	else
	{
		if (m_scrollPos < (signalsCount - maxVisibleSignals))
		{
			m_scrollPos++;
		}
	}
}

void signalsScrollPgUp(void)
{
	const struct VduSymbolHeader* fontHeader = fontSymbolHeader(RVDU_SYSTEM_FONT, (alt_u32)' ');
	if ((fontHeader == NULL) || (fontHeader->height == 0u))
	{
		RN_ASSERT((fontHeader != NULL) && (fontHeader->height != 0u));
		return;
	}
	alt_u16 fontHeight = fontHeader->height;

	alt_u16 screenHeight = vduConfig()->display0.height - m_controlAreaHeight;

	alt_u16 maxVisibleSignals = ((screenHeight - fontHeight) / fontHeight) + 1u;

	alt_u16 signalsCount = appSignalsCount();
	
	if (signalsCount <= maxVisibleSignals)
	{
		m_scrollPos = 0; 
	}
	else
	{
		if (m_scrollPos > maxVisibleSignals)
		{
			m_scrollPos -= maxVisibleSignals;
		}
		else
		{
			m_scrollPos = 0;
		}
	}
}

void signalsScrollPgDown(void)
{
	const struct VduSymbolHeader* fontHeader = fontSymbolHeader(RVDU_SYSTEM_FONT, (alt_u32)' ');
	if ((fontHeader == NULL) || (fontHeader->height == 0u))
	{
		RN_ASSERT((fontHeader != NULL) && (fontHeader->height != 0u));
		return;
	}
	alt_u16 fontHeight = fontHeader->height;

	alt_u16 screenHeight = vduConfig()->display0.height - m_controlAreaHeight;

	alt_u16 maxVisibleSignals = ((screenHeight - fontHeight) / fontHeight) + 1u;

	alt_u16 signalsCount = appSignalsCount();

	if (signalsCount <= maxVisibleSignals)
	{
		m_scrollPos = 0; 
	}
	else
	{
		m_scrollPos += maxVisibleSignals;

		if (m_scrollPos > (signalsCount - maxVisibleSignals))
		{
			m_scrollPos = signalsCount - maxVisibleSignals;
		}
	}
}