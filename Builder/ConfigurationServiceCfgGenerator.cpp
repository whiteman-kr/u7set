#include "ConfigurationServiceCfgGenerator.h"
#include "../lib/WUtils.h"

namespace Builder
{

	ConfigurationServiceCfgGenerator::ConfigurationServiceCfgGenerator(DbController* db,
															   Hardware::Software* software,
															   SignalSet* signalSet,
															   Hardware::EquipmentSet* equipment,
															   BuildResultWriter* buildResultWriter) :
		SoftwareCfgGenerator(db, software, signalSet, equipment, buildResultWriter)
	{
	}

	ConfigurationServiceCfgGenerator::~ConfigurationServiceCfgGenerator()
	{
	}

	bool ConfigurationServiceCfgGenerator::generateConfiguration()
	{
		bool result = false;

		do
		{
			if (writeSettings() == false) break;
			if (writeBatFile() == false) break;
			if (writeShFile() == false) break;

			result = true;
		}
		while(false);

		return result;
	}

	bool ConfigurationServiceCfgGenerator::writeSettings()
	{
		CfgServiceSettings settings;

		bool result = true;

		result &= DeviceHelper::getIpPortProperty(m_software, CfgServiceSettings::PROP_CLIENT_REQUEST_IP, CfgServiceSettings::PROP_CLIENT_REQUEST_PORT, &settings.clientRequestIP, false, "", 0, m_log);
		result &= DeviceHelper::getIPv4Property(m_software, CfgServiceSettings::PROP_CLIENT_REQUEST_NETMASK, &settings.clientRequestNetmask, false, "", m_log);

		result &= buildClientsList(&settings);

		if (result == false)
		{
			return false;
		}

		XmlWriteHelper xml(m_cfgXml->xmlWriter());

		result = settings.writeToXml(xml);

		return result;
	}

	bool ConfigurationServiceCfgGenerator::writeBatFile()
	{
		TEST_PTR_RETURN_FALSE(m_software);

		QString content = getBuildInfoCommentsForBat();

		content += "CfgSrv.exe";
		content += " -e";
		content += " -id=" + m_software->equipmentIdTemplate();

		// build path
		//
		QString appDataPath = QDir::fromNativeSeparators(m_buildResultWriter->outputPath());

		if (appDataPath.endsWith("/") == true)
		{
			appDataPath.truncate(appDataPath.length() - 1);
		}

		QString buildDir = QString("%1-%2/build")
				.arg(m_dbController->currentProject().projectName())
				.arg(m_buildResultWriter->buildInfo().typeStr());

		content += " -b=" + appDataPath + "/" + buildDir;

		HostAddressPort clientRequestIP;

		if (DeviceHelper::getIpPortProperty(m_software,
											CfgServiceSettings::PROP_CLIENT_REQUEST_IP,
											CfgServiceSettings::PROP_CLIENT_REQUEST_PORT,
											&clientRequestIP,
											false,
											"", PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST,
											m_log) == false)
		{
			return false;
		}

		content += " -ip=" + clientRequestIP.addressPortStr() + "\n";

		BuildFile* buildFile = m_buildResultWriter->addFile(DIR_RUN_SERVICE_SCRIPTS, m_software->equipmentIdTemplate().toLower() + ".bat", content);

		TEST_PTR_RETURN_FALSE(buildFile);

		return true;
	}

	bool ConfigurationServiceCfgGenerator::writeShFile()
	{
		TEST_PTR_RETURN_FALSE(m_software);

		QString content = getBuildInfoCommentsForSh();

		content += "./CfgSrv";
		content += " -e";
		content += " -id=" + m_software->equipmentIdTemplate();

		// build path
		//
		QString appDataPath = QDir::fromNativeSeparators(m_buildResultWriter->outputPath());

		if (appDataPath.endsWith("/") == true)
		{
			appDataPath.truncate(appDataPath.length() - 1);
		}

		QString buildDir = QString("%1-%2/build")
				.arg(m_dbController->currentProject().projectName())
				.arg(m_buildResultWriter->buildInfo().typeStr());

		content += " -b=" + appDataPath + "/" + buildDir;

		HostAddressPort clientRequestIP;

		if (DeviceHelper::getIpPortProperty(m_software,
											CfgServiceSettings::PROP_CLIENT_REQUEST_IP,
											CfgServiceSettings::PROP_CLIENT_REQUEST_PORT,
											&clientRequestIP,
											false,
											"", PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST,
											m_log) == false)
		{
			return false;
		}

		content += " -ip=" + clientRequestIP.addressPortStr() + "\n";

		BuildFile* buildFile = m_buildResultWriter->addFile(DIR_RUN_SERVICE_SCRIPTS, m_software->equipmentIdTemplate().toLower() + ".sh", content);

		TEST_PTR_RETURN_FALSE(buildFile);

		return true;
	}

	bool ConfigurationServiceCfgGenerator::buildClientsList(CfgServiceSettings* settings)
	{
		TEST_PTR_LOG_RETURN_FALSE(settings, m_log);

		const QString PROP_CFG_SERVICE_ID1(ServiceSettings::PROP_CFG_SERVICE_ID1);
		const QString PROP_CFG_SERVICE_ID2(ServiceSettings::PROP_CFG_SERVICE_ID2);

		bool result = true;

		settings->clients.clear();

		for(Hardware::Software* software : m_softwareList)
		{
			if (software == nullptr)
			{
				assert(false);
				continue;
			}

			if (software->equipmentIdTemplate() == equipmentID())
			{
				continue;			// exclude yourself
			}

			QString ID1;

			if (DeviceHelper::isPropertyExists(software, PROP_CFG_SERVICE_ID1) == true)
			{
				result &= DeviceHelper::getStrProperty(software, PROP_CFG_SERVICE_ID1, &ID1, m_log);
			}

			QString ID2;

			if (DeviceHelper::isPropertyExists(software, PROP_CFG_SERVICE_ID2) == true)
			{
				result &= DeviceHelper::getStrProperty(software, PROP_CFG_SERVICE_ID2, &ID2, m_log);
			}

			if (ID1 == m_software->equipmentIdTemplate() || ID2 == m_software->equipmentIdTemplate())
			{
				settings->clients.append(QPair<QString, E::SoftwareType>(software->equipmentIdTemplate(), software->type()));
			}
		}

		return result;
	}

}
