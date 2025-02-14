#include "ArchUtils.h"
#include <CommonLib/Times.h>

using namespace std;

ArchUtils::ArchUtils(const QString& workDir) :
	m_workDir(workDir)
{
}

void ArchUtils::dump(const QString& archFile, bool lt, bool st, bool pt)
{
	QFile rdFile(archFile);

	print << archFile;

	if (rdFile.open(QIODevice::ReadOnly) == false)
	{
		cout << "Arch file open error";
		return;
	}

	QString dumpFileName = archFile + ".dump";

	QFile wrFile(dumpFileName);

	if (wrFile.open(QIODevice::WriteOnly | QIODevice::Text) == false)
	{
		cout << "Dump file creation error";
		return;
	}

	QTextStream dump(&wrFile);

	QDateTime dateTime;

	bool res = getFileStartTime(archFile, &dateTime);

	if (res == false)
	{
		cout << "File name parsing error";
		return;
	}

	dump << QString("Partition start time: %1 UTC+0\n\n").arg(getTimeStr(dateTime));

	const QString SEPARATOR("   ");

	dump << "Offset        ";
	dump << "PcktNo  ";

	if (lt == true)
	{
		dump << QString("Local Time").leftJustified(23, ' ') << SEPARATOR;
	}

	if (st == true)
	{
		dump << QString("System Time (UTC+0)").leftJustified(39, ' ') << SEPARATOR;
	}

	if (pt == true ||
		(pt == false && lt == false && st == false))
	{
		dump << QString("Plant Time").leftJustified(23, ' ') << SEPARATOR;
	}

	dump << QString("State Flags").leftJustified(35, ' ') << SEPARATOR;
	dump << QString("Archive Reason").leftJustified(24, ' ') << SEPARATOR;
	dump << QString("Value");

	dump << "\n\n";

	int recordNo = 0;
	qint64 offset = 0;

	do
	{
		ArchFileRecord record;

		qint64 read = rdFile.read(reinterpret_cast<char*>(&record), sizeof(ArchFileRecord));

		if (read < sizeof(ArchFileRecord))
		{
			break;
		}

		dump << QString("%1  ").arg(offset, 12, 10, QChar('0'));

		if (record.isNotCorrupted() == false)
		{
			dump << QString("Corrupted record (index %1)\n").arg(recordNo);
			recordNo++;
			offset += ArchFileRecord::SIZE;
			continue;
		}

		recordNo++;

		dump << QString("%1").arg(record.state.packetNo, 5, 10, QChar('0'));
		dump << SEPARATOR;

		if (lt == true)
		{
			dump << getTimeStr(record.getTime(E::TimeType::Local)) << SEPARATOR;
		}

		if (st == true)
		{
			dump << QString("%1").arg(record.state.systemTime, 13, 10, QChar('0'));
			dump << SEPARATOR << getTimeStr(record.getTime(E::TimeType::System)) << SEPARATOR;
		}

		if (pt == true ||
			(pt == false && lt == false && st == false))
		{
			dump << getTimeStr(record.getTime(E::TimeType::Plant)) << SEPARATOR;
		}

		dump << getFlagsStr(record) << SEPARATOR;

		dump << getValueStr(record);

		dump << "\n";

		offset += ArchFileRecord::SIZE;
	}
	while(1);

	dump << QString("\nRecords processed: %1").arg(recordNo);

	std::cout << (QString("Dump file - %1").arg(dumpFileName)).toStdString();
}

bool ArchUtils::getFileStartTime(const QString& fileName, QDateTime* dt)
{
	QFileInfo fi(fileName);

	if (fileName.endsWith(LONG_TERM_ARCHIVE_EXTENSION) == false &&
		fileName.endsWith(SHORT_TERM_ARCHIVE_EXTENSION) == false)
	{
		return false;
	}

	QString fn = fi.fileName();

	int year = fn.mid(0, 4).toInt();
	int month = fn.mid(5, 2).toInt();
	int day = fn.mid(8, 2).toInt();
	int hour = fn.mid(11, 2).toInt();
	int minute = fn.mid(14, 2).toInt();

	* dt = QDateTime(QDate(year, month, day), QTime(hour, minute, 0, 0), TIME_ZONE_UTC);

	return true;
}

QString ArchUtils::getTimeStr(const QDateTime& pt)
{
	return QString("%1.%2.%3 %4:%5:%6.%7").
					arg(pt.date().year()).arg(pt.date().month(), 2, 10, QChar('0')).arg(pt.date().day(), 2, 10, QChar('0')).
					arg(pt.time().hour(), 2, 10, QChar('0')).arg(pt.time().minute(), 2, 10, QChar('0')).
					arg(pt.time().second(), 2, 10, QChar('0')).arg(pt.time().msec(), 3, 10, QChar('0'));
}

QString ArchUtils::getTimeStr(qint64 ms)
{
	QDateTime dt = QDateTime::fromMSecsSinceEpoch(ms, TIME_ZONE_UTC);

	return getTimeStr(dt);
}

QString ArchUtils::getLocalTimeStr(const ArchFileRecord& ar)
{
	QString str = getTimeStr(ar.state.localTime) + " loc";
	return str;
}

QString ArchUtils::getSystemTimeStr(const ArchFileRecord& ar)
{
	QString str = getTimeStr(ar.state.systemTime) + " sys";
	return str;
}

QString ArchUtils::getPlantTimeStr(const ArchFileRecord& ar)
{
	QString str = getTimeStr(ar.state.plantTime) + " pln";
	return str;
}

QString ArchUtils::getFlagsStr(const ArchFileRecord& ar)
{
	return QString("[ %1 %2 %3 %4 %5 %6 %7 ]   [ %8 %9 %10 %11 %12 %13 ]").

			arg(ar.state.flags.valid == 1 ? "VLD" : "NVL").
			arg(ar.state.flags.stateAvailable == 1 ? "AVL " : "NAVL").
			arg(ar.state.flags.simulated == 1 ? "SIM " : "   ").
			arg(ar.state.flags.blocked == 1 ? "BLK " : "   ").
			arg(ar.state.flags.mismatch == 1 ? "MISM" : "    ").
			arg(ar.state.flags.aboveHighLimit == 1 ? "HLIM" : "    ").
			arg(ar.state.flags.belowLowLimit == 1 ? "LLIM" : "    ").

			arg(ar.state.flags.validityChange == 1 ? "VAL" : "   ").
			arg(ar.state.flags.simBlockMismatchChange == 1 ? "SBM" : "   ").
			arg(ar.state.flags.limitFlagsChange == 1 ? "LIM" : "   ").
			arg(ar.state.flags.autoPoint == 1 ? "AP" : "  ").
			arg(ar.state.flags.fineAperture == 1 ? "FA" : "  ").
			arg(ar.state.flags.coarseAperture == 1 ? "CA" : "  ");
}

QString ArchUtils::getValueStr(const ArchFileRecord& ar)
{
	return QString("%1").arg(ar.state.value);
}




