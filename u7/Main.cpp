#include "MainWindow.h"
#include "Settings.h"
#include "GlobalMessanger.h"
#include "../VFrame30/VFrame30Library.h"
#include "../lib/DbController.h"
#include "../lib/DeviceObject.h"
#include "../lib/PropertyObject.h"
#include "../lib/TimeStamp.h"
#include "../lib/LmDescription.h"
#include "../lib/Configurator.h"
#include "../Builder/Builder.h"
#include "../lib/Signal.h"
#include <QList>

#if __has_include("../gitlabci_version.h")
#	include "../gitlabci_version.h"
#endif

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


int main(int argc, char *argv[])
{
#if defined (Q_OS_WIN) && defined(Q_DEBUG)
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	// To see all memory leaks, not only in the own code, comment the next line
	prevHook = _CrtSetReportHook(reportingHook);
#endif

	QApplication a(argc, argv);

	// --
	//
	a.setApplicationName("u7");
	a.setOrganizationName("Radiy");
	a.setOrganizationDomain("radiy.com");

#ifdef GITLAB_CI_BUILD
	a.setApplicationVersion(QString("0.8.%1 (%2)").arg(CI_PIPELINE_ID).arg(CI_BUILD_REF_SLUG));
#else
	a.setApplicationVersion(QString("0.8.LOCALBUILD"));
#endif

	VFrame30::VFrame30Library::init();
	Hardware::init();
	DbController::init();
	Builder::init();

	GlobalMessanger::instance();		// Create instance of GlobalMessanger

	// Read settings
	//
	theSettings.load();

	qRegisterMetaType<std::vector<int>>();
	qRegisterMetaType<E::SignalType>();
	qRegisterMetaType<TimeStamp>();
	qRegisterMetaType<TimeSpan>();
	qRegisterMetaType<std::vector<UartPair>>();
	qRegisterMetaType<std::map<QString, std::vector<UartPair>>>();
	qRegisterMetaType<QVector<int>>();
	qRegisterMetaType<ID_AppSignalID>();
	qRegisterMetaType<QVector<ID_AppSignalID>>();

    // Start database communication thread
	//
	DbController dbController;

	dbController.disableProgress();

	dbController.setHost(theSettings.serverIpAddress());
	dbController.setPort(theSettings.serverPort());
	dbController.setServerUsername(theSettings.serverUsername());
	dbController.setServerPassword(theSettings.serverPassword());

	// --
	//
	MainWindow w(&dbController, nullptr);
	w.show();

	dbController.enableProgress();

	int result = a.exec();

	// Shutting down
	//
	Builder::shutdown();
	DbController::shutdown();
	VFrame30::VFrame30Library::shutdown();
	Hardware::shutdown();

	google::protobuf::ShutdownProtobufLibrary();

	// Exit
	//
	return result;
}

