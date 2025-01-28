#pragma once

#include "../OnlineLib/CircularLogger.h"
#include "../AppSignalLib/SimpleAppSignalState.h"
#include "ArchFileBuffer.h"
#include "ArchFilePartition.h"

namespace Proto
{
	class ArchSignal;
}

class ArchFile
{
public:
	inline static const int QUEUE_MAX_SIZE = 1280;	// == QUEUE_MIN_SIZE * 128 (2^7)

private:
	inline static const int QUEUE_MIN_SIZE = 10;

	inline static const double QUEUE_EMERGENCY_LIMIT = 0.7;		// 70%;
	inline static const double QUEUE_EXPAND_LIMIT = 0.8;		// 80%;
	inline static const double QUEUE_REDUCTION_LIMIT = 0.2;		// 20%;

public:
	ArchFile(const Proto::ArchSignal& protoArchSignal, const QString& archFullPath, CircularLoggerShared log);
	~ArchFile();

	bool pushState(const SimpleAppSignalState& state);

	bool flush(qint64 curPartition,
			   qint64* totalFushedStatesCount,
			   bool flushAnyway,
			   int minQueueSizeForFlushing,
			   ArchFileRecord* buffer,
			   int bufferSize,
			   const QThread* thread);

	void setRequiredImmediatelyFlushing(bool b) { m_requiredImmediatelyFlushing.store(b); }
	bool isRequiredImmediatelyFlushing() const { return m_requiredImmediatelyFlushing.load(); }

	Hash hash() const { return m_hash; }
	quint32 group() const { return static_cast<quint32>(m_hash & 0xFF); }

	QString appSignalID() const { return m_appSignalID; }

	bool isAnalog() const { return m_isAnalog; }
	bool isEmergency() const;

	QString path() const { return m_path; }

	static QVector<ArchFilePartition::Info> getArchPartitionsInfo(const QString& path);
	static QString getPartitionFileName(const QString& archFilePath, const ArchFilePartition::Info& pi);

	void shutdown(qint64 curPartition,
				  qint64* totalFlushedStatesCount,
				  ArchFileRecord* buffer,
				  int bufferSize,
				  const QThread* thread);

	bool maintenance(qint64 currentPartition,
					 qint64 msShortTermPeriod,
					 qint64 msLongTermPeriod,
					 int* deletedCount,
					 int* packedCount,
					 const RunOverrideThread* thread);
private:
	void startMaintenance();
	void stopMaintenance();

	bool packPartitions(const QVector<ArchFilePartition::Info>& partitionsInfo,
							qint64 currentPartition,
							qint64 msShortTermPeriod,
							int* packedCount,
							const RunOverrideThread* thread);

	bool packAnalogSignalPartition(const ArchFilePartition::Info& pi, const RunOverrideThread* thread);
	bool writeLtaFile(QFile& ltaFile, const char* buffer, int size);

	bool packDiscreteSignalPartition(const ArchFilePartition::Info& pi);

	bool deleteOldPartitions(const QVector<ArchFilePartition::Info>& partitionsInfo,
								qint64 currentPartition,
								qint64 msLongTermPeriod,
								int* deletedCount,
								const RunOverrideThread *thread);

	QString getPartitionFileName(const ArchFilePartition::Info& pi);

	void controlQueueSizeBeforePush(const QThread* thread);

private:
	CircularLoggerShared m_log;
	Hash m_hash = 0;
	QString m_appSignalID;
	bool m_isAnalog = false;

	//

	bool m_lastRecordInitialized = false;
	ArchFileRecord m_lastRecord;

	QMutex m_fileInMaintenanceMutex;
	bool m_fileInMaintenance = false;

	int m_statesCountAfterExpand = -1;

	//

	QString m_path;

	//

	ArchFilePartition m_writablePartition;

	FastThreadSafeQueue<ArchFileRecord> m_queue;

	std::atomic<bool> m_requiredImmediatelyFlushing = { false };
};


