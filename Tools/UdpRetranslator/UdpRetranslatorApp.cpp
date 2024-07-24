#include "UdpRetranslatorApp.h"
#include "WUtils.h"


UdpRetranslatorApp::UdpRetranslatorApp(int argc, char** argv) :
	m_argc(argc),
	m_argv(argv)
{
	m_appPathFile = m_argv[0];
}

int UdpRetranslatorApp::run()
{
	m_log = std::make_shared<CircularLogger>();
	circularLoggerInit(m_log, m_appPathFile, "udprtr", "", 10, 10);

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

		std::cout << "\nUnknown command line arguments.\n";
		printHelp();
		break;
	}

	//

	circularLoggerShutdown(m_log);

	return 0;
}

bool UdpRetranslatorApp::loadNpcapDlls()
{
#ifdef _WIN32

	_TCHAR npcapDllDir[1000];

	UINT len = GetSystemDirectory(npcapDllDir, 950);

	if (len == 0)
	{
		DEBUG_LOG_ERR(m_log, QString("GetSystemDirectory error: %1").arg(GetLastError()));
		return false;
	}

	_tcscat_s(npcapDllDir, 1000, _T("\\Npcap"));

	if (SetDllDirectory(npcapDllDir) == 0)
	{
		DEBUG_LOG_ERR(m_log, QString("SetDllDirectory error: %1").arg(GetLastError()));
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
	std::cout << "\n";
}

bool UdpRetranslatorApp::getCaptureDevices()
{
	return CaptureDevice::getCaptureDevices(&m_captureDevices, m_log);
}

bool UdpRetranslatorApp::printCaptureDevices()
{
	DEBUG_LOG_MSG(m_log, "");

	if(m_captureDevices.empty())
	{
		DEBUG_LOG_MSG(m_log, "\nNo capture devices found! Make sure Npcap is installed.\n");
		DEBUG_LOG_MSG(m_log, "");
		return false;
	}

	DEBUG_LOG_MSG(m_log, QString("Available capture devices:"));
	DEBUG_LOG_MSG(m_log, "");

	int devNo = 0;

	for(const CaptureDevice& capDev : m_captureDevices)
	{
		devNo++;

		DEBUG_LOG_MSG(m_log, QString("%1. %2").arg(devNo).arg(capDev.description()));
	}

	DEBUG_LOG_MSG(m_log, "");

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

bool UdpRetranslatorApp::runService()
{
	return true;
}
