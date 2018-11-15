#pragma once

#include "../lib/AppSignal.h"
#include "../lib/WUtils.h"
#include "../lib/Crc16.h"
#include "../lib/SimpleThread.h"

class Archive;

class Archive
{
	class Signal;
};

class FileArchWriter;

class ArchFile
{
private:
#pragma pack(push, 1)

	struct Record
	{
		struct
		{
			qint64 archID;
			qint64 systemTime;
			qint64 plantTime;

			AppSignalStateFlags flags;
			double value;
		} state;

		quint16 crc16;

		bool isValid() const;
	};

#pragma pack(pop)

	class Partition
	{

	};

public:
	ArchFile(Archive* archive, Archive::Signal* archiveSignal);
	~ArchFile();

	bool init(const FileArchWriter* writer, const QString& signalID, Hash hash, bool isAnalogSignal);

	bool pushState(qint64 archID, const SimpleAppSignalState& state);

	bool flush(qint64 curPartition, qint64* totalFushedStatesCount);

	bool queueIsEmpty() const { return m_queue->isEmpty(); }

	bool isEmergency() const;

	void shutdown(qint64 curPartition, qint64* totalFlushedStatesCount);

	QString path() const { return m_path; }

	static const QString EXTENSION;

private:
	bool writeFile(qint64 partitionSystemTime, Record* buffer, int statesCount, qint64* totalFushedStatesCount);
	void closeFile();

private:
	Archive* m_archive = nullptr;
	Archive::Signal* m_archiveSignal = nullptr;

	QString m_path;

	//

	QFile m_file;
	bool m_pathIsExists = false;
	bool m_fileIsOpened = false;
	bool m_fileIsAligned = false;
	qint64 m_prevPartition = -1;

	FastQueue<Record>* m_queue = nullptr;

	SimpleMutex m_flushMutex;

	const double QUEUE_EMERGENCY_LIMIT = 0.7;		// 70%
	const double QUEUE_EXPAND_LIMIT = 0.9;			// 90%

	static const int QUEUE_MIN_SIZE = 20;
	static const int QUEUE_MAX_SIZE = 1280;		// 20 * 2^6

	static Record m_buffer[QUEUE_MAX_SIZE];
};
