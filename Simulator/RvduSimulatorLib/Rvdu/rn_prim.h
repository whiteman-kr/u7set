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
-- *************************************************************
*/

#ifndef RN_PRIM_H
#define RN_PRIM_H

#include <stdbool.h>

#include <alt_types.h>

#include "defs.h"

#define ALIGN_V_TOP 0b100000u
#define ALIGN_V_CENTER 0b10000000u
#define ALIGN_V_BOTTOM 0b1000000u
#define ALIGN_H_LEFT 0b1u
#define ALIGN_H_CENTER 0b100u
#define ALIGN_H_RIGHT 0b10u

struct VduSchemaFileSchemaItem1;

struct Button
{
    alt_u16 code;
    alt_u16 x;
    alt_u16 y;
    alt_u16 w;
    alt_u16 h;
    char text[BUTTON_TEXT_MAX_LEN];
};

struct Rect
{
    alt_u16 x;
    alt_u16 y;
    alt_u16 w;
    alt_u16 h;
    char text[BUTTON_TEXT_MAX_LEN];
};

/*
Function of initalizing video frame memory.
Parameters:
	pVideoBuffer - pointer to CPU video buffer.
    screenWidth, screenHeight - size of the screen in pixels.
*/
void moduleRnPrimInitialize(alt_u32* pVideoBuffer, alt_u16 screenWidth, alt_u16 screenHeight);

/*
Logo display function.
Parameters:
    workFolder - full path to configuration files.
Returns true on success, false on failure.
*/
bool showLogo(const char* logoFile);

//  ---------------------- Video frame primitive drawing functions ---------------------------------------

// If the primitive is outside the screen, these functions write errors to the log.
//
void drawArea(const alt_u16 x, const alt_u16 y, const alt_u16 width, const alt_u16 height, const alt_u32 color);

void drawPixel(alt_u16 x, alt_u16 y, const alt_u16 lineWeight, alt_u32 color);

/*
The function draws a line from the position x1, y1 to, but not including, the point x2, y2.
This behavior is used by drawRect() to avoid double-drawing border pixels.
*/
void drawLine(alt_u16 x1, alt_u16 y1, alt_u16 x2, alt_u16 y2, const alt_u16 lineWeight, alt_u32 color);

void drawRect(const alt_u16 x, const alt_u16 y, const alt_u16 width, const alt_u16 height, const alt_u16 lineWeight, const alt_u32 color);

void drawErrorRect(const alt_u16 x, const alt_u16 y, const alt_u16 width, const alt_u16 height, const alt_u16 lineWeight, const alt_u32 color);

//  ---------------------- Schema items drawing functions ---------------------------------------

// The function draws the Line schema item

void drawItemLine(const struct VduSchemaFileSchemaItem1* const item);

// The function draws the Rect schema item

void drawItemRect(const struct VduSchemaFileSchemaItem1* const item);

// The function draws the Value schema item

void drawItemValue(const struct VduSchemaFileSchemaItem1* const item);

// The functions draw the image for Image of ImageValue schema items.
// If the image is outside the screen, writes errors to the log.
// If the point (0, 0) of the bitmap matches the schema's background color,
// this color is treated as transparent.
//
void drawImage(const struct VduSchemaFileSchemaItem1* const item);

// ----------------------------- String processing functions ---------------------------------------

/* This function returns witdh, in pixels, for the string str printed by font specified int fontIndex.
* Also, in charCount, returns how many symbols of str will fit in specified width.
Parameters:
    str - pointer to the string.
    fontIndex - index of the font.
    width - width, in pixels, of the desired text area.
    charsCount - pointer to the variable that receives the number of symbols.
Return value:
    Returns witdh, in pixels, for the string.
    Also, charsCount - symbols count in width area.
*/
alt_u16 stringWidthPx(const char* const str, const alt_u8 fontIndex, const alt_u16 width, alt_u16* const charsCount);

/*
This function processes the input string, replacing escape sequences with signal information.
The following are available:
    %% - Sign %
    %i - CustomAppSignalID
    %c - Signal caption
    %v - Signal value
    %V - Signal value and units
    %s - +/- signal value
    %S - +/- signal value and units
    %u - unit
    %e - Value in exponential form (1.0e-11)
    %E - Value in exponential form (1.0E-11)
    %x - Value in HEX (only for integer signal type)
    %X - Value in HEX (only for integer signal type)
    %H - 24 Hours
    %h - 12 Hours
	%m - Minutes
	%T - Seconds (big Ticks)
	%t - Milliseconds (small ticks)
	%A - Am/pm
	%Y - Year
	%M - Month
	%D - Day
Parameters:
    sourceStr - pointer to the source string.
    resultStr - pointer to the converted string.
    maxLength - limit on the length of the converted string in characters NOT including the terminating zero.
    signalIndex - signal index.
*/
void prepareStr(const char* const sourceStr, char* resultStr, const alt_u16 maxLength, const alt_u16 signalIndex);

/*
This function outputs the text string to the screen using coordinates, font index, colors and alignment.
*/
void printStr(const alt_u16 x, const alt_u16 y, const alt_u16 width, const alt_u16 height, const alt_u8 fontIndex,
    const alt_u32 color, const alt_u32 backColor,
    const alt_u32 align, const char* const str);

#endif
