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

	bool RamArea::writeBit(quint32 offsetW, quint16 data, quint32 bitNo)
	{
		if (contains(RamAccess::Write, offsetW) == false ||
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

	bool RamArea::readBit(quint32 offsetW, quint32 bitNo, quint16* data) const
	{
		if (contains(RamAccess::Read, offsetW) == false ||
			bitNo >= 16 ||
			data == nullptr)
		{
			return false;
		}

		int byteOffset = (offsetW - offset()) * 2;
		if (byteOffset >= m_data.size())
		{
			assert(byteOffset < m_data.size());
			return false;
		}

		quint16 word = qFromBigEndian<quint16>(m_data.constData() + byteOffset);

		word >>=  bitNo;
		word &= 0x01;

		*data = word;

		return true;
	}

	bool RamArea::writeWord(quint32 offsetW, quint16 data)
	{
		return writeData<quint16>(offsetW, data);
	}

	bool RamArea::readWord(quint32 offsetW, quint16* data) const
	{
		return readData<quint16>(offsetW, data);
	}

	bool RamArea::writeDword(quint32 offsetW, quint32 data)
	{
		return writeData<quint32>(offsetW, data);
	}

	bool RamArea::readDword(quint32 offsetW, quint32* data) const
	{
		return readData<quint32>(offsetW, data);
	}

	bool RamArea::writeSignedInt(quint32 offsetW, qint32 data)
	{
		return writeData<qint32>(offsetW, data);
	}

	bool RamArea::readSignedInt(quint32 offsetW, qint32* data) const
	{
		return readData<qint32>(offsetW, data);
	}

	template<typename TYPE>
	bool RamArea::writeData(quint32 offsetW, TYPE data)
	{
		int byteOffset = (offsetW - offset()) * 2;
		if (byteOffset < 0 ||
			byteOffset >= m_data.size() - sizeof(TYPE))
		{
			assert(byteOffset >= 0 &&
				   byteOffset - sizeof(TYPE) <= m_data.size());
			return false;
		}

		qToBigEndian<TYPE>(data, m_data.data() + byteOffset);
		return true;
	}

	template<typename TYPE>
	bool RamArea::readData(quint32 offsetW, TYPE* data) const
	{
		if (data == nullptr)
		{
			assert(data);
			return false;
		}

		int byteOffset = (offsetW - offset()) * 2;
		if (byteOffset < 0 ||
			byteOffset >= m_data.size() - sizeof(TYPE))
		{
			assert(byteOffset >= 0 &&
				   byteOffset - sizeof(TYPE) <= m_data.size());
			return false;
		}

		*data = qFromBigEndian<TYPE>(m_data.constData() + byteOffset);
		return true;
	}

	Ram::Ram()
	{
	}

	Ram::Ram(const Ram& that)
	{
		m_memoryAreas.reserve(that.m_memoryAreas.size());

		for (std::shared_ptr<RamArea> thatArea : that.m_memoryAreas)
		{
			std::shared_ptr<RamArea> area = std::make_shared<RamArea>(*thatArea.get());
			m_memoryAreas.push_back(area);
		}
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

	std::vector<RamAreaInfo> Ram::memoryAreasInfo() const
	{
		std::vector<RamAreaInfo> result;
		for (const auto& area : m_memoryAreas)
		{
			result.emplace_back(*area.get());
		}

		return result;
	}

	RamAreaInfo Ram::memoryAreaInfo(QString name) const
	{
		for (const auto& area : m_memoryAreas)
		{
			if (area->name() == name)
			{
				return *area.get();
			}
		}

		return RamAreaInfo();
	}

	RamAreaInfo Ram::memoryAreaInfo(int index) const
	{
		if (index < 0 || static_cast<size_t>(index) >= m_memoryAreas.size())
		{
			return RamAreaInfo();
		}

		return *(m_memoryAreas[index].get());
	}

	bool Ram::writeBit(quint32 offsetW, quint32 data, quint32 bitNo)
	{
		RamArea* area = memoryArea(RamAccess::Write, offsetW);
		if (area == nullptr)
		{
			return false;
		}

		bool ok = area->writeBit(offsetW, data, bitNo);
		return ok;
	}

	bool Ram::readBit(quint32 offsetW, quint32 bitNo, quint16* data) const
	{
		const RamArea* area = memoryArea(RamAccess::Read, offsetW);
		if (area == nullptr)
		{
			return false;
		}

		bool ok = area->readBit(offsetW, bitNo, data);
		return ok;
	}

	bool Ram::writeWord(quint32 offsetW, quint16 data)
	{
		RamArea* area = memoryArea(RamAccess::Write, offsetW);
		if (area == nullptr)
		{
			return false;
		}

		bool ok = area->writeWord(offsetW, data);
		return ok;
	}

	bool Ram::readWord(quint32 offsetW, quint16* data) const
	{
		const RamArea* area = memoryArea(RamAccess::Read, offsetW);
		if (area == nullptr)
		{
			return false;
		}

		bool ok = area->readWord(offsetW, data);
		return ok;
	}

	bool Ram::writeDword(quint32 offsetW, quint32 data)
	{
		RamArea* area = memoryArea(RamAccess::Write, offsetW);
		if (area == nullptr)
		{
			return false;
		}

		bool ok = area->writeDword(offsetW, data);
		return ok;
	}

	bool Ram::readDword(quint32 offsetW, quint32* data) const
	{
		const RamArea* area = memoryArea(RamAccess::Read, offsetW);
		if (area == nullptr)
		{
			return false;
		}

		bool ok = area->readDword(offsetW, data);
		return ok;
	}

	bool Ram::writeSignedInt(quint32 offsetW, qint32 data)
	{
		RamArea* area = memoryArea(RamAccess::Write, offsetW);
		if (area == nullptr)
		{
			return false;
		}

		bool ok = area->writeSignedInt(offsetW, data);
		return ok;
	}

	bool Ram::readSignedInt(quint32 offsetW, qint32* data) const
	{
		const RamArea* area = memoryArea(RamAccess::Read, offsetW);
		if (area == nullptr)
		{
			return false;
		}

		bool ok = area->readSignedInt(offsetW, data);
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

	const RamArea* Ram::memoryArea(RamAccess access, quint32 offsetW) const
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
