#include "../HardwareLib/DataProtocols.h"
#include "MainWindow.h"
#include "Options.h"
#include "version.h"

int main(int argc, char *argv[])
{
	// check version of RUP packets
	//
	#if PS_SUPPORT_VERSION != 5
		#error Current version of Rup packets is unknown
	#endif

	QApplication a(argc, argv);

	a.setApplicationName("Packet Source");
	a.setOrganizationName(Manufacturer::RADIY);
	a.setOrganizationDomain(Manufacturer::SITE);

	a.setApplicationVersion(QString("%1.%2.%3 (%4)")
							.arg(U7SET_MAJOR_VERSION)
							.arg(U7SET_MINOR_VERSION)
							.arg(U7SET_PATCH_VERSION)
							.arg(U7SET_BRANCH_NAME));

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
