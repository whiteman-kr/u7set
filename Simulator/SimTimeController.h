#ifndef TIMECONTROLLER_H
#define TIMECONTROLLER_H

#include <chrono>
#include <QObject>
#include <QMutex>


namespace Sim
{
	// Class for time sinchronization in simulation process
	//
	class TimeController : public QObject
	{
		Q_OBJECT

	public:
		explicit TimeController(QObject* parent = nullptr);

	public:
		void reset();
		void inc();

		std::chrono::microseconds duration() const;
		std::chrono::system_clock::time_point baseTime() const;
		std::chrono::microseconds currentTime() const;			// return BaseTime + Duration

	private:
		mutable QMutex m_mutex;

		std::chrono::microseconds m_duration{0};				// How long simulation is taking, usecs
		std::chrono::system_clock::time_point m_baseTime = std::chrono::system_clock::now();
	};

}

#endif // TIMECONTROLLER_H
