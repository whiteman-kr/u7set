#include "MemWriteMap.h"

MemWriteMap::MemWriteMap(int startAddr, int size, bool checkRewrite) :
	m_startAddr(startAddr),
	m_size(size),
	m_checkRewrite(checkRewrite)
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

		WriteMap::iterator it = m_writeMap.find(writeAddr);

		if (it == m_writeMap.end())
		{
			m_writeMap.insert(std::pair<int, int>(writeAddr, 1));
			continue;
		}

		int writeCount = m_writeMap[writeAddr];

		if (m_checkRewrite == true && writeCount > 0)
		{
			return MemWriteMap::Error::MemRewrite;
		}

		m_writeMap[writeAddr] = writeCount;
	}

	return MemWriteMap::Error::Ok;
}

bool MemWriteMap::addrInRange(int addr)
{
	return addrInRange(addr, 1);
}

bool MemWriteMap::addrInRange(int addr, int size)
{
	return addr >= m_startAddr && addr + size <= m_startAddr + m_size;
}
