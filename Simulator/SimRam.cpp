#include "SimRam.h"
#include <QtEndian>
#include <SimOverrideSignals.h>
#include <SimException.h>

namespace Sim
{

	RamAreaInfo::RamAreaInfo(E::LogicModuleRamAccess access, quint32 offset, quint32 size, QString name) :
		m_name(name),
		m_access(access),
		m_offset(offset),
		m_size(size)
	{
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

	//
	// RamArea
	//
	RamArea::RamArea(bool clearOnStartCycle) :
		RamAreaInfo(),
		m_clearOnStartCycle(clearOnStartCycle)
	{
	}

	RamArea::RamArea(E::LogicModuleRamAccess access, quint32 offset, quint32 size, bool clearOnStartCycle, QString name) :
		RamAreaInfo(access, offset, size, name),
		m_clearOnStartCycle(clearOnStartCycle)
	{
		m_data.fill(0, size * 2);
		return;
	}

	RamArea::~RamArea()
	{
		//qDebug() << "RamArea::~RamArea(), data ptr " << QString::number(reinterpret_cast<quint64>(m_data.data()), 16) << " name = " << name() << " offset = " << offset();
	}

	bool RamArea::clear()
	{
		m_data.fill(0x00);

		// Apply override
		//
		if (m_overrideData.empty() == false)
		{
			int zeroBasedOffsetW = 0;

			quint16* dataPtr = reinterpret_cast<quint16*>(m_data.data());
			for (qint32 i = 0; i < m_data.size() / 2; i++)
			{
				m_overrideData[zeroBasedOffsetW].applyOverlapping(dataPtr);
				dataPtr ++;
			}
		}

		return true;
	}

	bool RamArea::writeBuffer(quint32 offsetW, const QByteArray& data) noexcept
	{
		int byteOffset = (offsetW - offset()) * 2;
		if (byteOffset < 0 ||
			m_data.size() - byteOffset < data.size())
		{
			// Buffer must be completely inside area
			//
			Q_ASSERT(byteOffset >= 0 &&
					 m_data.size() - byteOffset >= data.size());
			return false;
		}

		m_data.replace(byteOffset, data.size(), data);

		// Apply override
		//
		if (m_overrideData.empty() == false)
		{
			int zeroBasedOffsetW = offsetW - offset();
			assert(zeroBasedOffsetW >= 0 && zeroBasedOffsetW < m_data.size() / 2);

			quint16* dataPtr = reinterpret_cast<quint16*>(m_data.data()) + zeroBasedOffsetW;
			for (qint32 i = 0; i < data.size() / 2; i++)
			{
				m_overrideData[zeroBasedOffsetW].applyOverlapping(dataPtr);
				dataPtr ++;
			}
		}

		return true;
	}

	bool RamArea::readToBuffer(quint32 offsetW, quint32 countW, QByteArray* data, bool applyOverride) const noexcept
	{
		if (data == nullptr)
		{
			assert(data);
			return false;
		}

		int countBytes = countW * 2;

		int byteOffset = (offsetW - offset()) * 2;
		if (byteOffset < 0 ||
			m_data.size() - byteOffset < countBytes)
		{
			// Buffer must be completely inside area
			//
			Q_ASSERT(byteOffset >= 0 &&
					 m_data.size() - byteOffset >= countBytes);
			return false;
		}

		if (data->size() != countBytes)
		{
			data->resize(countBytes);
		}

		// Copy memory
		//
		memcpy_s(data->data(), data->size(), m_data.constData() + byteOffset, countBytes);

		// Apply override to just copied memory
		//
		if (applyOverride == true &&
			m_overrideData.empty() == false)
		{
			int zeroBasedOffsetW = offsetW - offset();

			if (zeroBasedOffsetW < 0 ||
				zeroBasedOffsetW >= static_cast<int>(m_overrideData.size()))
			{
				Q_ASSERT(zeroBasedOffsetW >= 0 && zeroBasedOffsetW < m_overrideData.size());
				return false;
			}

			quint16* dataPtr = reinterpret_cast<quint16*>(data->data());
			for (quint32 i = 0; i < countW; i++)
			{
				m_overrideData[zeroBasedOffsetW].applyOverlapping(dataPtr);
				dataPtr ++;
			}
		}

		return true;
	}

	bool RamArea::writeBit(quint32 offsetW, quint16 bitNo, quint16 data, E::ByteOrder byteOrder) noexcept
	{
		// Cannot use contains function, cause such signals like _pblink stored in memory for reading, but actually I write there manually
		//
		const quint32 areaOffset = offset();

		if (offsetW < areaOffset ||
			offsetW > (areaOffset + size() - 2) ||
			(bitNo & ~0x0F) != 0)
		{
			return false;
		}

		//bitNo &= 0x0F;	// This situaltion is excluded by prev condition (bitNo & ~0x0F) != 0
		data &= 0x01;
		int byteOffset = (offsetW - areaOffset) * 2;

		if (byteOffset >= m_data.size())
		{
			Q_ASSERT(byteOffset < m_data.size());
			return false;
		}

		quint16 word = *reinterpret_cast<const quint16*>(m_data.constData() + byteOffset);
		applyOverride(offsetW, 1, &word);		// Apply override before converting data to target endian

		if (byteOrder == E::ByteOrder::BigEndian)
		{
			word = qFromBigEndian<quint16>(word);
		}
		else
		{
			word = qFromLittleEndian<quint16>(word);
		}

		word &= ~(0x01 << bitNo);
		word |= (data << bitNo);

		quint16 targetWord;
		if (byteOrder == E::ByteOrder::BigEndian)
		{
			qToBigEndian<quint16>(word, &targetWord);
		}
		else
		{
			qToLittleEndian<quint16>(word, &targetWord);
		}

		// Apply override
		//
		applyOverride(offsetW, 1, &targetWord);

		// Write to memory
		//
		*reinterpret_cast<quint16*>(m_data.data() + byteOffset) = targetWord;

		return true;
	}

	bool RamArea::readBit(quint32 offsetW, quint16 bitNo, quint16* data, E::ByteOrder byteOrder, bool applyOverride) const noexcept
	{
		if (contains(E::LogicModuleRamAccess::Read, offsetW) == false ||
			bitNo >= 16 ||
			data == nullptr)
		{
			return false;
		}

		//--
		//
		bitNo &= 0x0F;

		int byteOffset = (offsetW - offset()) * 2;
		if (byteOffset >= m_data.size())
		{
			Q_ASSERT(byteOffset < m_data.size());
			return false;
		}

		quint16 word = *reinterpret_cast<const quint16*>(m_data.constData() + byteOffset);

		if (applyOverride == true && access() == E::LogicModuleRamAccess::Read)
		{
			// This is read only memory (like incoming data from i/o modules)
			// Apply override mask for read operations
			//
			this->applyOverride(offsetW, 1, &word);	// Apply override to native endian, as it is storen in memory
		}

		if (byteOrder == E::ByteOrder::BigEndian)
		{
			word = qFromBigEndian<quint16>(word);
		}
		else
		{
			word = qFromLittleEndian<quint16>(word);
		}

		word >>=  bitNo;
		word &= 0x01;

		*data = word;

		return true;
	}

	bool RamArea::writeWord(quint32 offsetW, quint16 data, E::ByteOrder byteOrder) noexcept
	{
		return writeData<quint16>(offsetW, data, byteOrder);
	}

	bool RamArea::readWord(quint32 offsetW, quint16* data, E::ByteOrder byteOrder, bool applyOverride) const noexcept
	{
		return readData<quint16>(offsetW, data, byteOrder, applyOverride);
	}

	bool RamArea::writeDword(quint32 offsetW, quint32 data, E::ByteOrder byteOrder) noexcept
	{
		return writeData<quint32>(offsetW, data, byteOrder);
	}

	bool RamArea::readDword(quint32 offsetW, quint32* data, E::ByteOrder byteOrder, bool applyOverride) const noexcept
	{
		return readData<quint32>(offsetW, data, byteOrder, applyOverride);
	}

	bool RamArea::writeSignedInt(quint32 offsetW, qint32 data, E::ByteOrder byteOrder) noexcept
	{
		return writeData<qint32>(offsetW, data, byteOrder);
	}

	bool RamArea::readSignedInt(quint32 offsetW, qint32* data, E::ByteOrder byteOrder, bool applyOverride) const noexcept
	{
		return readData<qint32>(offsetW, data, byteOrder, applyOverride);
	}

	const QByteArray& RamArea::data() const noexcept
	{
		return m_data;
	}

	const std::vector<OverrideRamRecord>& RamArea::overrideData() const noexcept
	{
		return m_overrideData;
	}

	void RamArea::setRawData(const QByteArray& value, const std::vector<OverrideRamRecord>& overrideData) noexcept
	{
		m_data = value;
		m_overrideData = overrideData;
	}

	template<typename TYPE>
	bool RamArea::writeData(quint32 offsetW, TYPE data, E::ByteOrder byteOrder) noexcept
	{
		int byteOffset = (offsetW - offset()) * 2;
		if (byteOffset < 0 ||
			byteOffset >= m_data.size() - sizeof(TYPE))
		{
			Q_ASSERT(byteOffset >= 0 &&
				   byteOffset - sizeof(TYPE) <= m_data.size());
			return false;
		}

		TYPE valueToWrite;

		switch (byteOrder)
		{
		case E::BigEndian:
			valueToWrite = qToBigEndian<TYPE>(data);
			break;
		case E::LittleEndian:
			valueToWrite = qToLittleEndian<TYPE>(data);
			break;
		case E::NoEndian:
			valueToWrite = data;
			break;
		default:
			assert(false);
		}

		// Apply override to data
		//
		if (m_overrideData.empty() == false)
		{
			applyOverride(offsetW, sizeof(TYPE) / 2, reinterpret_cast<quint16*>(&valueToWrite));
		}

		// Write data to memory
		//
		*reinterpret_cast<TYPE*>(m_data.data() + byteOffset) = valueToWrite;

		return true;
	}

	template<typename TYPE>
	bool RamArea::readData(quint32 offsetW, TYPE* data, E::ByteOrder byteOrder, bool applyOverride) const noexcept
	{
		if (data == nullptr)
		{
			Q_ASSERT(data);
			return false;
		}

		int byteOffset = (offsetW - offset()) * 2;
		if (byteOffset < 0 ||
			byteOffset >= m_data.size() - sizeof(TYPE))
		{
			Q_ASSERT(byteOffset >= 0 &&
					byteOffset - sizeof(TYPE) <= m_data.size());
			return false;
		}

		TYPE rawValue = *reinterpret_cast<const TYPE*>(m_data.constData() + byteOffset);

		// Apply override
		//
		if (applyOverride == true && access() == E::LogicModuleRamAccess::Read)
		{
			// This is read only memory (like incoming data from i/o modules)
			// Apply override mask for read operations
			//
			this->applyOverride(offsetW, sizeof(TYPE) / 2, reinterpret_cast<quint16*>(&rawValue));
		}

		switch (byteOrder)
		{
		case E::BigEndian:
			*data = qFromBigEndian<TYPE>(rawValue);
			break;
		case E::LittleEndian:
			*data = qFromLittleEndian<TYPE>(rawValue);
			break;
		case E::NoEndian:
			*data = rawValue;
			break;
		default:
			assert(false);
			return false;
		}

		return true;
	}

	void RamArea::applyOverride(quint32 offsetW, quint32 countW, quint16* dataPtr) const noexcept
	{
		if (dataPtr == nullptr)
		{
			assert(dataPtr);
			return;
		}

		if (m_overrideData.empty() == true)
		{
			// No data to override
			//
			return;
		}

		int zeroBasedOffsetW = offsetW - offset();

		if (zeroBasedOffsetW < 0 ||
			zeroBasedOffsetW >= static_cast<int>(m_overrideData.size()) ||
			zeroBasedOffsetW + countW > size())
		{
			assert(zeroBasedOffsetW >= 0 && zeroBasedOffsetW < static_cast<int>(m_overrideData.size()));
			assert(zeroBasedOffsetW + countW <= size());
			return;
		}

		for (; countW > 0; countW--)
		{
			m_overrideData[zeroBasedOffsetW].applyOverlapping(dataPtr);

			zeroBasedOffsetW++;
			dataPtr++;
		}

		return;
	}


	bool RamArea::clearOnStartCycle()
	{
		return m_clearOnStartCycle;
	}

	void RamArea::setOverrideData(std::vector<OverrideRamRecord>&& overrideData) noexcept
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

	Ram::~Ram()
	{
	}

	Ram& Ram::operator=(const Ram& that)
	{
		// Deep copy
		//
		m_memoryAreas.clear();
		m_memoryAreas.reserve(that.m_memoryAreas.size());

		for (const RamArea& thatArea : that.m_memoryAreas)
		{
			m_memoryAreas.emplace_back(thatArea);
		}

		return *this;
	}

	void Ram::reset()
	{
		m_memoryAreas.clear();
		m_overrideSignalsLastCounter = -1;
		return;
	}

	bool Ram::addMemoryArea(E::LogicModuleRamAccess access, quint32 offsetW, quint32 sizeW, bool clearOnStartCycle, QString name)
	{
		RamArea ramArea{access, offsetW, sizeW, clearOnStartCycle, name};

		// Check that new memory area is not overlapped with existsings
		//
		for (const auto& area : m_memoryAreas)
		{
			if (area.overlapped(access, offsetW, sizeW) == true)
			{
				// Area with this type of access and addresses already exists
				//
				return false;
			}
		}

		m_memoryAreas.emplace_back(std::move(ramArea));

		return true;
	}

	void Ram::updateFrom(const Ram& source)
	{
		if (m_memoryAreas.size() != source.m_memoryAreas.size())
		{
			*this = source;		// Full deep copy
			return;
		}

		for (size_t i = 0; i < m_memoryAreas.size(); i++)
		{
			const RamArea& s = source.m_memoryAreas[i];
			RamArea& d = m_memoryAreas[i];

			if (s.size() == d.size() &&
				s.offset() == d.offset() &&
				s.access() == d.access())
			{
				d.setRawData(s.data(), s.overrideData());
			}
			else
			{
				*this = source;		// Full deep copy
				return;
			}
		}

		return;
	}

	std::vector<RamAreaInfo> Ram::memoryAreasInfo() const
	{
		std::vector<RamAreaInfo> result;
		for (const RamArea& area : m_memoryAreas)
		{
			result.emplace_back(area);
		}

		return result;
	}

	RamAreaInfo Ram::memoryAreaInfo(const QString& name) const
	{
		for (const RamArea& area : m_memoryAreas)
		{
			if (area.name() == name)
			{
				return area;
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

		return m_memoryAreas[index];
	}

	Ram::Handle Ram::memoryAreaHandle(E::LogicModuleRamAccess access, quint32 offsetW) const
	{
		const size_t memoryAreaCount = m_memoryAreas.size();
		for (size_t i = 0; i < memoryAreaCount; i++)
		{
			if (m_memoryAreas[i].contains(access, offsetW) == true)
			{
				return i;
			}
		}

		return std::numeric_limits<size_t>::max();
	}

	RamArea* Ram::memoryArea(Ram::Handle handle)
	{
		if (handle >= m_memoryAreas.size())
		{
			return nullptr;
		}

		return &m_memoryAreas[handle];
	}

	const RamArea* Ram::memoryArea(Handle handle) const
	{
		if (handle >= m_memoryAreas.size())
		{
			return nullptr;
		}

		return &m_memoryAreas[handle];
	}

	bool Ram::clearMemoryAreasOnStartCycle()
	{
		for (RamArea& memoryArea : m_memoryAreas)
		{
			if (memoryArea.clearOnStartCycle() == true)
			{
				memoryArea.clear();
			}
		}

		return true;
	}

	bool Ram::clearMemoryArea(quint32 offsetW, E::LogicModuleRamAccess access)
	{
		RamArea* area = memoryArea(access, offsetW);
		if (area == nullptr)
		{
			return false;
		}

		return area->clear();	// Fill with 0's
	}

	bool Ram::writeBuffer(quint32 offsetW, E::LogicModuleRamAccess access, const QByteArray& data) noexcept
	{
		RamArea* area = memoryArea(access, offsetW);
		if (area == nullptr)
		{
			return false;
		}

		return area->writeBuffer(offsetW, data);
	}

	bool Ram::readToBuffer(quint32 offsetW, E::LogicModuleRamAccess access, quint32 countW, QByteArray* data, bool applyOverride) noexcept
	{
		RamArea* area = memoryArea(access, offsetW);
		if (area == nullptr)
		{
			return false;
		}

		return area->readToBuffer(offsetW, countW, data, applyOverride);
	}

	bool Ram::writeBit(quint32 offsetW, quint16 bitNo, quint16 data, E::ByteOrder byteOrder) noexcept
	{
		RamArea* area = memoryArea(E::LogicModuleRamAccess::Write, offsetW);
		if (area == nullptr)
		{
			return false;
		}

		return area->writeBit(offsetW, bitNo, data, byteOrder);
	}

	bool Ram::readBit(quint32 offsetW, quint16 bitNo, quint16* data, E::ByteOrder byteOrder, bool applyOverride) const noexcept
	{
		const RamArea* area = memoryArea(E::LogicModuleRamAccess::Read, offsetW);
		if (area == nullptr)
		{
			return false;
		}

		return area->readBit(offsetW, bitNo, data, byteOrder, applyOverride);
	}

	bool Ram::writeBit(quint32 offsetW, quint16 bitNo, quint16 data, E::ByteOrder byteOrder, E::LogicModuleRamAccess access)
	{
		RamArea* area = memoryArea(access, offsetW);
		if (area == nullptr)
		{
			return false;
		}

		return area->writeBit(offsetW, bitNo, data, byteOrder);
	}

	bool Ram::readBit(quint32 offsetW, quint16 bitNo, quint16* data, E::ByteOrder byteOrder, E::LogicModuleRamAccess access, bool applyOverride) const
	{
		const RamArea* area = memoryArea(access, offsetW);
		if (area == nullptr)
		{
			return false;
		}

		return area->readBit(offsetW, bitNo, data, byteOrder, applyOverride);
	}

	bool Ram::writeWord(quint32 offsetW, quint16 data, E::ByteOrder byteOrder) noexcept
	{
		RamArea* area = memoryArea(E::LogicModuleRamAccess::Write, offsetW);
		if (area == nullptr)
		{
			return false;
		}

		return area->writeWord(offsetW, data, byteOrder);
	}

	bool Ram::readWord(quint32 offsetW, quint16* data, E::ByteOrder byteOrder, bool applyOverride) const noexcept
	{
		const RamArea* area = memoryArea(E::LogicModuleRamAccess::Read, offsetW);
		if (area == nullptr)
		{
			return false;
		}

		return area->readWord(offsetW, data, byteOrder, applyOverride);
	}

	bool Ram::writeWord(quint32 offsetW, quint16 data, E::ByteOrder byteOrder, E::LogicModuleRamAccess access)
	{
		RamArea* area = memoryArea(access, offsetW);
		if (area == nullptr)
		{
			return false;
		}

		return area->writeWord(offsetW, data, byteOrder);
	}

	bool Ram::readWord(quint32 offsetW, quint16* data, E::ByteOrder byteOrder, E::LogicModuleRamAccess access, bool applyOverride) const
	{
		const RamArea* area = memoryArea(access, offsetW);
		if (area == nullptr)
		{
			return false;
		}

		return area->readWord(offsetW, data, byteOrder, applyOverride);
	}

	bool Ram::writeDword(quint32 offsetW, quint32 data, E::ByteOrder byteOrder)
	{
		RamArea* area = memoryArea(E::LogicModuleRamAccess::Write, offsetW);
		if (area == nullptr)
		{
			return false;
		}

		return area->writeDword(offsetW, data, byteOrder);
	}

	bool Ram::readDword(quint32 offsetW, quint32* data, E::ByteOrder byteOrder, bool applyOverride) const
	{
		const RamArea* area = memoryArea(E::LogicModuleRamAccess::Read, offsetW);
		if (area == nullptr)
		{
			return false;
		}

		return area->readDword(offsetW, data, byteOrder, applyOverride);
	}

	bool Ram::writeDword(quint32 offsetW, quint32 data, E::ByteOrder byteOrder, E::LogicModuleRamAccess access)
	{
		RamArea* area = memoryArea(access, offsetW);
		if (area == nullptr)
		{
			return false;
		}

		return area->writeDword(offsetW, data, byteOrder);
	}

	bool Ram::readDword(quint32 offsetW, quint32* data, E::ByteOrder byteOrder, E::LogicModuleRamAccess access, bool applyOverride) const
	{
		const RamArea* area = memoryArea(access, offsetW);
		if (area == nullptr)
		{
			return false;
		}

		return area->readDword(offsetW, data, byteOrder, applyOverride);
	}

	bool Ram::writeFloat(quint32 offsetW, float data, E::ByteOrder byteOrder)
	{
		return writeDword(offsetW, *reinterpret_cast<quint32*>(&data), byteOrder);
	}

	bool Ram::readFloat(quint32 offsetW, float* data, E::ByteOrder byteOrder, bool applyOverride) const
	{
		return readDword(offsetW, reinterpret_cast<quint32*>(data), byteOrder, applyOverride);
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

	bool Ram::readSignedInt(quint32 offsetW, qint32* data, E::ByteOrder byteOrder, bool applyOverride) const
	{
		const RamArea* area = memoryArea(E::LogicModuleRamAccess::Read, offsetW);
		if (area == nullptr)
		{
			return false;
		}

		bool ok = area->readSignedInt(offsetW, data, byteOrder, applyOverride);
		return ok;
	}

	RamArea* Ram::memoryArea(E::LogicModuleRamAccess access, quint32 offsetW) noexcept
	{
		RamArea* result = nullptr;

		if (offsetW < static_cast<quint32>(std::numeric_limits<quint16>::max() / 2))
		{
			for (RamArea& area : m_memoryAreas)
			{
				if (area.contains(access, offsetW) == true)
				{
					result = &area;
					break;
				}
			}
		}
		else
		{
			// Look for the right area from back, most likely it will find area much faster.
			// Tried it on project, it works really well
			//
			for (auto rit = m_memoryAreas.rbegin(); rit != m_memoryAreas.rend(); ++rit)
			{
				if (rit->contains(access, offsetW) == true)
				{
					result = &(*rit);
					break;
				}
			}
		}

		assert(result);

		return result;
	}

	const RamArea* Ram::memoryArea(E::LogicModuleRamAccess access, quint32 offsetW) const noexcept
	{
		const RamArea* result = nullptr;

		if (offsetW < static_cast<quint32>(std::numeric_limits<quint16>::max() / 2))
		{
			for (const RamArea& area : m_memoryAreas)
			{
				if (area.contains(access, offsetW) == true)
				{
					result = &area;
					break;
				}
			}
		}
		else
		{
			// Look for the right area from back, most likely it will find area much faster.
			// Tried it on project, it works really well
			//
			for (auto rit = m_memoryAreas.rbegin(); rit != m_memoryAreas.rend(); ++rit)
			{
				if (rit->contains(access, offsetW) == true)
				{
					result = &(*rit);
					break;
				}
			}
		}

		assert(result);

		return result;
	}

	void Ram::updateOverrideData(const QString& equipmentId, const OverrideSignals* overrideSignals)
	{
		if (overrideSignals == nullptr)
		{
			Q_ASSERT(overrideSignals);
			return;
		}

		if (m_overrideSignalsLastCounter == overrideSignals->changesCounter())
		{
			// Data has not been changesd since last update
			//
			return;
		}

		for (RamArea& ramArea : m_memoryAreas)
		{
			std::vector<OverrideRamRecord> ovData = overrideSignals->ramOverrideData(equipmentId, ramArea);
			ramArea.setOverrideData(std::move(ovData));
		}

		m_overrideSignalsLastCounter = overrideSignals->changesCounter();
		return;
	}

}
