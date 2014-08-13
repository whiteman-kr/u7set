#include "Stable.h"
#include "MainWindow.h"
#include "../include/DbStore.h"
#include "Settings.h"
#include "../VFrame30/VFrame30Library.h"

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
	qRegisterMetaType<std::vector<DbFileInfo>>();
	qRegisterMetaType<std::vector<std::shared_ptr<DbFile>>>();
	qRegisterMetaType<std::vector<int>>();
	qRegisterMetaType<std::vector<DbFileInfo>>();

	// Start database communication thread
	//
	DbStore* dbStore = DbStore::create();

	dbStore->setHost(theSettings.serverIpAddress());
	dbStore->setPort(theSettings.serverPort());
	dbStore->setServerUsername(theSettings.serverUsername());
	dbStore->setServerPassword(theSettings.serverPassword());

	// --
	//
	MainWindow w(dbStore, nullptr);
    w.show();

	int result = a.exec();

	dbStore->destroy();

	VFrame30::VFrame30Library::Shutdown();

	// Exit
	//
	return result;
}
