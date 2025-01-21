#include <iostream>
#include "../../UtilsLib/WUtils.h"
#include <iostream>
#include <QCoreApplication>
#include <QFile>
#include <QDir>

#ifdef _WIN32
#include <windows.h>
#include <cstdio>
#endif

#include "Archivist.h"
#include "FileArchivist.h"
#include "DbArchivist.h"
#include <QLoggingCategory>

bool parseCmdLine(int argc, char* argv[], RequestParams* rp);

int main(int argc, char* argv[])
{
	QCoreApplication app(argc, argv);

//	QLoggingCategory::setFilterRules(QStringLiteral("*.debug=true"));

//	QLoggingCategory::defaultCategory()->setEnabled(QtDebugMsg, true);

	std::cout << "Std::Hello!";
	qDebug() << "Hello!";

/*#ifdef _WIN32
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);
#endif*/

/*	logger = std::make_shared<CircularLogger>();

	LOGGER_INIT(logger, QString(), "");

	logger->setLogCodeInfo(false);*/

//	RequestParams rp;

//	bool result = 0;

//	result = parseCmdLine(argc, argv, &rp);

//	RETURN_VALUE_IF_FALSE(result, 0);

//	app.exec();

//	QThread::msleep(2000);

//	LOGGER_SHUTDOWN(logger);

//	app.exec();

	return 0;
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
		qDebug() << "\nUse: Archivist.exe -f=RequestCfgFile\n     Archivist.exe -exampleCfg";
		return false;
	}

	if (argc == 2)
	{
		if (cmdLineParams[1].toLower() == "-examplecfg")
		{
			QString fileName = QDir::currentPath() + "/RequestCfg.txt";

			QFile f(fileName);

			if (f.open(QIODeviceBase::WriteOnly | QIODeviceBase::Truncate | QIODeviceBase::Text))
			{
				qDebug() << QString("Error opening file %1").arg(fileName);
				return false;
			}

		}
	}

	return result;
}
