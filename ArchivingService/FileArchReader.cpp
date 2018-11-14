#include "FileArchReader.h"

bool operator < (const FileArchReader::ArchPartition& afi1, const FileArchReader::ArchPartition& afi2)
{
	return afi1.systemTime < afi2.systemTime;
}

// ---------------------------------------------------------------------------------------------
//
// FileArchReader class implementattion
//
// ---------------------------------------------------------------------------------------------

FileArchReader::FileArchReader(const QString& signalArchPath, E::TimeType timeType, qint64 startTime, qint64 endTime) :
	m_signalArchPath(signalArchPath),
	m_timeType(timeType),
	m_startTime(startTime),
	m_endTime(endTime)
{
}

bool FileArchReader::findData()
{
	bool result = false;

	result = getArchPartitionsInfo();

	if (result == false)
	{
		return false;
	}

	result = findStartPosition();

	if (result == false)
	{
		return false;
	}

	return true;
}

bool FileArchReader::getArchPartitionsInfo()
{
	// Arch file name format: 2018_12_31_23_59.saf (year_month_day_hour_minute.saf)

	QRegExp archFileNameTemplate(QString("2[0-9][0-9][0-9]_[0-1][0-9]_[0-3][0-9]_[0-2][0-9]_[0-5][0-9].%1").arg(ArchFile::EXTENSION));

	QDirIterator di(m_signalArchPath);

	while(di.hasNext() == true)
	{
		QString nextFilePath = di.next();

		if (nextFilePath.isEmpty() == true)
		{
			break;
		}

		QFileInfo fi = di.fileInfo();

		if (fi.isFile() == false &&
			fi.fileName().contains(archFileNameTemplate) == false)
		{
			continue;
		}

		ArchPartition api;

		api.fileName = fi.fileName();

		int year = api.fileName.mid(0, 4).toInt();
		int month = api.fileName.mid(5, 2).toInt();
		int day = api.fileName.mid(8, 2).toInt();
		int hour = api.fileName.mid(11, 2).toInt();
		int minute = api.fileName.mid(14, 2).toInt();

		api.date = QDateTime(QDate(year, month, day), QTime(hour, minute, 0, 0), Qt::TimeSpec::UTC);
		api.systemTime = api.date.toMSecsSinceEpoch();

		m_archPartitionsInfo.append(api);
	}

	return true;
}

bool FileArchReader::findStartPosition()
{
	int partitionsCount = m_archPartitionsInfo.count();

	if (partitionsCount == 0)
	{
		return false;
	}

	// 1) Sort m_archPartitionsInfo by systemTime ascending
	//

	qSort(m_archPartitionsInfo);

	// 2) Find LAST partition where systemTime < m_startTime

	int startPartitionIndex = -1;

	for(int i = 0; i < partitionsCount; i++)
	{
		const ArchPartition& api = m_archPartitionsInfo[i];

		if (api.systemTime >= m_startTime)
		{
			break;
		}

		startPartitionIndex = i;
	}

	if (startPartitionIndex == -1)
	{
		startPartitionIndex = 0;
	}
}

bool readRecord(ArchPartition& partition, qint64 recordNo);

