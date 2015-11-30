#include "SerialDataTesterServer.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QCoreApplication::setOrganizationName("Radiy");
	QCoreApplication::setApplicationName("Serial Data Tester Server");

	SerialDataTesterServer w;
	w.show();

	return a.exec();
}
