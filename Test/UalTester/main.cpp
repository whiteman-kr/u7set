#include <QCoreApplication>
#include <UalTester.h>

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	a.setApplicationName("UalTester");
	a.setOrganizationName("Radiy");
	a.setOrganizationDomain("radiy.com");

	//

	std::shared_ptr<CircularLogger> logger = std::make_shared<CircularLogger>();

	LOGGER_INIT(logger, QString(), QString());

	logger->setLogCodeInfo(false);

	//

	UalTester ualTester(argc, argv, logger);

	if (ualTester.start() == false)
	{
		return -1;
	}

	int result = a.exec();

	ualTester.stop();

	LOGGER_SHUTDOWN(logger);

	QThread::msleep(500);			// waiting while logger flush buffers

	google::protobuf::ShutdownProtobufLibrary();

	return result;
}
