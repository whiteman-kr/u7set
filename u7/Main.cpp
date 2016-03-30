#include "Stable.h"
#include "MainWindow.h"
#include "Settings.h"
#include "GlobalMessanger.h"
#include "../VFrame30/VFrame30Library.h"
#include "../include/DbController.h"
#include "../include/DeviceObject.h"
#include "../include/PropertyObject.h"
#include "Builder/OptoModule.h"

#include <QtQml>

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

	VFrame30::VFrame30Library::Init();
	Hardware::Init();

	GlobalMessanger::instance();		// Create instance of GlobalMessanger

	// Read settings
	//
	theSettings.load();

	qRegisterMetaType<DbUser>();
	qRegisterMetaType<DbFileInfo>();
	qRegisterMetaType<DbFile>();
	qRegisterMetaType<DbChangesetInfo>();
	qRegisterMetaType<DbProject>();
	qRegisterMetaType<std::vector<DbProject>>();
	qRegisterMetaType<std::vector<DbFileInfo>>();
	qRegisterMetaType<std::vector<std::shared_ptr<DbFile>>>();
	qRegisterMetaType<std::vector<int>>();
	qRegisterMetaType<std::vector<DbFileInfo>>();
	qRegisterMetaType<E::SignalType>();

    qmlRegisterType<QJsVariantList>();
    qmlRegisterType<Hardware::OptoPort>();

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

	GlobalMessanger::free();		// Delete instance of GlobalMessanger

	VFrame30::VFrame30Library::Shutdown();
	Hardware::Shutdwon();

	google::protobuf::ShutdownProtobufLibrary();

	// Exit
	//
	return result;
}

