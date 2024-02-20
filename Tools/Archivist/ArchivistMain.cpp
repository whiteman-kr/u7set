#include <iostream>
#include "../../UtilsLib/WUtils.h"
#include "../../Protobuf/message.h"
#include "../../OnlineLib/CircularLogger.h"
#include <iostream>
#include "Archivist.h"
#include "FileArchivist.h"
#include "DbArchivist.h"

// To increase time that system waiting to the service shutting down, change value:
//
// HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\WaitToKillServiceTimeout.
//

int main(int argc, char* argv[])
{
	QCoreApplication app(argc, argv);

	CircularLoggerShared logger = std::make_shared<CircularLogger>();

	bool result = 0;

	Archivist* archivist = new DbArchivist(argc, argv);

	archivist->start();




//	LOGGER_INIT(logger, QString(), getServiceInstanceID(argc, argv));

//	logger->setLogCodeInfo(false);

	google::protobuf::ShutdownProtobufLibrary();

	LOGGER_SHUTDOWN(logger);

	return result;
}
