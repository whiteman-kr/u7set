#include <gtest/gtest.h>
#include <QCoreApplication>
#include <CommonLib/ConstStrings.h>

#ifdef Q_OS_WIN
#include <windows.h>
#include <crtdbg.h>
#endif

#include "Common.h"

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

	logger = std::make_shared<CircularLogger>();

	LOGGER_INIT(logger, QString(), "UTILS_TESTS_LOG");

	logger->setLogCodeInfo(false);

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
		DEBUG_LOG_ERR(logger, "Build path and/or profile name are not specified, UtilsTests will fail.");
		DEBUG_LOG_WRN(logger, "Use: ./UtilsTests [-build=build_dir] [-profile=profile_name]");
		return 1;
	}

	DEBUG_LOG_MSG(logger, QString("Build path:   %1").arg(buildPath));
	DEBUG_LOG_MSG(logger, QString("Profile name: %1").arg(profileName));

	//

	if (!loadConfiguration() ||
		!loadAppDataSources() ||
		!loadAppSignals())
	{
		return 1;
	}

	std::cout.flush();

	createAndInitSignalStates();

	//

	::testing::InitGoogleTest(&argc, argv);

	return RUN_ALL_TESTS();
}
