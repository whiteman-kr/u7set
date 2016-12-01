#pragma once

#include <QtGlobal>
#include <QMutex>

namespace Tuning
{

	class TuningMemory
	{
	public:
		TuningMemory();
		~TuningMemory();

		void init(int startAddr, int frameSizeW, int framesCount);

		void updateFrame(int startAddrW, int frameSizeB, const quint8* buffer);

	private:
		void allocateMemory();
		void freeMemory();

		QMutex m_memLock;

		int m_startAddrW = 0;
		int m_frameSizeW = 0;
		int m_framesCount = 0;

		quint16* m_memory = nullptr;
	};

}
