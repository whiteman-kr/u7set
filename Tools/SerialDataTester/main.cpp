#include "serialdatatester.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QCoreApplication::setOrganizationName("Radiy");
	QCoreApplication::setApplicationName("Serial Data Tester");

	SerialDataTester w;
	w.show();

	return a.exec();
}
