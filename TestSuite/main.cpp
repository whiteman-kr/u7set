#include "TestSuiteMainWindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	TestSuiteMainWindow w;
	w.show();
	return a.exec();
}
