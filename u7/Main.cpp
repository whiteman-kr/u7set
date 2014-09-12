#include "Stable.h"
#include "MainWindow.h"
#include "../include/DbController.h"
#include "Settings.h"
#include "../VFrame30/VFrame30Library.h"

#if defined(Q_OS_WIN) && defined(_MSC_VER)
	#include <vld.h>		// Enable Visula Leak Detector
	// vld.h includes windows.h wich redefine min/max stl functions
	#ifdef min
		#undef min
	#endif
	#ifdef max
		#undef max
	#endif
#endif


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	// --
	//
	a.setApplicationName("u7");
	a.setOrganizationName("Radiy");
	a.setOrganizationDomain("radiy.com");

	VFrame30::VFrame30Library::Init();

	// Read settings
	//
	theSettings.load();

	qRegisterMetaType<DbUser>();
	qRegisterMetaType<DbFileInfo>();
	qRegisterMetaType<DbFile>();
	qRegisterMetaType<DbChangesetInfo>();
	qRegisterMetaType<DbProject>();
	qRegisterMetaType<std::vector<DbProject>>();
	qRegisterMetaType<std::vector<DbFileInfo>>();
	qRegisterMetaType<std::vector<std::shared_ptr<DbFile>>>();
	qRegisterMetaType<std::vector<int>>();
	qRegisterMetaType<std::vector<DbFileInfo>>();

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
	MainWindow w(&dbController, nullptr);
    w.show();

	dbController.enableProgress();

	int result = a.exec();

	VFrame30::VFrame30Library::Shutdown();

	// Exit
	//
	return result;
}
