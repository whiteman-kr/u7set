#include "Ram.h"
#include <cassert>

namespace LmModel
{

	RamAreaInfo::RamAreaInfo(RamAccess access, quint32 offset, quint32 size, QString name) :
		m_name(name),
		m_access(access),
		m_offset(offset),
		m_size(size)
	{
	}

	bool RamAreaInfo::contains(RamAccess access, quint32 offsetW) const
	{
		if ((static_cast<int>(m_access) & static_cast<int>(access)) != 0 &&
			offsetW >= m_offset &&
			offsetW < (m_offset + m_size))
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

	QString RamAreaInfo::name() const
	{
		return m_name;
	}

	RamAccess RamAreaInfo::access() const
	{
		return m_access;
	}

	quint32 RamAreaInfo::offset() const
	{
		return m_offset;
	}

	quint32 RamAreaInfo::size() const
	{
		return m_size;
	}

	RamArea::RamArea(RamAccess access, quint32 offset, quint32 size, QString name) :
		RamAreaInfo(access, offset, size, name)
	{
		m_data.fill(0, size * 2);

		return;
	}

	bool RamArea::writeBit(quint32 offsetW, quint32 data, quint32 bitNo)
	{
		if (contains(RamAccess::WriteOnly, offsetW) == false ||
			bitNo >= 16)
		{
			return false;
		}

		data &= 0x01;
		int byteOffset = (offsetW - offset()) * 2;

		if (byteOffset >= m_data.size())
		{
			assert(byteOffset < m_data.size());
			return false;
		}

		quint16 word = qFromBigEndian<quint16>(m_data.constData() + byteOffset);

		word &= ~(0x01 << bitNo);
		word |= (data << bitNo);

		qToBigEndian<quint16>(word, m_data.data() + byteOffset);

		return true;
	}


	Ram::Ram()
	{
	}

	void Ram::reset()
	{
		m_memoryAreas.clear();
		return;
	}

	bool Ram::addMemoryArea(RamAccess access, quint32 offsetW, quint32 sizeW, QString name)
	{
		std::shared_ptr<RamArea> ramArea = std::make_shared<RamArea>(access, offsetW, sizeW, name);

		// Check that new memory area is not overlapped with existsings
		//
		for (const auto& area : m_memoryAreas)
		{
			if (area->overlapped(access, offsetW, sizeW) == true)
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

	bool Ram::writeBit(quint32 offsetW, quint32 data, quint32 bitNo)
	{
		RamArea* area = memoryArea(RamAccess::WriteOnly, offsetW);
		if (area == nullptr)
		{
			return false;
		}

		bool ok = area->writeBit(offsetW, data, bitNo);

		return ok;
	}

	RamArea* Ram::memoryArea(RamAccess access, quint32 offsetW)
	{
		for (std::shared_ptr<RamArea> area : m_memoryAreas)
		{
			if (area->contains(access, offsetW) == true)
			{
				return area.get();
			}
		}

		return nullptr;
	}

}
