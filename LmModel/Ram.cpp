#include "Ram.h"

namespace LmModel
{

	RamAreaInfo::RamAreaInfo(RamAccess access, quint32 offset, quint32 size, QString name) :
		m_name(name),
		m_access(access),
		m_offset(offset),
		m_size(size)
	{
	}

	bool RamAreaInfo::contains(RamAccess access, quint32 offset) const
	{
		if ((static_cast<int>(m_access) & static_cast<int>(access)) != 0 &&
			offset >= m_offset &&
			offset < (m_offset + m_size))
		{
			return true;
		}

		return false;
	}

	bool RamAreaInfo::overlapped(RamAccess access, quint32 offset, quint32 size) const
	{
		quint32 startA = m_offset;
		quint32 endA = m_offset + m_size;

		quint32 startB = offset;
		quint32 endB = offset + size;

		if ((static_cast<int>(m_access) & static_cast<int>(access)) != 0 &&
			startA < endB &&
			startB < endA)
		{
			return true;
		}

		return false;
	}

	RamArea::RamArea(RamAccess access, quint32 offset, quint32 size, QString name) :
		RamAreaInfo(access, offset, size, name)
	{
		m_data.fill(0, size * 2);

		return;
	}

	Ram::Ram()
	{
	}

	void Ram::reset()
	{
		m_memoryAreas.clear();
		return;
	}

	bool Ram::addMemoryArea(RamAccess access, quint32 offset, quint32 size, QString name)
	{
		std::shared_ptr<RamArea> ramArea = std::make_shared<RamArea>(access, offset, size, name);

		// Check that new memory area is not overlapped with existsings
		//
		for (const auto& area : m_memoryAreas)
		{
			if (area->overlapped(access, offset, size) == true)
			{
				// Area with this type of access and addresses already exists
				//
				return false;
			}
		}

		// --
		//
		m_memoryAreas.push_back(ramArea);

		return true;
	}


}
