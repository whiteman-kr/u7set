#pragma once

#include <limits>
#include <vector>

#include <QtGlobal>
#include <QFile>

namespace ArchV3Lib
{
	inline constexpr quint32 FLAG_PLANT_TIME_VALID = 0x20000000;

#pragma pack(push, 1)

	struct AnalogFileRecord
	{
		qint64 serverTimeUTC;
		qint32 plantTimeDelta;
		quint32 flags;
		double value;
	};

	struct DiscreteFileRecord
	{
		qint64 serverTimeUTC;
		qint32 plantTimeDelta;
		quint32 flags;
		quint8 state;
	};

#pragma pack(pop)

	inline constexpr size_t ANALOG_FILE_RECORD_SIZE = 24;
	static_assert(sizeof(AnalogFileRecord) == ANALOG_FILE_RECORD_SIZE);

	inline constexpr size_t DISCRETE_FILE_RECORD_SIZE = 17;
	static_assert(sizeof(DiscreteFileRecord) == DISCRETE_FILE_RECORD_SIZE);

	class ArchFileBase
	{
	public:
		ArchFileBase(const QString& filename) :
			m_file(filename)
		{
		}

		~ArchFileBase()
		{
			if (m_file.isOpen())
			{
				m_file.close();
			}
		}

		bool openFile()
		{ 
			if (m_file.isOpen() == true)
			{
				return true;
			}

			if (m_file.open(QIODevice::WriteOnly | QIODevice::Append) == false)
			{
				// log error
				return false;
			}

			return true;
		}

		void closeFile()
		{ 
			if (m_file.isOpen() == true)
			{
				m_file.close();
			}
		}

		bool write(const char* data, qint64 dataSize)
		{
			Q_ASSERT(dataSize >= 0);

			if (dataSize == 0)
			{
				return true;
			}

			if (openFile() == false)
			{
				return false;
			}

			const qint64 written = m_file.write(data, dataSize);

			if (written != dataSize)
			{
				// log error
				// truncate file to integral size
				return false;
			}

			return true;
		}

		void flush()
		{ 
			if (m_file.isOpen() == true)
			{
				m_file.flush();
			}
		}

	private:
		QFile m_file;
	};

	template<typename RecordTypeT>
	class ArchFile : public ArchFileBase
	{
	public:
		ArchFile(const QString& filename) :
			ArchFileBase(filename)
		{
		}

		~ArchFile()
		{
			write();
			flush();
		}

		bool isDiscreteFile() const { return (sizeof(RecordTypeT) == DISCRETE_FILE_RECORD_SIZE); }
		bool isAnalogFile() const { return (sizeof(RecordTypeT) == ANALOG_FILE_RECORD_SIZE); }

		void append(const RecordTypeT& record) { m_records.push_back(record); }

		bool write()
		{
			qint64 recordsSize = static_cast<qint64>(m_records.size() * sizeof(RecordTypeT));

			if (recordsSize == 0)
			{
				return true;
			}

			bool result = ArchFileBase::write(reinterpret_cast<const char*>(m_records.data()), recordsSize);

			if (result == true)
			{
				m_records.clear();
			}

			return true;
		}

		private:
			std::vector<RecordTypeT> m_records;
	};

	using AnalogArchFile = ArchFile<AnalogFileRecord>;
	using DiscreteArchFile = ArchFile<DiscreteFileRecord>;

	inline bool makePlantTimeDelta(qint64 serverTimeUTC, qint64 plantTime, qint32& plantTimeDelta)
	{
		const qint64 delta = plantTime - serverTimeUTC;

		if (delta < std::numeric_limits<qint32>::min() || delta > std::numeric_limits<qint32>::max())
		{
			plantTimeDelta = 0;
			return false;
		}

		plantTimeDelta = static_cast<qint32>(delta);
		return true;
	}

	inline qint64 restorePlantTime(qint64 serverTimeUTC, qint32 plantTimeDelta)
	{ 
		return serverTimeUTC + plantTimeDelta; 
	}

} // namespace ArchV3Lib