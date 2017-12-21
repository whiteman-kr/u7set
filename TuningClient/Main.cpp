#include "Main.h"
#include "MainWindow.h"
#include <QApplication>
#include "Settings.h"
#include "UserManager.h"
#include <QCommandLineParser>
#include "../VFrame30/VFrame30Library.h"
#include "version.h"

#if defined (Q_OS_WIN) && defined(Q_DEBUG)

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

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

QSharedMemory* theSharedMemorySingleApp = nullptr;

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


int main(int argc, char* argv[])
{

#if defined (Q_OS_WIN) && defined(Q_DEBUG)
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	// To see all memory leaks, not only in the own code, comment the next line
	prevHook = _CrtSetReportHook(reportingHook);
#endif

	int result = 0;

	QApplication a(argc, argv);
	a.setApplicationName("TuningClient");
	a.setOrganizationName("Radiy");
	a.setOrganizationDomain("radiy.com");

	a.setApplicationVersion(QString("0.1.%1 (%2)").arg(USED_SERVER_COMMIT_NUMBER).arg(BUILD_BRANCH));

	//int* x = new int[100];

	VFrame30::VFrame30Library::Init();

	theSettings.RestoreUser();
	theSettings.RestoreSystem();

	loadLanguage(theSettings.language());

	// Parse the command line
	//

	QCommandLineParser parser;

	parser.addHelpOption();
	parser.addVersionOption();

	// A string option with id (-id)

	QCommandLineOption idOption("id", "Set the TuningClient ID.", "TuningClient ID");
	parser.addOption(idOption);

	// A boolean option with simulation (-simulate)

	QCommandLineOption simulationOption("simulate", "Simulate signals values");
	parser.addOption(simulationOption);

	parser.process(*qApp);

	QString clientID = parser.value(idOption);

	if (clientID.isEmpty() == false)
	{
	    theSettings.setInstanceStrId(clientID);
	}

	theSettings.m_simulationMode = parser.isSet(simulationOption);

	//
	//

	SoftwareInfo softwareInfo;

	softwareInfo.init(E::SoftwareType::TuningClient, theSettings.instanceStrId(), 0, 1);

	// Check to run the application in one instance
	//
	theSharedMemorySingleApp = new QSharedMemory(QString("TuningClient") + theSettings.instanceStrId());

	if(theSharedMemorySingleApp->attach(QSharedMemory::ReadWrite) == false)
	{
		if(theSharedMemorySingleApp->create(sizeof(TuningClientSharedData)) == false)
		{
			qDebug() << "Failed to create QSharedMemory object!";
			assert(false);
		}
		else
		{
			bool ok = theSharedMemorySingleApp->lock();
			if (ok == true)
			{
				void* buffer = theSharedMemorySingleApp->data();

				TuningClientSharedData data;
				memcpy(buffer, &data, sizeof(TuningClientSharedData));

				ok = theSharedMemorySingleApp->unlock();
				if (ok == false)
				{
					qDebug() << "Failed to unlock QSharedMemory object!";
					assert(false);
				}
			}
			else
			{
				qDebug() << "Failed to lock QSharedMemory object!";
				assert(false);
			}

			// Run the application
			//
			theMainWindow = new MainWindow(softwareInfo);
			theMainWindow->show();

			result = a.exec();

			delete theMainWindow;

			theSettings.StoreUser();
		}
	}
	else
	{
		QMessageBox::critical(nullptr, QObject::tr("Error"), QObject::tr("Application is already running!"));

		bool ok = theSharedMemorySingleApp->lock();
		if (ok == true)
		{
			TuningClientSharedData* data = (TuningClientSharedData*)theSharedMemorySingleApp->data();

			data->showCommand = true;

			ok = theSharedMemorySingleApp->unlock();
			if (ok == false)
			{
				qDebug() << "Failed to unlock QSharedMemory object!";
				assert(false);
			}
		}
		else
		{
			qDebug() << "Failed to lock QSharedMemory object!";
			assert(false);
		}


		theSharedMemorySingleApp->detach();
	}

	delete theSharedMemorySingleApp;
	theSharedMemorySingleApp = nullptr;

	VFrame30::VFrame30Library::Shutdown();
	google::protobuf::ShutdownProtobufLibrary();

#if defined (Q_OS_WIN)
	_CrtDumpMemoryLeaks();
#endif

	return result;
}
