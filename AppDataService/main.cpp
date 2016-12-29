#include <QCoreApplication>
#include "AppDataService.h"


int main(int argc, char *argv[])
{
#if defined (Q_OS_WIN) && defined (Q_DEBUG)
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );	// Memory leak report on app exit
#endif

	INIT_LOGGER(argv[0]);

	LOG_MSG(QString("Run: ").arg(argv[0]));

	AppDataServiceWorker* dataServiceWorker = new AppDataServiceWorker("RPCT Application Data Service", argc, argv);

	ServiceStarter service(dataServiceWorker);

	int result = service.exec();

	google::protobuf::ShutdownProtobufLibrary();

	return result;
}
