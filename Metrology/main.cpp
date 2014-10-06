#include <QApplication>
#include "MainWindow.h"
#include "Options.h"

// -------------------------------------------------------------------------------------------------------------------

#if defined(Q_OS_WIN) && defined(_MSC_VER)
    #include <vld.h>		// Enable Visula Leak Detector
    // vld.h includes windows.h wich redefine min/max stl functions
    #ifdef min
        #undef min
    #endif
    #ifdef max
        #undef max
    #endif
#endif

// -------------------------------------------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setApplicationName("Metrology");
    a.setOrganizationName("Radiy");
    a.setOrganizationDomain("radiy.com");

    theOptions.load();

    MainWindow w;
    w.show();

    return a.exec();
}

// -------------------------------------------------------------------------------------------------------------------
