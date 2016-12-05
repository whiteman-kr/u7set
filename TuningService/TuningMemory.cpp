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

		int frameSizeW = frameSizeB / sizeof(quint16);

		if (frameSizeW != m_frameSizeW)
		{
			assert(false);
			return;
		}

		if ( (startAddrW + frameSizeW) > (m_startAddrW + m_frameSizeW * m_framesCount) )
		{
			assert(false);
			return;
		}

		int offset = startAddrW - m_startAddrW;

		AUTO_LOCK(m_memLock);

		if (m_memory == nullptr)
		{
			assert(false);
			return;
		}

		memcpy(m_memory + offset, buffer, m_frameSizeW * sizeof(quint16));
	}


	void TuningMemory::allocateMemory()
	{
		AUTO_LOCK(m_memLock)

		if (m_memory != nullptr)
		{
			assert(false);
			return;
		}

		m_memory = new quint16 [m_frameSizeW * m_framesCount];
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
