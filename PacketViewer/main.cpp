#include "SourceListWidget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QCoreApplication::setOrganizationName("Radiy");
	QCoreApplication::setOrganizationDomain("radiy.com");
	QCoreApplication::setApplicationName("Packet Viewer");

	SourceListWidget w;
	w.show();

	return a.exec();
}
