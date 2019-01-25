#include <QCoreApplication>
#include <UalTester.h>

// #include "../../lib/ProtoSerialization.h"

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	a.setApplicationName("UalTester");
	a.setOrganizationName("Radiy");
	a.setOrganizationDomain("radiy.com");

	UalTester ualTester(argc, argv);
	ualTester.start();

	// google::protobuf::ShutdownProtobufLibrary();

	return a.exec();
}
