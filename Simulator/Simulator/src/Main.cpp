#include "../../version.h"
#include "SimPropertyStorage.h"

#include <CommonLib/ConstStrings.h>
#include <HardwareLib/HardwareLibrary.h>
#include <LicenseLib/AppLicenser.h>
#include <SimulatorLib/SimConsoleLogFile.h>
#include <SimulatorUi/SimWidget.h>
#include <UiLib/OverrideWindows11Style.h>
#include <VFrame30/VFrame30Library.h>

#include <QApplication>
#include <QFileDialog>
#include <QSettings>

#include <google/protobuf/message_lite.h>


// Visual Leak Detector
//
#if defined(Q_OS_WIN) && defined(QT_DEBUG)
	#if __has_include("C:/Program Files (x86)/Visual Leak Detector/include/vld.h")
		#include "C:/Program Files (x86)/Visual Leak Detector/include/vld.h"
	#else
		#if __has_include("D:/Program Files (x86)/Visual Leak Detector/include/vld.h")
			#include "D:/Program Files (x86)/Visual Leak Detector/include/vld.h"
		#endif
	#endif
#endif // Visual Leak Detector


QString getProjectPath(QWidget* parent)
{
	QString lastPath = QSettings{}.value("LastPath").toString();

	lastPath = QFileDialog::getExistingDirectory(parent, QObject::tr("Open Build"), lastPath);
	if (lastPath.isEmpty() == false)
	{
		QSettings{}.setValue("LastPath", lastPath);
	}

	return lastPath;
}


int main(int argc, char* argv[])
{
	int result = 0;

	{
		QApplication a(argc, argv);

		UiLib::OverrideWindows11Style(a, argc, argv);

		// --
		//
		a.setApplicationName("Simulator");
		a.setOrganizationName(Manufacturer::RADIY);
		a.setOrganizationDomain(Manufacturer::SITE);

		a.setApplicationVersion(
			QString("%1.%2.%3 (%4)").arg(U7SET_MAJOR_VERSION).arg(U7SET_MINOR_VERSION).arg(U7SET_PATCH_VERSION).arg(U7SET_BRANCH_NAME));

		// Load resources
		//
		Q_INIT_RESOURCE(LicenseLib);
		Q_INIT_RESOURCE(TrendView);
		Q_INIT_RESOURCE(SimulatorLib);
		Q_INIT_RESOURCE(SimulatorUi);

		// Check license
		//
		if (LicenseLib::AppLicenser::guiAppStartValidation(QDateTime::fromSecsSinceEpoch(U7SET_BUILD_DATE_SECONDS).date()) == false)
		{
			return EXIT_FAILURE;
		}

		bool isAppLicenseValid =
			LicenseLib::AppLicenser::showRestrictionMessageBox(nullptr,
															   LicenseLib::AppLicenser{}.validator().validateAppSimulator(),
															   "Application Simulator");

		if (isAppLicenseValid == false)
		{
			return EXIT_FAILURE;
		}

		// --
		//
		Hardware::init();
		VFrame30::init();

		// --
		//
		// qRegisterMetaType<std::vector<int>>();
		// qRegisterMetaType<E::SignalType>();
		// qRegisterMetaType<TimeStamp>();
		// qRegisterMetaType<TimeSpan>();
		// qRegisterMetaType<QVector<int>>();
		// qRegisterMetaType<ID_AppSignalID>();
		// qRegisterMetaType<QVector<ID_AppSignalID>>();
		// qRegisterMetaType<QMap<QString, int>>("QMap<QString,int>");
		// qRegisterMetaType<std::optional<std::vector<int>>>("std::optional<std::vector<int>>");

		// Init TrendLib resources
		//
		Q_INIT_RESOURCE(TrendView);

		// --
		//
		SimPropertyStorage propertyStorage;

		auto w = new SimUi::SimWidget{std::make_shared<Sim::ConsoleLogFile>(),
									  {},
									  ::getProjectPath,
									  propertyStorage,
									  nullptr,
									  nullptr,
									  Qt::Window,
									  false,
									  nullptr};

		QSettings s;
		w->restoreGeometry(s.value("MainWindow/geometry").toByteArray());
		w->restoreState(s.value("MainWindow/windowState").toByteArray());

		w->show();

		result = a.exec();

		s.setValue("MainWindow/geometry", w->saveGeometry());
		s.setValue("MainWindow/windowState", w->saveState());

		delete w;             // Delete main windows before shutdown procedures

		VFrame30::shutdown(); // Shutting down
		Hardware::shutdown();

		google::protobuf::ShutdownProtobufLibrary();
	}

	return result;
}
