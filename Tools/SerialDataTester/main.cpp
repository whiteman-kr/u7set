#include "serialdatatester.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	SerialDataTester w;
	w.show();

	return a.exec();
}
