#include "../Builder/Builder.h"
#include "../CommonLib/Times.h"
#include "../HardwareLib/DeviceObject.h"
#include "../HardwareLib/HardwareLibrary.h"
#include "../HardwareLib/LmDescription.h"
#include "../HardwareLib/LogicModulesInfo.h"
#include "../Protobuf/google/protobuf/message.h"
#include "../UtilsLib/CrashExceptionHandler.h"
#include "../VFrame30/VFrame30Library.h"
#include "../lib/Configurator.h"
#include "../version.h"
#include "GlobalMessanger.h"
#include "MainWindow.h"
#include "Settings.h"


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
#endif	// Visual Leak Detector


int main(int argc, char *argv[])
{
	int result = 0;
	{
		QApplication a(argc, argv);

		// --
		//
		a.setApplicationName("u7");
		a.setOrganizationName(Manufacturer::RADIY);
		a.setOrganizationDomain(Manufacturer::SITE);

		a.setApplicationVersion(QString("%1.%2.%3 (%4)")
									.arg(U7SET_MAJOR_VERSION)
									.arg(U7SET_MINOR_VERSION)
									.arg(U7SET_PATCH_VERSION)
									.arg(U7SET_BRANCH_NAME));

		VFrame30::init();
		Hardware::init();
		DbController::init();
		Builder::init();

		GlobalMessanger::instance();		// Create instance of GlobalMessenger

		// --
		//
		qRegisterMetaType<std::vector<int>>();
		qRegisterMetaType<E::SignalType>();
		qRegisterMetaType<TimeStamp>();
		qRegisterMetaType<TimeSpan>();
		qRegisterMetaType<std::vector<UartPair>>();
		qRegisterMetaType<std::map<QString, std::vector<UartPair>>>();
		qRegisterMetaType<QVector<int>>();
		qRegisterMetaType<ID_AppSignalID>();
		qRegisterMetaType<QVector<ID_AppSignalID>>();
		qRegisterMetaType<QMap<QString, int>>("QMap<QString,int>");
		qRegisterMetaType<std::optional<std::vector<int>>>("std::optional<std::vector<int>>");

		// Read settings
		//
		theSettings.load();

		// Init TrendLib resources
		//
		Q_INIT_RESOURCE(TrendView);

		// Start database communication thread
		//
		DbController dbController;

		dbController.disableProgress();

		dbController.setHost(theSettings.serverHost());
		dbController.setPort(theSettings.serverPort());
		dbController.setServerUsername(theSettings.serverUsername());
		dbController.setServerPassword(theSettings.serverPassword());

		// --
		//
		MainWindow* w = new MainWindow(&dbController, nullptr);
		w->show();

#if defined (Q_OS_WIN)
        CrashExceptionHandler cdh;
        QObject::connect(&cdh, &CrashExceptionHandler::miniDumpCreated, w, &MainWindow::onMiniDumpCreated);
#endif

		dbController.enableProgress();

		result = a.exec();

		delete w;	// Delete main windows before shutown procedures

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

