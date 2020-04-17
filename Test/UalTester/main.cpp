#include <QCoreApplication>
#include <UalTester.h>

#include "../../lib/MemLeaksDetection.h"

int main(int argc, char *argv[])
{
	initMemoryLeaksDetection();

	QCoreApplication a(argc, argv);

	a.setApplicationName("UalTester");
	a.setOrganizationName("Radiy");
	a.setOrganizationDomain("radiy.com");

	UalTester ualTester(argc, argv);
	if (ualTester.start() == false)
	{
		return -1;
	}

	int result = a.exec();

	ualTester.stop();

	google::protobuf::ShutdownProtobufLibrary();

	dumpMemoryLeaks();

	return result;
}
