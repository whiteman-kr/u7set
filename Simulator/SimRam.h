#pragma once

#include <map>
#include <memory_resource>
#include <vector>
#include <memory>
#include <QByteArray>

class SimRamTests;

namespace Sim
{

	struct OverrideRamRecord
	{
		quint16 mask = 0;
		quint16 data = 0;

		void overlapRecord(OverrideRamRecord r) noexcept
		{
			mask |= r.mask;
			data |= r.data;
		}

		void applyOverlapping(quint16* ptrW) const noexcept
		{
			assert(ptrW);
			*ptrW &= ~mask;
			*ptrW |= data;
		}

		auto operator<=>(const OverrideRamRecord&) const = default;
	};


	class RamAreaInfo
	{
	public:
		RamAreaInfo() = default;
		RamAreaInfo(const RamAreaInfo&) = default;
		RamAreaInfo(RamAreaInfo&&) noexcept = default;
		RamAreaInfo(E::LogicModuleRamAccess access, quint32 offset, quint32 size, QString name);
		RamAreaInfo& operator=(const RamAreaInfo&) = default;
		RamAreaInfo& operator=(RamAreaInfo&&) noexcept = default;
		virtual ~RamAreaInfo() = default;

	public:
		[[nodiscard]] QString dump() const;

		[[nodiscard]] bool contains(E::LogicModuleRamAccess access, quint32 offsetW) const noexcept;
		[[nodiscard]] bool contains(quint32 offsetW) const noexcept;

		[[nodiscard]] bool overlapped(E::LogicModuleRamAccess access, quint32 offset, quint32 size) const noexcept;

	public:
		[[nodiscard]] const QString& name() const noexcept				{	return m_name;		}
		[[nodiscard]] E::LogicModuleRamAccess access() const noexcept	{	return m_access;	}
		[[nodiscard]] quint32 offset() const noexcept					{	return m_offset;	}
		[[nodiscard]] quint32 size() const noexcept						{	return m_size;		}

	private:
		QString m_name;
		E::LogicModuleRamAccess m_access = E::LogicModuleRamAccess::Read;
		quint32 m_offset = 0xFFFFFFFF;	// In words
		quint32 m_size = 0;				// In words
	};


	class RamArea final : public RamAreaInfo
	{
	public:
		RamArea(bool clearOnStartCycle);
		RamArea(const RamArea&) = default;
		RamArea(RamArea&&) noexcept = default;
		virtual ~RamArea() = default;

		RamArea& operator=(const RamArea&) = default;
		RamArea& operator=(RamArea&&) noexcept = default;

		RamArea(E::LogicModuleRamAccess access, quint32 offset, quint32 size, bool clearOnStartCycle, QString name);

	public:
		[[nodiscard]] QString dump() const;

		bool clear();

		bool writeBuffer(quint32 offsetW, const QByteArray& data) noexcept;

		template<typename CONTAINER>
		bool readToBuffer(quint32 offsetW, quint32 countW, CONTAINER* data, bool applyOverride) const noexcept;

		bool setMem(quint32 offsetW, quint32 sizeW, quint16 data);

		bool writeBit(quint32 offsetW, quint16 bitNo, quint16 data, E::ByteOrder byteOrder) noexcept;
		bool readBit(quint32 offsetW, quint16 bitNo, quint16* data, E::ByteOrder byteOrder, bool applyOverride) const noexcept;

		bool writeWord(quint32 offsetW, quint16 data, E::ByteOrder byteOrder) noexcept;
		bool readWord(quint32 offsetW, quint16* data, E::ByteOrder byteOrder, bool applyOverride) const noexcept;

		bool writeDword(quint32 offsetW, quint32 data, E::ByteOrder byteOrder) noexcept;
		bool readDword(quint32 offsetW, quint32* data, E::ByteOrder byteOrder, bool applyOverride) const noexcept;

		bool writeFloat(quint32 offsetW, float data, E::ByteOrder byteOrder) noexcept;
		bool readFloat(quint32 offsetW, float* data, E::ByteOrder byteOrder, bool applyOverride) const noexcept;

		bool writeSignedInt(quint32 offsetW, qint32 data, E::ByteOrder byteOrder) noexcept;
		bool readSignedInt(quint32 offsetW, qint32* data, E::ByteOrder byteOrder, bool applyOverride) const noexcept;

		[[nodiscard]] const QByteArray& data() const noexcept;
		[[nodiscard]] const std::vector<OverrideRamRecord>& overrideData() const noexcept;

		void setRawData(const QByteArray& data, const std::vector<OverrideRamRecord>& overrideData) noexcept;

	private:
		template<typename TYPE>
		bool writeData(quint32 offsetW, TYPE data, E::ByteOrder byteOrder) noexcept;

		template<typename TYPE>
		bool readData(quint32 offsetW, TYPE* data, E::ByteOrder byteOrder, bool applyOverride) const noexcept;

		bool applyOverride(quint32 offsetW, quint32 countW, quint16* dataPtr) const noexcept;

	public:
		bool clearOnStartCycle();

		void setOverrideData(std::vector<OverrideRamRecord>&& overrideData) noexcept;

	private:
		bool m_clearOnStartCycle = false;					// Clear memory area on start of work cycle
		QByteArray m_data;
		std::vector<OverrideRamRecord> m_overrideData;

		friend SimRamTests;
	};

	template<typename CONTAINER>
	bool RamArea::readToBuffer(quint32 offsetW, quint32 countW, CONTAINER* data, bool applyOverride) const noexcept
	{
		if (data == nullptr)
		{
			assert(data);
			return false;
		}

		int countBytes = countW * 2;
		int byteOffset = (offsetW - offset()) * 2;

//#ifdef QT_DEBUG
		if (byteOffset < 0 || m_data.size() - byteOffset < countBytes)
		{
			// Buffer must be completely inside area
			//
			Q_ASSERT(byteOffset >= 0 && m_data.size() - byteOffset >= countBytes);
			return false;
		}
//#endif

		if (static_cast<int>(data->size()) != countBytes)
		{
			data->resize(countBytes);
		}

		// Copy memory
		//
		std::memcpy(data->data(), m_data.constData() + byteOffset, countBytes);

		// Apply override to just copied memory
		//
		if (applyOverride == true &&
			m_overrideData.empty() == false)
		{
			int zeroBasedOffsetW = offsetW - offset();

//#ifdef QT_DEBUG
			if (zeroBasedOffsetW < 0 || zeroBasedOffsetW >= static_cast<int>(m_overrideData.size()))
			{
				Q_ASSERT(zeroBasedOffsetW >= 0 && zeroBasedOffsetW < static_cast<int>(m_overrideData.size()));
				return false;
			}
//#endif

			quint16* dataPtr = reinterpret_cast<quint16*>(data->data());
			for (quint32 i = 0; i < countW; i++)
			{
				m_overrideData[zeroBasedOffsetW++].applyOverlapping(dataPtr++);
			}
		}

		return true;
	}

	template<typename TYPE>
	bool RamArea::writeData(quint32 offsetW, TYPE data, E::ByteOrder byteOrder) noexcept
	{
		size_t byteOffset = (offsetW - offset()) * 2;

//#ifdef QT_DEBUG
		if (byteOffset > m_data.size() - sizeof(TYPE))
		{
			Q_ASSERT(false);
			return false;
		}
//#endif

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
			valueToWrite = {};
		}

		// Apply override to data
		//
		if (m_overrideData.empty() == false)
		{
			quint16 v[sizeof(TYPE) / 2];
			std::memcpy(v, &valueToWrite, sizeof(valueToWrite));

			applyOverride(offsetW, sizeof(TYPE) / 2, v);

			std::memcpy(&valueToWrite, v, sizeof(valueToWrite));
		}

		// Write data to memory
		//
		std::memcpy(m_data.data() + byteOffset, &valueToWrite, sizeof(valueToWrite));

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

		constexpr int wordCount = sizeof(TYPE) / sizeof(quint16);
		size_t byteOffset = (offsetW - offset()) * 2;

//#ifdef QT_DEBUG
		if (byteOffset > m_data.size() - sizeof(TYPE))
		{
			Q_ASSERT(false);
			return false;
		}
//#endif

		TYPE rawValue;
		std::memcpy(&rawValue, m_data.constData() + byteOffset, sizeof(TYPE));

		// Apply override
		//
		if (applyOverride == true && m_overrideData.empty() == false)
		{
			std::array<quint16, wordCount> rawValueWorded;
			static_assert(sizeof(rawValueWorded) == sizeof(TYPE));

			std::memcpy(rawValueWorded.data(), &rawValue, sizeof(TYPE));

			bool ok = this->applyOverride(offsetW, wordCount, rawValueWorded.data());
			Q_ASSERT(ok);

			std::memcpy(&rawValue, rawValueWorded.data(), sizeof(TYPE));
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


	class Ram
	{
	public:
		Ram() = default;
		Ram(const Ram& that);
		~Ram() = default;

		Ram& operator=(const Ram& that);

	public:
		[[nodiscard]] bool isNull() const;
		void reset();

		bool addMemoryArea(E::LogicModuleRamAccess access, quint32 offsetW, quint32 sizeW, bool clearOnStartCycle, QString name);			// offset and size in 16 bit words
		void updateFrom(const Ram& source);

		[[nodiscard]] QString dump(QString equipmnetId) const;

		using Handle = size_t;	// Handle is index in m_memoryAreas vector
		constexpr static Handle InvalidHandle = std::numeric_limits<Handle>::max();

		[[nodiscard]] Handle memoryAreaHandle(E::LogicModuleRamAccess access, quint32 offsetW) const;
		[[nodiscard]] RamArea* memoryArea(Handle handle);
		[[nodiscard]] const RamArea* memoryArea(Handle handle) const;

		[[nodiscard]] std::vector<RamArea*> memoryAreas();
		[[nodiscard]] std::vector<const RamArea*> memoryAreas() const;

	public:
		bool clearMemoryAreasOnStartCycle();
		bool clearMemoryArea(quint32 offsetW, E::LogicModuleRamAccess access);

		bool writeBuffer(quint32 offsetW, E::LogicModuleRamAccess access, const QByteArray& data) noexcept;
		bool writeBuffer(quint32 offsetW, E::LogicModuleRamAccess access, const std::vector<char>& data) noexcept;
		bool readToBuffer(quint32 offsetW, E::LogicModuleRamAccess access, quint32 countW, QByteArray* data, bool applyOverride = true) noexcept;
		bool readToBuffer(quint32 offsetW, E::LogicModuleRamAccess access, quint32 countW, std::vector<char>* data, bool applyOverride = true) noexcept;

		bool movMem(quint32 src, quint32 dst, quint32 sizeW);
		bool setMem(quint32 offsetW, quint32 sizeW, quint16 data);
		bool setMem(quint32 offsetW, quint32 sizeW, quint16 data, E::LogicModuleRamAccess access);

		bool writeBit(quint32 offsetW, quint16 bitNo, quint16 data, E::ByteOrder byteOrder) noexcept;
		bool readBit(quint32 offsetW, quint16 bitNo, quint16* data, E::ByteOrder byteOrder, bool applyOverride = true) const noexcept;

		bool writeBit(quint32 offsetW, quint16 bitNo, quint16 data, E::ByteOrder byteOrder, E::LogicModuleRamAccess access);
		bool readBit(quint32 offsetW, quint16 bitNo, quint16* data, E::ByteOrder byteOrder, E::LogicModuleRamAccess access, bool applyOverride = true) const;

		bool writeWord(quint32 offsetW, quint16 data, E::ByteOrder byteOrder) noexcept;
		bool readWord(quint32 offsetW, quint16* data, E::ByteOrder byteOrder, bool applyOverride = true) const noexcept;

		bool writeWord(quint32 offsetW, quint16 data, E::ByteOrder byteOrder, E::LogicModuleRamAccess access);
		bool readWord(quint32 offsetW, quint16* data, E::ByteOrder byteOrder, E::LogicModuleRamAccess access, bool applyOverride = true) const;

		bool writeDword(quint32 offsetW, quint32 data, E::ByteOrder byteOrder);
		bool readDword(quint32 offsetW, quint32* data, E::ByteOrder byteOrder, bool applyOverride = true) const;

		bool writeDword(quint32 offsetW, quint32 data, E::ByteOrder byteOrder, E::LogicModuleRamAccess access);
		bool readDword(quint32 offsetW, quint32* data, E::ByteOrder byteOrder, E::LogicModuleRamAccess access, bool applyOverride = true) const;

		bool writeFloat(quint32 offsetW, float data, E::ByteOrder byteOrder, E::LogicModuleRamAccess access);
		bool readFloat(quint32 offsetW, float* data, E::ByteOrder byteOrder, E::LogicModuleRamAccess access, bool applyOverride = true) const;

		bool writeSignedInt(quint32 offsetW, qint32 data, E::ByteOrder byteOrder, E::LogicModuleRamAccess access);
		bool readSignedInt(quint32 offsetW, qint32* data, E::ByteOrder byteOrder, E::LogicModuleRamAccess access, bool applyOverride = true) const;

	private:
		[[nodiscard]] RamArea* memoryArea(E::LogicModuleRamAccess access, quint32 offsetW) noexcept;
		[[nodiscard]] const RamArea* memoryArea(E::LogicModuleRamAccess access, quint32 offsetW) const noexcept;

	public:
		int overrideSignalsLastCounter(int newValue);

	private:
		// Pay attention to copy operator
		//
		std::vector<RamArea> m_memoryAreas;
		int m_overrideSignalsLastCounter = -1;

		// --
		//
		std::array<char, 4096> pool_buffer;
		std::pmr::monotonic_buffer_resource m_pool{pool_buffer.data(), pool_buffer.size()};

		std::pmr::map<quint32, size_t> m_readAreas{&m_pool};	// key is area offset, value is index
		std::pmr::map<quint32, size_t> m_writeAreas{&m_pool};	// key is area offset, value is index

		// --
		//
		friend SimRamTests;
	};
}

