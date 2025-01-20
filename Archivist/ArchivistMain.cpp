#include <iostream>
#include "../UtilsLib/WUtils.h"
#include <iostream>
#include <QCoreApplication>

#include "Archivist.h"
#include "FileArchivist.h"
#include "DbArchivist.h"

bool parseCmdLine(int argc, char* argv[], RequestParams* rp);

int main(int argc, char* argv[])
{
	QCoreApplication app(argc, argv);

	logger = std::make_shared<CircularLogger>();

	LOGGER_INIT(logger, QString(), "");

	logger->setLogCodeInfo(false);

	RequestParams rp;

	bool result = 0;

	result = parseCmdLine(argc, argv, &rp);

	if (result)
	{

//	Archivist* archivist = new FileArchivist(argc, argv);

//		archivist->start();

	//google::protobuf::ShutdownProtobufLibrary();

	}

	app.exec();

	QThread::msleep(2000);

	LOGGER_SHUTDOWN(logger);

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

	LOG_MSG(logger, QString("Runing: %1").arg(cmdLineParams.join(" ")));

	bool result = true;

	if (argc == 1)
	{
		std::cout << "\nUse: Archivist.exe -f=RequestCfgFile";
		std::cout << "\n     Archivist.exe -exampleCfg";
		return false;
	}

	if (argc == 2)
	{
		if (cmdLineParams[2].toLower() == "-exampleCfg")
		{
			QFile f;
		}
	}

	return result;
}
