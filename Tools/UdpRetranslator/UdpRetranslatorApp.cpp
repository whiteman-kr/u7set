#include "UdpRetranslatorApp.h"
#include "WUtils.h"

CircularLoggerShared logger;
UdpRetranslatorApp app;
QSettings settings(QSettings::SystemScope, "RadiyQt6", "UdpRetranslator");

UdpRetranslatorApp::UdpRetranslatorApp()
{
}

UdpRetranslatorApp::~UdpRetranslatorApp()
{
	circularLoggerShutdown(logger);
}

bool UdpRetranslatorApp::init(int argc, char** argv)
{
	m_argc = argc;
	m_argv = argv;

	m_appPathFile = m_argv[0];

	logger = std::make_shared<CircularLogger>();
	circularLoggerInit(logger, m_appPathFile, "udprtr", "", 10, 10);
	logger->setLogCodeInfo(false);

	return true;
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
			BREAK_IF_FALSE(getCaptureDevices());
			BREAK_IF_FALSE(printCaptureDevices());
			BREAK_IF_FALSE(readCfgFile(it->second));
			saveCfgFileName(it->second);
			BREAK_IF_FALSE(startRetranslate());
			break;
		}

		std::cout << "\nUnknown command line arguments.\n";
		printHelp();
		break;
	}

	return 0;
}

bool UdpRetranslatorApp::startRetranslate()
{
	DEBUG_LOG_MSG(logger, QString("Configuration file name '%1'").arg(settings.value(CFG_FILE_NAME).toString()));
	// start retranslating threads
	//
	std::vector<std::thread*> rtrThreads;

	int threadNo = 1;

	for(const RetranslateCfg& rtrCfg : m_retranslateCfgs)
	{
		auto it = std::find_if(m_captureDevices.begin(), m_captureDevices.end(),
							[&rtrCfg] (const CaptureDevice& capDevice)
							{
								return rtrCfg.captureDeviceDescription == capDevice.description();
							});

		if (it == m_captureDevices.end())
		{
			DEBUG_LOG_ERR(logger, QString("Capture device '%1' is not found!").arg(rtrCfg.captureDeviceDescription));
		}
		else
		{
			std::thread* rtrThread = new std::thread(&CaptureDevice::retranslate, it, rtrCfg, threadNo++);

			rtrThreads.emplace_back(rtrThread);
		}
	}

	m_quitRequested = false;

	std::unique_lock ul(m_waitQuitMutex);

	m_waitQuit.wait(ul, [this]() { return m_quitRequested; });

	ul.unlock();

	// stop retranslating threads
	//
	CaptureDevice::breakAllCaptures();

	for(std::thread* rtrThread : rtrThreads)
	{
		rtrThread->join();
		delete rtrThread;
	}

	return true;
}

void UdpRetranslatorApp::stopRetranslate()
{
	std::lock_guard lg(m_waitQuitMutex);

	m_quitRequested = true;

	m_waitQuit.notify_all();
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
	m_retranslateCfgs.clear();

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
				RetranslateCfg cc;

				cc.captureDeviceDescription = sl[1].trimmed();

				m_retranslateCfgs.push_back(cc);
			}
			else
			{
				DEBUG_LOG_ERR(logger, QString("Error parsing cfg line: %1").arg(cl));

				result = false;
			}
		}
		else
		{
			if (m_retranslateCfgs.size() == 0)
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

				m_retranslateCfgs.back().rtrEntry.push_back(re);
			}
		}
	}

	if (result == true)
	{
		DEBUG_LOG_MSG(logger, QString("Configuration file %1 successfully parsed").arg(cfgFileName));
	}
	else
	{
		DEBUG_LOG_MSG(logger, QString("Configuration file %1 parsing error!").arg(cfgFileName));
	}

	return result;
}

bool UdpRetranslatorApp::saveCfgFileName(const QString& cfgFileName)
{
	bool result = false;

	settings.setValue(CFG_FILE_NAME, cfgFileName);
	settings.sync();

	switch(settings.status())
	{
	case QSettings::NoError:
		DEBUG_LOG_MSG(logger, "Settings save - Ok");
		result = true;
		break;

	case QSettings::AccessError:
		DEBUG_LOG_ERR(logger, "Settings save AccessError. Run UdpRetranslator with Administrator permissions.");
		break;

	case QSettings::FormatError:
		DEBUG_LOG_ERR(logger, "Settings save FormatError.");
		break;

	default:
		Q_ASSERT(false);
	}

	return result;
}

//

SERVICE_STATUS srvStatus;
SERVICE_STATUS_HANDLE srvStatusHandle = NULL;

TCHAR serviceName[] = _T("UdpRetranslator");

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
	DEBUG_LOG_MSG(logger, "ServiceMain: started");

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

	std::jthread srvThread(&UdpRetranslatorApp::startRetranslate, &app);

	// Tell the service controller we are stopped
	//
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
	DEBUG_LOG_MSG(logger, QString("ServiceCtrlHandler: receives CtrlCode - %1").arg(ctrlCode));

	switch (ctrlCode)
	{
	case SERVICE_CONTROL_STOP :

		DEBUG_LOG_MSG(logger, QString("ServiceCtrlHandler: receives SERVICE_CONTROL_STOP"));

		if (srvStatus.dwCurrentState != SERVICE_RUNNING)
		{
			break;
		}

		srvStatus.dwControlsAccepted = 0;
		srvStatus.dwCurrentState = SERVICE_STOP_PENDING;
		srvStatus.dwWin32ExitCode = 0;
		srvStatus.dwCheckPoint = 4;

		if (SetServiceStatus (srvStatusHandle, &srvStatus) == FALSE)
		{
			DEBUG_LOG_ERR(logger, QString("ServiceCtrlHandler: SetServiceStatus SERVICE_STOP_PENDING returned error: %1").arg(GetLastError()));
		}

		DEBUG_LOG_MSG(logger, QString("ServiceMain: SetServiceStatus SERVICE_STOP_PENDING - Ok"));

		app.stopRetranslate();

		break;

	default:
		DEBUG_LOG_WRN(logger, QString("ServiceCtrlHandler: has no processing for this CtrlCode"));
		break;
	}
}

BOOL WINAPI consoleCtrlHandler(_In_ DWORD dwCtrlType)
{
	switch (dwCtrlType)
	{
	case CTRL_C_EVENT:
		std::cout << "\nCtrl+C pressed by user\n\n";
		CaptureDevice::breakAllCaptures();
		UdpRetranslatorApp::stopRetranslate();
		return TRUE;

	default:
		// Pass signal on to the next handler
		return FALSE;
	}
}


