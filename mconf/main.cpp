#include "Stable.h"
#include "moduleconfigurator.h"
#include "../include/ConfigData.h"
#include <QtWidgets/QApplication>

OutputLog theLog;

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QCoreApplication::setOrganizationName("Radiy");
	QCoreApplication::setOrganizationDomain("radiy.com");
	QCoreApplication::setApplicationName("ModuleConfigurator");

	qRegisterMetaType<ConfigDataReader>();

	ModuleConfigurator w;
	w.show();
	int result = a.exec();
	
	return result;
}
