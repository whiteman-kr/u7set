#include "rn_kbd.h"
#include "rn_diag.h"
#include "rn_utils.h"

#include <threads.h>

// Private module data
//
static mtx_t m_cs;

static bool m_moduleInitialized = false;
static bool m_keyboardConnected = false;
static alt_u16 m_code = 0xffu;
static bool m_keyPressed = false;
//
// End of Private data

void moduleRnKbdInitialize(void)
{
	if (m_moduleInitialized == true)
	{
		RN_ASSERT(false);
		return;
	}
	m_moduleInitialized = true;
	m_keyboardConnected = true;

	mtx_init(&m_cs, mtx_plain);

}

void moduleRnKbdUninitialize(void)
{
	m_moduleInitialized = false;
	m_keyboardConnected = false;

	mtx_destroy(&m_cs);
}

void setPressedKey(alt_u16 code)
{
	mtx_lock(&m_cs);

	m_code = code;
	m_keyPressed = true;

	mtx_unlock(&m_cs);
}

bool isPressedKey(void)
{
	if (m_moduleInitialized == false)
	{
		RN_ASSERT_MSG(false, "rn_kbd is not initialized.");
		return false;
	}

	if (m_keyboardConnected == false)
	{
		return false;
	}

	mtx_lock(&m_cs);

	bool result = m_keyPressed == true;
	m_keyPressed = false;

	mtx_unlock(&m_cs);

	return result;
}

alt_u16 pressedKey(void)
{
	if (m_moduleInitialized == false)
	{
		RN_ASSERT_MSG(false, "rn_kbd is not initialized.");
		return 0xffu;
	}

	if (m_keyboardConnected == false)
	{
		return 0xffu;
	}

	mtx_lock(&m_cs);

	alt_u16 result = m_code;

	mtx_unlock(&m_cs);

	return result;
}
