#include "stable.h"
#include "moduleconfigurator.h"
#include "../include/configdata.h"
#include <QtWidgets/QApplication>

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
