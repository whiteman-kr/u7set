#include "../Builder/Builder.h"
#include "../UtilsLib/CrashExceptionHandler.h"
#include "../version.h"

#include <CommonLib/Times.h>
#include <CommonStdLib/u7_vld.h>
#include <HardwareLib/HardwareLibrary.h>
#include <LicenseLib/AppLicenser.h>
#include <UiLib/OverrideWindows11Style.h>
#include <VFrame30/VFrame30Library.h>

#include <google/protobuf/message_lite.h>

#include "AppSettings.h"
#include "GlobalMessanger.h"
#include "MainWindow.h"
#include "Settings.h"


int main(int argc, char* argv[])
{
	Vld::setVldReportFilterHook();

	int result = 0;

	{
		QApplication app(argc, argv);

		// Override Windows11 style, the implementation just ugly.
		//
		UiLib::OverrideWindows11Style(app, argc, argv);

		// --
		//
		app.setApplicationName("u7");
		app.setOrganizationName(Manufacturer::RADIY);
		app.setOrganizationDomain(Manufacturer::SITE);

		app.setApplicationVersion(
			QString("%1.%2.%3 (%4)").arg(U7SET_MAJOR_VERSION).arg(U7SET_MINOR_VERSION).arg(U7SET_PATCH_VERSION).arg(U7SET_BRANCH_NAME));

		Q_INIT_RESOURCE(LicenseLib);
		Q_INIT_RESOURCE(TrendView);
		Q_INIT_RESOURCE(SimulatorLib);
		Q_INIT_RESOURCE(SimulatorUi);

		// --
		//
		if (LicenseLib::AppLicenser::guiAppStartValidation(QDateTime::fromSecsSinceEpoch(U7SET_BUILD_DATE_SECONDS).date()) == false)
		{
			return EXIT_FAILURE;
		}

		bool isAppLicenseValid = LicenseLib::AppLicenser::showRestrictionMessageBox(nullptr,
																					LicenseLib::AppLicenser{}.validator().validateAppU7(),
																					"Application u7 (RPCT)");

		if (isAppLicenseValid == false)
		{
			return EXIT_FAILURE;
		}

		// --
		//
		VFrame30::init();
		Hardware::init();
		DbController::init();
		Builder::init();

		GlobalMessanger::instance(); // Create instance of GlobalMessenger

		// --
		//
		qRegisterMetaType<std::vector<int>>();
		qRegisterMetaType<E::SignalType>();
		qRegisterMetaType<TimeStamp>();
		qRegisterMetaType<TimeSpan>();
		qRegisterMetaType<QVector<int>>();
		qRegisterMetaType<ID_AppSignalID>();
		qRegisterMetaType<QVector<ID_AppSignalID>>();
		qRegisterMetaType<QMap<QString, int>>("QMap<QString,int>");
		qRegisterMetaType<std::optional<std::vector<int>>>("std::optional<std::vector<int>>");

		// Read settings
		//
		theAppSettings.load();
		theSettings.load();

		// Init TrendLib resources
		//


		// Start database communication thread
		//
		DbController dbController;

		dbController.disableProgress();

		dbController.setHost(theAppSettings.serverHost());
		dbController.setPort(theAppSettings.serverPort());
		dbController.setServerUsername(theAppSettings.serverUsername());
		dbController.setServerPassword(theAppSettings.serverPassword());

		// --
		//
		MainWindow* w = new MainWindow(&dbController, nullptr);
		w->show();

#if defined(Q_OS_WIN)
		CrashExceptionHandler cdh;
		QObject::connect(&cdh, &CrashExceptionHandler::miniDumpCreated, w, &MainWindow::onMiniDumpCreated);
#endif

		dbController.enableProgress();

		result = app.exec();

		delete w; // Delete main windows before shutdown procedures

		// Shutting down
		//
		Builder::shutdown();
		DbController::shutdown();
		VFrame30::shutdown();
		Hardware::shutdown();

		google::protobuf::ShutdownProtobufLibrary();
	}

	// Exit
	//
	return result;
}
