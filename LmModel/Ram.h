#ifndef LMRAM_H
#define LMRAM_H
#include <QByteArray>
#include <map>
#include <vector>
#include <memory>

namespace LmModel
{
	enum class RamAccess
	{
		ReadOnly  = 0x01,
		WriteOnly = 0x02,
		ReadWrite = 0x03
	};

	class RamAreaInfo
	{
	public:
		RamAreaInfo() = default;
		RamAreaInfo(const RamAreaInfo&) = default;
		RamAreaInfo(RamAccess access, quint32 offset, quint32 size, QString name);

	public:
		bool contains(RamAccess access, quint32 offset) const;
		bool overlapped(RamAccess access, quint32 offset, quint32 size) const;

	private:
		QString m_name;
		RamAccess m_access = RamAccess::ReadOnly;
		quint32 m_offset = 0xFFFFFFFF;
		quint32 m_size = 0;
	};


	class RamArea : public RamAreaInfo
	{
	public:
		RamArea(RamAccess access, quint32 offset, quint32 size, QString name);

	private:
		QByteArray m_data;
	};

	class Ram
	{
	public:
		Ram();

	public:
		void reset();
		bool addMemoryArea(RamAccess access, quint32 offset, quint32 size, QString name);			// offset and size in 16 bit words

	private:
		std::vector<std::shared_ptr<RamArea>> m_memoryAreas;
	};

}

#endif // LMRAM_H
