#include <gtest/gtest.h>
#include <QCoreApplication>
#include "Common.h"
#include <CommonLib/ConstStrings.h>

#ifdef Q_OS_WIN
#include <windows.h>
#include <crtdbg.h>
#endif

std::shared_ptr<CircularLogger> logger;
QString buildPath;
QString profileName;

QByteArray appDataService_configurationXml;

bool loadAppDataServiceCfgXml();

int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN
	_set_error_mode(_OUT_TO_STDERR);

	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
	_CrtSetReportMode(_CRT_ERROR,  _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ERROR,  _CRTDBG_FILE_STDERR);
	_CrtSetReportMode(_CRT_WARN,   _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN,   _CRTDBG_FILE_STDERR);

	SetErrorMode(SEM_FAILCRITICALERRORS |
				 SEM_NOGPFAULTERRORBOX |
				 SEM_NOALIGNMENTFAULTEXCEPT |
				 SEM_NOOPENFILEERRORBOX);

	_set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);

	GTEST_FLAG_SET(death_test_style, "threadsafe");
#endif

	QCoreApplication app{argc, argv};

	app.setOrganizationName(Manufacturer::RADIY);

	//

	QStringList arguments = app.arguments();

	for (const QString& a : arguments)
	{
		if (a.startsWith("-build=") == true)
		{
			buildPath = a;
			buildPath.replace("-build=", "", Qt::CaseInsensitive);
			continue;
		}

		if (a.startsWith("-profile=") == true)
		{
			profileName = a;
			profileName.replace("-profile=", "", Qt::CaseInsensitive);
			continue;
		}
	}

	if (buildPath.isEmpty() == true || profileName.isEmpty() == true)
	{
		std::cout << "Warning: Build path and/or profile name are not specified, UtilsTests will fail.\n";
		std::cout << "Use: ./UtilsTests [-build=build_dir] [-profile=profile_name]\n\n";
		return 1;
	}

	//

	if (!loadAppDataServiceCfgXml())
	{
		std::cout << "Error load AppDataService Configuration.xml file\n";
		return 1;
	}

	//

	logger = std::make_shared<CircularLogger>();

	LOGGER_INIT(logger, QString(), "UTILS_TESTS_LOG");

	logger->setLogCodeInfo(false);

	::testing::InitGoogleTest(&argc, argv);

	return RUN_ALL_TESTS();
}

bool loadAppDataServiceCfgXml()
{
	QString filePath = buildPath + "/SYSTEMID_RACK01_WS00_ADS/Configuration.xml";

	QFile f(filePath);

	if (!f.open(QIODeviceBase::ReadOnly))
	{
		return false;
	}

	appDataService_configurationXml = f.readAll();

	return true;
}
