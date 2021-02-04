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

	bool ConfigurationServiceCfgGenerator::createSettingsProfile(const QString& profile)
	{
		CfgServiceSettingsGetter settingsGetter;

		if (settingsGetter.readFromDevice(m_context, m_software) == false)
		{
			return false;
		}

		return m_settingsSet.addProfile<CfgServiceSettingsGetter>(profile, settingsGetter);
	}

	bool ConfigurationServiceCfgGenerator::generateConfigurationStep1()
	{
		bool result = false;

		do
		{
			if (writeBatFile() == false) break;
			if (writeShFile() == false) break;

			result = true;
		}
		while(false);

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

		BuildFile* buildFile = m_buildResultWriter->addFile(Directory::RUN_SERVICE_SCRIPTS, m_software->equipmentIdTemplate().toLower() + ".bat", content);

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

		BuildFile* buildFile = m_buildResultWriter->addFile(Directory::RUN_SERVICE_SCRIPTS, m_software->equipmentIdTemplate().toLower() + ".sh", content);

		TEST_PTR_RETURN_FALSE(buildFile);

		return true;
	}

}
