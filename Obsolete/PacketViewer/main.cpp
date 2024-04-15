#include "SourceListWidget.h"
#include <QApplication>
#include "../../lib/ConstStrings.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QCoreApplication::setOrganizationName(Manufacturer::RADIY);
	QCoreApplication::setOrganizationDomain(Manufacturer::SITE);
	QCoreApplication::setApplicationName("Packet Viewer");

	SourceListWidget w;
	w.show();

	return a.exec();
}
