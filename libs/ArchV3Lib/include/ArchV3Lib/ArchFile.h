#pragma once

#include <limits>
#include <vector>

#include <QtGlobal>
#include <QFile>

#include <AppSignal.pb.h>

#include "../../../../UtilsLib/WUtils.h"

#include "DbTypes.h"

namespace ArchV3
{
	inline constexpr quint32 FLAG_PLANT_TIME_VALID = 0x20000000;

	class ArchWriter;

#pragma pack(push, 1)

	struct AnalogFileRecord
	{
		qint64 serverTimeUTC;
		qint32 plantTimeDelta;
		qint16 localTimeOffsetMinutes;

		quint32 flags;
		double value;

		quint8 crc8;
	};

	struct DiscreteFileRecord
	{
		qint64 serverTimeUTC;
		qint32 plantTimeDelta;
		qint16 localTimeOffsetMinutes;

		quint32 flags;
		quint8 state;

		quint8 crc8;
	};

#pragma pack(pop)

	static_assert(std::is_trivially_copyable_v<AnalogFileRecord>);
	static_assert(std::is_trivially_copyable_v<DiscreteFileRecord>);

	inline constexpr size_t ANALOG_FILE_RECORD_SIZE = 27;
	static_assert(sizeof(AnalogFileRecord) == ANALOG_FILE_RECORD_SIZE);

	inline constexpr size_t DISCRETE_FILE_RECORD_SIZE = 20;
	static_assert(sizeof(DiscreteFileRecord) == DISCRETE_FILE_RECORD_SIZE);

	enum class CheckFileResult
	{
		Matched,
		Changed,
		CheckError
	};

	class ArchFileBase
	{
	public:
		ArchFileBase(const QString& archPath, const QString& appSignalID);
		virtual ~ArchFileBase();

		QString appSignalID() const;
		bool setFilePath(const QString& path);
		bool setFileName(const QString& filename);

		void setActiveFile(const ArchFileInfo& afi);
		bool hasActiveFile() const;

		static size_t recordSize(E::SignalType st);
		static CheckFileResult checkFile(const QString& archPath, const ArchFileInfo& afi, ArchFileInfo* checkedAfi);

		// Getters

		bool isOpen() const;
		bool openFile();
		void closeFile();
		bool flushBuffer(const char* data, size_t recordsCount, qint64 timeUTC);
		bool write(qint64 timeUTC);

		qint64 fileSize() const;
		qint64 lastWriteTime() const;
		qint64 lastFlushTime() const;

		bool prepareForNextState(ArchFileInfo* afi);

		virtual bool pushState(const Proto::AppSignalState& state, qint64 timeUTC, ArchFileInfo* actualAfi) = 0;
		virtual size_t bufferSize() const = 0;
		virtual size_t recordSize() const = 0;
		virtual bool bufferIsFull() const = 0;

		static void readClusterSize(const QString& archDir);
		static quint32 clusterSize();

	protected:
		bool writeRaw(const char* data, qint64 dataSize, qint64 timeUTC);

	private:
		const QString& m_archPath;
		QString m_appSignalID;
		QString m_filename;

		qint64 m_archFileID = 0;
		qint64 m_fileSize = 0;
		qint64 m_recordCount = 0;
		qint64 m_lastWriteTime = 0;
		qint64 m_lastFlushTime = 0;

		QFile m_file;

		inline static quint32 m_clusterSize = 0;
	};

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

	template<typename RecordTypeT>
	class ArchFile : public ArchFileBase
	{
	public:
		ArchFile(const QString& archPath, const QString& appSignalID) :
			ArchFileBase(archPath, appSignalID)
		{
		}

		~ArchFile()
		{
		}

		bool isDiscreteFile() const { return (sizeof(RecordTypeT) == DISCRETE_FILE_RECORD_SIZE); }
		bool isAnalogFile() const { return (sizeof(RecordTypeT) == ANALOG_FILE_RECORD_SIZE); }

		bool pushState(const Proto::AppSignalState& state, qint64 timeUTC, ArchFileInfo* actualAfi) override
		{
			TEST_PTR_RETURN_FALSE(actualAfi);

			RecordTypeT record;

			record.serverTimeUTC = state.systemtime();

			bool plantTimeValid = makePlantTimeDelta(state.systemtime(), state.planttime(), record.plantTimeDelta);

			record.flags = state.flags();

			if (plantTimeValid == true)
			{
				record.flags |= FLAG_PLANT_TIME_VALID;
			}
			else
			{
				record.flags &= (~FLAG_PLANT_TIME_VALID);
			}

			if constexpr (std::is_same_v<RecordTypeT, AnalogFileRecord>)
			{
				record.value = state.value();
			}
			else if constexpr (std::is_same_v<RecordTypeT, DiscreteFileRecord>)
			{
				record.state = state.value() == 0 ? 0 : 1;
			}

			return pushToBuffer(record, timeUTC);
		}

		bool pushToBuffer(const RecordTypeT& record, qint64 timeUTC)
		{
			bool flushed = false;

			if (bufferIsFull())
			{
				// Defensive check. The buffer should never be full at this point.
				//
				flushed |= flushBuffer(reinterpret_cast<const char*>(m_buffer.data()), m_buffer.size(), timeUTC);
			}

			m_buffer.emplace_back(record);

			if (bufferIsFull())
			{
				flushed |= flushBuffer(reinterpret_cast<const char*>(m_buffer.data()), m_buffer.size(), timeUTC);
			}

			return flushed;
		}

		size_t bufferSize() const override
		{ 
			return m_buffer.size();
		}

		size_t recordSize() const override
		{ 
			return sizeof(RecordTypeT);
		}

		bool bufferIsFull() const override
		{ 
			return (m_buffer.size() == m_buffer.capacity());
		}

		private:
			std::vector<RecordTypeT> m_buffer;
	};

	using AnalogArchFile = ArchFile<AnalogFileRecord>;
	using DiscreteArchFile = ArchFile<DiscreteFileRecord>;

} // namespace ArchV3Lib