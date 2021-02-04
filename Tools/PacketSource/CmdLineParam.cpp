#include "CmdLineParam.h"

#include <QCoreApplication>
#include <QDir>
#include <iostream>

#include "../../lib/SocketIO.h"

// -------------------------------------------------------------------------------------------------------------------
//
// CmdLineParam class implementation
//
// -------------------------------------------------------------------------------------------------------------------

// main keys
//
const char* const CmdLineParam::SETTING_CFG_EQUIPMENT_ID = "EquipmentID of CfgService";
const char* const CmdLineParam::SETTING_CFG_SERVICE_IP = "CfgServiceIP";
const char* const CmdLineParam::SETTING_ADS_EQUIPMENT_ID = "EquipmentID of AppDataService";
const char* const CmdLineParam::SETTING_UAL_TESTER_IP = "UalTesterIP";

// optional keys
//
const char* const CmdLineParam::SETTING_SOURCES_FOR_RUN  = "EquipmentID";
const char* const CmdLineParam::SETTING_OPTION_FILENAME = "OptionFileName";

const char* const CmdLineParam::REQUIRED_OPTIONS =	"Options: \"-cfgid\", \"-cfgip\", \"-adsid\" and \"-utip\" - must be filled!\n" \
													"For example: -cfgid=EQUIPMENT_ID_CFG -cfgip=127.0.0.1 -adsid=EQUIPMENT_ID_ADS -utip=127.0.0.1\n";

CmdLineParam::CmdLineParam()
{
}

CmdLineParam::CmdLineParam(int& argc, char** argv)
{
	getParams(argc, argv);
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
	m_cmdLineParser.addSingleValueOption("cfgid", SETTING_CFG_EQUIPMENT_ID, "EquipmentID of software \"Configuration Service\".", "EQUIPMENT_ID_CFG");
	m_cmdLineParser.addSingleValueOption("cfgip", SETTING_CFG_SERVICE_IP, "IP-address of software \"Configuration Service\".", "IPv4");
	m_cmdLineParser.addSingleValueOption("adsid", SETTING_ADS_EQUIPMENT_ID, "EquipmentID of software \"Application Data Service\".", "EQUIPMENT_ID_ADS");
	m_cmdLineParser.addSingleValueOption("utip", SETTING_UAL_TESTER_IP, "IP-addres for listening commands from UalTester.", "IPv4");

		// optional keys
		//
	m_cmdLineParser.addSingleValueOption("rsid", SETTING_SOURCES_FOR_RUN, "EquipmentID of source for run.", "SOURCE_EQUIPMENT_ID");
	m_cmdLineParser.addSingleValueOption("b", SETTING_OPTION_FILENAME, "Options of command line in the file name.", "OptionsFileName.txt");
}

bool CmdLineParam::parse()
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
			std::cout << "Option \"-b\" is not compatible with other options" << std::endl;
			return false;
		}

		m_optionFileName = m_cmdLineParser.settingValue(SETTING_OPTION_FILENAME);
		if (m_optionFileName.isEmpty() == true)
		{
			std::cout << "Option \"-b\" is empty" << std::endl;
			return false;
		}
		else
		{
			if (QFileInfo::exists(m_optionFileName) == false)
			{
				std::cout << "Error: Options file " << m_optionFileName.toLocal8Bit().constData() << " is not exist" << std::endl;
				return false;
			}

			QFile optionsFile(m_optionFileName);
			if (optionsFile.open(QIODevice::ReadOnly) == false)
			{
				std::cout << "Error: Options file " << m_optionFileName.toLocal8Bit().constData() << " has not been read" << std::endl;
				return false;
			}

			QString optionsStr = optionsFile.readAll();

			optionsFile.close();

			if (optionsStr.isEmpty() == true)
			{
				std::cout << "Error: Options file " << m_optionFileName.toLocal8Bit().constData() << " is empty" << std::endl;
				return false;
			}

			updateStartSourcesParam(optionsStr);

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
	m_cfgEquipmentID = m_cmdLineParser.settingValue(SETTING_CFG_EQUIPMENT_ID);
	m_cfgIP = m_cmdLineParser.settingValue(SETTING_CFG_SERVICE_IP);
	m_adsEquipmentID = m_cmdLineParser.settingValue(SETTING_ADS_EQUIPMENT_ID);
	m_utIP = m_cmdLineParser.settingValue(SETTING_UAL_TESTER_IP);

		// optional keys
		//
	QString startSources = m_cmdLineParser.settingValue(SETTING_SOURCES_FOR_RUN);

	// parsing params of keys from cmd line
	//
		// parse main settings
		//
	if (m_cfgEquipmentID.isEmpty() == true)
	{
		std::cout << "Error: Configuration Service EquipmentID is empty" << std::endl;
		std::cout << REQUIRED_OPTIONS;
		return false;
	}

	if (m_cfgIP.isEmpty() == true)
	{
		std::cout << "Error: IP-addres of Configuration Service is empty" << std::endl;
		std::cout << REQUIRED_OPTIONS;
		return false;
	}

	m_cfgSrvIP.setAddressPortStr(m_cfgIP, PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST);

	if (m_cfgSrvIP.isValidIPv4(m_cfgSrvIP.addressStr()) == false)
	{
		std::cout << "Error: IP-addres of Configuration Service is not valid" << std::endl;
		std::cout << REQUIRED_OPTIONS;
		return false;
	}

	if (m_adsEquipmentID.isEmpty() == true)
	{
		std::cout << "Error: Application Data Service EquipmentID is empty" << std::endl;
		std::cout << REQUIRED_OPTIONS;
		return false;
	}

	if (m_utIP.isEmpty() == true )
	{
		std::cout << "Error: IP-addres for listening commands from UalTester is empty" << std::endl;
		std::cout << REQUIRED_OPTIONS;
		return false;
	}

	m_ualTesterIP.setAddressPortStr(m_utIP, PORT_TUNING_SERVICE_CLIENT_REQUEST);

	if (m_ualTesterIP.isValidIPv4(m_ualTesterIP.addressStr()) == false)
	{
		std::cout << "Error: IP-addres for listening commands from UalTester is not valid" << std::endl;
		std::cout << REQUIRED_OPTIONS;
		return false;
	}

		// parse optional settings
		//
	if (startSources.isEmpty() == false)
	{
		m_sourcesForRunList = startSources.split(',');
		for(const QString& sourceEquipmentID : m_sourcesForRunList)
		{
			if (sourceEquipmentID.isEmpty() == true)
			{
				continue;
			}

			// maybe check EquipmentID of source
		}
	}

	return true;
}

void CmdLineParam::updateStartSourcesParam(QString& cmdLine)
{
	const QString key = "-rsid=";
	QString params = cmdLine;

	int posParam = params.indexOf(key);
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

	if (params[key.length()] == ',')
	{
		params.remove(key.length(), 1);
	}

	if (params.endsWith(',') == true)
	{
		params[params.length() - 1] = ' ';
	}

	// if we have only word -rsid= without params
	//
	if (params.length() < key.length() + 1)
	{
		return;
	}

	cmdLine.replace(orignalParams, params);
}


