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

// -------------------------------------------------------------------------------------------------------------------

UalTester::UalTester(int& argc, char** argv)
{
	getCmdLineParams(argc, argv);
}

// -------------------------------------------------------------------------------------------------------------------

UalTester::~UalTester()
{
}

// -------------------------------------------------------------------------------------------------------------------

void UalTester::getCmdLineParams(int& argc, char** argv)
{
	CommandLineParser cmdLineParser(argc, argv);

	cmdLineParser.addSingleValueOption("cfgip1", SETTING_CFG_SERVICE_IP1, "IP-addres of first Configuration Service.", "IPv4");
	cmdLineParser.addSingleValueOption("cfgip2", SETTING_CFG_SERVICE_IP2, "IP-addres of second Configuration Service.", "IPv4");
	cmdLineParser.addSingleValueOption("id", SETTING_EQUIPMENT_ID, "EquipmentID.", "EQUIPMENT_ID");
	cmdLineParser.addSingleValueOption("-f", SETTING_TEST_FILE_NAME, "Test file name", "TestFileName");

	cmdLineParser.parse();

	m_cfgServiceIP1Str = cmdLineParser.settingValue(SETTING_CFG_SERVICE_IP1);
	m_cfgServiceIP2Str = cmdLineParser.settingValue(SETTING_CFG_SERVICE_IP2);
	m_equipmentID = cmdLineParser.settingValue(SETTING_EQUIPMENT_ID);
	m_testFileName = cmdLineParser.settingValue(SETTING_TEST_FILE_NAME);
}

// -------------------------------------------------------------------------------------------------------------------

bool UalTester::cmdLineParamsIsValid()
{
	if (m_cfgServiceIP1Str.isEmpty() == true && m_cfgServiceIP2Str.isEmpty() == true )
	{
		qDebug() << "IP-addres of Configuration Service is empty";
		return false;
	}

	m_cfgSocketAddress1.setAddressPortStr(m_cfgServiceIP1Str, PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST);
	m_cfgSocketAddress2.setAddressPortStr(m_cfgServiceIP2Str, PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST);

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
		qDebug() << "Test file name is epmpty";
		return false;
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

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

// -------------------------------------------------------------------------------------------------------------------
