#include "UalTester.h"
#include <QDebug>

#include "../../lib/CommandLineParser.h"
#include "../../lib/SocketIO.h"

// -------------------------------------------------------------------------------------------------------------------
//
// UalTester class implementation
//
// -------------------------------------------------------------------------------------------------------------------

const char* const UalTester::SETTING_CFG_SERVICE_IP1 = "CfgServiceIP1";
const char* const UalTester::SETTING_CFG_SERVICE_IP2 = "CfgServiceIP2";
const char* const UalTester::SETTING_EQUIPMENT_ID = "EquipmentID";
const char* const UalTester::SETTING_TEST_FILE_NAME  = "TestFileName";

const char* const UalTester::SETTING_ERROR_IGNORE = "ErrorIngnore";
const char* const UalTester::SETTING_TEST_ID = "TestID";
const char* const UalTester::SETTING_FROM_TEST_ID = "FromTestID";
const char* const UalTester::SETTING_TRACE = "Trace";
const char* const UalTester::SETTING_REPORT = "ReportFileName";
const char* const UalTester::SETTING_PRESET_LM = "TestFileName";

UalTester::UalTester(int& argc, char** argv)
{
	getCmdLineParams(argc, argv);
}

UalTester::~UalTester()
{
}

void UalTester::getCmdLineParams(int& argc, char** argv)
{
	CommandLineParser cmdLineParser(argc, argv);

	// main settings
	//
	cmdLineParser.addSingleValueOption("cfgip1", SETTING_CFG_SERVICE_IP1, "IP-addres of first Configuration Service.", "IPv4");
	cmdLineParser.addSingleValueOption("cfgip2", SETTING_CFG_SERVICE_IP2, "IP-addres of second Configuration Service.", "IPv4");
	cmdLineParser.addSingleValueOption("id", SETTING_EQUIPMENT_ID, "EquipmentID.", "EQUIPMENT_ID");
	cmdLineParser.addSingleValueOption("f", SETTING_TEST_FILE_NAME, "Test file name", "TestFileName");

	// optional settings
	//
	cmdLineParser.addSingleValueOption("errignore", SETTING_ERROR_IGNORE, "Stop testing if errors are found.", "No");
	cmdLineParser.addSingleValueOption("test", SETTING_TEST_ID, "Run a specific test", "TEST_ID");
	cmdLineParser.addSingleValueOption("from", SETTING_FROM_TEST_ID, "Starting from the specified test", "TEST_ID");
	cmdLineParser.addSingleValueOption("trace", SETTING_TRACE, "Line test execution report", "No");
	cmdLineParser.addSingleValueOption("report", SETTING_REPORT, "Report file name", "ReportFileName");
	cmdLineParser.addSingleValueOption("lm", SETTING_PRESET_LM, "Run only tests compatible with the specified LM preset", "LM_ID");

	cmdLineParser.parse();

	// main settings
	//
	m_cfgServiceIP1 = cmdLineParser.settingValue(SETTING_CFG_SERVICE_IP1);
	m_cfgServiceIP2 = cmdLineParser.settingValue(SETTING_CFG_SERVICE_IP2);
	m_equipmentID = cmdLineParser.settingValue(SETTING_EQUIPMENT_ID);
	m_testFileName = cmdLineParser.settingValue(SETTING_TEST_FILE_NAME);

	// optional settings
	//
	m_errorIngnoreStr = cmdLineParser.settingValue(SETTING_ERROR_IGNORE);
	m_testID = cmdLineParser.settingValue(SETTING_TEST_ID);
	m_fromTestID = cmdLineParser.settingValue(SETTING_FROM_TEST_ID);
	m_traceStr = cmdLineParser.settingValue(SETTING_TRACE);
	m_reportFileName = cmdLineParser.settingValue(SETTING_REPORT);
	m_presetLM = cmdLineParser.settingValue(SETTING_PRESET_LM);
}

bool UalTester::cmdLineParamsIsValid()
{
	// main settings
	//

	if (m_cfgServiceIP1.isEmpty() == true && m_cfgServiceIP2.isEmpty() == true )
	{
		qDebug() << "IP-addres of Configuration Service is empty";
		return false;
	}

	m_cfgSocketAddress1.setAddressPortStr(m_cfgServiceIP1, PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST);
	m_cfgSocketAddress2.setAddressPortStr(m_cfgServiceIP2, PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST);

	if (m_cfgSocketAddress1.isValidIPv4(m_cfgSocketAddress1.addressStr()) == false)
	{
		qDebug() << "IP-addres of first Configuration Service is not valid";
		return false;
	}

	if (m_cfgSocketAddress2.isValidIPv4(m_cfgSocketAddress2.addressStr()) == false)
	{
		qDebug() << "IP-addres of second Configuration Service is not valid";
		return false;
	}

	if (m_equipmentID.isEmpty() == true)
	{
		qDebug() << "EquipmentID is epmpty";
		return false;
	}

	if (m_testFileName.isEmpty() == true)
	{
		qDebug() << "Test file name is empty";
		return false;
	}

	// optional settings
	//

	if (m_errorIngnoreStr.isEmpty() == false)
	{
		bool paramOk = false;

		if (m_errorIngnoreStr.compare("yes", Qt::CaseInsensitive) == 0)
		{
			paramOk = true;
			m_errorIngnore = true;
		}

		if (m_errorIngnoreStr.compare("no", Qt::CaseInsensitive) == 0)
		{
			paramOk = true;
			m_errorIngnore = false;
		}

		if (paramOk == false)
		{
			qDebug() << "Invalid value of the parameter \"errignore\", specify \"yes\" or \"no\"";
			return false;
		}
	}

	if (m_traceStr.isEmpty() == false)
	{
		bool paramOk = false;

		if (m_traceStr.compare("yes", Qt::CaseInsensitive) == 0)
		{
			paramOk = true;
			m_trace = true;
		}

		if (m_traceStr.compare("no", Qt::CaseInsensitive) == 0)
		{
			paramOk = true;
			m_trace = false;
		}

		if (paramOk == false)
		{
			qDebug() << "Invalid value of the parameter \"trace\", specify \"yes\" or \"no\"";
			return false;
		}
	}

	return true;
}

bool UalTester::start()
{
	if (cmdLineParamsIsValid() == false )
	{
		return false;
	}

	//SoftwareInfo softwareInfo;
	//si.init(E::SoftwareType::Unknown, "", 1, 0);

	// -------------------------------------

	// init config socket thread
	//
	//	m_pConfigSocket = new ConfigSocket(m_cfgSocketAddress1, m_cfgSocketAddress2, softwareInfo);

	//	connect(m_pConfigSocket, &ConfigSocket::socketConnected, this, &MainWindow::configSocketConnected, Qt::QueuedConnection);
	//	connect(m_pConfigSocket, &ConfigSocket::socketDisconnected, this, &MainWindow::configSocketDisconnected, Qt::QueuedConnection);
	//	connect(m_pConfigSocket, &ConfigSocket::configurationLoaded, this, &MainWindow::configSocketConfigurationLoaded);

	//	m_pConfigSocket->start();

	// -------------------------------------

	// init tuning socket thread
	//
	//HostAddressPort tuningSocketAddress; = theOptions.socket().client(SOCKET_TYPE_TUNING).address(SOCKET_SERVER_TYPE_PRIMARY);

	//m_pTuningSocket = new TuningSocket(softwareInfo, tuningSocketAddress);
	//m_pTuningSocketThread = new SimpleThread(m_pTuningSocket);

	// m_pTuningSocketThread->start();

	return true;
}
