#ifndef LMRAM_H
#define LMRAM_H
#include <map>
#include <vector>
#include <memory>
#include <QByteArray>
#include "../lib/Types.h"
#include "SimOverrideSignals.h"


namespace Sim
{

	class RamAreaInfo
	{
	public:
		RamAreaInfo() = default;
		RamAreaInfo(const RamAreaInfo&) = default;
		RamAreaInfo(RamAreaInfo&&) noexcept = default;
		RamAreaInfo(E::LogicModuleRamAccess access, quint32 offset, quint32 size, QString name);
		RamAreaInfo& operator=(const RamAreaInfo&) = default;
		RamAreaInfo& operator=(RamAreaInfo&&) = default;
		virtual ~RamAreaInfo() = default;

	public:
		bool contains(E::LogicModuleRamAccess access, quint32 offsetW) const noexcept
		{
			return  offsetW >= m_offset &&
					offsetW < (m_offset + m_size) &&
					(static_cast<int>(m_access) & static_cast<int>(access)) != 0;
		}

		bool overlapped(E::LogicModuleRamAccess access, quint32 offset, quint32 size) const;

	public:
		QString name() const;
		E::LogicModuleRamAccess access() const	{	return m_access;	}
		quint32 offset() const					{	return m_offset;	}
		quint32 size() const					{	return m_size;		}

	private:
		QString m_name;
		E::LogicModuleRamAccess m_access = E::LogicModuleRamAccess::Read;
		quint32 m_offset = 0xFFFFFFFF;
		quint32 m_size = 0;
	};


	class RamArea final : public RamAreaInfo
	{
	public:
		RamArea() = default;
		RamArea(const RamArea&) = default;
		RamArea(RamArea&&) noexcept = default;
		RamArea& operator=(const RamArea&) = default;
		RamArea& operator=(RamArea&&) = default;

		RamArea(E::LogicModuleRamAccess access, quint32 offset, quint32 size, QString name);

	public:
		bool writeBit(quint32 offsetW, quint16 bitNo, quint16 data, E::ByteOrder byteOrder) noexcept;
		bool readBit(quint32 offsetW, quint16 bitNo, quint16* data, E::ByteOrder byteOrder) const noexcept;

		bool writeWord(quint32 offsetW, quint16 data, E::ByteOrder byteOrder) noexcept;
		bool readWord(quint32 offsetW, quint16* data, E::ByteOrder byteOrder) const noexcept;

		bool writeDword(quint32 offsetW, quint32 data, E::ByteOrder byteOrder) noexcept;
		bool readDword(quint32 offsetW, quint32* data, E::ByteOrder byteOrder) const noexcept;

		bool writeSignedInt(quint32 offsetW, qint32 data, E::ByteOrder byteOrder) noexcept;
		bool readSignedInt(quint32 offsetW, qint32* data, E::ByteOrder byteOrder) const noexcept;

		const QByteArray& data() const noexcept;
		const std::vector<OverrideRamRecord>& overrideData() const noexcept;

		void setRawData(const QByteArray& data, const std::vector<OverrideRamRecord>& overrideData) noexcept;

	private:
		template<typename TYPE> bool writeData(quint32 offsetW, TYPE data, E::ByteOrder byteOrder) noexcept;
		template<typename TYPE> bool readData(quint32 offsetW, TYPE* data, E::ByteOrder byteOrder) const noexcept;

		template<typename TYPE> void applyOverride(quint32 offsetW) noexcept;

	public:
		void setOverrideData(std::vector<OverrideRamRecord>&& overrideData) noexcept;

	private:
		QByteArray m_data;
		std::vector<OverrideRamRecord> m_overrideData;
	};


	class Ram
	{
	public:
		Ram();
		Ram(const Ram& that);
		Ram& operator=(const Ram& that);

	public:
		void reset();
		bool addMemoryArea(E::LogicModuleRamAccess access, quint32 offsetW, quint32 sizeW, QString name);			// offset and size in 16 bit words

		void updateFrom(const Ram& source);

		std::vector<RamAreaInfo> memoryAreasInfo() const;
		RamAreaInfo memoryAreaInfo(const QString& name) const;
		RamAreaInfo memoryAreaInfo(int index) const;

		bool writeBit(quint32 offsetW, quint16 bitNo, quint16 data, E::ByteOrder byteOrder) noexcept;
		bool readBit(quint32 offsetW, quint16 bitNo, quint16* data, E::ByteOrder byteOrder) const noexcept;

		bool writeBit(quint32 offsetW, quint16 bitNo, quint16 data, E::ByteOrder byteOrder, E::LogicModuleRamAccess access);
		bool readBit(quint32 offsetW, quint16 bitNo, quint16* data, E::ByteOrder byteOrder, E::LogicModuleRamAccess access) const;

		bool writeWord(quint32 offsetW, quint16 data, E::ByteOrder byteOrder) noexcept;
		bool readWord(quint32 offsetW, quint16* data, E::ByteOrder byteOrder) const noexcept;

		bool writeWord(quint32 offsetW, quint16 data, E::ByteOrder byteOrder, E::LogicModuleRamAccess access);
		bool readWord(quint32 offsetW, quint16* data, E::ByteOrder byteOrder, E::LogicModuleRamAccess access) const;

		bool writeDword(quint32 offsetW, quint32 data, E::ByteOrder byteOrder);
		bool readDword(quint32 offsetW, quint32* data, E::ByteOrder byteOrder) const;

		bool writeFloat(quint32 offsetW, float data, E::ByteOrder byteOrder);
		bool readFloat(quint32 offsetW, float* data, E::ByteOrder byteOrder) const;

		bool writeSignedInt(quint32 offsetW, qint32 data, E::ByteOrder byteOrder);
		bool readSignedInt(quint32 offsetW, qint32* data, E::ByteOrder byteOrder) const;

	private:
		RamArea* memoryArea(E::LogicModuleRamAccess access, quint32 offsetW) noexcept;
		const RamArea* memoryArea(E::LogicModuleRamAccess access, quint32 offsetW) const noexcept;

	public:
		void updateOverrideData(const QString& equipmentId, const Sim::OverrideSignals* overrideSignals);

	private:
		// Pay attention to copy operator
		//
		std::vector<RamArea> m_memoryAreas;
		int m_overrideSignalsLastCounter = -1;

		mutable RamArea* m_lastAccessedMemoryArea = nullptr;		// Cached value
	};
}

#endif // LMRAM_H
