#include "ArchUtils.h"

using namespace std;

ArchUtils::ArchUtils(const QString& workDir) :
	m_workDir(workDir)
{
}

void ArchUtils::dump(const QString& archFile, bool lt, bool st, bool pt)
{
	QRegExp archFileNameTemplate(ARCH_FILE_NAME_TEMPLATE);

	if (archFile.contains(archFileNameTemplate) == false)
	{
		cout << "Arch file name is wrong";
		return;
	}

	QFile rdFile(archFile);

	if (rdFile.open(QIODevice::ReadOnly) == false)
	{
		cout << "Arch file open error";
		return;
	}

	QFile wrFile(archFile + ".dump");

	if (wrFile.open(QIODevice::WriteOnly | QIODevice::Text) == false)
	{
		cout << "Dump file creation error";
		return;
	}

	QTextStream dump(&wrFile);

	QDateTime dt;

	bool res = getFileStartTime(archFile, &dt);

	if (res == false)
	{
		cout << "File name parsing error";
		return;
	}

	dump << QString("Partition start time: %1 sys\n\n").arg(getTimeStr(dt));

	int recordNo = 0;

	do
	{
		ArchFileRecord record;

		qint64 read = rdFile.read(reinterpret_cast<char*>(&record), sizeof(ArchFileRecord));

		if (read < sizeof(ArchFileRecord))
		{
			break;
		}

		if (record.isNotCorrupted() == false)
		{
			dump << QString("Corrupted record (index %1)\n").arg(recordNo);
			recordNo++;
			continue;
		}

		recordNo++;

		dump << QString("%1    ").arg(record.state.packetNo, 5, 10, QChar('0'));

		QDateTime dt;

		if (lt == false && st == false && pt == false)
		{
			dump << getLocalTimeStr(record) << "    ";
			dump << getSystemTimeStr(record) << "    ";
			dump << getPlantTimeStr(record) << "    ";
		}
		else
		{
			if (lt == true)
			{
				dump << getLocalTimeStr(record) << "    ";
			}

			if (st == true)
			{
				dump << getSystemTimeStr(record) << "    ";
			}

			if (pt == true)
			{
				dump << getPlantTimeStr(record) << "    ";
			}
		}

		dump << getFlagsStr(record) << "    ";

		dump << getValueStr(record) << "    ";

		dump << "\n";
	}
	while(1);

	dump << QString("\nRecords processed: %1").arg(recordNo);
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

	* dt = QDateTime(QDate(year, month, day), QTime(hour, minute, 0, 0), Qt::TimeSpec::UTC);

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
	QDateTime dt = QDateTime::fromMSecsSinceEpoch(ms, Qt::OffsetFromUTC);

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
	return QString("st = [ %1 %2 %3 %4 %5 %6 %7 ] rs = [ %8 %9 %10 %11 %12 %13 ]").

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




