#include <iostream>
#include "../../UtilsLib/WUtils.h"
#include "../../Proto/serialization.pb.h"
#include "../../OnlineLib/CircularLogger.h"

// To increase time that system waiting to the service shutting down, change value:
//
// HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\WaitToKillServiceTimeout.
//

int main(int argc, char* argv[])
{
	QCoreApplication app(argc, argv);

	bool result = 0;

	CircularLoggerShared logger = std::make_shared<CircularLogger>();

//	LOGGER_INIT(logger, QString(), getServiceInstanceID(argc, argv));

//	logger->setLogCodeInfo(false);

	std::cout << "\nHello!\n\n";

	google::protobuf::ShutdownProtobufLibrary();

	LOGGER_SHUTDOWN(logger);

	return result;
}
