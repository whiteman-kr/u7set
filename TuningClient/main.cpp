#include "MainWindow.h"
#include <QApplication>
#include "Settings.h"
#include "UserManager.h"

#if defined (Q_OS_WIN) && defined(Q_DEBUG)

_CRT_REPORT_HOOK prevHook = nullptr;

#define FALSE   0
#define TRUE    1

int reportingHook(int, char* userMessage, int*)
{
	// This function is called several times for each memory leak.
	// Each time a part of the error message is supplied.
	// This holds number of subsequent detail messages after
	// a leak was reported
	const int numFollowupDebugMsgParts = 2;
	static bool ignoreMessage = false;
	static int debugMsgPartsCount = 0;
	static int leakCounter = 0;

	// check if the memory leak reporting starts
	if ((strcmp(userMessage,"Detected memory leaks!\n") == 0)
			|| ignoreMessage)
	{
		// check if the memory leak reporting ends
		if (strcmp(userMessage,"Object dump complete.\n") == 0)
		{
			_CrtSetReportHook(prevHook);
			ignoreMessage = false;
			if (leakCounter > 0)
			{
				return FALSE;
			}
		}
		else
		{
			ignoreMessage = true;
		}

		// something from our own code?
		if(strstr(userMessage, ".cpp") == NULL)
		{
			if(debugMsgPartsCount++ < numFollowupDebugMsgParts
					&& strcmp(userMessage,"Detected memory leaks!\n") != 0
					&& strcmp(userMessage,"Dumping objects ->\n") != 0)
			{
				// give it back to _CrtDbgReport() to be printed to the console
				return FALSE;
			}
			else
			{
				return TRUE;  // ignore it
			}
		}
		else
		{
			debugMsgPartsCount = 0;
			leakCounter++;

			// give it back to _CrtDbgReport() to be printed to the console
			return FALSE;
		}
	}
	else
	{
		// give it back to _CrtDbgReport() to be printed to the console
		return FALSE;
	}
}

#endif

QTranslator m_translator; // contains the translations for this application

void switchTranslator(QTranslator& translator, const QString& filename)
{
    // remove the old translator
    qApp->removeTranslator(&translator);

    // load the new translator
    if(translator.load(filename))
    {
        qApp->installTranslator(&translator);
    }
}

void loadLanguage(const QString& rLanguage)
{
    QLocale locale = QLocale(rLanguage);
    QLocale::setDefault(locale);

    QString langPath = QApplication::applicationDirPath();
    langPath.append("/languages");

    switchTranslator(m_translator, QString("%1/TuningClient_%2.qm").arg(langPath).arg(rLanguage));
}


int main(int argc, char *argv[])
{
#if defined (Q_OS_WIN) && defined(Q_DEBUG)
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	// To see all memory leaks, not only in the own code, comment the next line
	//prevHook = _CrtSetReportHook(reportingHook);
#endif


	QApplication a(argc, argv);
	a.setApplicationName("TuningClient");
	a.setOrganizationName("Radiy");
	a.setOrganizationDomain("radiy.com");

	theSettings.RestoreUser();
	theSettings.RestoreSystem();
	theUserManager.Restore();

    int result = 0;

    loadLanguage(theSettings.language());

    QSharedMemory* sharedMemorySingleApp = new QSharedMemory(QString("TuningClient") + theSettings.instanceStrId());

    if(sharedMemorySingleApp->attach(QSharedMemory::ReadOnly) == false)
    {
        if(sharedMemorySingleApp->create(1) == false)
        {
            QMessageBox::critical(nullptr, QObject::tr("Error"), QObject::tr("Failed to create QSharedMemory object!"));
            assert(false);
        }
        else
        {
            // run the application
            //
            theMainWindow = new MainWindow();
            theMainWindow->show();

            result = a.exec();

            delete theMainWindow;

            theSettings.StoreUser();
        }
    }
    else
    {
        QMessageBox::critical(nullptr, QObject::tr("Error"), QObject::tr("Application is already running!"));
        sharedMemorySingleApp->detach();
    }

    delete sharedMemorySingleApp;

	google::protobuf::ShutdownProtobufLibrary();

	return result;
}
