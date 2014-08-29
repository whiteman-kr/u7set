#include "mainwindow.h"
#include <QtSingleApplication>
#include <QTranslator>
#include <QSettings>

int main(int argc, char *argv[])
{
    QtSingleApplication a(argc, argv);

    if (a.isRunning())
    {
        a.sendMessage("");
        return 0;
    }

    QCoreApplication::setOrganizationName("Radiy");
    QCoreApplication::setOrganizationDomain("inter.project@radiy.com");
    QCoreApplication::setApplicationName("ServiceControlManager");
    a.setQuitOnLastWindowClosed(false);
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
    //a.setActivationWindow(&w);
    w.connect(&a, SIGNAL(messageReceived(const QString&)), SLOT(openEditor()));
    w.showMaximized();

    return a.exec();
}
