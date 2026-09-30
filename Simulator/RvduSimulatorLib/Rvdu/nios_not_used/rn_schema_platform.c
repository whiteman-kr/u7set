/*
Module title: Schemas Processing helper functions.
(C) 2026 RPC "Radiy".
All rights reserved.
The information presented here is confidential
and is the intellectual property of PJSC "RPC "Radiy".
Any use of this information is strictly
prohibited without the written consent of PJSC "RPC "Radiy".
Module description: Provides the helper functionality for displaying RVDU schemas.

-- *************************************************************
-- Version v1.00
-- Date:	30.09.2026.
-- Designed by: Vintenko B.U.
-- Reviewed by: Nesterenko M.V.

-- *************************************************************
*/

#include <stdbool.h>


static bool m_popUpTerminateFlag = false;

void popUpSchemaSetTerminate(void)
{
	m_popUpTerminateFlag = true;
}

void popUpSchemaResetTerminate(void)
{ 
	m_popUpTerminateFlag = false; 
}

bool popUpSchemaTerminate(void)
{ 
	return m_popUpTerminateFlag; 
}

