#include "SourceListWidget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
#if defined (Q_OS_WIN) && defined (Q_DEBUG)
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );	// Memory leak report on app exit
#endif

	QApplication a(argc, argv);
	QCoreApplication::setOrganizationName("Radiy");
	QCoreApplication::setOrganizationDomain("radiy.com");
	QCoreApplication::setApplicationName("Packet Viewer");

	SourceListWidget w;
	w.show();

	return a.exec();
}
