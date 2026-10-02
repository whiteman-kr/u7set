/*
Module title: Primitives Draw
(C) 2026 RPC "Radiy".
All rights reserved.
The information presented here is confidential
and is the intellectual property of PJSC "RPC "Radiy".
Any use of this information is strictly
prohibited without the written consent of PJSC "RPC "Radiy".
Module description: Renders basic graphical primitives.

-- *************************************************************
-- Version v1.00
-- Date:	31,03,2026.
-- Designed by: Mazyr S.M. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Vintenko B.U.

-- Changes: RADIX-3966, rn_prim initial update

-- *************************************************************
-- Version v1.01
-- Date:	20,04,2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966, https://jira.radiy.com/browse/RADIX-3969
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-3966, rn_prim module refactoring
-- Add arbitrary lines for drawLine
-- Add checking limits of different coordinates
-- Add checking limits of string lengths
-- Add vary screen resolution support

-- Changes: RADIX-3969, add logging and debugging support
-- Add log errors and messages

-- *************************************************************

-- Version v1.02
-- Date:	09,05,2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-4009
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-4009, adding API support for working with signals by AppSignalIDs
-- RADIX-3973: add drawPixel function
-- RADIX-3974, add 64-bit architecture support
-- add extra codes: %S, %V (value+units), %H: hour 24, %h: hour 12, %m: min, %T: sec (big Ticks), %t: ms (small ticks), %A: Am/pm, %Y: year, %M: month, %D: day

-- *************************************************************

-- Version v1.03
-- Date:	28,07,2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-3966, code is made MISRA-2025 compatible
-- add using Bresenhams algorithm for drawing thick arbitrary lines

-- *************************************************************

-- Version v1.04
-- Date:	08,04,2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-3966
-- areas, lines, rectangles, and text lines can be partially placed outside of the screen. If they are fully outside, an error is generated.
--- Logo size can be smaller than screen size
-- *************************************************************
*/

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "fat_filelib.h"

#include "rn_prim.h"
#include "rn_fofp.h"
#include "rn_utils.h"
#include "rn_scfp.h"
#include "rn_sifp.h"
#include "rn_diag.h"

#include "defs.h"

// Private module data
//
static alt_u32* m_pVideoBuffer = NULL;	// EMIF_0_ECC_CORE_BASE
static alt_u16 m_screenWidth = 0;
static alt_u16 m_screenHeight = 0;
static bool m_moduleInitialized = false;
//
// End of Private data

void moduleRnPrimInitialize(alt_u32* pVideoBuffer, alt_u16 screenWidth, alt_u16 screenHeight)
{
	if (m_moduleInitialized == true)
	{
		RN_ASSERT(false);
		return;
	}
	m_moduleInitialized = true;

	m_pVideoBuffer = pVideoBuffer;
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	return;
}

bool showLogo(const char* logoFile)
{
	if (logoFile == NULL)
	{
		RN_ASSERT(logoFile != NULL);
		return false;
	}

	if (m_moduleInitialized == false)
	{
		RN_ASSERT_MSG(false, "showLogo: module is not initialized, moduleRnPrimInitialize shall be executed!.");
		return false;
	}

#define MAX_LOGO_ROW_SIZE (((MAX_SCREEN_WIDTH * 3u) + 3u) & (~3u))

	void* file = fl_fopen(logoFile, "rb");
	if (file == NULL)
	{
		logError(OpenFileError, "showLogo: file open error.");
		return false;
	}

	bool readOk = true;

	// Read the BMP header
	//
	struct BMPFileHeader bmpHeader = {0};
	struct BMPInfoHeader bmpInfo = { 0 };

	if (freadBigData((alt_u8*)&bmpHeader, sizeof(struct BMPFileHeader), file) == false)
	{
		logError(ReadFileError, "showLogo: file header read error.");
		readOk = false;
	}

	if (readOk == true)
	{
		if (freadBigData((alt_u8*)&bmpInfo, sizeof(struct BMPInfoHeader), file) == false)
		{
			logError(ReadFileError, "showLogo: file info read error.");
			readOk = false;
		}
	}

	// Check the BMP header
	//
	if (readOk == true)
	{
		if ((bmpHeader.bfType != 0x4d42u) || // BM
			(bmpInfo.biSize != 40u) ||
			(bmpInfo.biPlanes != 1u) ||
			(bmpInfo.biBitCount != 24u) ||
			(bmpInfo.biCompression != 0u))
		{
			logError(FileFormatError, "showLogo: file format error.");
			readOk = false;
		}
	}

	if (readOk == true)
	{
		if (((alt_u16)bmpInfo.biWidth > m_screenWidth) ||
			((alt_u16)bmpInfo.biHeight > m_screenHeight))
		{
			logError(FileFormatError, "showLogo: image size error.");
			readOk = false;
		}
	}

	alt_u16 left = (m_screenWidth - (alt_u16)bmpInfo.biWidth) / 2u;
	alt_u16 top = (m_screenHeight - (alt_u16)bmpInfo.biHeight) / 2u;
	alt_u16 width = (alt_u16)bmpInfo.biWidth;
	alt_u16 height = (alt_u16)bmpInfo.biHeight;

	// Read the BMP data
	//
	if (readOk == true)
	{
		static alt_u8 row[MAX_LOGO_ROW_SIZE] = { 0 };

		const alt_u16 rowSize = ((width * 3u) + 3u) & (~3u);

		for (alt_u16 y = 0u; y < height; y++)
		{
			if (freadBigData(row, rowSize, file) == false)
			{
				logError(UnexpectedEOF, "showLogo: file read error.");
				
				readOk = false;
				break;
			}

			const alt_u8* rowPtr = row;

			for (alt_u16 x = 0; x < width; x++)
			{
				alt_u8 b = *rowPtr;
				rowPtr++;
				alt_u8 g = *rowPtr;
				rowPtr++;
				alt_u8 r = *rowPtr;
				rowPtr++;

				alt_u32 data = 0xff000000u | ((alt_u32)r << 16u) | ((alt_u32)g << 8u) | (alt_u32)b;

				alt_u16 i = x + left;
				alt_u16 j = height - y + top - 1;

				if ((i < m_screenWidth) && (j < m_screenHeight))
				{
					*(m_pVideoBuffer + i + (j * m_screenWidth)) = data;
				}
			}
		}
	}

	(void)fl_fclose(file);

	return readOk;
}

static bool checkScreenArea(const alt_u16 x1, const alt_u16 y1, const alt_u16 x2, const alt_u16 y2)
{
	if (((x1 > m_screenWidth) && (x2 > m_screenWidth)) ||
		((y1 > m_screenHeight) && (y2 > m_screenHeight)))
	{
		return false;
	}

	return true;
}

void drawArea(const alt_u16 x, const alt_u16 y, const alt_u16 width, const alt_u16 height, const alt_u32 color)
{
	if (m_moduleInitialized == false)
	{
		RN_ASSERT_MSG(false, "rn_prim is not initialized.");
		return;
	}

	if ((width == 0u) || (height == 0u))
	{
		// Rect is invisible
		//
		return;
	}

	if (checkScreenArea(x, y, x + width, y + height) == false)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "drawArea: try to draw by invalid coordinates: (x = %d, y = %d, w = %d, h = %d).", (alt_16)x, (alt_16)y, (alt_16)width, (alt_16)height);
		logWarning(DrawError, msg);
		return;
	}

	for (alt_u16 y1 = 0; y1 < height; y1++)
	{
		alt_u16 j = y + y1;

		for (alt_u16 x1 = 0; x1 < width; x1++)
		{
			alt_u16 i = x + x1;

			if ((i >= m_screenWidth) || (j >= m_screenHeight))
			{
				continue;
			}

			*(m_pVideoBuffer + i + (j * m_screenWidth)) = color;
		}
	}
	return;
}

static alt_u32 float_to_u32(float value)
{
	if (value <= 0.0f)
	{
		return 0u;
	}

	if (value >= (float)UINT32_MAX)
	{
		return UINT32_MAX;
	}

	return (alt_u32)value;
}

static inline void drawPixelAlpha(alt_u16 x, alt_u16 y, float alpha, alt_u32 color)
{
	if (m_moduleInitialized == false)
	{
		RN_ASSERT_MSG(false, "rn_prim is not initialized.");
		return;
	}

	if ((x >= m_screenWidth) || (y >= m_screenHeight))
	{
		return;
	}

	alt_u32* pixel = m_pVideoBuffer + x + (y * m_screenWidth);

	alt_u32 dst = *pixel;

	alt_u32 sr = (color >> 16u) & 0xFFu;
	alt_u32 sg = (color >> 8u) & 0xFFu;
	alt_u32 sb = (color) & 0xFFu;

	alt_u32 dr = (dst >> 16u) & 0xFFu;
	alt_u32 dg = (dst >> 8u) & 0xFFu;
	alt_u32 db = (dst) & 0xFFu;

	float rf = ((float)sr * alpha) + ((float)dr * (1.0f - alpha));
	float gf = ((float)sg * alpha) + ((float)dg * (1.0f - alpha));
	float bf = ((float)sb * alpha) + ((float)db * (1.0f - alpha));

	alt_u32 r = float_to_u32(rf);
	alt_u32 g = float_to_u32(gf);
	alt_u32 b = float_to_u32(bf);

	*pixel = (r << 16u) | (g << 8u) | b;
}

void drawPixel(alt_u16 x, alt_u16 y, const alt_u16 lineWeight, alt_u32 color)
{
	if (m_moduleInitialized == false)
	{
		RN_ASSERT_MSG(false, "rn_prim is not initialized.");
		return;
	}

	if ((x >= m_screenWidth) || (y >= m_screenHeight))
	{
		return;
	}

	for (alt_16 j = ((alt_16)y - (((alt_16)lineWeight - 1) / 2)); j <= ((alt_16)y + ((alt_16)lineWeight / 2)); j++)
	{
		for (alt_16 i = ((alt_16)x - (((alt_16)lineWeight - 1) / 2)); i <= ((alt_16)x + ((alt_16)lineWeight / 2)); i++)
		{
			if ((i < 0) || (j < 0) ||
				(i >= (alt_16)m_screenWidth) || (j >= (alt_16)m_screenHeight))
			{
				continue;
			}

			*(m_pVideoBuffer + (alt_u16)i + ((alt_u16)j * m_screenWidth)) = color;
		}
	}

	return;
}

static alt_u16 abs16(alt_16 value)
{
	if (value < 0)
	{
		return (alt_u16)(-value);
	}

	return (alt_u16)value;
}

/*
 * Draw a vertical span.
 *
 * x       - X coordinate
 * y       - center Y coordinate
 * width   - span width
 */
static void drawVerticalSpan(
	alt_16 x,
	alt_16 y,
	alt_u16 width,
	alt_u32 color)
{
	alt_16 half;
	alt_16 start;
	alt_16 end;
	alt_16 yy;

	half = (alt_16)width / 2;

	if ((width & 1u) != 0u)
	{
		start = (alt_16)(y - half);
		end = (alt_16)(y + half);
	}
	else
	{
		start = (alt_16)(y - half);
		end = (alt_16)(y + half - 1);
	}

	for (yy = start; yy <= end; yy++)
	{
		if ((x >= 0) && (yy >= 0))
		{
			drawPixel((alt_u16)x, (alt_u16)yy, 1, color);
		}
	}
}


/*
 * Draw a horizontal span.
 */
static void drawHorizontalSpan(alt_16 x, alt_16 y, alt_u16 width, alt_u32 color)
{
	alt_16 half;
	alt_16 start;
	alt_16 end;
	alt_16 xx;

	half = (alt_16)width / 2;

	if ((width & 1u) != 0u)
	{
		start = (alt_16)(x - half);
		end = (alt_16)(x + half);
	}
	else
	{
		start = (alt_16)(x - half);
		end = (alt_16)(x + half - 1);
	}

	for (xx = start; xx <= end; xx++)
	{
		if ((xx >= 0) && (y >= 0))
		{
			drawPixel((alt_u16)xx, (alt_u16)y, 1, color);
		}
	}
}


/*
 * Draw an arbitrary-angle thick line.
 *
 * Integer Bresenham algorithm.
 *
 * lineWeight:
 *     1  -> 1 pixel
 *     2  -> 2 pixels
 *     16 -> 16 pixels
 *     etc.
 */
static void drawBresenhamLine(alt_u16 x1, alt_u16 y1, alt_u16 x2, alt_u16 y2, alt_u16 lineWeight, alt_u32 color)
{
	alt_16 x0;
	alt_16 y0;
	alt_16 xEnd;
	alt_16 yEnd;

	alt_16 dx;
	alt_16 dy;

	alt_16 sx;
	alt_16 sy;

	alt_16 err;

	alt_u16 absDx;
	alt_u16 absDy;

	if (lineWeight == 0u)
	{
		return;
	}

	x0 = (alt_16)x1;
	y0 = (alt_16)y1;

	xEnd = (alt_16)x2;
	yEnd = (alt_16)y2;

	dx = (alt_16)(xEnd - x0);
	dy = (alt_16)(yEnd - y0);

	absDx = abs16(dx);
	absDy = abs16(dy);

	sx = (dx >= 0) ? 1 : -1;
	sy = (dy >= 0) ? 1 : -1;

	/*
	 * Special case: single point.
	 */
	if ((absDx == 0u) && (absDy == 0u))
	{
		drawVerticalSpan(
			x0,
			y0,
			lineWeight,
			color);

		return;
	}

	/*
	 * Bresenham error term.
	 *
	 * Use the standard form:
	 *
	 *     err = dx - dy
	 *
	 * where dx and dy are positive magnitudes.
	 */
	err = (alt_16)absDx - (alt_16)absDy;

	/*
	 * The thickness is applied in the direction
	 * perpendicular to the dominant axis.
	 *
	 * For shallow lines:
	 *
	 *     ---------
	 *     |||||||||
	 *     ---------
	 *
	 * use vertical spans.
	 *
	 * For steep lines:
	 *
	 *        ||
	 *        ||
	 *     =========
	 *     =========
	 *
	 * use horizontal spans.
	 */
	while (true)
	{
		if (absDx >= absDy)
		{
			/*
			 * Mostly horizontal.
			 *
			 * A vertical span is perpendicular
			 * to the dominant direction.
			 */
			drawVerticalSpan(
				x0,
				y0,
				lineWeight,
				color);
		}
		else
		{
			/*
			 * Mostly vertical.
			 *
			 * A horizontal span is perpendicular
			 * to the dominant direction.
			 */
			drawHorizontalSpan(
				x0,
				y0,
				lineWeight,
				color);
		}

		if ((x0 == xEnd) && (y0 == yEnd))
		{
			break;
		}

		{
			alt_16 e2 = (alt_16)(2 * err);

			if (e2 > -(alt_16)absDy)
			{
				err = (alt_16)(err - (alt_16)absDy);
				x0 = (alt_16)(x0 + sx);
			}

			if (e2 < (alt_16)absDx)
			{
				err = (alt_16)(err + (alt_16)absDx);
				y0 = (alt_16)(y0 + sy);
			}
		}
	}
}

static inline float fpart(float x)
{
	return x - floorf(x);
}

static inline float rfpart(float x)
{
	return 1.0f - fpart(x);
}

static inline int ipart(float x)
{
	return (int)x;
}

static void drawXiaolinLine(alt_u16 x1, alt_u16 y1, alt_u16 x2, alt_u16 y2, alt_u16 lineWeight, alt_u32 color)
{
	for (alt_16 i = -(alt_16)lineWeight / 2; i <= (alt_16)lineWeight / 2; i++)
	{
		alt_16 x0 = (alt_16)x1;
		alt_16 y0 = (alt_16)y1 + i;
		alt_16 x1i = (alt_16)x2;
		alt_16 y1i = (alt_16)y2 + i;

		alt_16 steep = (alt_16)abs(y1i - y0) > (alt_16)abs(x1i - x0) ? 1 : 0;

		if (steep != 0)
		{
			alt_16 tmp;
			tmp = x0; x0 = y0; y0 = tmp;
			tmp = x1i; x1i = y1i; y1i = tmp;
		}

		if (x0 > x1i)
		{
			alt_16 tmp;
			tmp = x0; x0 = x1i; x1i = tmp;
			tmp = y0; y0 = y1i; y1i = tmp;
		}

		float dx = (float)x1i - (float)x0;
		float dy = (float)y1i - (float)y0;

		float gradient = (dx == 0.0f) ? 1.0f : (dy / dx);

		float ya = (float)y0;

		for (int xa = x0; xa <= x1i; xa++)
		{
			alt_16 y_int = (alt_16)ipart(ya);
			float frac = fpart(ya);

			if (steep != 0)
			{
				drawPixelAlpha((alt_u16)y_int, (alt_u16)xa, rfpart(ya), color);
				drawPixelAlpha((alt_u16)y_int + 1u, (alt_u16)xa, frac, color);
			}
			else
			{
				drawPixelAlpha((alt_u16)xa, (alt_u16)y_int, rfpart(ya), color);
				drawPixelAlpha((alt_u16)xa, (alt_u16)y_int + 1u, frac, color);
			}

			ya += gradient;
		}
	}
}

void drawLine(alt_u16 x1, alt_u16 y1, alt_u16 x2, alt_u16 y2, const alt_u16 lineWeight, alt_u32 color)
{
	if (m_moduleInitialized == false)
	{
		RN_ASSERT_MSG(false, "rn_prim is not initialized.");
		return;
	}

	if (checkScreenArea(x1, y1, x2, y2) == false)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "drawLine: try to draw by invalid coordinates: (x1 = %d, y1 = %d, x2 = %d, y2 = %d).", (alt_16)x1, (alt_16)y1, (alt_16)x2, (alt_16)y2);
		logWarning(DrawError, msg);
		return;
	}

	// Calculate width and height of the line
	//
	alt_u16 w = (alt_u16)abs(x2 - x1) + 1u;
	alt_u16 h = (alt_u16)abs(y2 - y1) + 1u;

	if (h == 1u)	// Horizontal line
	{
		for (alt_16 j = ((alt_16)y1 - (((alt_16)lineWeight - 1) / 2)); j <= ((alt_16)y1 + ((alt_16)lineWeight / 2)); j++)
		{
			for (alt_16 i = (alt_16)x1; i != (alt_16)x2; i = ((alt_16)x1 <= (alt_16)x2) ? (i + 1) : (i - 1))
			{
				if ((i < 0) || (j < 0) ||
					(i >= (alt_16)m_screenWidth) || (j >= (alt_16)m_screenHeight))
				{
					continue;
				}

				*(m_pVideoBuffer + (alt_u16)i + ((alt_u16)j * m_screenWidth)) = color;
			}
		}
	}
	else if (w == 1u)	// Vertical line
	{
		for (alt_16 i = ((alt_16)x1 - (((alt_16)lineWeight - 1) / 2)); i <= ((alt_16)x1 + ((alt_16)lineWeight / 2)); i++)
		{
			for (alt_16 j = (alt_16)y1; j != (alt_16)y2; j = ((alt_16)y1 <= (alt_16)y2) ? (j + 1) : (j - 1))
			{
				if ((i < 0) || (j < 0) ||
					(i >= (alt_16)m_screenWidth) || (j >= (alt_16)m_screenHeight))
				{
					continue;
				}

				*(m_pVideoBuffer + (alt_u16)i + ((alt_u16)j * m_screenWidth)) = color;
			}
		}
	}
	else if (w == h)	// Diagonal line
	{
		alt_32 st = 0;

		for (alt_16 k = 0; k <= (alt_16)lineWeight; k++)
		{
			alt_16 i = (alt_16)x1;
			alt_16 j = (alt_16)y1;

			while (i != (alt_16)x2)
			{
				if ((i >= 0) &&
					((j + st) >= 0) &&
					(i < (alt_16)m_screenWidth) &&
					((j + st) < (alt_16)m_screenHeight))
				{
					alt_32 ofs = (alt_32)i + ((alt_32)j + st) * (alt_32)m_screenWidth;
					*(m_pVideoBuffer + ofs) = color;
				}

				if (y1 < y2)
				{
					j++;
				}
				else
				{
					j--;
				}

				if (x1 < x2)
				{
					i++;
				}
				else
				{
					i--;
				}
			}
			st = (st < 1) ? (abs(st) + 1) : (-st);
		}
	}
	else
	{
		if (lineWeight > 1u)
		{
			drawBresenhamLine(x1, y1, x2, y2, lineWeight, color);
		}
		else
		{
			// Arbitrary lines with line weight = 1 using Xiaolin Wu�s algorithm
			drawXiaolinLine(x1, y1, x2, y2, lineWeight, color);
		}
	}
	return;
}

void drawRect(const alt_u16 x, const alt_u16 y, const alt_u16 width, const alt_u16 height, const alt_u16 lineWeight, const alt_u32 color)
{
	if (m_moduleInitialized == false)
	{
		RN_ASSERT_MSG(false, "rn_prim is not initialized.");
		return;
	}

	if ((width == 0u) || (height == 0u))
	{
		// Rect is invisible
		//
		return;
	}

	if (checkScreenArea(x, y, x + width, y + height) == false)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "drawRect: try to draw by invalid coordinates: (x = %d, y = %d, w = %d, h = %d).", (alt_16)x, (alt_16)y, (alt_16)width, (alt_16)height);
		logWarning(DrawError, msg);
		return;
	}

	if (checkScreenArea(x - ((lineWeight - 1u) / 2u), y, x + width + (lineWeight / 2u) + 1u, y) == true)
	{
		drawLine(x - ((lineWeight - 1u) / 2u), y, x + width + (lineWeight / 2u) + 1u, y, lineWeight, color);
	}

	if (checkScreenArea(x - ((lineWeight - 1u) / 2u), y + height, x + width + (lineWeight / 2u) + 1u, y + height) == true)
	{
		drawLine(x - ((lineWeight - 1u) / 2u), y + height, x + width + (lineWeight / 2u) + 1u, y + height, lineWeight, color);
	}

	if (checkScreenArea(x, y, x, y + height) == true)
	{
		drawLine(x, y, x, y + height, lineWeight, color);
	}

	if (checkScreenArea(x + width, y, x + width, y + height) == true)
	{
		drawLine(x + width, y, x + width, y + height, lineWeight, color);
	}

	return;
}

void drawErrorRect(const alt_u16 x, const alt_u16 y, const alt_u16 width, const alt_u16 height, const alt_u16 lineWeight, const alt_u32 color)
{
	if ((x < m_screenWidth) && (y < m_screenHeight) && ((x + width) < m_screenWidth) && ((y + height) < m_screenHeight))
	{
		drawRect(x, y, width, height, lineWeight, color);
	}
}

/*
The symbol data consists of an array of 8-bit values.
The high-order six bits contain the number of pixels with the specified transparency.
The low-order two bits contain information about the pixel's transparency.
11 corresponds to 0-x percent transparency;
10 corresponds to x - x percent transparency;
01 corresponds to x - x percent transparency;
00 corresponds to x - 100 percent transparency.
*/
static inline alt_u16 drawSymbolUtf8(const alt_u16 x, const alt_u16 y,
	const alt_u32 color, const alt_u32 backColor,
	const struct VduSymbolHeader* const currSymbol,
	const alt_u8* fontDataAddr)
{
	if (checkScreenArea(x, y, x + currSymbol->width, y + currSymbol->height) == false)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "drawSymbolUtf8: try to draw by invalid coordinates: (x = %d, y = %d, w = %d, h = %d).",
			(alt_16)x, (alt_16)y, (alt_16)currSymbol->width, (alt_16)currSymbol->height);
		logWarning(DrawError, msg);
		return 0;
	}

	alt_u32 color66a =
		((((color & 0xff0000u) * 2u / 3u) + ((backColor & 0xff0000u) / 3u)) & 0xff0000u) |
		((((color & 0xff00u) * 2u / 3u) + ((backColor & 0xff00u) / 3u)) & 0xff00u) |
		((((color & 0xffu) * 2u / 3u) + ((backColor & 0xffu) / 3u)));

	alt_u32 color33a =
		((((color & 0xff0000u) / 3u) + ((backColor & 0xff0000u) * 2u / 3u)) & 0xff0000u) |
		((((color & 0xff00u) / 3u) + ((backColor & 0xff00u) * 2u / 3u)) & 0xff00u) |
		((((color & 0xffu) / 3u) + ((backColor & 0xffu) * 2u / 3u)));

	const alt_u8* symbolarray = (const alt_u8*)fontDataAddr + currSymbol->offset;

	alt_u8 num_7_2 = *symbolarray >> 2u;
	alt_u8 alpha_1_0 = *symbolarray & 0x3u;
	alt_u16 last_row = y + currSymbol->height;

	for (alt_u16 i = y; i < last_row; i++)
	{
		alt_u32* p = m_pVideoBuffer + x + (i * m_screenWidth);
		for (alt_u16 j = 0; j < currSymbol->width; j++)
		{
			if (alpha_1_0 != 0u) // if (alpha_1_0) ???
			{
				if (alpha_1_0 == 3u)
				{
					*p = color;
				}
				else if (alpha_1_0 == 2u)
				{
					*p = color66a;
				}
				else
				{
					*p = color33a;
				}
			}
			num_7_2--;
			if (num_7_2 == 0u)
			{
				symbolarray++;
				num_7_2 = *symbolarray >> 2u;
				alpha_1_0 = *symbolarray & 0x3u;
			}
			p++;
		}
	}
	return currSymbol->width;
}

void drawItemLine(const struct VduSchemaFileSchemaItem1* const item)
{
	drawLine(
		item->line.x1,
		item->line.y1,
		item->line.x2,
		item->line.y2,
		item->line.weight,
		item->line.color);
}

void drawItemRect(const struct VduSchemaFileSchemaItem1* const item)
{
	alt_u32 backColor = item->rect.fillColor;
	if (item->rect.fill == true)
	{
		if ((backColor & 0xff000000u) != 0u)
		{
			drawArea(
				item->rect.left,
				item->rect.top,
				item->rect.width,
				item->rect.height,
				backColor);
		}
	}

	if (item->rect.drawRect == true)
	{
		drawRect(
			item->rect.left,
			item->rect.top,
			item->rect.width,
			item->rect.height,
			item->rect.weight,
			item->rect.lineColor);
	}

	size_t textLength = strlen(item->rect.text);
	if (textLength > 0u)
	{
		printStr(
			item->rect.left,
			item->rect.top,
			item->rect.width,
			item->rect.height,
			(alt_u8)item->rect.fontIndex,
			item->rect.textColor,
			backColor,
			item->rect.align,
			item->rect.text);
	}
}

void drawItemValue(const struct VduSchemaFileSchemaItem1* const item)
{
	char outstr[STRING_MAX_LEN + 1u/*for ending zero*/] = { 0 };
	alt_u32 backColor = item->value.fillColor;
	if ((backColor & 0xff000000u) != 0u)
	{
		drawArea(
			item->value.left,
			item->value.top,
			item->value.width,
			item->value.height,
			backColor);
	}

	if (item->value.drawRect == true)
	{
		drawRect(
			item->value.left,
			item->value.top,
			item->value.width,
			item->value.height,
			item->value.weight,
			item->value.lineColor);
	}

	if (item->value.appSignalCount > 0u)
	{
		alt_u16 signalIndex = schemaItemValueAppSignalIndex(&item->value, 0/*index*/);
		if (signalIndex == INVALID_SIGNAL_INDEX)
		{
			RN_ASSERT(false);
		}
		else
		{
			prepareStr(item->value.text, outstr, STRING_MAX_LEN, signalIndex);
		}
	}
	else
	{
		prepareStr(item->value.text, outstr, STRING_MAX_LEN, INVALID_SIGNAL_INDEX);
	}

	printStr(
		item->value.left,
		item->value.top,
		item->value.width,
		item->value.height,
		(alt_u8)item->value.fontIndex,
		item->value.textColor,
		backColor,
		item->value.align,
		outstr);
}

void drawImage(const struct VduSchemaFileSchemaItem1* const item)
{
	if (m_moduleInitialized == false)
	{
		RN_ASSERT_MSG(false, "rn_prim is not initialized.");
		return;
	}

	if (item == NULL)
	{
		RN_ASSERT(item != NULL);
		return;
	}

	alt_u16 left = 0;
	alt_u16 top = 0;
	alt_u16 width = 0;
	alt_u16 height = 0;
	alt_u32 hash = 0;

	// Processing VduFileSchemaItemImage
	//
	if (item->itemType == TYPE_IMAGE)
	{
		left = item->image.left;
		top = item->image.top;
		width = item->image.width;
		height = item->image.height;
		hash = item->image.imageHash;
	}
	// Processing VduFileSchemaItemImageValue
	//
	else if (item->itemType == TYPE_IMAGEVALUE)
	{
		left = item->imageValue.left;
		top = item->imageValue.top;
		width = item->imageValue.width;
		height = item->imageValue.height;
		hash = item->imageValue.images[item->imageValue.currentImage].imageHash;
	}
	else
	{
		return;
	}

	if (checkScreenArea(left, top, left + width, top + height) == false)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "drawImage: try to draw by invalid coordinates: (x = %d, y = %d, w = %d, h = %d).",
			(alt_16)left, (alt_16)top, (alt_16)width, (alt_16)height);
		logWarning(DrawError, msg);
		return;
	}

	const alt_u8* bmpData = bitmapData(hash);
	if (bmpData == NULL)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "drawImage: image with hash %x was not found.", (unsigned int)hash);
		logWarning(ImageWasNotFound, msg);
		return;
	}

	static const alt_u8 BMP_HEADER_SIZE = 54;
	bmpData += BMP_HEADER_SIZE;

	alt_u16 rowSize = ((width * 3u) + 3u) & ~3u;

	bool transparentBackground = false; // use bitmap point(0,0) as transparent color
	bool firstPoint = true;
	alt_u32 firstPointColor = 0;

	for (alt_u16 y = 0; y < height; y++)
	{
		const alt_u8* rowPtr = bmpData + (rowSize * (height - y - 1u));

		for (alt_u16 x = 0; x < width; x++)
		{
			alt_u8 b = *rowPtr;
			rowPtr++;
			alt_u8 g = *rowPtr;
			rowPtr++;
			alt_u8 r = *rowPtr;
			rowPtr++;

			alt_u32 data = ((alt_u32)r << 16u) | ((alt_u32)g << 8u) | (alt_u32)b;

			// Determine if color at (0, 0) matches the schema's background.
			//
			if (firstPoint == true)
			{
				firstPoint = false;
				firstPointColor = data;

				if ((vduCurrentSchema()->schemaFile->schemaProperties.backgroundColor & 0x00ffffffu) == firstPointColor)
				{
					transparentBackground = true;
				}
			}

			// If point is not transparent, draw it
			//
			if ((transparentBackground == false) || (data != firstPointColor))
			{
				alt_u16 i = x + left;
				alt_u16 j = y + top;

				if ((i < m_screenWidth) && (j < m_screenHeight))
				{
					*(m_pVideoBuffer + i + (j * m_screenWidth)) = data;
				}
			}
		}
	}
	return;
}

alt_u16 stringWidthPx(const char* const str, const alt_u8 fontIndex, const alt_u16 width, alt_u16* const charsCount)
{
	if (width == 0u)
	{
		// Rect is invisible
		//
		return 0;
	}

	struct VduSymbolHeader currSymbol;

	alt_u16 wstrMax = 0;
	alt_u16 wstrCurrent = 0;
	*charsCount = 0;

	const alt_u8* pSymbol1 = (const alt_u8*)str;
	while (*pSymbol1 != 0)
	{
		if (*pSymbol1 == '\n')
		{
			if (wstrCurrent > wstrMax)
			{
				wstrMax = wstrCurrent;
			}
			wstrCurrent = 0;
			*charsCount = 0;
		}

		alt_u8 utfByteSize = findUtf8Character(fontIndex, pSymbol1, &currSymbol);
		if (utfByteSize != 0u)
		{
			pSymbol1 += utfByteSize;
			wstrCurrent += currSymbol.width;

			if (wstrCurrent < width)
			{
				(*charsCount)++;
			}
		}
		else
		{
			pSymbol1++;
		}
	}

	if (wstrCurrent > wstrMax)
	{
		wstrMax = wstrCurrent;
	}

	return wstrMax;
}

void prepareStr(const char* const sourceStr, char* resultStr, const alt_u16 maxLength, const alt_u16 signalIndex)
{
	if ((sourceStr == NULL) || (resultStr == NULL) || (maxLength == 0u))
	{
		RN_ASSERT_MSG(false, "prepareStr: invalid input parameters.");
		return;
	}

#define FORMAT_STR_MAX_LENGHT 16u
	char convertFormat[FORMAT_STR_MAX_LENGHT];

	char* resultStrLocal = resultStr;
	*resultStrLocal = '\0';

	alt_u16 charsRest = maxLength;
	const char* sourceStrLocal = sourceStr;
	while (*sourceStrLocal != '\0')
	{
		if (*sourceStrLocal == '%')  // Search for a character '%'
		{
			sourceStrLocal++;

			alt_u16 len = 0;

			char code = *sourceStrLocal;

			// Process codes that do not require signal index and signal param

			switch (code)
			{
			case '\0':
			{
				*resultStrLocal = '\0';
				return;
			}

			case '%':
			{
				*resultStrLocal = *sourceStrLocal;
				len = 1;
			}
			break;

			case 'H':	// hour 24
			case 'h':	// hour 12
			case 'm':	// min
			case 'T':	// sec	(big Ticks)
			case 't':	// ms	(small ticks)
			case 'A':	// Am/pm

			case 'Y':	// year
			case 'M':	// month
			case 'D':	// day
			{
				static LocalTime tm;
				tm = currentTime();

				switch (code)
				{
				case 'H':	(void)snprintf(resultStrLocal, maxLength, "%02u", tm.wHour);										break;
				case 'h':	(void)snprintf(resultStrLocal, maxLength, "%02u", (tm.wHour > 12u) ? (tm.wHour - 12u) : tm.wHour);	break;
				case 'm':	(void)snprintf(resultStrLocal, maxLength, "%02u", tm.wMinute);										break;
				case 'T':	(void)snprintf(resultStrLocal, maxLength, "%02u", tm.wSecond);										break;
				case 't':	(void)snprintf(resultStrLocal, maxLength, "%02u", tm.wMilliseconds);								break;
				case 'A':	(void)snprintf(resultStrLocal, maxLength, "%s", (tm.wHour > 12u) ? "PM" : "AM");					break;

				case 'Y':	(void)snprintf(resultStrLocal, maxLength, "%02u", tm.wYear);	break;
				case 'M':	(void)snprintf(resultStrLocal, maxLength, "%02u", tm.wMonth);	break;
				case 'D':	(void)snprintf(resultStrLocal, maxLength, "%02u", tm.wDay);	break;
				default:
				{
					RN_ASSERT(false);
					return;
				}
				}

				len = (alt_u16)strlen(resultStrLocal);
			}
			break;
			default:
			{
				len = 0u;
			}
			break;
			}

			// Next switch requires signal index and signal param

			if (len == 0u)
			{
				if (signalIndex != INVALID_SIGNAL_INDEX)
				{
					const struct VduAppSignalParam* signalParam = appSignalParamByIndex(signalIndex);
					if (signalParam == NULL)
					{
						char msg[STRING_MAX_LEN];
						(void)snprintf(msg, STRING_MAX_LEN, "prepareStr: signal with index %u not found while processing a macro '%%%c'.", (unsigned int)signalIndex, code);
						logWarning(SignalWasNotFound, msg);
						return;
					}

					switch (code)
					{
					case 'i':
					{
						len = (alt_u16)strlen(customAppSignalID(signalIndex));

						if (len < charsRest)
						{
							(void)strncat(resultStrLocal, customAppSignalID(signalIndex), maxLength);
						}
						else
						{
							*resultStrLocal = '\0';	// Make string null-terminated
							return;
						}
					}
					break;

					case 'c':
					{
						len = (alt_u16)strlen(signalCaption(signalIndex));

						if (len < charsRest)
						{
							(void)strncat(resultStrLocal, signalCaption(signalIndex), maxLength);
						}
						else
						{
							*resultStrLocal = '\0';	// Make string null-terminated
							return;
						}
					}
					break;

					case 'u':
					{
						len = (alt_u16)strlen(signalUnit(signalIndex));

						if (len < charsRest)
						{
							(void)strncat(resultStrLocal, signalUnit(signalIndex), maxLength);
						}
						else
						{
							*resultStrLocal = '\0';	// Make string null-terminated
							return;
						}
					}
					break;

					case 'v':
					case 's':
					{
						if (signalParam == NULL)
						{
							RN_ASSERT(signalParam != NULL);
							return;
						}

						// VduSignalType AnalogFloat32
						//
						if (signalParam->vduSignalType == (alt_u16)AnalogFloat32)
						{
							(void)snprintf(convertFormat, FORMAT_STR_MAX_LENGHT, (code == 'v') ? "%%.%df" /*for %v*/ : "%%+.%df" /*for %s*/, signalParam->decimalPlaces);

							float res = 0.0f;
							if ((signalAvailableByIndex(signalIndex) == false) || (readSignalValueFloatByIndex(signalIndex, &res) == false))
							{
								(void)snprintf(resultStrLocal, maxLength, "???");
							}
							else
							{
								(void)snprintf(resultStrLocal, maxLength, convertFormat, res);
							}
						}
						// VduSignalType discrete or integer
						//
						else
						{
							alt_32 res = 0;
							if ((signalAvailableByIndex(signalIndex) == false) || (readSignalValueIntByIndex(signalIndex, &res) == false))
							{
								(void)snprintf(resultStrLocal, maxLength, "???");
							}
							else
							{
								(void)snprintf(resultStrLocal, maxLength, (code == 'v') ? "%d" /*for %v*/ : "%+d" /*for %s*/, (int)res);
							}
						}

						len = (alt_u16)strlen(resultStrLocal);
					}
					break;

					case 'V':
					case 'S':
					{
						if (signalParam == NULL)
						{
							RN_ASSERT(signalParam != NULL);
							return;
						}

						// VduSignalType AnalogFloat32
						//
						if (signalParam->vduSignalType == (alt_u16)AnalogFloat32)
						{
							(void)snprintf(convertFormat, FORMAT_STR_MAX_LENGHT, (code == 'V') ? "%%.%df %%s" /*for %V*/ : "%%+.%df %%s" /*for %S*/, signalParam->decimalPlaces);

							float res = 0.0;
							if ((signalAvailableByIndex(signalIndex) == false) || (readSignalValueFloatByIndex(signalIndex, &res) == false))
							{
								(void)snprintf(resultStrLocal, maxLength, "???");
							}
							else
							{
								(void)snprintf(resultStrLocal, maxLength, convertFormat, res, signalUnit(signalIndex));
							}
						}
						// VduSignalType discrete or integer
						//
						else
						{
							alt_32 res = 0;
							if ((signalAvailableByIndex(signalIndex) == false) || (readSignalValueIntByIndex(signalIndex, &res) == false))
							{
								(void)snprintf(resultStrLocal, maxLength, "???");
							}
							else
							{
								(void)snprintf(resultStrLocal, maxLength, (code == 'V') ? "%d %s" /*for %V*/ : "%+d %s" /*for %S*/, (int)res, signalUnit(signalIndex));
							}
						}

						len = (alt_u16)strlen(resultStrLocal);
					}
					break;

					case 'e':
					case 'E':
					{
						if (signalParam == NULL)
						{
							RN_ASSERT(signalParam != NULL);
							return;
						}

						(void)snprintf(convertFormat, FORMAT_STR_MAX_LENGHT, (code == 'e') ? "%%.%de" : "%%.%dE", signalParam->decimalPlaces);

						// VduSignalType AnalogFloat32
						//
						if (signalParam->vduSignalType == (alt_u16)AnalogFloat32)
						{
							float res = 0.0;
							if ((signalAvailableByIndex(signalIndex) == false) || (readSignalValueFloatByIndex(signalIndex, &res) == false))
							{
								(void)snprintf(resultStrLocal, maxLength, "???");
							}
							else
							{
								(void)snprintf(resultStrLocal, maxLength, convertFormat, res);
							}
						}
						// VduSignalType discrete or integer
						//
						else
						{
							alt_32 res = 0;
							if ((signalAvailableByIndex(signalIndex) == false) || (readSignalValueIntByIndex(signalIndex, &res) == false))
							{
								(void)snprintf(resultStrLocal, maxLength, "???");
							}
							else
							{
								(void)snprintf(resultStrLocal, maxLength, convertFormat, res);
							}
						}

						len = (alt_u16)strlen(resultStrLocal);
					}
					break;

					case 'x':
					case 'X':
					{
						if (signalParam == NULL)
						{
							RN_ASSERT(signalParam != NULL);
							return;
						}

						// VduSignalType not AnalogFloat32
						//
						if (signalParam->vduSignalType != (alt_u16)AnalogFloat32)
						{
							alt_32 res = 0;
							if ((signalAvailableByIndex(signalIndex) == false) || (readSignalValueIntByIndex(signalIndex, &res) == false))
							{
								(void)snprintf(resultStrLocal, maxLength, "???");
							}
							else
							{
								(void)snprintf(resultStrLocal, maxLength, (code == 'x') ? "%.8x" : "%.8X", (unsigned int)res);
							}
						}

						len = (alt_u16)strlen(resultStrLocal);
					}
					break;

					default:
					{
						// Unknown code, output the code
						//
						(void)snprintf(resultStrLocal, maxLength, "%%%c", code);
						len = (alt_u16)strlen(resultStrLocal);
					}
					break;
					} // switch
				}
				else
				{
					// SignalIndex == INVALID_INDEX, output the code
					//
					(void)snprintf(resultStrLocal, maxLength, "%%%c", code);
					len = (alt_u16)strlen(resultStrLocal);
				}
			}

			// Advance the source pointer by one character
			//
			sourceStrLocal++;

			// Advance the target pointer to the last char
			//
			for (alt_u16 i = 0; i < len; i++)
			{
				resultStrLocal++;
				charsRest--;
				if (charsRest == 0u)
				{
					*resultStrLocal = '\0';	// Make string null-terminated
					return;
				}
			}
		}
		// Its not a character '%' - just copy
		//
		else
		{
			*resultStrLocal = *sourceStrLocal;
			resultStrLocal++;
			sourceStrLocal++;
			charsRest--;
			if (charsRest == 0u)
			{
				*resultStrLocal = '\0';	// Make string null-terminated
				return;
			}
		}
	}
	*resultStrLocal = '\0';	// Make string null-terminated
	return;
}

void printStr(const alt_u16 x, const alt_u16 y, const alt_u16 width, const alt_u16 height, const alt_u8 fontIndex,
	const alt_u32 color, const alt_u32 backColor,
	const alt_u32 align, const char* const str)
{
	if (m_moduleInitialized == false)
	{
		RN_ASSERT_MSG(false, "rn_prim is not initialized.");
		return;
	}

	if ((width == 0u) || (height == 0u))
	{
		// Rect is invisible
		//
		return;
	}

	if (checkScreenArea(x, y, x + width, y + height) == false)
	{
		char msg[STRING_MAX_LEN];
		(void)snprintf(msg, STRING_MAX_LEN, "printStr: try to draw by invalid coordinates: '%s', (x = %d, y = %d, w = %d, h = %d).",
			str, (alt_16)x, (alt_16)y, (alt_16)width, (alt_16)height);
		logWarning(DrawError, msg);
		return;
	}


	const alt_u8* const fontDataAddr = fontData(fontIndex);
	if (fontDataAddr == NULL)
	{
		RN_ASSERT(fontDataAddr != NULL);
		return;
	}

	struct VduSymbolHeader currSymbol;
	(void)findUtf8Character(fontIndex, (const alt_u8*)str, &currSymbol);

	// Counting the number of lines in a text
	//
	alt_u16 rowsQuantity = 1;
	for (const alt_u8* pSymbol = (const alt_u8*)str; *pSymbol != 0; pSymbol++)
	{
		if (*pSymbol == 0xa)
		{
			rowsQuantity++;
		}
	}

	//Calculating the vertical position of text
	//
	alt_u16 placementAcisY;
	if ((align & ALIGN_V_TOP) != 0u) // if ( align & ALIGN_V_TOP)
	{
		placementAcisY = y;
	}
	else
	{
		if ((align & ALIGN_V_BOTTOM) != 0u) // if (align & ALIGN_V_BOTTOM) ???
		{
			placementAcisY = y + height - (currSymbol.height * rowsQuantity);
		}
		else
		{
			placementAcisY = y + ((height - (currSymbol.height * rowsQuantity)) / 2u);
		}
	}

	// Output text line by line
	//
	alt_u16 placementAcisX;
	alt_u8 utfByteSize;
	const alt_u8* pSymbol1 = (const alt_u8*)str;
	const alt_u8* pSymbol2 = (const alt_u8*)str;
	while ((rowsQuantity--) != 0)
	{
		// Calculating the horizontal position of text
		//
		if ((align & ALIGN_H_LEFT) != 0u) // if (align & ALIGN_H_LEFT) ???
		{
			placementAcisX = x;
		}
		else
		{
			alt_u16 wstr = 0;
			while (((*pSymbol1 != 0) && (*pSymbol1 != 0xa)) == true)
			{
				utfByteSize = findUtf8Character(fontIndex, pSymbol1, &currSymbol);
				if (utfByteSize != 0u)
				{
					pSymbol1 += utfByteSize;
					wstr += currSymbol.width;

					if (wstr > width)
					{
						char msg[STRING_MAX_LEN];
						(void)snprintf(msg, STRING_MAX_LEN, "printStr: text is too long: '%s', (x = %d, y = %d, w = %d, h = %d).",
							str, x, y, width, height);
						logWarning(DrawError, msg);
						return;
					}
				}
				else
				{
					pSymbol1++;
				}
			}
			pSymbol1++;

			if ((align & ALIGN_H_RIGHT) != 0u) // if (align & ALIGN_H_RIGHT) ???
			{
				placementAcisX = x + width - wstr;
			}
			else
			{
				placementAcisX = x + ((width - wstr) / 2u);
			}
		}

		while (((*pSymbol2 != 0) && (*pSymbol2 != 0xa)) == true)
		{
			utfByteSize = findUtf8Character(fontIndex, pSymbol2, &currSymbol);
			if (utfByteSize != 0u)
			{
				pSymbol2 += utfByteSize;

				if (((placementAcisX + currSymbol.width) >= m_screenWidth) ||
					((placementAcisY + currSymbol.height) >= m_screenHeight))
				{
					// move to the next symbol, do not draw current by invalid coordinates
					//
					placementAcisX += currSymbol.width;
				}
				else
				{
					placementAcisX += drawSymbolUtf8(placementAcisX, placementAcisY, color, backColor, &currSymbol, fontDataAddr);
				}
			}
			else
			{
				pSymbol2++;
			}
		}
		pSymbol2++;

		placementAcisY += currSymbol.height;
	}
	return;
}