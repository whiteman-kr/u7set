#pragma once

#include <QtGlobal>
#include <QDateTime>

#include "../lib/Types.h"
#include "ArchWriterThread.h"

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
	FileArchReader(Archive* archive, const ArchRequestParam& param);

	bool findData();

private:


private:
	bool getArchPartitionsInfo();
	bool findStartPosition();

	static const qint64 FIRST_RECORD = 0;
	static const qint64 LAST_RECORD = -1;

	bool readRecord(ArchPartition& partition, qint64 recordNo);

private:
	Archive* m_archive = nullptr;
	ArchRequestParam m_requestParam;

	//

	QVector<ArchPartition> m_archPartitionsInfo;
};

bool operator < (const FileArchReader::ArchPartition& afi1, const FileArchReader::ArchPartition& afi2);

