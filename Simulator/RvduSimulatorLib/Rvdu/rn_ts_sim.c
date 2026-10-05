#include "rn_ts.h"
#include "rn_diag.h"
#include "rn_utils.h"

#include <threads.h>

// Private module data
//
static mtx_t m_cs;

static bool m_moduleInitialized = false;

static bool m_tchscrnEvent = false;
static alt_u16 m_tchscrnPosX = 0;
static alt_u16 m_tchscrnPosY = 0;
//
// End of Private data

void moduleRnTsInitialize(alt_u16 screenWidth, alt_u16 screenHeight)
{
	(void)screenWidth;
	(void)screenHeight;

	if (m_moduleInitialized == true)
	{
		RN_ASSERT(false);
		return;
	}
	m_moduleInitialized = true;

	mtx_init(&m_cs, mtx_plain);
}

void moduleRnTsUninitialize(void)
{
	m_moduleInitialized = false;
	m_tchscrnEvent = false;
	m_tchscrnPosX = 0u;
	m_tchscrnPosY = 0u;

	mtx_destroy(&m_cs);
}

void setTouchEvent(alt_u16 x, alt_u16 y)
{
	if (m_moduleInitialized == false)
	{
		RN_ASSERT(false);
		return;
	}

	mtx_lock(&m_cs);

	m_tchscrnEvent = true;
	m_tchscrnPosX = x;
	m_tchscrnPosY = y;

	mtx_unlock(&m_cs);
}

// This function resets the touch event.
//
static void resetTouchEvent(void)
{
	if (m_moduleInitialized == false)
	{
		RN_ASSERT(false);
		return;
	}

	mtx_lock(&m_cs);

	if (m_tchscrnEvent == true)
	{
		m_tchscrnEvent = false;
	}

	mtx_unlock(&m_cs);
}

bool checkTouchZone(const alt_u16 x, const alt_u16 y, const alt_u16 zoneWidth, const alt_u16 zoneHeight)
{
	if (m_moduleInitialized == false)
	{
		RN_ASSERT(false);
		return false;
	}

	mtx_lock(&m_cs);

	bool result = false;

	if (m_tchscrnPosX >= x)
	{
		if (m_tchscrnPosY >= y)
		{
			if (m_tchscrnPosX < (x + zoneWidth))
			{
				if (m_tchscrnPosY < (y + zoneHeight))
				{
					result = true;
				}
			}
		}
	}

	mtx_unlock(&m_cs);

	return result;
}

bool isTouchEvent(void)
{
	if (m_moduleInitialized == false)
	{
		RN_ASSERT(false);
		return false;
	}

	mtx_lock(&m_cs);

	bool result = m_tchscrnEvent == true;

	mtx_unlock(&m_cs);

	resetTouchEvent();

	return result;
}
