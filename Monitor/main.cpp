#include "../lib/SimpleThread.h"
#include "../lib/Tuning/TuningSignalManager.h"
#include "../VFrame30/VFrame30Library.h"
#include "Settings.h"
#include "MonitorMainWindow.h"
#include "MonitorConfigController.h"
#include "TcpSignalClient.h"

#if __has_include("../gitlabci_version.h")
#	include "../gitlabci_version.h"
#endif

AppSignalManager theSignals;
TuningSignalManager theTuningSignals;

MonitorMainWindow* theMonitorMainWindow = nullptr;


int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	// --
	//
	a.setApplicationName("Monitor3");
	a.setOrganizationName("Radiy");
	a.setOrganizationDomain("radiy.com");

#ifdef GITLAB_CI_BUILD
	a.setApplicationVersion(QString("3.8.%1 (%2)").arg(CI_PIPELINE_ID).arg(CI_BUILD_REF_SLUG));
#else
	a.setApplicationVersion(QString("3.8.LOCALBUILD"));
#endif

	// --
	//
	qDebug() << "GUI Thread ID: " << QThread::currentThreadId();

	qRegisterMetaType<TimeStamp>();
	qRegisterMetaType<TimeSpan>();

	// Read settings
	//
	theSettings.load();

	// Init TrendLib resources
	//
	Q_INIT_RESOURCE(TrendView);

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
	VFrame30::VFrame30Library::init();
	//Hardware::Init();

	SoftwareInfo si;
	si.init(E::SoftwareType::Monitor, theSettings.instanceStrId(), 0, 1);

	// --
	//
	int result = 0;
	{
		MonitorMainWindow w(si);
		theMonitorMainWindow = &w;
		w.show();

		// --
		//
		result = a.exec();
	}

	// Shutting down
	//

	VFrame30::VFrame30Library::shutdown();
	//Hardware::Shutdwon();
	google::protobuf::ShutdownProtobufLibrary();

	return result;
}
