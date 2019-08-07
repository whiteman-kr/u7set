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

		Q_ASSERT(false);
		//int to_do_TimeController_inc;

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

	std::chrono::microseconds TimeController::currentTime() const
	{
		QMutexLocker guard(&m_mutex);
		return std::chrono::duration_cast<std::chrono::microseconds>(m_baseTime.time_since_epoch()) + m_duration;
	}

}
