#include "Widget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	a.setApplicationName("RupPacketSender");
	a.setOrganizationName("Radiy");
	a.setOrganizationDomain("radiy.com");

	Widget w;
	w.show();

	return a.exec();
}
