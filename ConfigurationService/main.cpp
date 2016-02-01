#include <QCoreApplication>

#include "ConfigurationService.h"


int main(int argc, char *argv[])
{
	ServiceStarter serviceStarter(argc, argv, "RPCT Configuration Service", new ConfigurationServiceWorker());

	return serviceStarter.exec();
}
