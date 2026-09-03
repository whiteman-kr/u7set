/*
Module title: Trends Processing
(C) 2026 RPC "Radiy".
All rights reserved.
The information presented here is confidential
and is the intellectual property of PJSC "RPC "Radiy".
Any use of this information is strictly
prohibited without the written consent of PJSC "RPC "Radiy".
Module description: Processes trends data.

-- *************************************************************
-- Version v1.00
-- Date:	01.06.2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Nesterenko M.V.

-- *************************************************************

-- Version v1.01
-- Date:	28,07,2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-3966, code is made MISRA-2025 compatible

-- *************************************************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "rn_prim.h"
#include "rn_sifp.h"
#include "rn_diag.h"
#include "rn_fofp.h"
#include "rn_utils.h"
#include "rn_scfp.h"
#include "rn_trendproc.h"
#include "rn_trendview.h"

#include "defs.h"

// Private module data
//
static alt_u16 m_screenWidth = 0;
static alt_u16 m_screenHeight = 0;
static bool m_moduleInitialized = false;

//
// End of Private data

static void drawTimeGrid(const VduSchemaFileSchemaItemTrend1* const trend, alt_u16 left, alt_u16 top, alt_u16 width, alt_u16 height, alt_u16 fontHeight,
	alt_u16 horzGridCount, alt_u64 timeCode, bool outputTime);

static void drawScaleGrid(const VduSchemaFileSchemaItemTrend1* const trend, alt_u16 laneIndex, alt_u16 laneCount, alt_u16 vertGridCount,
	alt_u16 left, alt_u16 top, alt_u16 width, alt_u16 height, alt_u16 fontHeight,
	alt_u32 backColor, alt_u16 scaleAreaWidth,
	const struct VduAppSignalParam* const asp,
	const struct TrendSignal* const tsp);

static void drawCurve(const VduSchemaFileSchemaItemTrend1* const trend, alt_u16 trendSignalIndex, alt_u16 left, alt_u16 top, alt_u16 width, alt_u16 height);

void moduleRnTrendViewInitialize(alt_u16 screenWidth, alt_u16 screenHeight)
{
	if (m_moduleInitialized == true)
	{
		RN_ASSERT(false);
		return;
	}
	m_moduleInitialized = true;

	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	return;
}

void drawTrend(const VduSchemaFileSchemaItemTrend1* const trend)
{
	alt_u32 signalBackColor[2] = { trend->backColor1st, trend->backColor2nd };

	if (m_moduleInitialized == false)
	{
		RN_ASSERT_MSG(false, "rn_trendview is not initialized.");
		return;
	}

	if (trendIsConsistent(trend) == false)
	{
		// Trend has invalid properties
		//
		return;
	}

	if ((trend->width == 0u) || (trend->height == 0u))
	{
		// Rect is invisible
		//
		return;
	}

	if ((trend->left >= m_screenWidth) ||
		(trend->top >= m_screenHeight) ||
		((trend->left + trend->width) > m_screenWidth) ||
		((trend->top + trend->height) > m_screenHeight))
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "drawTrend: try to draw by invalid coordinates: (x = %d, y = %d, w = %d, h = %d).",
			trend->left, trend->top, trend->width, trend->height);
		logWarning(DrawError, msg);
		return;
	}

	//
	drawArea(trend->left, trend->top, trend->width, trend->height, trend->backColor);

	// Trend area
	//
	const alt_u16 fontHeight = fontSymbolHeader(trend->fontIndex, (alt_u32)' ')->height;
	
	if (trend->columnCount == 0u)
	{
		RN_ASSERT(false);
		return;
	}
	
	const alt_u16 topIdent = (trend->indentTop == -1) ? 
		TREND_DEFAULT_IDENT : 
		(alt_u16)trend->indentTop;
	
	const alt_u16 bottomIdent = (trend->indentBottom == -1) ? 
		(trend->showDateLabels ? (fontHeight * 2u) : (fontHeight + TREND_DEFAULT_IDENT)) :
		(alt_u16)trend->indentBottom;

	const alt_u16 rightIdent = (trend->indentRight == -1) ? 
		TREND_DEFAULT_IDENT : 
		(alt_u16)trend->indentRight;
	
	const alt_u16 textIdent = TREND_DEFAULT_IDENT;
	
	const alt_u16 leftIdent = trend->width - (alt_u16)trend->columnCount - rightIdent;

	// Calculate lanes count
	//
	alt_u16 discreteCount = 0;
	alt_u16 analogCount = 0;

	for (alt_u16 si = 0; si < trend->signalCount; si++)
	{
		if (trend->trendSignals[si].flags.bits.visible == false)
		{
			continue;
		}

		const struct VduAppSignalParam* const asp = appSignalParamByIndex((alt_u16)trend->trendSignals[si].appSignalIndex);
		if (asp == NULL)
		{
			RN_ASSERT(asp != NULL);
			return;
		}

		if (asp->vduSignalType == (alt_u16)Discrete)
		{
			discreteCount++;
		}
		else
		{
			analogCount++;
		}
	}

	alt_u16 discreteLaneCount = discreteCount;
	alt_u16 analogLaneCount = analogCount;

	if (trend->viewMode == (alt_u16)Overlapped)
	{
		if (analogCount > 0u)
		{
			analogLaneCount = 1u;
		}
	}

	// Calculate lanes dimensions
	//
	const alt_u16 laneLeft = trend->left + leftIdent;
	alt_u16 laneTop = trend->top + topIdent;
	const alt_u16 laneWidth = (alt_u16)trend->columnCount;

	// Draw the background
	//
	drawRect(laneLeft - 1u, laneTop - 1u, laneWidth + 1u, trend->height - topIdent - bottomIdent + 1u, 1u, trend->lineColor);

	// Calculate lanes height
	//
	alt_u16 laneCount = discreteLaneCount + analogLaneCount;
	if (laneCount == 0u)
	{
		return;
	}

	float discreteLaneHeightF = (((float)trend->height - (float)topIdent - (float)bottomIdent) / (float)laneCount) + 0.5f;
	alt_u16 discreteLaneHeight = (alt_u16)(discreteLaneHeightF);
	if ((analogCount > 0u) && (discreteLaneHeight > 50u))
	{
		discreteLaneHeight = 50u;
	}

	alt_u16 analogLaneHeight = 0;
	if (analogCount > 0u)
	{
		float analogLaneHeightF = ((((float)trend->height - (float)topIdent - (float)bottomIdent - (float)discreteLaneHeight * (float)discreteLaneCount) / (float)analogLaneCount) + 0.5f);
		analogLaneHeight = (alt_u16)(analogLaneHeightF);
	}

	// Vertical discrete grid properties
	//
	const alt_u16 vertGridMax = 80;
	alt_u16 vertGridDiscreteCount = discreteLaneHeight / vertGridMax;
	if (vertGridDiscreteCount <= 1u)
	{
		vertGridDiscreteCount = 2u;
	}

	// Vertical analog grid properties
	//
	alt_u16 vertGridAnalogCount = analogLaneHeight / vertGridMax;
	if (vertGridAnalogCount <= 1u)
	{
		vertGridAnalogCount = 2u;
	}

	alt_u64 timeCode = currentTimeInSecs();

	// Horizontal grid properties
	//
	const alt_u16 horzGridMax = 180;
	alt_u16 horzGridCount = laneWidth / horzGridMax;
	if (horzGridCount <= 1u)
	{
		horzGridCount = 2u;
	}

	// Draw discrete lanes
	//
	alt_u16 laneIndex = 0;

	for (alt_u16 si = 0; si < trend->signalCount; si++)
	{
		const struct TrendSignal* const tsp = &trend->trendSignals[si];
		
		if (tsp->flags.bits.visible == false)
		{
			continue;
		}

		const struct VduAppSignalParam* const asp = appSignalParamByIndex((alt_u16)tsp->appSignalIndex);
		if (asp == NULL)
		{
			RN_ASSERT(asp != NULL);
			return;
		}
		
		if (asp->vduSignalType == (alt_u16)Discrete)
		{
			bool lastLane = laneIndex == (laneCount - 1u);

			drawArea(laneLeft, laneTop, laneWidth, discreteLaneHeight, signalBackColor[laneIndex & 1u]);
			
			drawTimeGrid(trend, laneLeft, laneTop, laneWidth, discreteLaneHeight, fontHeight,
				horzGridCount, timeCode, lastLane == true);
			
			drawCurve(trend, si, laneLeft, laneTop, laneWidth, discreteLaneHeight);
			
			drawScaleGrid(trend, 0/*laneIndex*/, 1/*laneCount*/, vertGridDiscreteCount,
				laneLeft, laneTop, laneWidth, discreteLaneHeight, fontHeight,
				signalBackColor[laneIndex & 1u], leftIdent - textIdent, asp, tsp);

			laneTop += discreteLaneHeight;
			laneIndex++;
		}
	}

	// Draw analog lanes
	//
	if ((analogCount > 0u) && (trend->viewMode == (alt_u16)Overlapped))
	{
		drawArea(laneLeft, laneTop, laneWidth, analogLaneHeight, signalBackColor[laneIndex & 1u]);
	}

	alt_u16 analogSignalIndex = 0;

	for (alt_u16 si = 0; si < trend->signalCount; si++)
	{
		const struct TrendSignal* const tsp = &trend->trendSignals[si];

		if (tsp->flags.bits.visible == false)
		{
			continue;
		}

		const struct VduAppSignalParam* const asp = appSignalParamByIndex((alt_u16)tsp->appSignalIndex);
		if (asp == NULL)
		{
			RN_ASSERT(asp != NULL);
			return;
		}

		if (asp->vduSignalType != (alt_u16)Discrete)
		{
			bool lastLane = laneIndex == (laneCount - 1u);

			if (trend->viewMode == (alt_u16)Separate)
			{
				drawArea(laneLeft, laneTop, laneWidth, analogLaneHeight, signalBackColor[laneIndex & 1u]);
			}

			drawTimeGrid(trend, laneLeft, laneTop, laneWidth, analogLaneHeight, fontHeight, horzGridCount, timeCode, lastLane == true);

			drawCurve(trend, si, laneLeft, laneTop, laneWidth, analogLaneHeight);
			
			drawScaleGrid(trend, analogSignalIndex, ((trend->viewMode == (alt_u16)Separate) ? 1u : analogCount), vertGridAnalogCount,
				laneLeft, laneTop, laneWidth, analogLaneHeight, fontHeight,
				signalBackColor[laneIndex & 1u], (leftIdent - textIdent), asp, tsp);

			if (trend->viewMode == (alt_u16)Separate)
			{
				laneTop += analogLaneHeight;
				analogSignalIndex = 0;
			}
			else
			{
				analogSignalIndex++;
			}
			laneIndex++;
		}
	}
}

static void drawTimeGrid(const VduSchemaFileSchemaItemTrend1* const trend, 
	alt_u16 left, alt_u16 top, alt_u16 width, alt_u16 height, 
	alt_u16 fontHeight,
	alt_u16 horzGridCount,
	alt_u64 timeCode,
	bool outputTime)
{
	alt_u16 firstSignalDataIndex = trend->trendSignals[0].trendSignalDataRecordIndex;

	const struct TrendSignalData* const tsd = trendSignalData(firstSignalDataIndex);
	if (tsd == NULL)
	{
		RN_ASSERT(tsd != NULL);
		return;
	}
	if ((tsd->columnCount == 0u) || (tsd->durationSecs < TREND_MIN_DURATION) || (tsd->durationSecs > TREND_MAX_DURATION))
	{
		RN_ASSERT(false);
		return;
	}
	
	if (horzGridCount > 0u)
	{
		const float secsPerColumnF = (float)tsd->durationSecs / (float)tsd->columnCount;	// columns per second

		const alt_u16 timeGridInterval = (alt_u16)(tsd->durationSecs / horzGridCount);
		const alt_u16 pixelGridInterval = tsd->columnCount / horzGridCount;

		alt_u64 startTimeStamp = timeCode - tsd->durationSecs;	// Timestamp of the leftmost trend coordinate
		
		alt_u64 gridTimeStamp = startTimeStamp;		// Timestamp, aligned by the grid step to the right 
		gridTimeStamp /= timeGridInterval;
		gridTimeStamp *= timeGridInterval;
		gridTimeStamp += timeGridInterval;

		float xPosF = (((float)gridTimeStamp - (float)startTimeStamp) / secsPerColumnF);	// X-coordinate of the first grid float
		alt_u16 xPos = (alt_u16)(xPosF);													// X-coordinate of the first grid

		for (alt_u16 i = 0; i < horzGridCount; i++)
		{
			if (xPos >= width)
			{
				break;
			}

			drawLine(left + xPos, top, left + xPos, top + height, 1u, trend->lineColor);

			if ((trend->showTimeLabels == true) && (outputTime == true))
			{
				alt_u32 tc = (alt_u32)(gridTimeStamp % 86400u);
				alt_u16 h = (alt_u16)(tc / 3600u) % 24u;
				tc %= 3600u;
				alt_u16 m = (alt_u16)(tc / 60u);
				tc %= 60u;
				alt_u16 s = (alt_u16)tc;

				char str[STRING_MAX_LEN];

				if (trend->use24hTimeFormat == true)
				{
					(void)snprintf(str, STRING_MAX_LEN, "%02u:%02u:%02u", h, m, s);
				}
				else
				{
					(void)snprintf(str, STRING_MAX_LEN, "%02u:%02u:%02u %s", (h % 12u), m, s, (h >= 12u) ? "PM" : "AM");
				}

				alt_u16 charsCount = 0;
				alt_u16 msgWidth = stringWidthPx(str, (alt_u8)trend->fontIndex, width, &charsCount);

				alt_u16 timeLabelLeft = left + xPos - (msgWidth / 2u);
				if ((timeLabelLeft >= left) && (timeLabelLeft + msgWidth) < (left + width))
				{
					printStr(timeLabelLeft, top + height, msgWidth, fontHeight, (alt_u8)trend->fontIndex, 0x000000,
						trend->backColor, ALIGN_H_LEFT | ALIGN_V_TOP, str);
				}
			}

			if ((trend->showDateLabels == true) && (outputTime == true))
			{
				LocalTime ts = localTime(gridTimeStamp);

				char str[STRING_MAX_LEN];
				if (trend->use24hTimeFormat == true)
				{
					(void)snprintf(str, STRING_MAX_LEN, "%02u.%02u.%04u", ts.wDay, ts.wMonth + 1u, ts.wYear);
				}
				else
				{
					(void)snprintf(str, STRING_MAX_LEN, "%02u/%02u/%04u", ts.wDay, ts.wMonth + 1u, ts.wYear);
				}

				alt_u16 charsCount = 0u;
				alt_u16 msgWidth = stringWidthPx(str, (alt_u8)trend->fontIndex, width, &charsCount);

				alt_u16 dateLabelLeft = left + xPos - (msgWidth / 2u);
				if ((dateLabelLeft >= left) && (dateLabelLeft + msgWidth) < (left + width))
				{
					printStr(dateLabelLeft, top + height + fontHeight, msgWidth, fontHeight, (alt_u8)trend->fontIndex, 0x000000u,
						trend->backColor, ALIGN_H_LEFT | ALIGN_V_TOP, str);
				}
			}

			// Advance the timestamp
			//
			gridTimeStamp += timeGridInterval;

			// Advance the X-coordinate
			//
			xPos += pixelGridInterval;
		}
	}

	/*
	if (trend->showDateLabels == true && outputTime == true)
	{
		char str[STRING_MAX_LEN];
		if (trend->use24hTimeFormat == true)
		{
			(void)snprintf(str, STRING_MAX_LEN, "%02d.%02d.%04d", tsd->currentTime.wDay, tsd->currentTime.wMonth, tsd->currentTime.wYear);
		}
		else
		{
			(void)snprintf(str, STRING_MAX_LEN, "%02d/%02d/%04d", tsd->currentTime.wMonth, tsd->currentTime.wDay, tsd->currentTime.wYear);
		}

		alt_u16 charsCount = 0;
		alt_u16 msgWidth = stringWidthPx(str, (alt_u8)trend->fontIndex, width, &charsCount);

		alt_u16 dateLabelLeft = left + width - msgWidth;
		printStr(dateLabelLeft, top + height + fontHeight, msgWidth, fontHeight, (alt_u8)trend->fontIndex, 0x000000,
			trend->backColor, ALIGN_H_LEFT | ALIGN_V_TOP, str);
	}*/
}

static void valueString(const VduValue* val,
	enum VduSignalType type,
	alt_u16 decimalPlaces,
	enum TrendScaleType scaleType,
	enum DisplayValueFormat format,
	const char* const units, 
	char* str)
{
	if (format == Auto)
	{
		if (decimalPlaces > 4u)
		{
			format = Exponential;
		}
		else
		{
			format = Decimal;
		}
	}

	if (type == AnalogFloat32)
	{
		char strFormat[STRING_MAX_LEN];

		// For logarithmic scale, print value as 1e+-VALUE
		//
		if (scaleType == Logarithmic)
		{
			if (val->data.f32 >= 0.0f)
			{
				(void)snprintf(strFormat, STRING_MAX_LEN, "1e+%%.%df %%s", decimalPlaces);
			}
			else
			{
				(void)snprintf(strFormat, STRING_MAX_LEN, "1e%%.%df %%s", decimalPlaces);
			}
			(void)snprintf(str, STRING_MAX_LEN, strFormat, val->data.f32, units);
		}
		else
		{
			// For decimal value, print value accrording to format
			//
			switch (format)
			{
			case Decimal:
				(void)snprintf(strFormat, STRING_MAX_LEN, "%%.%df %%s", decimalPlaces);
				break;
			case Exponential:
				(void)snprintf(strFormat, STRING_MAX_LEN, "%%.%de %%s", decimalPlaces);
				break;
			default:
				RN_ASSERT(false);
				return;
			}
			(void)snprintf(str, STRING_MAX_LEN, strFormat, val->data.f32, units);
		}
	}
	else if (type == AnalogSignedInt32)
	{
		// For logarithmic scale, print value as 1e+-VALUE
		//
		if (scaleType == Logarithmic)
		{
			if (val->data.i32 >= 0)
			{
				(void)snprintf(str, STRING_MAX_LEN, "1e+%d %s", (int)val->data.i32, units);
			}
			else
			{
				(void)snprintf(str, STRING_MAX_LEN, "1e-%d %s", (int)val->data.i32, units);
			}
		}
		else
		{
			// For decimal value, print value accrording to format
			(void)snprintf(str, STRING_MAX_LEN, "%d %s", (int)val->data.i32, units);
		}
	}
	else
	{
		RN_ASSERT(false);	// No discretes
	}

}

static void drawScaleGrid(const VduSchemaFileSchemaItemTrend1* const trend, alt_u16 laneIndex, alt_u16 laneCount, alt_u16 vertGridCount,
	alt_u16 left, alt_u16 top, alt_u16 width, alt_u16 height, alt_u16 fontHeight,
	alt_u32 backColor, alt_u16 scaleAreaWidth,
	const struct VduAppSignalParam* const asp,
	const struct TrendSignal* const tsp)
{
	// Print signal description
	//
	char str[STRING_MAX_LEN] = { 0 };

	if ((trend->showSignalIds == true) && (trend->showSignalCaptions == true))
	{
		(void)snprintf(str, STRING_MAX_LEN, "%s - %s", customAppSignalID(asp->signalIndex), signalCaption(asp->signalIndex));
	}
	else if(trend->showSignalIds == true)
	{
		(void)snprintf(str, STRING_MAX_LEN, "%s", customAppSignalID(asp->signalIndex));
	}
	else if (trend->showSignalCaptions == true)
	{
		(void)snprintf(str, STRING_MAX_LEN, "%s", signalCaption(asp->signalIndex));
	}
	else
	{
	}

	if (strlen(str) > 0u)
	{
		printStr(left, top + (laneIndex * fontHeight), width, fontHeight, (alt_u8)trend->fontIndex, tsp->color,
			backColor, ALIGN_H_LEFT | ALIGN_V_TOP, str);
	}

	if (trend->showSignalScales == true)
	{
		// Print signal limits
		//
		alt_u16 col = laneIndex / 2u;
		alt_u16 row = laneIndex % 3u;

		alt_u16 decimalPlaces = (tsp->decimalPlaces == -1) ? asp->decimalPlaces : (alt_u16)tsp->decimalPlaces;

		if (asp->vduSignalType != (alt_u16)Discrete)
		{
			valueString(&tsp->highViewLimit, asp->vduSignalType, decimalPlaces, trend->scaleType, 
				tsp->valueFormat, signalUnit(asp->signalIndex), str);

			printStr(trend->left, top + (row * fontHeight), scaleAreaWidth, fontHeight, (alt_u8)trend->fontIndex, tsp->color,
				backColor, (col == 0u) ? (ALIGN_H_RIGHT | ALIGN_V_TOP) : (ALIGN_H_LEFT | ALIGN_V_TOP), str);

			valueString(&tsp->lowViewLimit, asp->vduSignalType, decimalPlaces, trend->scaleType, 
				tsp->valueFormat, signalUnit(asp->signalIndex), str);

			printStr(trend->left, top + height - ((row + 1u) * fontHeight), scaleAreaWidth, fontHeight, (alt_u8)trend->fontIndex, tsp->color,
				backColor, (col == 0u) ? (ALIGN_H_RIGHT | ALIGN_V_TOP) : (ALIGN_H_LEFT | ALIGN_V_TOP), str);
		}
		else
		{
			printStr(trend->left, top + (row * fontHeight), scaleAreaWidth, fontHeight, (alt_u8)trend->fontIndex, tsp->color,
				backColor, (col == 0u) ? (ALIGN_H_RIGHT | ALIGN_V_TOP) : (ALIGN_H_LEFT | ALIGN_V_TOP), "1");

			printStr(trend->left, top + height - ((row + 1u) * fontHeight), scaleAreaWidth, fontHeight, (alt_u8)trend->fontIndex, tsp->color,
				backColor, (col == 0u) ? (ALIGN_H_RIGHT | ALIGN_V_TOP) : (ALIGN_H_LEFT | ALIGN_V_TOP), "0");
		}

		// Print grid values
		//
		if ((asp->vduSignalType != (alt_u16)Discrete) && (vertGridCount > 0u))
		{
			alt_u16 yStep = height / vertGridCount;

			VduValue valueStep = { 0 };
			VduValue valuePos = { 0 };

			if (asp->vduSignalType == (alt_u16)AnalogFloat32)
			{
				valueStep.data.f32 = (tsp->highViewLimit.data.f32 - tsp->lowViewLimit.data.f32) / (float)vertGridCount;
				valuePos.data.f32 = tsp->highViewLimit.data.f32;

				valueStep.type = (alt_u16)f32;
				valuePos.type = valueStep.type;
			}
			else if (asp->vduSignalType == (alt_u16)AnalogSignedInt32)
			{
				valueStep.data.i32 = (tsp->highViewLimit.data.i32 - tsp->lowViewLimit.data.i32) / (alt_32)vertGridCount;
				valuePos.data.i32 = tsp->highViewLimit.data.i32;

				valueStep.type = (alt_u16)i32;
				valuePos.type = valueStep.type;
			}
			else
			{
			}

			for (alt_u16 i = 1u; i < vertGridCount; i++)
			{
				alt_u16 yPos = yStep * i;

				drawLine(left, top + yPos, left + width, top + yPos, 1, trend->lineColor);

				if (asp->vduSignalType == (alt_u16)AnalogFloat32)
				{
					valuePos.data.f32 -= valueStep.data.f32;
				}
				else
				{
					valuePos.data.i32 -= valueStep.data.i32;
				}

				valueString(&valuePos, asp->vduSignalType, decimalPlaces, trend->scaleType,
					tsp->valueFormat, signalUnit(asp->signalIndex), str);

				alt_u16 textY = top + yPos + (laneIndex * fontHeight) - (laneCount * fontHeight) / 2u;

				printStr(trend->left, textY, scaleAreaWidth, fontHeight, (alt_u8)trend->fontIndex, tsp->color,
					backColor, (col == 0u) ? (ALIGN_H_RIGHT | ALIGN_V_TOP) : (ALIGN_H_LEFT | ALIGN_V_TOP), str);
			}
		}
	}
}

static void drawCurve(const VduSchemaFileSchemaItemTrend1* const trend, alt_u16 trendSignalIndex, alt_u16 left, alt_u16 top, alt_u16 width, alt_u16 height)
{
	const struct TrendSignal* const tsp = &trend->trendSignals[trendSignalIndex];

	const struct TrendSignalData* const tsd = trendSignalData(tsp->trendSignalDataRecordIndex);
	if (tsd == NULL)
	{
		RN_ASSERT(tsd != NULL);
		return;
	}
	if ((tsd->columnCount == 0u) || (tsd->durationSecs < TREND_MIN_DURATION) || (tsd->durationSecs > TREND_MAX_DURATION))
	{
		RN_ASSERT(false);
		return;
	}

	const struct VduAppSignalParam* const asp = appSignalParamByIndex((alt_u16)tsp->appSignalIndex);
	if (asp == NULL)
	{
		RN_ASSERT(asp != NULL);
		return;
	}

	// Calcuate Y-scaling
	//
	float yScale = 1.0f;

	if (asp->vduSignalType == (alt_u16)AnalogFloat32)
	{
		yScale = ((float)height - 1.0f) / (tsp->highViewLimit.data.f32 - tsp->lowViewLimit.data.f32);
	}
	else if (asp->vduSignalType == (alt_u16)AnalogSignedInt32)
	{
		yScale = ((float)height - 1.0f) / ((float)tsp->highViewLimit.data.i32 - (float)tsp->lowViewLimit.data.i32);
	}
	else if (asp->vduSignalType == (alt_u16)Discrete)
	{
		yScale = ((float)height - 1.0f) / (float)(1.1f - (-0.1f));
	}
	else
	{
	}

	// Draw a curve
	//
	for (alt_u16 x = 0; x < width; x++)
	{
		alt_u16 col = tsd->currentColumn + x + 1u;	// leftmost coordinate is the oldest (wrapped) column #1
		if (col >= tsd->columnCount)
		{
			col -= tsd->columnCount;
		}

		struct TrendColumnData column = tsd->columns[col];

		// Convert values to logarithmic
		//
		if ((column.valid == true) && (trend->scaleType == (alt_u16)Logarithmic))
		{
			if (asp->vduSignalType == (alt_u16)AnalogFloat32)
			{
				if ((column.maxValue.data.f32 > 0.0f) && (column.minValue.data.f32 > 0.0f))
				{
					column.maxValue.data.f32 = (float)log10(column.maxValue.data.f32);
					column.minValue.data.f32 = (float)log10(column.minValue.data.f32);
				}
				else if (column.maxValue.data.f32 > 0.0f)
				{
					// The order is important here
					column.minValue.data.f32 = (float)log10(column.maxValue.data.f32);
					column.maxValue.data.f32 = (float)log10(column.maxValue.data.f32);
				}
				else if (column.minValue.data.f32 > 0.0f)
				{
					// The order is important here
					column.maxValue.data.f32 = (float)log10(column.minValue.data.f32);
					column.minValue.data.f32 = (float)log10(column.minValue.data.f32);
				}
				else
				{
					column.valid = false;
				}
			} else if (asp->vduSignalType == (alt_u16)AnalogSignedInt32)
			{
				if ((column.maxValue.data.i32 > 0) && (column.minValue.data.i32 > 0))
				{
					column.maxValue.data.i32 = (alt_32)log10(column.maxValue.data.i32);
					column.minValue.data.i32 = (alt_32)log10(column.minValue.data.i32);
				}
				else if (column.maxValue.data.i32 > 0)
				{
					// The order is important here
					column.minValue.data.i32 = (alt_32)log10(column.maxValue.data.i32);
					column.maxValue.data.i32 = (alt_32)log10(column.maxValue.data.i32);
				}
				else if (column.minValue.data.i32 > 0)
				{
					// The order is important here
					column.maxValue.data.i32 = (alt_32)log10(column.minValue.data.i32);
					column.minValue.data.i32 = (alt_32)log10(column.minValue.data.i32);
				}
				else
				{
					column.valid = false;
				}
			}
			else
			{
			}
		}

		if (column.valid == true)
		{
			// Limit values to view limits
			//
			if (asp->vduSignalType == (alt_u16)AnalogFloat32)
			{
				if (column.maxValue.data.f32 > tsp->highViewLimit.data.f32)
				{
					column.maxValue.data.f32 = tsp->highViewLimit.data.f32;
				}
				if (column.minValue.data.f32 < tsp->lowViewLimit.data.f32)
				{
					column.minValue.data.f32 = tsp->lowViewLimit.data.f32;
				}
			}
			else if (asp->vduSignalType == (alt_u16)AnalogSignedInt32)
			{
				if (column.maxValue.data.i32 > tsp->highViewLimit.data.i32)
				{
					column.maxValue.data.i32 = tsp->highViewLimit.data.i32;
				}
				if (column.minValue.data.i32 < tsp->lowViewLimit.data.i32)
				{
					column.minValue.data.i32 = tsp->lowViewLimit.data.i32;
				}
			}
			else
			{
			}

			// Calculate coordinates
			//
			alt_u16 y1 = 0u;
			alt_u16 y2 = 0u;

			if (asp->vduSignalType == (alt_u16)AnalogFloat32)
			{
				float y1Pos = ((column.minValue.data.f32 - tsp->lowViewLimit.data.f32) * yScale) + 0.5f;
				y1 = top + height - 1u - (alt_u16)(y1Pos);
				float y2Pos = ((column.maxValue.data.f32 - tsp->lowViewLimit.data.f32) * yScale) + 0.5f;
				y2 = top + height - 1u - (alt_u16)(y2Pos);
			}
			else if (asp->vduSignalType == (alt_u16)AnalogSignedInt32)
			{
				float y1Pos = (((float)column.minValue.data.i32 - (float)tsp->lowViewLimit.data.i32) * yScale) + 0.5f;
				y1 = top + height - 1u - (alt_u16)(y1Pos);
				float y2Pos = (((float)column.maxValue.data.i32 - (float)tsp->lowViewLimit.data.i32) * yScale) + 0.5f;
				y2 = top + height - 1u - (alt_u16)(y2Pos);
			}
			else if (asp->vduSignalType == (alt_u16)Discrete)
			{
				float y1Pos = ((float)column.minValue.data.i32 - (-0.1f)) * yScale;
				y1 = top + height - 1u - (alt_u16)(y1Pos);
				float y2Pos = ((float)column.maxValue.data.i32 - (-0.1f)) * yScale;
				y2 = top + height - 1u - (alt_u16)(y2Pos);
			}
			else
			{
			}

			if ((y1 < top) || (y2 < top) || (y1 >= (top + height)) || (y2 >= (top + height)))
			{
				continue;
			}

			if (y1 == y2)
			{
				drawPixel(left + x, y1, tsp->lineWeight, tsp->color);
			}
			else
			{
				drawLine(left + x, y1, left + x, y2, tsp->lineWeight, tsp->color);
			}
		}
	}
}

