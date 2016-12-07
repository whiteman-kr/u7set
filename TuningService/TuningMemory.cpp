#include <cassert>

#include "../lib/WUtils.h"

#include "TuningMemory.h"

namespace Tuning
{


	// ----------------------------------------------------------------------------------
	//
	// TuningMemory class implementation
	//
	// ----------------------------------------------------------------------------------

	TuningMemory::TuningMemory()
	{
	}


	TuningMemory::~TuningMemory()
	{
		freeMemory();
	}


	void TuningMemory::init(int startAddr, int frameSizeW, int framesCount)
	{
		m_startAddrW = startAddr;
		m_frameSizeW = frameSizeW;
		m_framesCount = framesCount;

		m_startAddrB = startAddr * sizeof(quint16);
		m_frameSizeB = frameSizeW * sizeof(quint16);

		if (frameSizeW * framesCount > 1024 * 1024)
		{
			assert(false);
			return;
		}

		allocateMemory();
	}


	void TuningMemory::updateFrame(int startAddrW, int frameSizeB, const quint8* buffer)
	{
		if (startAddrW < m_startAddrW)
		{
			assert(false);
			return;
		}

		if (frameSizeB != m_frameSizeB)
		{
			assert(false);
			return;
		}

		if ( (startAddrW + m_frameSizeW) > (m_startAddrW + m_frameSizeW * m_framesCount) )
		{
			assert(false);
			return;
		}

		int offsetB = (startAddrW - m_startAddrW) * sizeof(quint16);

		AUTO_LOCK(m_memLock);

		if (m_memory == nullptr)
		{
			assert(false);
			return;
		}

		memcpy(m_memory + offsetB, buffer, m_frameSizeB);
	}


	void TuningMemory::allocateMemory()
	{
		AUTO_LOCK(m_memLock)

		if (m_memory != nullptr)
		{
			assert(false);
			return;
		}

		m_memory = new quint8 [m_frameSizeB * m_framesCount];
	}


	void TuningMemory::freeMemory()
	{
		AUTO_LOCK(m_memLock)

		if (m_memory == nullptr)
		{
			return;
		}

		delete [] m_memory;
		m_memory = nullptr;
	}

}
