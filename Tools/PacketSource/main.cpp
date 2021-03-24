#include <QApplication>

#include "MainWindow.h"
#include "Options.h"
#include "../Proto/ProtoSerialization.h"

#if __has_include("../../gitlabci_version.h")
#	include "../../gitlabci_version.h"
#endif

int main(int argc, char *argv[])
{
	// check version of RUP packets
	//
	#if RUP_VERSION != PS_SUPPORT_VERSION
		#error Current version of Rup packets is unknown
	#endif

	QApplication a(argc, argv);

	a.setApplicationName("Packet Source");
	a.setOrganizationName("Radiy");
	a.setOrganizationDomain("radiy.com");


#ifdef GITLAB_CI_BUILD
	a.setApplicationVersion(QString("1.8.%1 (%2)").arg(CI_PIPELINE_ID).arg(CI_BUILD_REF_SLUG));
#else
	a.setApplicationVersion(QString("1.8.LOCALBUILD"));
#endif

	// init Options
	//
	Options options;
	options.load();

	//
	//
	MainWindow* pMainWindow = new MainWindow(options);
	pMainWindow->show();

	int result = a.exec();

	delete pMainWindow;

	options.unload();

	google::protobuf::ShutdownProtobufLibrary();

	return result;
}
