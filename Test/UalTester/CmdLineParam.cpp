#include "CmdLineParam.h"

#include <QDebug>
#include <QFileInfo>

#include "../../lib/CommandLineParser.h"
#include "../../lib/SocketIO.h"

// -------------------------------------------------------------------------------------------------------------------
//
// CmdLineParam class implementation
//
// -------------------------------------------------------------------------------------------------------------------

const char* const CmdLineParam::SETTING_CFG_SERVICE_IP1 = "CfgServiceIP1";
const char* const CmdLineParam::SETTING_CFG_SERVICE_IP2 = "CfgServiceIP2";
const char* const CmdLineParam::SETTING_EQUIPMENT_ID = "EquipmentID";
const char* const CmdLineParam::SETTING_TEST_FILE_NAME  = "TestFileName";

const char* const CmdLineParam::SETTING_ERROR_IGNORE = "ErrorIngnore";
const char* const CmdLineParam::SETTING_TEST_ID = "TestID";
const char* const CmdLineParam::SETTING_FROM_TEST_ID = "FromTestID";
const char* const CmdLineParam::SETTING_TRACE = "Trace";
const char* const CmdLineParam::SETTING_REPORT = "ReportFileName";
const char* const CmdLineParam::SETTING_PRESET_LM = "PresetLM";

CmdLineParam::CmdLineParam()
{
}

CmdLineParam::~CmdLineParam()
{
}

void CmdLineParam::getParams(int& argc, char** argv)
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

bool CmdLineParam::paramIsValid()
{
	// main settings
	//
	if (m_cfgServiceIP1.isEmpty() == true && m_cfgServiceIP2.isEmpty() == true )
	{
		qDebug() << "Error: IP-addres of Configuration Service is empty";
		return false;
	}

	m_cfgSocketAddress1.setAddressPortStr(m_cfgServiceIP1, PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST);
	m_cfgSocketAddress2.setAddressPortStr(m_cfgServiceIP2, PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST);

	if (m_cfgSocketAddress1.isValidIPv4(m_cfgSocketAddress1.addressStr()) == false)
	{
		qDebug() << "Error: IP-addres of first Configuration Service is not valid";
		return false;
	}

	if (m_cfgSocketAddress2.isValidIPv4(m_cfgSocketAddress2.addressStr()) == false)
	{
		qDebug() << "Error: IP-addres of second Configuration Service is not valid";
		return false;
	}

	if (m_equipmentID.isEmpty() == true)
	{
		qDebug() << "Error: EquipmentID is epmpty";
		return false;
	}

	if (m_testFileName.isEmpty() == true)
	{
		qDebug() << "Error: Test file name is empty";
		return false;
	}

	if (QFileInfo::exists(m_testFileName) == false)
	{
		qDebug() << "Error: Tets file" << m_testFileName << "is not exist";
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
			qDebug() << "Error: Invalid value of the parameter \"errignore\", specify \"yes\" or \"no\"";
			return false;
		}
	}

	if (m_traceStr.isEmpty() == false)
	{
		bool paramOk = false;

		if (m_traceStr.compare("yes", Qt::CaseInsensitive) == 0)
		{
			paramOk = true;
			m_enableTrace = true;
		}

		if (m_traceStr.compare("no", Qt::CaseInsensitive) == 0)
		{
			paramOk = true;
			m_enableTrace = false;
		}

		if (paramOk == false)
		{
			qDebug() << "Error: Invalid value of the parameter \"trace\", specify \"yes\" or \"no\"";
			return false;
		}
	}

	if (m_reportFileName.isEmpty() == false)
	{
		QFile reportFile(m_reportFileName);
		if (reportFile.open(QIODevice::WriteOnly) == false)
		{
			qDebug() << "Error: Report file" << m_reportFileName << "has not been created";
			return false;
		}

		reportFile.write(QString("Report from: %1\r\n\r\n").arg(currentTimeStr()).toUtf8());
		reportFile.write(QString("Test File: %1\r\n\r\n").arg(m_testFileName).toUtf8());

		reportFile.close();
	}

	return true;
}

int CmdLineParam::getStartTestIndex(const QVector<TestItem>& testList)
{
	if (m_fromTestID.isEmpty() == true)
	{
		return 0;
	}

	int startTestIndex = -1;

	int testCount = testList.count();
	for(int testIndex = 0; testIndex < testCount; testIndex++)
	{
		TestItem test = testList.at(testIndex);
		if (m_fromTestID != test.testID())
		{
			continue;
		}

		startTestIndex = testIndex;
		break;
	}

	return startTestIndex;
}

bool CmdLineParam::enableExecuteTest(const QString& testID)
{
	if (m_testID.isEmpty() == true)
	{
		return true;
	}

	if (m_testID == testID)
	{
		return true;
	}

	return false;
}

bool CmdLineParam::enableExecuteTestForLM(TestItem test)
{
	if (m_presetLM.isEmpty() == true)
	{
		return true;
	}

	bool foundPreset = false;

	int presetCount = test.compatibleList().count();
	for (int i = 0; i < presetCount; i++)
	{
		if (m_presetLM == test.compatibleList().at(i))
		{
			foundPreset = true;
			break;
		}
	}

	if (foundPreset == true)
	{
		return true;
	}

	return false;
}

QString CmdLineParam::currentTimeStr()
{
	QString timeStr;

	QDateTime ct = QDateTime::currentDateTime();

	timeStr.sprintf("%02d-%02d-%04d %02d:%02d:%02d",
					ct.date().day(),
					ct.date().month(),
					ct.date().year(),

					ct.time().hour(),
					ct.time().minute(),
					ct.time().second());

	return timeStr;
}
