#include "SimRam.h"
#include <cassert>
#include <QtEndian>
#include <SimOverrideSignals.h>

namespace Sim
{

	RamAreaInfo::RamAreaInfo(E::LogicModuleRamAccess access, quint32 offset, quint32 size, QString name) :
		m_name(name),
		m_access(access),
		m_offset(offset),
		m_size(size)
	{
	}

	bool RamAreaInfo::contains(E::LogicModuleRamAccess access, quint32 offsetW) const
	{
		if ((static_cast<int>(m_access) & static_cast<int>(access)) != 0 &&
			offsetW >= m_offset &&
			offsetW < (m_offset + m_size))
		{
			return true;
		}

		return false;
	}

	bool RamAreaInfo::overlapped(E::LogicModuleRamAccess access, quint32 offset, quint32 size) const
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

	E::LogicModuleRamAccess RamAreaInfo::access() const
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

	RamArea::RamArea(E::LogicModuleRamAccess access, quint32 offset, quint32 size, QString name) :
		RamAreaInfo(access, offset, size, name)
	{
		m_data.fill(0, size * 2);

		return;
	}

	bool RamArea::writeBit(quint32 offsetW, quint32 bitNo, quint16 data, E::ByteOrder byteOrder)
	{
		if (contains(E::LogicModuleRamAccess::Write, offsetW) == false ||
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

		quint16 word = 0;

		if (byteOrder == E::ByteOrder::BigEndian)
		{
			word = qFromBigEndian<quint16>(m_data.constData() + byteOffset);
		}
		else
		{
			word = qFromLittleEndian<quint16>(m_data.constData() + byteOffset);
		}

		word &= ~(0x01 << bitNo);
		word |= (data << bitNo);

		if (byteOrder == E::ByteOrder::BigEndian)
		{
			qToBigEndian<quint16>(word, m_data.data() + byteOffset);
		}
		else
		{
			qToLittleEndian<quint16>(word, m_data.data() + byteOffset);
		}

		// Apply override
		//
		if (m_overrideData.empty() == false)
		{
			applyOverride<quint16>(offsetW);
		}

		return true;
	}

	bool RamArea::readBit(quint32 offsetW, quint32 bitNo, quint16* data, E::ByteOrder byteOrder) const
	{
		if (contains(E::LogicModuleRamAccess::Read, offsetW) == false ||
			bitNo >= 16 ||
			data == nullptr)
		{
			return false;
		}

		// Apply override
		//
		if (access() == E::LogicModuleRamAccess::Read)
		{
			// This is read only memory (like incoming data from i/o modules)
			// Apply override mask for read operations
			// Probably in future it's better to apply ovvreide mask to RESULT of reading?
			//
			if (m_overrideData.empty() == false)
			{
				const_cast<RamArea*>(this)->applyOverride<quint16>(offsetW);
			}
		}

		//--
		//
		int byteOffset = (offsetW - offset()) * 2;
		if (byteOffset >= m_data.size())
		{
			assert(byteOffset < m_data.size());
			return false;
		}

		quint16 word = 0;

		if (byteOrder == E::ByteOrder::BigEndian)
		{
			word = qFromBigEndian<quint16>(m_data.constData() + byteOffset);
		}
		else
		{
			word = qFromLittleEndian<quint16>(m_data.constData() + byteOffset);
		}

		word >>=  bitNo;
		word &= 0x01;

		*data = word;

		return true;
	}

	bool RamArea::writeWord(quint32 offsetW, quint16 data, E::ByteOrder byteOrder)
	{
		return writeData<quint16>(offsetW, data, byteOrder);
	}

	bool RamArea::readWord(quint32 offsetW, quint16* data, E::ByteOrder byteOrder) const
	{
		return readData<quint16>(offsetW, data, byteOrder);
	}

	bool RamArea::writeDword(quint32 offsetW, quint32 data, E::ByteOrder byteOrder)
	{
		return writeData<quint32>(offsetW, data, byteOrder);
	}

	bool RamArea::readDword(quint32 offsetW, quint32* data, E::ByteOrder byteOrder) const
	{
		return readData<quint32>(offsetW, data, byteOrder);
	}

	bool RamArea::writeSignedInt(quint32 offsetW, qint32 data, E::ByteOrder byteOrder)
	{
		return writeData<qint32>(offsetW, data, byteOrder);
	}

	bool RamArea::readSignedInt(quint32 offsetW, qint32* data, E::ByteOrder byteOrder) const
	{
		return readData<qint32>(offsetW, data, byteOrder);
	}

	template<typename TYPE>
	bool RamArea::writeData(quint32 offsetW, TYPE data, E::ByteOrder byteOrder)
	{
		int byteOffset = (offsetW - offset()) * 2;
		if (byteOffset < 0 ||
			byteOffset >= m_data.size() - sizeof(TYPE))
		{
			assert(byteOffset >= 0 &&
				   byteOffset - sizeof(TYPE) <= m_data.size());
			return false;
		}

		if (byteOrder == E::BigEndian)
		{
			qToBigEndian<TYPE>(data, m_data.data() + byteOffset);
		}
		else
		{
			qToLittleEndian<TYPE>(data, m_data.data() + byteOffset);
		}

		// Apply override
		//
		if (m_overrideData.empty() == false)
		{
			applyOverride<TYPE>(offsetW);
		}

		return true;
	}

	template<typename TYPE>
	bool RamArea::readData(quint32 offsetW, TYPE* data, E::ByteOrder byteOrder) const
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

		// Apply override
		//
		if (access() == E::LogicModuleRamAccess::Read)
		{
			// This is read only memory (like incoming data from i/o modules)
			// Apply override mask for read operations
			// Probably in future it's better to apply ovvreide mask to RESULT of reading?
			//
			if (m_overrideData.empty() == false)
			{
				const_cast<RamArea*>(this)->applyOverride<quint16>(offsetW);
			}
		}

		if (byteOrder == E::BigEndian)
		{
			*data = qFromBigEndian<TYPE>(m_data.constData() + byteOffset);
		}
		else
		{
			*data = qFromLittleEndian<TYPE>(m_data.constData() + byteOffset);
		}
		return true;
	}

	template<typename TYPE>
	void RamArea::applyOverride(quint32 offsetW)
	{
		static_assert(sizeof(TYPE) >= 2 && sizeof(TYPE) <= 8);

		if (m_overrideData.empty() == true)
		{
			// No data to override
			//
			return;
		}

		int zeroBasedOffsetW = offsetW - offset();

		if (zeroBasedOffsetW < 0 ||
			zeroBasedOffsetW >= static_cast<int>(m_overrideData.size()))
		{
			assert(false);
			return;
		}

		quint16* ptrW = reinterpret_cast<quint16*>(m_data.data()) + zeroBasedOffsetW;

		if constexpr (sizeof(TYPE) == 2)
		{
			m_overrideData[zeroBasedOffsetW].applyOverlapping(ptrW);
		}

		if constexpr (sizeof(TYPE) == 4)
		{
			m_overrideData[zeroBasedOffsetW + 0].applyOverlapping(ptrW + 0);
			m_overrideData[zeroBasedOffsetW + 1].applyOverlapping(ptrW + 1);
		}

		if constexpr (sizeof(TYPE) == 8)
		{
			m_overrideData[zeroBasedOffsetW + 0].applyOverlapping(ptrW + 0);
			m_overrideData[zeroBasedOffsetW + 1].applyOverlapping(ptrW + 1);
			m_overrideData[zeroBasedOffsetW + 2].applyOverlapping(ptrW + 2);
			m_overrideData[zeroBasedOffsetW + 3].applyOverlapping(ptrW + 3);
		}

		return;
	}

	void RamArea::setOverrideData(std::vector<OverrideRamRecord> overrideData)
	{
		m_overrideData = std::move(overrideData);
	}

	Ram::Ram()
	{
	}

	Ram::Ram(const Ram& that)
	{
		*this = that;
	}

	Ram& Ram::operator=(const Ram& that)
	{
		// Deep copy
		//
		m_memoryAreas.clear();
		m_memoryAreas.reserve(that.m_memoryAreas.size());

		for (const std::shared_ptr<RamArea>& thatArea : that.m_memoryAreas)
		{
			std::shared_ptr<RamArea> area = std::make_shared<RamArea>(*thatArea.get());
			m_memoryAreas.push_back(area);
		}

		return *this;
	}

	void Ram::reset()
	{
		m_memoryAreas.clear();
		m_overrideSignalsLastCounter = -1;
		return;
	}

	bool Ram::addMemoryArea(E::LogicModuleRamAccess access, quint32 offsetW, quint32 sizeW, QString name)
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

	bool Ram::writeBit(quint32 offsetW, quint32 bitNo, quint32 data, E::ByteOrder byteOrder)
	{
		RamArea* area = memoryArea(E::LogicModuleRamAccess::Write, offsetW);
		if (area == nullptr)
		{
			return false;
		}

		bool ok = area->writeBit(offsetW, bitNo, data, byteOrder);
		return ok;
	}

	bool Ram::readBit(quint32 offsetW, quint32 bitNo, quint16* data, E::ByteOrder byteOrder) const
	{
		const RamArea* area = memoryArea(E::LogicModuleRamAccess::Read, offsetW);
		if (area == nullptr)
		{
			return false;
		}

		bool ok = area->readBit(offsetW, bitNo, data, byteOrder);
		return ok;
	}

	bool Ram::writeWord(quint32 offsetW, quint16 data, E::ByteOrder byteOrder)
	{
		RamArea* area = memoryArea(E::LogicModuleRamAccess::Write, offsetW);
		if (area == nullptr)
		{
			return false;
		}

		bool ok = area->writeWord(offsetW, data, byteOrder);
		return ok;
	}

	bool Ram::readWord(quint32 offsetW, quint16* data, E::ByteOrder byteOrder) const
	{
		const RamArea* area = memoryArea(E::LogicModuleRamAccess::Read, offsetW);
		if (area == nullptr)
		{
			return false;
		}

		bool ok = area->readWord(offsetW, data, byteOrder);
		return ok;
	}

	bool Ram::writeDword(quint32 offsetW, quint32 data, E::ByteOrder byteOrder)
	{
		RamArea* area = memoryArea(E::LogicModuleRamAccess::Write, offsetW);
		if (area == nullptr)
		{
			return false;
		}

		bool ok = area->writeDword(offsetW, data, byteOrder);
		return ok;
	}

	bool Ram::readDword(quint32 offsetW, quint32* data, E::ByteOrder byteOrder) const
	{
		const RamArea* area = memoryArea(E::LogicModuleRamAccess::Read, offsetW);
		if (area == nullptr)
		{
			return false;
		}

		bool ok = area->readDword(offsetW, data, byteOrder);
		return ok;
	}

	bool Ram::writeFloat(quint32 offsetW, float data, E::ByteOrder byteOrder)
	{
		return writeDword(offsetW, *reinterpret_cast<quint32*>(&data), byteOrder);
	}

	bool Ram::readFloat(quint32 offsetW, float* data, E::ByteOrder byteOrder) const
	{
		return readDword(offsetW, reinterpret_cast<quint32*>(data), byteOrder);
	}

	bool Ram::writeSignedInt(quint32 offsetW, qint32 data, E::ByteOrder byteOrder)
	{
		RamArea* area = memoryArea(E::LogicModuleRamAccess::Write, offsetW);
		if (area == nullptr)
		{
			return false;
		}

		bool ok = area->writeSignedInt(offsetW, data, byteOrder);
		return ok;
	}

	bool Ram::readSignedInt(quint32 offsetW, qint32* data, E::ByteOrder byteOrder) const
	{
		const RamArea* area = memoryArea(E::LogicModuleRamAccess::Read, offsetW);
		if (area == nullptr)
		{
			return false;
		}

		bool ok = area->readSignedInt(offsetW, data, byteOrder);
		return ok;
	}

	RamArea* Ram::memoryArea(E::LogicModuleRamAccess access, quint32 offsetW)
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

	const RamArea* Ram::memoryArea(E::LogicModuleRamAccess access, quint32 offsetW) const
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

	bool Ram::allowOverride() const
	{
		return m_allowOverride;
	}

	void Ram::setAllowOverride(bool value)
	{
		m_allowOverride = value;
	}

	void Ram::updateOverrideData(QString equipmentId, const OverrideSignals* overrideSignals)
	{
		if (overrideSignals == nullptr)
		{
			assert(overrideSignals);
			return;
		}

		if (m_overrideSignalsLastCounter == overrideSignals->changesCounter())
		{
			// Data has not been changesd since last update
			//
			return;
		}

		for (std::shared_ptr<RamArea>& ramArea : m_memoryAreas)
		{
			std::vector<OverrideRamRecord> ovData = overrideSignals->ramOverrideData(equipmentId, *ramArea.get());
			ramArea->setOverrideData(std::move(ovData));
		}

		m_overrideSignalsLastCounter = overrideSignals->changesCounter();
		return;
	}

}
