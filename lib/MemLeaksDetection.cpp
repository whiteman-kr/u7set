#include "MemLeaksDetection.h"

#if defined (Q_OS_WIN) && defined(Q_DEBUG)

#include <stdlib.h>
#include <crtdbg.h>

#define FALSE   0
#define TRUE    1

int __cdecl reportingHook(int, char* userMessage, int*)
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

void initMemoryLeaksDetection()
{
#if defined (Q_OS_WIN) && defined(Q_DEBUG)

	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportHook2(_CRT_RPTHOOK_INSTALL, reportingHook);

#endif
}

void dumpMemoryLeaks()
{
#if defined (Q_OS_WIN) && defined(Q_DEBUG)

	_CrtDumpMemoryLeaks();
	_CrtSetReportHook2(_CRT_RPTHOOK_REMOVE, reportingHook);

#endif
}


