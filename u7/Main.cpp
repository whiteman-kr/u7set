#include "MainWindow.h"
#include "Settings.h"
#include "GlobalMessanger.h"
#include "../VFrame30/VFrame30Library.h"
#include "../lib/DbController.h"
#include "../lib/DeviceObject.h"
#include "../lib/PropertyObject.h"
#include "../lib/TimeStamp.h"
#include "../lib/LmDescription.h"
#include "../lib/Configurator.h"
#include "../lib/LogicModulesInfo.h"
#include "../Builder/Builder.h"
#include "../lib/Signal.h"
#include <QList>

#include "../lib/SoftwareXmlReader.h"

#if __has_include("../gitlabci_version.h")
#	include "../gitlabci_version.h"
#endif

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	// --
	//
	a.setApplicationName("u7");
	a.setOrganizationName("Radiy");
	a.setOrganizationDomain("radiy.com");

#ifdef GITLAB_CI_BUILD
	a.setApplicationVersion(QString("0.8.%1 (%2)").arg(CI_PIPELINE_ID).arg(CI_BUILD_REF_SLUG));
#else
	a.setApplicationVersion(QString("0.8.LOCALBUILD"));
#endif

	VFrame30::VFrame30Library::init();
	Hardware::init();
	DbController::init();
	Builder::init();

	GlobalMessanger::instance();		// Create instance of GlobalMessanger

	// Read settings
	//
	theSettings.load();

	qRegisterMetaType<std::vector<int>>();
	qRegisterMetaType<E::SignalType>();
	qRegisterMetaType<TimeStamp>();
	qRegisterMetaType<TimeSpan>();
	qRegisterMetaType<std::vector<UartPair>>();
	qRegisterMetaType<std::map<QString, std::vector<UartPair>>>();
	qRegisterMetaType<QVector<int>>();
	qRegisterMetaType<ID_AppSignalID>();
	qRegisterMetaType<QVector<ID_AppSignalID>>();

	// Init TrendLib resources
	//
	Q_INIT_RESOURCE(TrendView);

	// Start database communication thread
	//
	DbController dbController;

	dbController.disableProgress();

	dbController.setHost(theSettings.serverIpAddress());
	dbController.setPort(theSettings.serverPort());
	dbController.setServerUsername(theSettings.serverUsername());
	dbController.setServerPassword(theSettings.serverPassword());

	// --
	//
	MainWindow* w = new MainWindow(&dbController, nullptr);
	w->show();

	dbController.enableProgress();

	int result = a.exec();

	delete w;	// Delete main windows before shutown procedures

	// Shutting down
	//
	Builder::shutdown();
	DbController::shutdown();
	VFrame30::VFrame30Library::shutdown();
	Hardware::shutdown();

	google::protobuf::ShutdownProtobufLibrary();

	// Exit
	//
	return result;
}

