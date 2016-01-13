#include <QCoreApplication>

#include "ConfigurationService.h"


int main(int argc, char *argv[])
{
	ConfigurationService service(argc, argv);

	return service.exec();
}
