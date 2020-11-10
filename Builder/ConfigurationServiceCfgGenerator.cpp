#include "ConfigurationServiceCfgGenerator.h"
#include "../lib/WUtils.h"

namespace Builder
{

	ConfigurationServiceCfgGenerator::ConfigurationServiceCfgGenerator(Context* context, Hardware::Software* software) :
		SoftwareCfgGenerator(context, software)
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

	bool ConfigurationServiceCfgGenerator::getSettingsXml(QXmlStreamWriter& xmlWriter)
	{
		if (m_settings.isInitialized() == false)
		{
			bool result = m_settings.readFromDevice(m_software, m_log);

			result &= buildClientsList(&m_settings);

			RETURN_IF_FALSE(result);

			m_settings.setInitialized();
		}

		XmlWriteHelper xml(xmlWriter);

		return m_settings.writeToXml(xml);
	}

	bool ConfigurationServiceCfgGenerator::writeSettings()
	{
		return getSettingsXml(m_cfgXml->xmlWriter());
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

		QString buildDir = QString("%1/build")
		        .arg(m_dbController->currentProject().projectName());

		content += " -b=" + appDataPath + "/" + buildDir;

		HostAddressPort clientRequestIP;

		if (DeviceHelper::getIpPortProperty(m_software,
		                                    EquipmentPropNames::CLIENT_REQUEST_IP,
		                                    EquipmentPropNames::CLIENT_REQUEST_PORT,
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

		QString buildDir = QString("%1/build")
		        .arg(m_dbController->currentProject().projectName());

		content += " -b=" + appDataPath + "/" + buildDir;

		HostAddressPort clientRequestIP;

		if (DeviceHelper::getIpPortProperty(m_software,
		                                    EquipmentPropNames::CLIENT_REQUEST_IP,
		                                    EquipmentPropNames::CLIENT_REQUEST_PORT,
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

		const QString PROP_CFG_SERVICE_ID1(EquipmentPropNames::CFG_SERVICE_ID1);
		const QString PROP_CFG_SERVICE_ID2(EquipmentPropNames::CFG_SERVICE_ID2);

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
