#include <QApplication>

#include "MainWindow.h"
#include "Options.h"
#include "../../lib/ProtoSerialization.h"

#if __has_include("../../gitlabci_version.h")
#	include "../../gitlabci_version.h"
#endif

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	a.setApplicationName("Packet Source");
	a.setOrganizationName("Radiy");
	a.setOrganizationDomain("radiy.com");


#ifdef GITLAB_CI_BUILD
	a.setApplicationVersion(QString("1.8.%1 (%2)").arg(CI_PIPELINE_ID).arg(CI_BUILD_REF_SLUG));
#else
	a.setApplicationVersion(QString("1.8.LOCALBUILD"));
#endif

	theOptions.load();

	MainWindow w;
	w.show();

	int result = a.exec();

	theOptions.unload();

	google::protobuf::ShutdownProtobufLibrary();

	return result;
}
