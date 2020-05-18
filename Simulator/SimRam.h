#pragma once

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
		QString dump() const;

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
		RamArea(bool clearOnStartCycle);
		RamArea(const RamArea&) = default;
		RamArea(RamArea&&) noexcept = default;
		virtual ~RamArea();

		RamArea& operator=(const RamArea&) = default;
		RamArea& operator=(RamArea&&) = default;

		RamArea(E::LogicModuleRamAccess access, quint32 offset, quint32 size, bool clearOnStartCycle, QString name);

	public:
		QString dump() const;

		bool clear();

		bool writeBuffer(quint32 offsetW, const QByteArray& data) noexcept;
		bool readToBuffer(quint32 offsetW, quint32 countW, QByteArray* data, bool applyOverride) const noexcept;

		bool writeBit(quint32 offsetW, quint16 bitNo, quint16 data, E::ByteOrder byteOrder) noexcept;
		bool readBit(quint32 offsetW, quint16 bitNo, quint16* data, E::ByteOrder byteOrder, bool applyOverride) const noexcept;

		bool writeWord(quint32 offsetW, quint16 data, E::ByteOrder byteOrder) noexcept;
		bool readWord(quint32 offsetW, quint16* data, E::ByteOrder byteOrder, bool applyOverride) const noexcept;

		bool writeDword(quint32 offsetW, quint32 data, E::ByteOrder byteOrder) noexcept;
		bool readDword(quint32 offsetW, quint32* data, E::ByteOrder byteOrder, bool applyOverride) const noexcept;

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

		void applyOverride(quint32 offsetW, quint32 countW, quint16* dataPtr) const noexcept;

	public:
		bool clearOnStartCycle();

		void setOverrideData(std::vector<OverrideRamRecord>&& overrideData) noexcept;

	private:
		bool m_clearOnStartCycle = false;					// Clear memory area on start of work cycle
		QByteArray m_data;
		std::vector<OverrideRamRecord> m_overrideData;
	};


	class Ram
	{
	public:
		Ram();
		Ram(const Ram& that);
		~Ram();

		Ram& operator=(const Ram& that);


	public:
		void reset();
		bool addMemoryArea(E::LogicModuleRamAccess access, quint32 offsetW, quint32 sizeW, bool clearOnStartCycle, QString name);			// offset and size in 16 bit words

		void updateFrom(const Ram& source);

		QString dump(QString equipmnetId) const;

		std::vector<RamAreaInfo> memoryAreasInfo() const;
		RamAreaInfo memoryAreaInfo(const QString& name) const;
		RamAreaInfo memoryAreaInfo(int index) const;

	using Handle = size_t;	// Handle is just index in m_memoryAreas vector

		[[nodiscard]] Handle memoryAreaHandle(E::LogicModuleRamAccess access, quint32 offsetW) const;
		[[nodiscard]] RamArea* memoryArea(Handle handle);
		[[nodiscard]] const RamArea* memoryArea(Handle handle) const;

	public:
		bool clearMemoryAreasOnStartCycle();
		bool clearMemoryArea(quint32 offsetW, E::LogicModuleRamAccess access);

		bool writeBuffer(quint32 offsetW, E::LogicModuleRamAccess access, const QByteArray& data) noexcept;
		bool readToBuffer(quint32 offsetW, E::LogicModuleRamAccess access, quint32 countW, QByteArray* data, bool applyOverride = true) noexcept;

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

		bool writeFloat(quint32 offsetW, float data, E::ByteOrder byteOrder);
		bool readFloat(quint32 offsetW, float* data, E::ByteOrder byteOrder, bool applyOverride = true) const;

		bool writeSignedInt(quint32 offsetW, qint32 data, E::ByteOrder byteOrder);
		bool readSignedInt(quint32 offsetW, qint32* data, E::ByteOrder byteOrder, bool applyOverride = true) const;

	private:
		[[nodiscard]] RamArea* memoryArea(E::LogicModuleRamAccess access, quint32 offsetW) noexcept;
		[[nodiscard]] const RamArea* memoryArea(E::LogicModuleRamAccess access, quint32 offsetW) const noexcept;

	public:
		void updateOverrideData(const QString& lmEquipmentId, const Sim::OverrideSignals* overrideSignals);

	private:
		// Pay attention to copy operator
		//
		std::vector<RamArea> m_memoryAreas;
		int m_overrideSignalsLastCounter = -1;
	};
}

