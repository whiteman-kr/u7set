#include "TuningMainWindow.h"
#include <QApplication>
#include "../TuningService/TuningDataSource.h"


#if defined(Q_OS_WIN) && defined(_MSC_VER)
	#include <vld.h>		// Enable Visual Leak Detector
	// vld.h includes windows.h wich redefine min/max stl functions
	#ifdef min
		#undef min
	#endif
	#ifdef max
		#undef max
	#endif
#endif


int main(int argc, char *argv[])
{
	QString cfgPath;

	for(int i = 0; i < argc; i++)
	{
		QString arg(argv[i]);

		if (arg.startsWith("-b="))
		{
			cfgPath = arg.mid(3);
			continue;
		}
	}

	qRegisterMetaType<Tuning::TuningDataSourceState>("TuningDataSourceState");
	qRegisterMetaType<FotipFrame>("FotipFrame");

	QApplication a(argc, argv);
	TuningMainWindow w(cfgPath);
	w.show();

	int result =  a.exec();

	google::protobuf::ShutdownProtobufLibrary();

	return result;
}
