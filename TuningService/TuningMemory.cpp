#include <cassert>

#include "../UtilsLib/WUtils.h"

#include "TuningMemory.h"

#include <cstring>
#include "../CommonLib/Types.h"

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

		m_startAddrB = startAddr * WORD_SIZE_IN_BYTES;
		m_frameSizeB = frameSizeW * WORD_SIZE_IN_BYTES;

		if (frameSizeW * framesCount > 1024 * 1024)
		{
			assert(false);
			return;
		}

		allocateMemory();
	}

	bool TuningMemory::updateFrame(int startAddrW, int frameSizeB, const quint8* buffer)
	{
		if (startAddrW < m_startAddrW)
		{
			return false;
		}

		if (frameSizeB != m_frameSizeB)
		{
			return false;
		}

		if ( (startAddrW + m_frameSizeW) > (m_startAddrW + m_frameSizeW * m_framesCount) )
		{
			return false;
		}

		int offsetB = (startAddrW - m_startAddrW) * sizeof(quint16);

		AUTO_LOCK(m_memLock);

		if (m_memory == nullptr)
		{
			return false;
		}

		memcpy(m_memory + offsetB, buffer, m_frameSizeB);

		return true;
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
