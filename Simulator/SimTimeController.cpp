#include "SimTimeController.h"

namespace Sim
{

	TimeController::TimeController(QObject* parent) :
		QObject(parent)
	{
	}
	
	void TimeController::reset()
	{
		using namespace std::chrono_literals;

		QMutexLocker guard(&m_mutex);

		m_duration = 0us;
		m_baseTime = std::chrono::system_clock::now();

		return;
	}
	
	void TimeController::inc()
	{
		QMutexLocker guard(&m_mutex);

		int to_do_TimeController_inc;

		return;
	}

	std::chrono::microseconds TimeController::duration() const
	{
		QMutexLocker guard(&m_mutex);
		return m_duration;
	}

	std::chrono::system_clock::time_point TimeController::baseTime() const
	{
		QMutexLocker guard(&m_mutex);
		return m_baseTime;
	}

	std::chrono::system_clock::time_point TimeController::currentTime() const
	{
		QMutexLocker guard(&m_mutex);
		return m_baseTime + m_duration;
	}

}
