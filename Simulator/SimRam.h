#ifndef LMRAM_H
#define LMRAM_H
#include <QByteArray>
#include <map>
#include <vector>
#include <memory>
#include "../lib/Types.h"

namespace Sim
{
	enum class RamAccess
	{
		Read  = 0x01,
		Write = 0x02,
		ReadWrite = 0x03
	};

	class RamAreaInfo
	{
	public:
		RamAreaInfo() = default;
		RamAreaInfo(const RamAreaInfo&) = default;
		RamAreaInfo(RamAccess access, quint32 offset, quint32 size, QString name);

	public:
		bool contains(RamAccess access, quint32 offsetW) const;
		bool overlapped(RamAccess access, quint32 offset, quint32 size) const;

	public:
		QString name() const;
		RamAccess access() const;
		quint32 offset() const;
		quint32 size() const;

	private:
		QString m_name;
		RamAccess m_access = RamAccess::Read;
		quint32 m_offset = 0xFFFFFFFF;
		quint32 m_size = 0;
	};


	class RamArea : public RamAreaInfo
	{
	public:
		RamArea() = default;
		RamArea(const RamArea&) = default;
		RamArea& operator=(const RamArea&) = default;

		RamArea(RamAccess access, quint32 offset, quint32 size, QString name);

	public:
		bool writeBit(quint32 offsetW, quint32 bitNo, quint16 data, E::ByteOrder byteOrder);
		bool readBit(quint32 offsetW, quint32 bitNo, quint16* data, E::ByteOrder byteOrder) const;

		bool writeWord(quint32 offsetW, quint16 data, E::ByteOrder byteOrder);
		bool readWord(quint32 offsetW, quint16* data, E::ByteOrder byteOrder) const;

		bool writeDword(quint32 offsetW, quint32 data, E::ByteOrder byteOrder);
		bool readDword(quint32 offsetW, quint32* data, E::ByteOrder byteOrder) const;

		bool writeSignedInt(quint32 offsetW, qint32 data, E::ByteOrder byteOrder);
		bool readSignedInt(quint32 offsetW, qint32* data, E::ByteOrder byteOrder) const;

	private:
		template<typename TYPE> bool writeData(quint32 offsetW, TYPE data, E::ByteOrder byteOrder);
		template<typename TYPE> bool readData(quint32 offsetW, TYPE* data, E::ByteOrder byteOrder) const;

	private:
		QByteArray m_data;
	};


	class Ram
	{
	public:
		Ram();
		Ram(const Ram& that);
		Ram& operator=(const Ram& that);

	public:
		void reset();
		bool addMemoryArea(RamAccess access, quint32 offsetW, quint32 sizeW, QString name);			// offset and size in 16 bit words

		std::vector<RamAreaInfo> memoryAreasInfo() const;
		RamAreaInfo memoryAreaInfo(QString name) const;
		RamAreaInfo memoryAreaInfo(int index) const;

		bool writeBit(quint32 offsetW, quint32 bitNo, quint32 data, E::ByteOrder byteOrder);
		bool readBit(quint32 offsetW, quint32 bitNo, quint16* data, E::ByteOrder byteOrder) const;

		bool writeWord(quint32 offsetW, quint16 data, E::ByteOrder byteOrder);
		bool readWord(quint32 offsetW, quint16* data, E::ByteOrder byteOrder) const;

		bool writeDword(quint32 offsetW, quint32 data, E::ByteOrder byteOrder);
		bool readDword(quint32 offsetW, quint32* data, E::ByteOrder byteOrder) const;

		bool writeFloat(quint32 offsetW, float data, E::ByteOrder byteOrder);
		bool readFloat(quint32 offsetW, float* data, E::ByteOrder byteOrder) const;

		bool writeSignedInt(quint32 offsetW, qint32 data, E::ByteOrder byteOrder);
		bool readSignedInt(quint32 offsetW, qint32* data, E::ByteOrder byteOrder) const;

	private:
		RamArea* memoryArea(RamAccess access, quint32 offsetW);
		const RamArea* memoryArea(RamAccess access, quint32 offsetW) const;

	private:
		// Pay attention to copy operator
		//
		std::vector<std::shared_ptr<RamArea>> m_memoryAreas;
	};

}

#endif // LMRAM_H
