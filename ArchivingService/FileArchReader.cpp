#include "FileArchReader.h"

bool operator < (const FileArchReader::ArchFileInfo& afi1, const FileArchReader::ArchFileInfo& afi2)
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

	result = getArchFileInfo();

	return true;
}

bool FileArchReader::getArchFileInfo()
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

		ArchFileInfo afi;

		afi.fileName = fi.fileName();

		int year = afi.fileName.mid(0, 4).toInt();
		int month = afi.fileName.mid(5, 2).toInt();
		int day = afi.fileName.mid(8, 2).toInt();
		int hour = afi.fileName.mid(11, 2).toInt();
		int minute = afi.fileName.mid(14, 2).toInt();

		afi.date = QDateTime(QDate(year, month, day), QTime(hour, minute, 0, 0), Qt::TimeSpec::UTC);
		afi.systemTime = afi.date.toMSecsSinceEpoch();

		m_archFileInfo.append(afi);
	}

	qSort(m_archFileInfo);

	return true;
}


