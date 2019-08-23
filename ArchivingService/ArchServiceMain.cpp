#include "ArchivingService.h"
#include "../lib/WUtils.h"
#include "../lib/MemLeaksDetection.h"

// To increase time that system waiting to the service shutting down, change value:
//
// HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\WaitToKillServiceTimeout.
//

int main(int argc, char *argv[])
{
	initMemoryLeaksDetection();

	QCoreApplication app(argc, argv);

	std::shared_ptr<CircularLogger> logger = std::make_shared<CircularLogger>();

	LOGGER_INIT(logger);

	logger->setLogCodeInfo(false);

	SoftwareInfo si;

	si.init(E::SoftwareType::ArchiveService, "", 1, 0);

	ArchivingService archServiceWorker(si, "RPCT Archiving Service", argc, argv, logger);

	ServiceStarter serviceStarter(app, archServiceWorker, logger);

	int result = serviceStarter.exec();

	google::protobuf::ShutdownProtobufLibrary();

	LOGGER_SHUTDOWN(logger);

	dumpMemoryLeaks();

	return result;
}
