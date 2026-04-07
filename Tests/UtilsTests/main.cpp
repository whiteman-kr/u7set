#include <gtest/gtest.h>
#include <QCoreApplication>
#include <CommonLib/ConstStrings.h>
#include "../../UtilsLib/HighResolutionTimerGuard.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <crtdbg.h>
#endif

#include "Common.h"

int main(int argc, char *argv[])
{
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
	QCoreApplication app{argc, argv};

	app.setOrganizationName(Manufacturer::RADIY);

	HighResolutionTimerGuard highResTimerGuard;

	Q_UNUSED(highResTimerGuard);

	//

	LoggerGuard lg;

	//

	QStringList arguments = app.arguments();

	if (isGTestDeathChild(arguments) == true)
	{
		::testing::InitGoogleTest(&argc, argv);

		return RUN_ALL_TESTS();
	}

	//

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
		std::cout << "UtilsTests error args\n";

		logMsg("Build path and/or profile name are not specified, UtilsTests will fail.");
		logMsg("Use: ./UtilsTests [-build=build_dir] [-profile=profile_name]");

		return 1;
	}

	logMsg(QString("Build path:   %1").arg(buildPath));
	logMsg(QString("Profile name: %1").arg(profileName));

	//

	discretesLogWriter = startDiscretesLogWriter("TEST_COMPILER", "EQUIPMENT_ID");

	bool res = true;

	res &= loadConfiguration();
	RETURN_VALUE_IF_FALSE(res, 1);

	res &= loadAppSignals();
	RETURN_VALUE_IF_FALSE(res, 2);

	res &= loadAppDataSources();
	RETURN_VALUE_IF_FALSE(res, 3);

	createAndInitSignalStates();
	prepareAppDataSources();
	createAndStartAppDataReceiver();

	//

	::testing::InitGoogleTest(&argc, argv);

	auto result = RUN_ALL_TESTS();

	stopAppDataReceiver();
	stopDiscretesLogWriter(discretesLogWriter);

	return result;
}
