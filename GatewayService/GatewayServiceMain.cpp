#include "GatewayService.h"

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);

	QThread::currentThread()->setObjectName("MainThread");

	std::shared_ptr<CircularLogger> logger = std::make_shared<CircularLogger>();

	LOGGER_INIT(logger, QString(), getServiceInstanceID(argc, argv));

	logger->setLogCodeInfo(false);

	SoftwareInfo si;

	si.init(E::SoftwareType::GatewayService, "", 1, 0);

	GatewayServiceWorker gatewayServiceWorker(si,
											  Service::getServiceInstanceName("Gateway Service", argc, argv),
											  argc, argv, logger,
											  E::ServiceRunMode::ConsoleApp);	// run mode will be refined after cmd line processing

	ServiceStarter serviceStarter(app, gatewayServiceWorker, logger);

	int result = serviceStarter.exec();

	google::protobuf::ShutdownProtobufLibrary();

	LOGGER_SHUTDOWN(logger);

	return result;
}
