#include "MemWriteMap.h"

MemWriteMap::MemWriteMap(int startAddr, int size, bool checkRewrite) :
	m_startAddr(startAddr),
	m_size(size),
	m_checkRewrite(checkRewrite)
{
}

MemWriteMap::~MemWriteMap()
{
}

MemWriteMap::Error MemWriteMap::write(int addr, int size)
{
	if (addrInRange(addr, size) == false)
	{
		return MemWriteMap::Error::OutOfRange;
	}

	for(int i = 0; i < size; i++ )
	{
		int writeAddr = addr + i;

		int writeCount = m_writeMap.value(writeAddr, 0);

		if (writeCount == 0)
		{
			m_writeMap.insert(writeAddr, 1);
			continue;
		}

		if (m_checkRewrite == true)
		{
			return MemWriteMap::Error::MemRewrite;
		}

		writeCount++;

		m_writeMap.insert(writeAddr, writeCount);
	}

	return MemWriteMap::Error::Ok;
}

void MemWriteMap::getNonWrittenAreas(AreaList* areaList)
{
	if (areaList == nullptr)
	{
		assert(false);
		return;
	}

	Area* area = nullptr;

	for(int i = 0; i < m_size; i++ )
	{
		int writeAddr = m_startAddr + i;

		if (m_writeMap.contains(writeAddr) == false)
		{
			// addr is not written
			//
			if (area == nullptr)
			{
				// start new non written area
				//
				area = new Area(writeAddr, 1);
			}
			else
			{
				// extend current non written area
				//
				area->second++;
			}

			continue;
		}

		// addr is written

		if (area != nullptr)
		{
			// finalize current non written area
			//
			areaList->append(*area);

			delete area;

			area = nullptr;
		}
	}

	if (area != nullptr)
	{
		// finalize last non written area
		//
		areaList->append(*area);

		delete area;
	}
}

bool MemWriteMap::addrInRange(int addr)
{
	return addrInRange(addr, 1);
}

bool MemWriteMap::addrInRange(int addr, int size)
{
	return addr >= m_startAddr && addr + size <= m_startAddr + m_size;
}
