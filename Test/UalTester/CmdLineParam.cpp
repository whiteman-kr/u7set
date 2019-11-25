#include "CmdLineParam.h"

#include <QDebug>
#include <QFileInfo>

#include "../../lib/SocketIO.h"

// -------------------------------------------------------------------------------------------------------------------
//
// CmdLineParam class implementation
//
// -------------------------------------------------------------------------------------------------------------------

// main keys
//
const char* const CmdLineParam::SETTING_CFG_SERVICE_IP1 = "CfgServiceIP1";
const char* const CmdLineParam::SETTING_CFG_SERVICE_IP2 = "CfgServiceIP2";
const char* const CmdLineParam::SETTING_EQUIPMENT_ID = "EquipmentID";
const char* const CmdLineParam::SETTING_TEST_FILE_NAME  = "TestFileName";

// optional keys
//
const char* const CmdLineParam::SETTING_ERROR_IGNORE = "ErrorIngnore";
const char* const CmdLineParam::SETTING_TEST_ID = "TestID";
const char* const CmdLineParam::SETTING_FROM_TEST_ID = "FromTestID";
const char* const CmdLineParam::SETTING_TRACE = "Trace";
const char* const CmdLineParam::SETTING_REPORT_FILENAME = "ReportFileName";
const char* const CmdLineParam::SETTING_PRESET_LM = "PresetLM";
const char* const CmdLineParam::SETTING_OPTION_FILENAME = "OptionFileName";

const char* const CmdLineParam::REQUIRED_OPTIONS =	"Options \"-cfgip1\", \"-cfgip2\", \"-id\" and \"-f\" must be filled\n" \
													"For example: -cfgip1=127.0.0.1 -cfgip2=192.168.0.1 -id=EQUIPMENT_ID_UALTESTER -f=test.txt\n";


CmdLineParam::CmdLineParam()
{
}

CmdLineParam::~CmdLineParam()
{
}

void CmdLineParam::getParams(int& argc, char** argv)
{
	m_cmdLineParser.setCmdLineArgs(argc, argv);

	// desciption keys from cmd line
	//
	m_cmdLineParser.addSimpleOption("h", "Print this help.");
		// main keys
		//
	m_cmdLineParser.addSingleValueOption("cfgip1", SETTING_CFG_SERVICE_IP1, "IP-addres of first Configuration Service.", "IPv4");
	m_cmdLineParser.addSingleValueOption("cfgip2", SETTING_CFG_SERVICE_IP2, "IP-addres of second Configuration Service.", "IPv4");
	m_cmdLineParser.addSingleValueOption("id", SETTING_EQUIPMENT_ID, "EquipmentID of software \"TestClient\".", "EQUIPMENT_ID_UALTESTER");
	m_cmdLineParser.addSingleValueOption("f", SETTING_TEST_FILE_NAME, "Test file name.", "TestFileName.txt");

		// optional keys
		//
	m_cmdLineParser.addSingleValueOption("errignore", SETTING_ERROR_IGNORE, "Stop testing if errors was found.", "No");
	m_cmdLineParser.addSingleValueOption("test", SETTING_TEST_ID, "Run a specific test.", "TEST_ID");
	m_cmdLineParser.addSingleValueOption("from", SETTING_FROM_TEST_ID, "Run from the specific test.", "TEST_ID");
	m_cmdLineParser.addSingleValueOption("trace", SETTING_TRACE, "Print full line-by-line report of test.", "No");
	m_cmdLineParser.addSingleValueOption("report", SETTING_REPORT_FILENAME, "Report file name of test results.", "ReportFileName.txt");
	m_cmdLineParser.addSingleValueOption("lm", SETTING_PRESET_LM, "Run only tests compatible with the specified LM preset.", "LM_ID");
	m_cmdLineParser.addSingleValueOption("b", SETTING_OPTION_FILENAME, "Options of command line in the file name.", "OptionsFileName.txt");
}

bool CmdLineParam::paramIsValid()
{
	m_cmdLineParser.parse();

	// print Help and exit if "-h" is set
	//
	if (m_cmdLineParser.argCount() == 1 || m_cmdLineParser.optionIsSet("h") == true)
	{
		std::cout << m_cmdLineParser.helpText().toLocal8Bit().constData();
		std::cout << REQUIRED_OPTIONS;
		return false;
	}

	// if "-b" is set than take param form file
	//
	if (m_cmdLineParser.optionIsSet("b") == true)
	{
		if (m_cmdLineParser.argCount() > 2)
		{
			std::cout << "Option \"-b\" is not compatible with other options\n";
			return false;
		}

		m_optionFileName = m_cmdLineParser.settingValue(SETTING_OPTION_FILENAME);
		if (m_optionFileName.isEmpty() == true)
		{
			std::cout << "Option \"-b\" is empty";
			return false;
		}
		else
		{
			if (QFileInfo::exists(m_optionFileName) == false)
			{
				std::cout << "Error: Options file " << m_optionFileName.toLocal8Bit().constData() << " is not exist\n";
				return false;
			}

			QFile optionsFile(m_optionFileName);
			if (optionsFile.open(QIODevice::ReadOnly) == false)
			{
				std::cout << "Error: Options file " << m_optionFileName.toLocal8Bit().constData() << " has not been read\n";
				return false;
			}

			QString optionsStr = optionsFile.readAll();
			optionsFile.close();

			if (optionsStr.isEmpty() == true)
			{
				std::cout << "Error: Options file " << m_optionFileName.toLocal8Bit().constData() << " is empty\n";
				return false;
			}

			updateTestFilesParam(optionsStr);

			optionsStr.replace('\r', ' ');
			optionsStr.replace('\n', ' ');
			optionsStr.replace('\t', ',');

			QStringList args = optionsStr.split(' ');

			args.insert(0, QCoreApplication::applicationFilePath());

			QVector<char*> argv;
			for (int i = 0; i < args.count(); i++)
			{
				unsigned int strLen = static_cast<unsigned int>(args[i].length()) + 1;
				char* pArg = new char[strLen];
				memcpy(pArg, args[i].toLatin1().data(), strLen);
				argv.append(pArg);
			}

			m_cmdLineParser.setCmdLineArgs(args.count(), argv.data());
			m_cmdLineParser.parse();
		}
	}

	// get params of keys from cmd line
	//
		// main keys
		//
	m_cfgServiceIP1 = m_cmdLineParser.settingValue(SETTING_CFG_SERVICE_IP1);
	m_cfgServiceIP2 = m_cmdLineParser.settingValue(SETTING_CFG_SERVICE_IP2);
	m_equipmentID = m_cmdLineParser.settingValue(SETTING_EQUIPMENT_ID);
	QString testFileName = m_cmdLineParser.settingValue(SETTING_TEST_FILE_NAME);

		// optional keys
		//
	m_errorIngnoreStr = m_cmdLineParser.settingValue(SETTING_ERROR_IGNORE);
	m_testID = m_cmdLineParser.settingValue(SETTING_TEST_ID);
	m_fromTestID = m_cmdLineParser.settingValue(SETTING_FROM_TEST_ID);
	m_traceStr = m_cmdLineParser.settingValue(SETTING_TRACE);
	m_reportFileName = m_cmdLineParser.settingValue(SETTING_REPORT_FILENAME);
	m_presetLM = m_cmdLineParser.settingValue(SETTING_PRESET_LM);

	// main settings
	//
	if (m_cfgServiceIP1.isEmpty() == true && m_cfgServiceIP2.isEmpty() == true )
	{
		std::cout << "Error: IP-addres of Configuration Service is empty\n";
		std::cout << REQUIRED_OPTIONS;
		return false;
	}

	m_cfgSocketAddress1.setAddressPortStr(m_cfgServiceIP1, PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST);
	m_cfgSocketAddress2.setAddressPortStr(m_cfgServiceIP2, PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST);

	if (m_cfgSocketAddress1.isValidIPv4(m_cfgSocketAddress1.addressStr()) == false)
	{
		std::cout << "Error: IP-addres of first Configuration Service is not valid\n";
		std::cout << REQUIRED_OPTIONS;
		return false;
	}

	if (m_cfgSocketAddress2.isValidIPv4(m_cfgSocketAddress2.addressStr()) == false)
	{
		std::cout << "Error: IP-addres of second Configuration Service is not valid\n";
		std::cout << REQUIRED_OPTIONS;
		return false;
	}

	if (m_equipmentID.isEmpty() == true)
	{
		std::cout << "Error: EquipmentID is epmpty\n";
		std::cout << REQUIRED_OPTIONS;
		return false;
	}

	if (testFileName.isEmpty() == true)
	{
		std::cout << "Error: Test file name is empty\n";
		std::cout << REQUIRED_OPTIONS;
		return false;
	}

	m_testFileNameList = testFileName.split(',');
	for(const QString& testFileName : m_testFileNameList)
	{
		if (testFileName.isEmpty() == true)
		{
			continue;
		}

		if (QFileInfo::exists(testFileName) == false)
		{
			std::cout << "Error: Tets file " << testFileName.toLocal8Bit().constData() << " is not exist\n";
			return false;
		}
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
			std::cout << "Error: Invalid value of the parameter \"errignore\", specify \"yes\" or \"no\"\n";
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
			std::cout << "Error: Invalid value of the parameter \"trace\", specify \"yes\" or \"no\"\n";
			return false;
		}
	}

	if (m_reportFileName.isEmpty() == false)
	{
		QFile reportFile(m_reportFileName);
		if (reportFile.open(QIODevice::WriteOnly) == false)
		{
			std::cout << "Error: Report file " << m_reportFileName.toLocal8Bit().constData() << " has not been created\n";
			return false;
		}

		reportFile.write(QString("Report from: %1\r\n\r\n").arg(currentTimeStr()).toUtf8());

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

void CmdLineParam::updateTestFilesParam(QString& cmdLine)
{
	QString params = cmdLine;

	int posParam = params.indexOf("-f=");
	if (posParam == -1)
	{
		return;
	}

	params.remove(0, posParam);

	int posNextParam = params.indexOf("-", 1);
	if (posNextParam == -1)
	{
		posNextParam = cmdLine.length();
	}

	params.remove(posNextParam, params.length());

	QString orignalParams = params;

	params.replace('\r', ',');
	params.replace('\n', ',');
	params.replace('\t', ',');
	params.replace(' ', ',');
	params.replace(QRegExp("[,]{2,}"),",");

	if (params[3] == ',')
	{
		params.remove(3, 1);
	}

	if (params.endsWith(',') == true)
	{
		params[params.length() - 1] = ' ';
	}

	if (params.length() < 4)
	{
		return;
	}

	cmdLine.replace(orignalParams, params);
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
