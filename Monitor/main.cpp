#include "MonitorMainWindow.h"
#include "Settings.h"
#include "../VFrame30/VFrame30Library.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	// --
	//
	a.setApplicationName("Monitor 3.0");
	a.setOrganizationName("Radiy");
	a.setOrganizationDomain("radiy.com");

	VFrame30::VFrame30Library::Init();
	//Hardware::Init();

	// Read settings
	//
	theSettings.load();

	// --
	//
	MonitorMainWindow w;
	w.show();

	int result = a.exec();

	// Shutting down
	//
	VFrame30::VFrame30Library::Shutdown();
	//Hardware::Shutdwon();
	//google::protobuf::ShutdownProtobufLibrary();

	return result;
}
