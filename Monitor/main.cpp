#include "Stable.h"
#include "MonitorMainWindow.h"
#include "Settings.h"
#include <QMessageBox>
#include "../VFrame30/VFrame30Library.h"
#include "MonitorConfigController.h"
#include "../include/SimpleThread.h"
#include "TcpSignalClient.h"


AppSignalManager theSignals;


int main(int argc, char *argv[])
{
#if defined (Q_OS_WIN) && defined (Q_DEBUG)
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );	// Memory leak report on app exit
#endif

	QApplication a(argc, argv);

	// --
	//
	a.setApplicationName("Monitor 3.0");
	a.setOrganizationName("Radiy");
	a.setOrganizationDomain("radiy.com");

	// Read settings
	//
	theSettings.load();

	// --
	//
	VFrame30::VFrame30Library::Init();
	Hardware::Init();

	// Configuration Service communication
	//
	MonitorConfigController configController(theSettings.configuratorAddress1(),
											 theSettings.configuratorAddress2());

	HostAddressPort aaa("192.168.1.1", 13323);
	SimpleThread* tcpClientThread = new SimpleThread(new TcpSignalClient(aaa, aaa));

//	SimpleThread* tcpClientThread = new SimpleThread(new TcpSignalClient(theSettings.configuratorAddress1(),
//																		 theSettings.configuratorAddress2()));

	tcpClientThread->start();

	// --
	//
	MonitorMainWindow w(&configController);
	w.show();

	// --
	//
	int result = a.exec();

	// Shutting down
	//
	tcpClientThread->quitAndWait(10000);
	delete tcpClientThread;

	VFrame30::VFrame30Library::Shutdown();
	Hardware::Shutdwon();
	//google::protobuf::ShutdownProtobufLibrary();

	return result;
}
