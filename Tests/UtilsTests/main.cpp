#include <gtest/gtest.h>
#include <QCoreApplication>
#include "Common.h"
#include <CommonLib/ConstStrings.h>

#ifdef Q_OS_WIN
#include <windows.h>
#include <crtdbg.h>
#endif

std::shared_ptr<CircularLogger> logger;

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

	logger = std::make_shared<CircularLogger>();

	LOGGER_INIT(logger, QString(), "UTILS_TESTS_LOG");

	logger->setLogCodeInfo(false);

	::testing::InitGoogleTest(&argc, argv);

	return RUN_ALL_TESTS();
}
