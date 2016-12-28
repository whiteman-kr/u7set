#include <QCoreApplication>
#include "TuningService.h"


int main(int argc, char *argv[])
{
#if defined (Q_OS_WIN) && defined (Q_DEBUG)
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );	// Memory leak report on app exit
#endif

	INIT_LOGGER(argv[0]);

	const char* appName = "RPCT Tuning Service";

	QCoreApplication::setOrganizationName("Radiy");
	QCoreApplication::setApplicationName(appName);

	Tuning::TuningServiceWorker* tuningServiceWorker = new Tuning::TuningServiceWorker();

	ServiceStarter serviceStarter(argc, argv, appName, tuningServiceWorker);

	int result = serviceStarter.exec();

	google::protobuf::ShutdownProtobufLibrary();

	return result;
}
