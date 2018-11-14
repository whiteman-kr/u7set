#pragma once

#include <QtGlobal>
#include <QDateTime>

#include "../lib/Types.h"
#include "FileArchWriter.h"

class FileArchReader
{
public:
	struct ArchPartition
	{
		QString fileName;
		QDateTime date;
		qint64 systemTime = 0;
		int recordsCount = 0;
	};

public:
	FileArchReader(const QString& signalArchPath, E::TimeType timeType, qint64 startTime, qint64 endTime);

	bool findData();

private:


private:
	bool getArchPartitionsInfo();
	bool findStartPosition();

	static const qint64 FIRST_RECORD = 0;
	static const qint64 LAST_RECORD = -1;

	bool readRecord(ArchPartition& partition, qint64 recordNo);

private:
	QString m_signalArchPath;
	E::TimeType m_timeType = E::TimeType::System;
	qint64 m_startTime = 0;
	qint64 m_endTime = 0;

	//

	QVector<ArchPartition> m_archPartitionsInfo;
};

bool operator < (const FileArchReader::ArchPartition& afi1, const FileArchReader::ArchPartition& afi2);

