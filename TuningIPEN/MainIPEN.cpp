#include "TuningMainWindow.h"
#include <QApplication>
#include "TuningIPENSource.h"
#include "TuningIPENSocket.h"

int main(int argc, char *argv[])
{
#if defined (Q_OS_WIN) && defined (Q_DEBUG)
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );	// Memory leak report on app exit
#endif

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

	return result;
}
