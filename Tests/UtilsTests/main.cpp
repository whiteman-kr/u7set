#include <gtest/gtest.h>
#include <QCoreApplication>
#include <CommonLib/ConstStrings.h>

#ifdef Q_OS_WIN
#include <windows.h>
#include <crtdbg.h>
#endif

#include "Common.h"

class LoggerGuard
{
public:
	LoggerGuard()
	{
		Q_ASSERT(logger == nullptr);
		logger = std::make_shared<CircularLogger>();
		LOGGER_INIT(logger, QString(), "UTILS_TESTS_LOG");
		logger->setLogCodeInfo(false);
	}

	~LoggerGuard()
	{
		LOGGER_SHUTDOWN(logger);
	}
};

int main(int argc, char *argv[])
{
	std::cout << "UtilsTests started\n";

#ifdef Q_OS_WIN
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);

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

	std::cout << "UtilsTests set options Ok\n";

	QCoreApplication app{argc, argv};

	app.setOrganizationName(Manufacturer::RADIY);

	//

	LoggerGuard lg;

	std::cout << "UtilsTests logger created Ok\n";

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

	std::cout << "UtilsTests args parsed\n";

	if (buildPath.isEmpty() == true || profileName.isEmpty() == true)
	{
		std::cout << "UtilsTests error args\n";

		DEBUG_LOG_ERR(logger, "Build path and/or profile name are not specified, UtilsTests will fail.");
		DEBUG_LOG_WRN(logger, "Use: ./UtilsTests [-build=build_dir] [-profile=profile_name]");

		return 1;
	}

	std::cout << "UtilsTests args OK\n";

	DEBUG_LOG_MSG(logger, QString("Build path:   %1").arg(buildPath));
	DEBUG_LOG_MSG(logger, QString("Profile name: %1").arg(profileName));

	//

	if (!loadConfiguration() ||
		!loadAppDataSources() ||
		!loadAppSignals())
	{
		std::cout << "UtilsTests error load files\n";

		return 1;
	}

	std::cout << "UtilsTests files load Ok\n";

	createAndInitSignalStates();

	//

	::testing::InitGoogleTest(&argc, argv);

	return RUN_ALL_TESTS();
}
