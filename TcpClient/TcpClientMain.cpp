#include "TcpClientMainWindow.h"
#include <QApplication>



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
	QApplication a(argc, argv);
	TcpClientMainWindow w;
	w.show();

	return a.exec();
}
