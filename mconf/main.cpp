#include "Stable.h"
#include "ModuleConfigurator.h"
//#include "../include/ConfigData.h"
#include <QtWidgets/QApplication>

#if __has_include("../gitlabci_version.h")
#	include "../gitlabci_version.h"
#endif

OutputLog theLog;

Q_DECLARE_METATYPE(std::vector<quint8>)

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QCoreApplication::setOrganizationName("Radiy");
	QCoreApplication::setOrganizationDomain("radiy.com");
	QCoreApplication::setApplicationName("ModuleConfigurator");

#ifdef GITLAB_CI_BUILD
	a.setApplicationVersion(QString("0.8.%1 (%2)").arg(CI_PIPELINE_IID).arg(CI_BUILD_REF_SLUG));
#else
	QCoreApplication::setApplicationVersion(QString("0.8.LOCALBUILD"));
#endif

    qRegisterMetaType<std::vector<quint8>>();

	ModuleConfigurator w;
	w.show();
	int result = a.exec();
	
	return result;
}

