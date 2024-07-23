#include "CircularLogger.h"

// Visual Leak Detector
//
#if defined(Q_OS_WIN) && defined(QT_DEBUG)
	#if __has_include("C:/Program Files (x86)/Visual Leak Detector/include/vld.h")
		#include "C:/Program Files (x86)/Visual Leak Detector/include/vld.h"
	#else
		#if __has_include("D:/Program Files (x86)/Visual Leak Detector/include/vld.h")
			#include "D:/Program Files (x86)/Visual Leak Detector/include/vld.h"
		#endif
	#endif
#endif	// Visual Leak Detector

SERVICE_STATUS srvStatus;
SERVICE_STATUS_HANDLE srvStatusHandle = NULL;
HANDLE srvStopEvent = INVALID_HANDLE_VALUE;

TCHAR serviceName[] = _T("udprtr");

VOID serviceMain(DWORD argc, LPTSTR* argv);
VOID serviceCtrlHandler(DWORD CtrlCode);

int main(int argc, char** argv)
{
	std::shared_ptr<CircularLogger> log = std::make_shared<CircularLogger>();

	circularLoggerInit(log, argv[0], "udprtr", "", 10, 10);

	QThread::msleep(2000);

	DEBUG_LOG_MSG(log, "Log Message");
	DEBUG_LOG_MSG(log, "Log Message 2");

	QThread::msleep(2000);

	circularLoggerShutdown(log);

	return 1;

/*	SERVICE_TABLE_ENTRY serviceTable[] =
	{
		{
			.lpServiceName = serviceName,
			.lpServiceProc = serviceMain
		},

		{
			.lpServiceName = NULL,
			.lpServiceProc = NULL
		}
	};

	StartServiceCtrlDispatcher(serviceTable);*/
}

VOID serviceMain(DWORD argc, LPTSTR* argv)
{
	DWORD status = E_FAIL;

	// Register our service control handler with the SCM
	srvStatusHandle = RegisterServiceCtrlHandler (serviceName, serviceCtrlHandler);

	if (srvStatusHandle == NULL)
	{
		return;
	}

	ZeroMemory(&srvStatus, sizeof(srvStatus));

	srvStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	srvStatus.dwControlsAccepted = 0;
	srvStatus.dwCurrentState = SERVICE_START_PENDING;
	srvStatus.dwWin32ExitCode = 0;
	srvStatus.dwServiceSpecificExitCode = 0;
	srvStatus.dwCheckPoint = 0;

	if (SetServiceStatus (srvStatusHandle , &srvStatus) == FALSE)
	{
		OutputDebugString(_T(
			"My Sample Service: ServiceMain: SetServiceStatus returned error"));
	}

	/*
	 * Perform tasks necessary to start the service here
	 */

	// Create a service stop event to wait on later
	srvStopEvent = CreateEvent (NULL, TRUE, FALSE, NULL);

	if (srvStopEvent == NULL)
	{
		// Error creating event
		// Tell service controller we are stopped and exit
		srvStatus.dwControlsAccepted = 0;
		srvStatus.dwCurrentState = SERVICE_STOPPED;
		srvStatus.dwWin32ExitCode = GetLastError();
		srvStatus.dwCheckPoint = 1;

		if (SetServiceStatus (srvStatusHandle, &srvStatus) == FALSE)
		{
			OutputDebugString(_T(
				"My Sample Service: ServiceMain: SetServiceStatus returned error"));
		}

		return;
	}

	// Tell the service controller we are started
	//
	srvStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	srvStatus.dwCurrentState = SERVICE_RUNNING;
	srvStatus.dwWin32ExitCode = 0;
	srvStatus.dwCheckPoint = 0;

	if (SetServiceStatus (srvStatusHandle, &srvStatus) == FALSE)
	{
		OutputDebugString(_T(
			"My Sample Service: ServiceMain: SetServiceStatus returned error"));
	}

	// Start a thread that will perform the main task of the service
	//
//	HANDLE hThread = CreateThread (NULL, 0, ServiceWorkerThread, NULL, 0, NULL);

	// Wait until our worker thread exits signaling that the service needs to stop
	//
//	WaitForSingleObject (hThread, INFINITE);

	/*
	 * Perform any cleanup tasks
	 */

	CloseHandle(srvStopEvent);

	// Tell the service controller we are stopped
	srvStatus.dwControlsAccepted = 0;
	srvStatus.dwCurrentState = SERVICE_STOPPED;
	srvStatus.dwWin32ExitCode = 0;
	srvStatus.dwCheckPoint = 3;

	if (SetServiceStatus (srvStatusHandle, &srvStatus) == FALSE)
	{
		OutputDebugString(_T(
			"My Sample Service: ServiceMain: SetServiceStatus returned error"));
	}

	return;
}

VOID serviceCtrlHandler(DWORD CtrlCode)
{

}

