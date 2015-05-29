#include "MonitorMainWindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MonitorMainWindow w;
	w.show();

	return a.exec();
}
