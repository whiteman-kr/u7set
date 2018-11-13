#pragma once

#include <QtGlobal>
#include <QDateTime>

#include "../lib/Types.h"
#include "FileArchWriter.h"

class FileArchReader
{
public:
	struct ArchFileInfo
	{
		QString fileName;
		QDateTime date;
		qint64 systemTime;
	};

public:
	FileArchReader(const QString& signalArchPath, E::TimeType timeType, qint64 startTime, qint64 endTime);

	bool findData();

private:


private:
	bool getArchFileInfo();

private:
	QString m_signalArchPath;
	E::TimeType m_timeType = E::TimeType::System;
	qint64 m_startTime = 0;
	qint64 m_endTime = 0;

	//

	QVector<ArchFileInfo> m_archFileInfo;
};

bool operator < (const FileArchReader::ArchFileInfo& afi1, const FileArchReader::ArchFileInfo& afi2);

