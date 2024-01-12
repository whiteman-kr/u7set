#include "../ServiceLib/ServiceStarter.h"
#include "GatewayService.h"
#include "version.h"

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);

	app.setApplicationName(Manufacturer::GATEWAY_SERVICE);
	app.setOrganizationName(Manufacturer::RADIY);
	app.setOrganizationDomain(Manufacturer::SITE);

	app.setApplicationVersion(QString("%1.%2.%3 (%4)").
									arg(U7SET_MAJOR_VERSION).
									arg(U7SET_MINOR_VERSION).
									arg(U7SET_PATCH_VERSION).
									arg(U7SET_BRANCH_NAME));

	QThread::currentThread()->setObjectName("MainThread");

	std::shared_ptr<CircularLogger> logger = std::make_shared<CircularLogger>();

	LOGGER_INIT(logger, QString(), getServiceInstanceID(argc, argv));

	logger->setLogCodeInfo(false);

	SoftwareInfo si(E::SoftwareType::GatewayService, "");

	GatewayServiceWorker gatewayServiceWorker(si,
											  Service::getServiceInstanceName(Manufacturer::GATEWAY_SERVICE, argc, argv),
											  argc, argv, logger);

	ServiceStarter serviceStarter(app, gatewayServiceWorker, logger);

	int result = serviceStarter.exec();

	google::protobuf::ShutdownProtobufLibrary();

	LOGGER_SHUTDOWN(logger);

	return result;
}
