#include <iostream>
#include "../UtilsLib/WUtils.h"
#include <iostream>
#include <QCoreApplication>
#include <QFile>
#include <QDir>

#include "Print.h"

// #ifdef _WIN32
// #include <windows.h>
// #include <cstdio>
// #endif

#include "Archivist.h"
#include "FileArchivist.h"
#include "DbArchivist.h"


bool parseCmdLine(int argc, char* argv[], RequestParams* rp);
bool parseCfgFile(const QString& cfgFileName, RequestParams* rp);

int main(int argc, char* argv[])
{
	QCoreApplication app(argc, argv);


/*	logger = std::make_shared<CircularLogger>();

	LOGGER_INIT(logger, QString(), "");

	logger->setLogCodeInfo(false);*/

	RequestParams rp;

	bool result = 0;

	result = parseCmdLine(argc, argv, &rp);

	FileArchivist fa(rp);

	fa.copyArchive();

//	RETURN_VALUE_IF_FALSE(result, 0);

//	app.exec();

//	QThread::msleep(2000);

//	LOGGER_SHUTDOWN(logger);

//	app.exec();

	return result;
}

bool parseCmdLine(int argc, char* argv[], RequestParams* rp)
{
	TEST_PTR_RETURN_FALSE(rp);

	QStringList cmdLineParams;

	for(int i = 0; i < argc; i++)
	{
		cmdLineParams.append(argv[i]);
	}

//	LOG_MSG(logger, QString("Runing: %1").arg(cmdLineParams.join(" ")));

	bool result = true;

	if (argc == 1)
	{
		print << "\nUse: Archivist.exe -f=RequestCfgFile\n     Archivist.exe -exampleCfg\n\n";
		return false;
	}

	if (argc == 2)
	{
		if (cmdLineParams[1].toLower() == "-examplecfg")
		{
			QString fileName = QDir::currentPath() + "/RequestCfg.txt";

			QFile f(fileName);

			if (!f.open(QIODeviceBase::WriteOnly | QIODeviceBase::Truncate | QIODeviceBase::Text))
			{
				print << QString("\nError opening file %1\n\n").arg(fileName);
				return false;
			}

			QDateTime now = QDateTime::currentDateTime();
			QDateTime prev = now.addDays(-1);

			QTextStream ts(&f);

			const QChar ZERO('0');

			ts << "archive = D:/project/SYSTEM_RACK_WS_ARHS\t# location of archive\n";
			ts << QString("begin = %1:%2:%3 %4.%5.%6\t\t\t# request start time\n").
								arg(prev.time().hour(), 2, 10, ZERO).
								arg(prev.time().minute(), 2, 10, ZERO).
								arg(prev.time().second(), 2, 10, ZERO).
								arg(prev.date().day(), 2, 10, ZERO).
								arg(prev.date().month(), 2, 10, ZERO).
								arg(prev.date().year(), 4, 10, ZERO);

			ts << QString("end = %1:%2:%3 %4.%5.%6\t\t\t# request end time\n").
				  arg(now.time().hour(), 2, 10, ZERO).
				  arg(now.time().minute(), 2, 10, ZERO).
				  arg(now.time().second(), 2, 10, ZERO).
				  arg(now.date().day(), 2, 10, ZERO).
				  arg(now.date().month(), 2, 10, ZERO).
				  arg(now.date().year(), 4, 10, ZERO);

			ts << QString("signals = All\t\t\t\t\t# signals filter\n");
			ts << QString("destLocation = D:/Temp# archive copy location\n");

			f.close();

			print << QString("\nFile saved: %1\n\n").arg(fileName);
		}

		if (cmdLineParams[1].toLower().startsWith("-f"))
		{
			QStringList sl = cmdLineParams[1].split("=");

			if (sl.size() < 2)
			{
				print << QString("\nRequest configuration file name is not set!1\n\n");
				return false;
			}

			result = parseCfgFile(sl[1], rp);
		}
	}

	return result;
}

bool parseCfgFile(const QString& cfgFileName, RequestParams* rp)
{
	TEST_PTR_RETURN_FALSE(rp);

	QFile f(cfgFileName);

	if (f.open(QIODeviceBase::ReadOnly) == false)
	{
		print << QString("\nError open file: %1\n\n").arg(cfgFileName);
		return false;
	}

	bool result = true;

	QByteArray data = f.readAll();

	f.close();

	QStringList sl = QString(data).split("\n");

	int lineNo = 1;

	rp->signalsList = "All";

	for(int i = 0; i < sl.size(); i++, lineNo++)
	{
		QString s = sl[i].trimmed();

		if (s.startsWith("#") == true)
		{
			continue;
		}

		QStringList pl = s.split("#", Qt::SkipEmptyParts);

		if (pl.size() < 1)
		{
			continue;
		}

		s = pl[0];

		pl = s.split("=", Qt::SkipEmptyParts);

		if (pl.size() < 2)
		{
			print << QString("\nConfiguration param error in line %1\n\n").arg(lineNo);
			return false;
		}

		QString param = pl[0].trimmed().toLower();
		QString paramValue = pl[1].trimmed();

		//

		if (param == "archive")
		{
			if (paramValue.isEmpty())
			{
				print << QString("\nArchive location is not specified, line %1\n\n").arg(lineNo);
				return false;
			}

			if (QDir().exists(paramValue) == false)
			{
				print << QString("\nArchive location %1 is not found, line %2\n\n").
										arg(paramValue).arg(lineNo);
				return false;
			}

			rp->archiveLocation = QDir::toNativeSeparators(paramValue);
			continue;
		}

		//

		if (param == "begin")
		{
			QDateTime bdt = QDateTime::fromString(paramValue, "hh:mm:ss dd.MM.yyyy");

			if (bdt.isValid() == false)
			{
				print << QString("\nError archive copy begin time, line %1. Specify time on format HH:MM:SS DD.MM.YYYY.\n\n").arg(lineNo);
				return false;
			}

			rp->begin = bdt;
			continue;
		}

		//

		if (param == "end")
		{
			QDateTime edt = QDateTime::fromString(paramValue, "hh:mm:ss dd.MM.yyyy");

			if (edt.isValid() == false)
			{
				print << QString("\nError archive copy end time, line %1. Specify time on format HH:MM:SS DD.MM.YYYY.\n\n").arg(lineNo);
				return false;
			}

			rp->end = edt;
			continue;
		}

		//

		if (param == "signals")
		{
			if (paramValue.isEmpty())
			{
				rp->signalsList = "All";
			}
			rp->signalsList = paramValue;
			continue;
		}

		//

		if (param == "destlocation")
		{
			if (paramValue.isEmpty())
			{
				print << QString("\nArchive copy location is not specified, line %1\n\n").arg(lineNo);
				return false;
			}

			QString tempDir = paramValue + QString("/tempDir%1").arg(QDateTime::currentMSecsSinceEpoch());

			if (QDir().mkdir(tempDir) == false)
			{
				print << QString("\nArchive copy location is not writable, line %1\n\n").arg(lineNo);
				return false;
			}

			QDir().rmdir(tempDir);

			rp->destLocation = QDir::toNativeSeparators(paramValue);
			continue;
		}

		print << QString("\nUnknown configuration param in line %1\n\n").arg(lineNo);
		return false;
	}

	if (rp->archiveLocation.isEmpty() ||
		rp->begin.isValid() == false ||
		rp->end.isValid() == false ||
		rp->destLocation.isEmpty())
	{
		print << QString("\nConfiguration file parsing error!\n\n");
		return false;
	}

	if (rp->begin > rp->end)
	{
		QDateTime dt = rp->begin;
		rp->begin = rp->end;
		rp->end = dt;
	}

	return result;
}
