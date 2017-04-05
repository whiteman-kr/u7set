#include <QApplication>

#include "MainWindow.h"
#include "Options.h"

#include "../lib/ProtoSerialization.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setApplicationName("Metrology");
    a.setOrganizationName("Radiy");
    a.setOrganizationDomain("radiy.com");

    theOptions.load();

    MainWindow w;
    w.show();

    int result = a.exec();

    theOptions.unload();

    google::protobuf::ShutdownProtobufLibrary();

    return result;
}

// -------------------------------------------------------------------------------------------------------------------
