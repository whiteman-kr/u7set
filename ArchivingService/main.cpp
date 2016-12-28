#include <QCoreApplication>
#include "ArchivingService.h"

int main(int argc, char *argv[])
{
#if defined (Q_OS_WIN) && defined (Q_DEBUG)
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );	// Memory leak report on app exit
#endif

	INIT_LOGGER(argv[0]);

	ArchivingServiceWorker* archivingServiceWorker = new ArchivingServiceWorker();

	ServiceStarter service(argc, argv, "RPCT Archiving Service", archivingServiceWorker);

	int result = service.exec();

	google::protobuf::ShutdownProtobufLibrary();

	return result;
}
