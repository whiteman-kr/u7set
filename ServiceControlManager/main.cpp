#include "mainwindow.h"
#include <QtSingleApplication>
#include <QTranslator>
#include <QSettings>
#include "version.h"

#include "../include/SocketIO.h"


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


int main(int argc, char *argv[])
{
    QtSingleApplication a(argc, argv);

    if (a.isRunning())
    {
        a.sendMessage("");
        return 0;
    }

    bool closeToTray = false;
    QString trayParam = "--tray";
    for (int i = 0; i < argc; i++)
    {
        if (argv[i] == trayParam)
        {
            closeToTray = true;
            break;
        }
    }

    QCoreApplication::setOrganizationName("Radiy");
    QCoreApplication::setOrganizationDomain("inter.project@radiy.com");
    QCoreApplication::setApplicationName("ServiceControlManager");
    if (closeToTray)
    {
        a.setQuitOnLastWindowClosed(false);
    }
    a.setWindowIcon(QIcon(":/images/SearchComputer.png"));

    QSettings settings;
    QString locale = settings.value("locale", QLocale::system().name()).toString().left(2);
    if (locale != "en")
    {
        QTranslator *qtTranslator = new QTranslator(qApp);
        qtTranslator->load(QString("qt_%1.qm").arg(locale),":/translations");
        qApp->installTranslator(qtTranslator);

        QTranslator *qtbaseTranslator = new QTranslator(qApp);
        qtbaseTranslator->load(QString("qtbase_%1.qm").arg(locale),":/translations");
        qApp->installTranslator(qtbaseTranslator);

        QTranslator *appTranslator = new QTranslator(qApp);
        appTranslator->load(QString("ServiceControlManager_%1.qm").arg(locale),":/translations");
        qApp->installTranslator(appTranslator);
    }

    MainWindow w;
    w.connect(&a, &QtSingleApplication::messageReceived, &w, &MainWindow::openEditor);
    w.showMaximized();

    return a.exec();
}
