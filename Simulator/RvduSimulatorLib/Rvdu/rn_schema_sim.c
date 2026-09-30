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
#include <threads.h>
#include <assert.h>

static bool m_moduleInitialized = false;
static mtx_t m_schemaMutex;

void moduleRnSchemaInitialize(void)
{
	if (m_moduleInitialized == true)
	{
		assert(false);
		return;
	}
	m_moduleInitialized = true;

	// Initialize critical section
	mtx_init(&m_schemaMutex, mtx_plain);
}

void moduleRnSchemaUninitialize(void)
{
	mtx_destroy(&m_schemaMutex);

	m_moduleInitialized = false;
}

static bool m_popUpTerminateFlag = false;

void popUpSchemaSetTerminate(void)
{
	mtx_lock(&m_schemaMutex);
	
	m_popUpTerminateFlag = true;
	
	mtx_unlock(&m_schemaMutex);
}

void popUpSchemaResetTerminate(void)
{ 
	mtx_lock(&m_schemaMutex);

	m_popUpTerminateFlag = false;

	mtx_unlock(&m_schemaMutex);
}

bool popUpSchemaTerminate(void)
{ 
	mtx_lock(&m_schemaMutex);

	bool result = m_popUpTerminateFlag;
	
	mtx_unlock(&m_schemaMutex);

	return result;
}

