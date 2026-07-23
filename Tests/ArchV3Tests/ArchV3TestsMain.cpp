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
		std::cout << "ArchV3Tests error args\n";

		logMsg("Build path and/or profile name are not specified, ArchV3Tests will fail.");
		logMsg("Use: ./ArchV3Tests [-build=build_dir] [-profile=profile_name]");

		return 1;
	}

	logMsg(QString("Build path:   %1").arg(buildPath));
	logMsg(QString("Profile name: %1").arg(profileName));

	//

	bool res = true;

	res &= loadArchInfoV3Data();
	RETURN_VALUE_IF_FALSE(res, 1);

	//res &= loadAppSignals();
	//RETURN_VALUE_IF_FALSE(res, 2);

//	createAndInitSignalStates();

	::testing::InitGoogleTest(&argc, argv);

	auto result = RUN_ALL_TESTS();

	cleanup();

	return result;
}
