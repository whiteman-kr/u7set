#ifndef DEBUGINSTCOUNTER_H
#define DEBUGINSTCOUNTER_H

#include <QtCore/QMutex>
#include <QtCore/QDebug>
#include <assert.h>
#include <iostream>

namespace VFrame30
{
	template <typename ClassType>
	class DebugInstCounter
	{
	protected:
		DebugInstCounter()
		{
			debugMutex.lock();

			InstanceCounter ++;
			MaxInstanceCounter = std::max(MaxInstanceCounter, InstanceCounter);

			debugMutex.unlock();
		}

		DebugInstCounter(const DebugInstCounter&)
		{
			debugMutex.lock();

			InstanceCounter ++;
			MaxInstanceCounter = std::max(MaxInstanceCounter, InstanceCounter);

			debugMutex.unlock();
		}

		virtual ~DebugInstCounter()
		{
			debugMutex.lock();

			InstanceCounter --;
			assert(InstanceCounter >= 0);

			debugMutex.unlock();
		}

	protected:
		DebugInstCounter& operator= (const DebugInstCounter&)
		{
			// Do nothing
			//
			return *this;
		}

	public:
		static void PrintRefCounter(const wchar_t* className)
		{
			qDebug() << className << "\t" << "InstanceCounter: " << InstanceCounter << "\t\t" << "MaxInstanceCounter: " << MaxInstanceCounter;
		}

	private:
		static QMutex debugMutex;
		static int InstanceCounter;
		static int MaxInstanceCounter;
	};	


	template<typename ClassType> 
	QMutex DebugInstCounter<ClassType>::debugMutex;

	template<typename ClassType> 
	int DebugInstCounter<ClassType>::InstanceCounter = 0;

	template<typename ClassType> 
	int DebugInstCounter<ClassType>::MaxInstanceCounter = 0;

}

#endif // DEBUGINSTCOUNTER_H
