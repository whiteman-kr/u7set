/*
Module title: Trends View
(C) 2026 RPC "Radiy".
All rights reserved.
The information presented here is confidential
and is the intellectual property of PJSC "RPC "Radiy".
Any use of this information is strictly
prohibited without the written consent of PJSC "RPC "Radiy".
Module description: Shows trends data.

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

#ifndef RN_TRENDVIEW_H
#define RN_TRENDVIEW_H

#include <alt_types.h>

/*
Function of initalizing trends drawing module.
Parameters:
	screenWidth, screenHeight - size of the screen in pixels.
*/

void moduleRnTrendViewInitialize(alt_u16 screenWidth, alt_u16 screenHeight);

/*
Video frame trend drawing function.
If the trend is outside the screen, these functions write errors to the log.
Parameters:
	trend - pointer to trend object.
*/

void drawTrend(const VduSchemaFileSchemaItemTrend1* const trend);


#endif
