#include "TuningMainWindow.h"
#include <QApplication>
#include "../TuningService/TuningSource.h"


int main(int argc, char *argv[])
{
#if defined (Q_OS_WIN) && defined (Q_DEBUG)
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );	// Memory leak report on app exit
#endif

	QString buildPath;

	for(int i = 0; i < argc; i++)
	{
		QString arg(argv[i]);

		if (arg.startsWith("-b="))
		{
			buildPath = arg.mid(3);
			continue;
		}
	}

	qRegisterMetaType<TuningIPEN::TuningSourceState>("TuningDataSourceState");
	qRegisterMetaType<FotipFrame>("FotipFrame");

	QApplication a(argc, argv);
	TuningMainWindow w(buildPath);
	w.show();

	int result =  a.exec();

	google::protobuf::ShutdownProtobufLibrary();

	return result;
}
