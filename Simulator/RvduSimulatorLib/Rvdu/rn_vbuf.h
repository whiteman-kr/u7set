/*
Module title: Video Buffer Operation.
(C) 2026 RPC "Radiy".
All rights reserved.
The information presented here is confidential
and is the intellectual property of PJSC "RPC "Radiy".
Any use of this information is strictly
prohibited without the written consent of PJSC "RPC "Radiy".
Module description: Manages video buffers for rendering and display output.

-- *************************************************************
-- Version v1.00
-- Date:	31,03,2026.
-- Designed by: Mazyr S.M. https://jira.radiy.com/browse/RADIX-3966
-- Reviewed by: Vintenko B.U.

-- Changes: RADIX-3966, rn_vbuf initial update

-- *************************************************************
-- Version v1.01
-- Date:	09,04,2026.
-- Designed by: Vintenko B.U. https://jira.radiy.com/browse/RADIX-3966, https://jira.radiy.com/browse/RADIX-3969
-- Reviewed by: Nesterenko M.V.

-- Changes: RADIX-3966, rn_vbuf module refactoring
-- Add log error for showLogo function
-- Add vary screen resolution support

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

#ifndef RN_VBUF_H
#define RN_VBUF_H

#include <alt_types.h>

#include <stdbool.h>

// Functions for working with video frame memory (array)

/*
Function of initalizing video frame memory.
Parameters:
    pVideoBuffer - pointer to CPU video buffer.
    pHdmiBuffer - pointer to HDMI video buffer.
    screenWidth, screenHeight - size of the screen in pixels.
*/
void moduleRnVbufInitialize(alt_u32* const pVideoBuffer, alt_u32* const pHdmiBuffer, alt_u16 screenWidth, alt_u16 screenHeight);

void moduleRnVbufUninitialize(void);

/*
Function of clearing video frame memory.
Parameters:
    backroundColor - video frame fill color.
Returns true on success, false when module is not initialized.
*/
bool writeVideoBuffer(const alt_u32 backgroundColor);

/*
Function of recording a video memory page with a video frame array
Returns true on success, false when module is not initialized.
*/
bool pushVideoBuffer(void);

#endif
