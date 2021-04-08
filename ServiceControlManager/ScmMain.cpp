#include "MainWindow.h"
#include <QApplication>
#include <QSystemSemaphore>
#include <QSharedMemory>
#include <QMessageBox>
#include <QTranslator>
#include <QSettings>
#include <google/protobuf/stubs/common.h>

#include "../OnlineLib/SocketIO.h"
#include "../OnlineLib/Tcp.h"


const char* const semaphoreString = "ServiceControlManagerSemaphore";
const char* const sharedMemoryString = "ServiceControlManagerSharedMemory";


int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QSystemSemaphore semaphore(semaphoreString, 1);
	bool isAlreadyRunning = false;
	semaphore.acquire();

	// For Linux: Clearing memory if previosly program crashed (pointer counter should be actual after releasing QSharedMemory)
	{
		QSharedMemory sharedMemory(sharedMemoryString);
		sharedMemory.attach();
	}

	QSharedMemory sharedMemory(sharedMemoryString);
	if (sharedMemory.attach())
	{
		isAlreadyRunning = true;
	}
	else
	{
		sharedMemory.create(1);
		isAlreadyRunning = false;
	}

	semaphore.release();

	if (isAlreadyRunning)
	{
		QMessageBox::information(nullptr, "Attention", "Another instance of ServiceControlManager is already running, check tray please");
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

	QTranslator* qtTranslator = nullptr;
	QTranslator* qtbaseTranslator = nullptr;
	QTranslator* appTranslator = nullptr;

    if (locale != "en")
    {
		qtTranslator = new QTranslator(qApp);
        qtTranslator->load(QString("qt_%1.qm").arg(locale),":/translations");
        qApp->installTranslator(qtTranslator);

		qtbaseTranslator = new QTranslator(qApp);
        qtbaseTranslator->load(QString("qtbase_%1.qm").arg(locale),":/translations");
        qApp->installTranslator(qtbaseTranslator);

		appTranslator = new QTranslator(qApp);
        appTranslator->load(QString("ServiceControlManager_%1.qm").arg(locale),":/translations");
        qApp->installTranslator(appTranslator);
    }

	SoftwareInfo si;

	si.init(E::SoftwareType::ServiceControlManager, "SCM", 1, 0);

	MainWindow* mainWindow = new MainWindow(si);

	mainWindow->showMaximized();

	atexit(google::protobuf::ShutdownProtobufLibrary);

	int result = a.exec();

	delete mainWindow;

	DELETE_IF_NOT_NULL(qtTranslator);
	DELETE_IF_NOT_NULL(qtbaseTranslator);
	DELETE_IF_NOT_NULL(appTranslator);

	return result;
}
