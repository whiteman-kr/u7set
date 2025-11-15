#pragma once

#ifdef _WIN32
	#include <Windows.h>
	#include <mmsystem.h>
#endif

class HighResolutionTimerGuard
{
public:
	HighResolutionTimerGuard()
	{
#ifdef _WIN32
		constexpr UINT TIMER_RESOLUTION_MS = 1;
		MMRESULT result = timeBeginPeriod(TIMER_RESOLUTION_MS);
		m_enabled = (result == TIMERR_NOERROR);
#endif
	}

	~HighResolutionTimerGuard() noexcept
	{
#ifdef _WIN32
		if (m_enabled)
		{
			constexpr UINT TIMER_RESOLUTION_MS = 1;
			timeEndPeriod(TIMER_RESOLUTION_MS);
		}
#endif
	}

	HighResolutionTimerGuard(const HighResolutionTimerGuard&) = delete;
	HighResolutionTimerGuard& operator=(const HighResolutionTimerGuard&) = delete;

private:
	bool m_enabled = false;
};
