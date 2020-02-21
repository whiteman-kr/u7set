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

	int result = -1;

	UalTester ualTester(argc, argv);

	bool started = ualTester.start();
	if (started == true)
	{
		result = a.exec();
	}

	google::protobuf::ShutdownProtobufLibrary();

	dumpMemoryLeaks();

	return result;
}
