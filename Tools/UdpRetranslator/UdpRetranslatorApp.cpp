#include "UdpRetranslatorApp.h"
#include "WUtils.h"

CircularLoggerShared logger;

UdpRetranslatorApp::UdpRetranslatorApp(int argc, char** argv) :
	m_argc(argc),
	m_argv(argv)
{
	m_appPathFile = m_argv[0];

	logger = std::make_shared<CircularLogger>();
	circularLoggerInit(logger, m_appPathFile, "udprtr", "", 10, 10);
	logger->setLogCodeInfo(false);
}

UdpRetranslatorApp::~UdpRetranslatorApp()
{
	circularLoggerShutdown(logger);
}

int UdpRetranslatorApp::run()
{
	RETURN_VALUE_IF_FALSE(loadNpcapDlls(), 1);

	parseCmdLineArgs();

	if (m_cmdLineArgs.empty())
	{
		runService();
		return 0;
	}

	while(1)
	{
		if (m_cmdLineArgs.contains(ARG_HELP) || m_cmdLineArgs.empty())
		{
			printHelp();
			break;
		}

		if (m_cmdLineArgs.contains(ARG_DEV_LIST))
		{
			BREAK_IF_FALSE(getCaptureDevices());
			BREAK_IF_FALSE(printCaptureDevices());
			break;
		}

		if (m_cmdLineArgs.contains(ARG_TEST_CAP))
		{
			BREAK_IF_FALSE(getCaptureDevices());
			BREAK_IF_FALSE(printCaptureDevices());
			BREAK_IF_FALSE(testCaptureDevice());
			break;
		}

		auto it = m_cmdLineArgs.find(ARG_CFG);

		if (it != m_cmdLineArgs.end())
		{
			BREAK_IF_FALSE(readCfgFile(it->second));
			BREAK_IF_FALSE(retranslate());
		}

		std::cout << "\nUnknown command line arguments.\n";
		printHelp();
		break;
	}

	return 0;
}

bool UdpRetranslatorApp::loadNpcapDlls()
{
#ifdef _WIN32

	_TCHAR npcapDllDir[1000];

	UINT len = GetSystemDirectory(npcapDllDir, 950);

	if (len == 0)
	{
		DEBUG_LOG_ERR(logger, QString("GetSystemDirectory error: %1").arg(GetLastError()));
		return false;
	}

	_tcscat_s(npcapDllDir, 1000, _T("\\Npcap"));

	if (SetDllDirectory(npcapDllDir) == 0)
	{
		DEBUG_LOG_ERR(logger, QString("SetDllDirectory error: %1").arg(GetLastError()));
		return false;
	}

#endif

	return true;
}

void UdpRetranslatorApp::parseCmdLineArgs()
{
	m_cmdLineArgs.clear();

	for(int i = 1; i < m_argc; i++)
	{
		QString arg(m_argv[i]);

		QStringList l = arg.split("=", Qt::SkipEmptyParts);

		if (l.size() == 1)
		{
			m_cmdLineArgs.emplace(l[0].trimmed().toLower(), QString());
		}
		else
		{
			m_cmdLineArgs.emplace(l[0].trimmed().toLower(), l[1].trimmed());
		}
	}
}

void UdpRetranslatorApp::printHelp()
{
	std::cout << "\nUse UdpRtr.exe [options]\n\n";
	std::cout << "where options is:\n\n";
	std::cout << QString("%1\t\tprint this help\n").arg(ARG_HELP).toStdString();
//	std::cout << "-e\t\trun UDP retranslator as console application\n";
	std::cout << QString("%1\tprint list of capture devices\n").arg(ARG_DEV_LIST).toStdString();
	std::cout << QString("%1\ttest capturing on device\n").arg(ARG_TEST_CAP).toStdString();
	std::cout << QString("%1=cfgFileName\tload config file and start UDP retranslation\n").arg(ARG_CFG).toStdString();
	std::cout << "\n";
}

bool UdpRetranslatorApp::getCaptureDevices()
{
	return CaptureDevice::getCaptureDevices(&m_captureDevices, logger);
}

bool UdpRetranslatorApp::printCaptureDevices()
{
	DEBUG_LOG_MSG(logger, "");

	if(m_captureDevices.empty())
	{
		DEBUG_LOG_MSG(logger, "\nNo capture devices found! Make sure Npcap is installed.\n");
		DEBUG_LOG_MSG(logger, "");
		return false;
	}

	DEBUG_LOG_MSG(logger, QString("Available capture devices:"));
	DEBUG_LOG_MSG(logger, "");

	int devNo = 0;

	for(const CaptureDevice& capDev : m_captureDevices)
	{
		devNo++;

		DEBUG_LOG_MSG(logger, QString("%1. %2").arg(devNo).arg(capDev.description()));
	}

	DEBUG_LOG_MSG(logger, "");

	return true;
}

bool UdpRetranslatorApp::testCaptureDevice()
{
	int devNo = 0;

	while(1)
	{
		std::cout << "To test capturing enter a number of device or 0 to exit program: ";
		std::cin >> devNo;

		if (devNo == 0)
		{
			return false;
		}

		if (devNo < 0 || devNo > TO_INT(m_captureDevices.size()))
		{
			std::cout << "\nWrong number of capture device\n\n";
		}

		break;
	}

	if (devNo == 0)
	{
		return false;
	}

	devNo--;

	CaptureDevice& capDevice = m_captureDevices[devNo];

	std::cout << QString("\nTest capturing on device '%1' (press Ctrl+C to exit program)\n\n").
					arg(capDevice.description()).toStdString();

	capDevice.testCapturing();

	return false;
}

bool UdpRetranslatorApp::readCfgFile(const QString& cfgFileName)
{
	m_captureCfgs.clear();

	QFile cfgFile(cfgFileName);

	bool res = cfgFile.open(QIODeviceBase::ReadOnly | QIODeviceBase::Text);

	if (res == false)
	{
		DEBUG_LOG_ERR(logger, QString("Error open configuration file %1").arg(cfgFileName));
		return false;
	}

	QStringList cfg = QString(cfgFile.readAll()).split("\n", Qt::SkipEmptyParts);

	for(QString& cl : cfg)
	{
		cl = cl.trimmed();
	}

	bool result = true;

	for(QString& cl : cfg)
	{
		if (cl.startsWith("captureFrom") == true)
		{
			QStringList sl = cl.split("=", Qt::SkipEmptyParts);

			if (sl.size() == 2)
			{
				CaptureCfg cc;

				cc.captureDeviceDescription = sl[1];

				m_captureCfgs.push_back(cc);
			}
			else
			{
				DEBUG_LOG_ERR(logger, QString("Error parsing cfg line: %1").arg(cl));
			}

			result = false;
		}
		else
		{
			if (m_captureCfgs.size() == 0)
			{
				DEBUG_LOG_ERR(logger, QString("Sentence 'captureFrom' not found!"));
				result = false;
				continue;
			}

			QStringList sl = cl.split("->", Qt::SkipEmptyParts);

			if (sl.size() != 3)
			{
				DEBUG_LOG_ERR(logger, QString("Error parsing cfg line: %1").arg(cl));

				result = false;
			}
			else
			{
				RetranslateEntry re;
				HostAddressPort hp;

				//

				bool res = hp.setAddressPortStr(sl[0].trimmed(), 0);

				if (res == false)
				{
					DEBUG_LOG_ERR(logger, QString("Wrong source IP:port - %1").arg(sl[0].trimmed()));
					result = false;
				}
				else
				{
					re.srcAddr = hp;
				}

				//

				res = hp.setAddressPortStr(sl[1].trimmed(), 0);

				if (res == false)
				{
					DEBUG_LOG_ERR(logger, QString("Wrong destination IP:port - %1").arg(sl[1].trimmed()));
					result = false;
				}
				else
				{
					re.destAddr = hp;
				}

				//

				res = hp.setAddressPortStr(sl[2].trimmed(), 0);

				if (res == false)
				{
					DEBUG_LOG_ERR(logger, QString("Wrong sendTo IP:port - %1").arg(sl[2].trimmed()));
					result = false;
				}
				else
				{
					re.sendToAddr = hp;
				}

				m_captureCfgs.back().rtrEntry.push_back(re);
			}
		}
	}

	return result;
}

bool UdpRetranslatorApp::retranslate()
{
	return true;
}

SERVICE_STATUS srvStatus;
SERVICE_STATUS_HANDLE srvStatusHandle = NULL;
HANDLE srvStopEvent = INVALID_HANDLE_VALUE;

TCHAR serviceName[] = _T("udprtr");

VOID serviceMain(DWORD argc, LPTSTR* argv);
VOID serviceCtrlHandler(DWORD CtrlCode);

bool UdpRetranslatorApp::runService()
{
	SERVICE_TABLE_ENTRY serviceTable[] =
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

	StartServiceCtrlDispatcher(serviceTable);

	return true;
}

VOID serviceMain(DWORD argc, LPTSTR* argv)
{
	// Register our service control handler with the SCM
	//
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
		DEBUG_LOG_ERR(logger, QString("ServiceMain: SetServiceStatus SERVICE_START_PENDING returned error: %1").arg(GetLastError()));
		return;
	}

	DEBUG_LOG_MSG(logger, QString("ServiceMain: SetServiceStatus SERVICE_START_PENDING - Ok"));

	/*
	 * Perform tasks necessary to start the service here
	 */

	// Create a service stop event to wait on later
	//
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
			DEBUG_LOG_ERR(logger, QString("ServiceMain: SetServiceStatus(2) returned error: %1").arg(GetLastError()));
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
		DEBUG_LOG_ERR(logger, QString("ServiceMain: SetServiceStatus SERVICE_RUNNING returned error: %1").arg(GetLastError()));
	}

	DEBUG_LOG_MSG(logger, QString("ServiceMain: SetServiceStatus SERVICE_RUNNING - Ok"));

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
		DEBUG_LOG_ERR(logger, QString("ServiceMain: SetServiceStatus SERVICE_STOPPED returned error: %1").arg(GetLastError()));
		return;
	}

	DEBUG_LOG_MSG(logger, QString("ServiceMain: SetServiceStatus SERVICE_STOPPED - Ok"));

	return;
}

VOID serviceCtrlHandler(DWORD ctrlCode)
{
	switch (ctrlCode)
	{
	case SERVICE_CONTROL_STOP :

		if (srvStatus.dwCurrentState != SERVICE_RUNNING)
		{
			break;
		}

		/*
		 * Perform tasks necessary to stop the service here
		 */

		srvStatus.dwControlsAccepted = 0;
		srvStatus.dwCurrentState = SERVICE_STOP_PENDING;
		srvStatus.dwWin32ExitCode = 0;
		srvStatus.dwCheckPoint = 4;

		if (SetServiceStatus (srvStatusHandle, &srvStatus) == FALSE)
		{
			DEBUG_LOG_ERR(logger, QString("ServiceMain: SetServiceStatus SERVICE_STOP_PENDING returned error: %1").arg(GetLastError()));
		}

		// This will signal the worker thread to start shutting down
		//
		SetEvent (srvStopEvent);
		break;

	default:
		break;
	}
}


