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

	QApplication a(argc, argv);
	TuningIPEN::TuningMainWindow w(argc, argv);
	w.show();

	int result =  a.exec();

	google::protobuf::ShutdownProtobufLibrary();

	return result;
}
