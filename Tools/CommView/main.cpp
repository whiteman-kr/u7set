#include <QApplication>

#include "MainWindow.h"
#include "Options.h"


int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	a.setApplicationName("CommView");
	a.setOrganizationName("Radiy");
	a.setOrganizationDomain("radiy.com");

	theOptions.load();

	MainWindow* pMainWindow = new MainWindow;
	pMainWindow->show();

	int result = a.exec();

	delete pMainWindow;

	theOptions.unload();

	return result;
}
