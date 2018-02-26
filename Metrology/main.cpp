#include <QApplication>

#include "MainWindow.h"
#include "Options.h"
#include "version.h"

#include "../lib/ProtoSerialization.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setApplicationName("Metrology");
    a.setOrganizationName("Radiy");
    a.setOrganizationDomain("radiy.com");

	a.setApplicationVersion(QString("1.6.%1 (%2)").arg(USED_SERVER_COMMIT_NUMBER).arg(BUILD_BRANCH));

    theOptions.load();

	SoftwareInfo si;

	assert(false);			// WM: chek equipmentID in next code!!!!
	si.init(E::SoftwareType::Metrology, "???", 1, 0);

	MainWindow w(si);
    w.show();

    int result = a.exec();

    theOptions.unload();

    google::protobuf::ShutdownProtobufLibrary();

    return result;
}

// -------------------------------------------------------------------------------------------------------------------
