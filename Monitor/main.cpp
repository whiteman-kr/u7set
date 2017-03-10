#include "Stable.h"
#include "MonitorMainWindow.h"
#include "Settings.h"
#include <QMessageBox>
#include "../VFrame30/VFrame30Library.h"
#include "MonitorConfigController.h"
#include "../lib/SimpleThread.h"
#include "TcpSignalClient.h"


AppSignalManager theSignals;
MonitorMainWindow* theMonitorMainWindow = nullptr;


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

	// --
	//
	qDebug() << "GUI Thread ID: " << QThread::currentThreadId();

	// Read settings
	//
	theSettings.load();

	// Create memory segment for check single Instance
	//

	QSharedMemory instanceChecker;
	instanceChecker.setKey(MonitorMainWindow::getInstanceKey());

	bool ok = instanceChecker.create(sizeof(char));

	if (ok == true)
	{
		// If it was created, we should initialize it with 0
		//

		instanceChecker.lock();

		char* sharedData = static_cast<char*>(instanceChecker.data());
		*sharedData = 0;

		instanceChecker.unlock();
	}
	else
	{
		if (instanceChecker.error() == QSharedMemory::SharedMemoryError::AlreadyExists &&
		        theSettings.singleInstance() == true)
		{
			// In other way, if memory segment exists, write there
			// value "1" and exit.

			instanceChecker.attach();
			instanceChecker.lock();

			char* sharedData = static_cast<char*>(instanceChecker.data());
			*sharedData = 1;

			instanceChecker.unlock();
			qDebug() << "Another instance is active";

			return 0;
		}
		else
		{
			// If other error occured - show it on debug console
			//
			qDebug() << "Shared memory: " << instanceChecker.errorString();
		}
	}

	// --
	//
	VFrame30::VFrame30Library::Init();
	//Hardware::Init();

	// --
	//
	MonitorMainWindow w;
	theMonitorMainWindow = &w;
	w.show();

	// --
	//
	int result = a.exec();

	// Shutting down
	//

	VFrame30::VFrame30Library::Shutdown();
	//Hardware::Shutdwon();
	google::protobuf::ShutdownProtobufLibrary();

	return result;
}
