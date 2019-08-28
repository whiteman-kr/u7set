#include "TuningMainWindow.h"
#include "TuningIPENSource.h"
#include "TuningIPENSocket.h"
#include "../lib/MemLeaksDetection.h"

int main(int argc, char *argv[])
{
	initMemoryLeaksDetection();

	qRegisterMetaType<TuningIPEN::TuningSourceState>("TuningDataSourceState");
	qRegisterMetaType<TuningIPEN::FotipFrame>("FotipFrame");

	std::shared_ptr<CircularLogger> logger = std::make_shared<CircularLogger>();

	LOGGER_INIT(logger);

	logger->setLogCodeInfo(false);

	QApplication a(argc, argv);

	SoftwareInfo si;

	si.init(E::SoftwareType::TuningService, "", 1, 0);

	TuningIPEN::TuningMainWindow w(si, argc, argv, logger);

	w.show();

	int result =  a.exec();

	google::protobuf::ShutdownProtobufLibrary();

	LOGGER_SHUTDOWN(logger);

	dumpMemoryLeaks();

	return result;
}
