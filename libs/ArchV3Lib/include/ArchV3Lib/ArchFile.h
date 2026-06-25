#pragma once

#include <limits>
#include <vector>

#include <QtGlobal>
#include <QFile>

namespace ArchV3
{
	inline constexpr quint32 FLAG_PLANT_TIME_VALID = 0x20000000;

	class ArchWriter;

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
		ArchFileBase(ArchWriter& archWriter);
		virtual ~ArchFileBase();

		bool setFilePath(const QString& path);
		bool setFileName(const QString& filename);

		// Getters

		bool isOpen() const;
		qint64 fileSize() const;
		qint64 lastWriteTime() const;
		qint64 lastFlushTime() const;
		virtual size_t bufferedRecordsCount() const = 0;

		//

		bool openFile();
		void closeFile();

		virtual bool write(qint64 timeUTC) = 0;
		virtual bool flush(qint64 timeUTC);

	protected:
		bool writeRaw(const char* data, qint64 dataSize, qint64 timeUTC);

	private:
		ArchWriter& m_archWriter;
		QString m_path;
		QString m_filename;

		QFile m_file;

		qint64 m_fileSize = 0;
		qint64 m_lastWriteTime = 0;
		qint64 m_lastFlushTime = 0;
	};

	template<typename RecordTypeT>
	class ArchFile : public ArchFileBase
	{
	public:
		ArchFile(ArchWriter& archWriter) :
			ArchFileBase(archWriter)
		{
		}

		~ArchFile()
		{
		}

		bool isDiscreteFile() const { return (sizeof(RecordTypeT) == DISCRETE_FILE_RECORD_SIZE); }
		bool isAnalogFile() const { return (sizeof(RecordTypeT) == ANALOG_FILE_RECORD_SIZE); }

		void append(const RecordTypeT& record) { m_records.push_back(record); }

		bool write(qint64 timeUTC) override
		{
			qint64 recordsSize = static_cast<qint64>(m_records.size() * sizeof(RecordTypeT));

			if (recordsSize == 0)
			{
				return true;
			}

			bool result = writeRaw(reinterpret_cast<const char*>(m_records.data()), recordsSize, timeUTC);

			if (result == true)
			{
				m_records.clear();
			}

			return result;
		}

		bool flush(qint64 timeUTC) override
		{
			bool result = write(timeUTC);

			if (result == true)
			{
				result = ArchFileBase::flush(timeUTC);
			}

			return result;
		}

		size_t bufferedRecordsCount() const override
		{ 
			return m_records.size();
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