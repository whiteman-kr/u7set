#include "Stable.h"
#include "ModuleConfigurator.h"
//#include "../include/ConfigData.h"
#include <QtWidgets/QApplication>

OutputLog theLog;

Q_DECLARE_METATYPE(std::vector<quint8>)

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QCoreApplication::setOrganizationName("Radiy");
	QCoreApplication::setOrganizationDomain("radiy.com");
	QCoreApplication::setApplicationName("ModuleConfigurator");

    qRegisterMetaType<std::vector<quint8>>();

	ModuleConfigurator w;
	w.show();
	int result = a.exec();
	
	return result;
}

