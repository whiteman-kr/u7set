#include "ConfigurationServiceCfgGenerator.h"
#include "../lib/ServiceSettings.h"
#include "../lib/WUtils.h"
#include "../Settings.h"

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
			if (checkProperties() == false) break;
			if (writeBatFile() == false) break;
			if (writeShFile() == false) break;

			result = true;
		}
		while(false);

		return result;
	}

	bool ConfigurationServiceCfgGenerator::checkProperties()
	{
		QString clientRequestIP;
		QString clientRequestNetmask;
		int clientRequestPort = 0;

		bool result = true;

		result &= DeviceHelper::getIPv4Property(m_software, CfgServiceSettings::PROP_CLIENT_REQUEST_IP, &clientRequestIP, false, m_log);
		result &= DeviceHelper::getIPv4Property(m_software, CfgServiceSettings::PROP_CLIENT_REQUEST_NETMASK, &clientRequestNetmask, false, m_log);
		result &= DeviceHelper::getIntProperty(m_software, CfgServiceSettings::PROP_CLIENT_REQUEST_PORT, &clientRequestPort, m_log);

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
		QString appDataPath = QDir::fromNativeSeparators(theSettings.buildOutputPath());

		if (appDataPath.endsWith("/") == true)
		{
			appDataPath.truncate(appDataPath.length() - 1);
		}

		QString buildDir = QString("%1-%2/build")
				.arg(m_dbController->currentProject().projectName())
				.arg(m_buildResultWriter->buildInfo().typeStr());

		content += " -b=" + appDataPath + "/" + buildDir;

		content += " -ip=" + m_software->propertyByCaption(CfgServiceSettings::PROP_CLIENT_REQUEST_IP)->value().toString() + "\n";

		BuildFile* buildFile = m_buildResultWriter->addFile(BuildResultWriter::RUN_SERVICE_SCRIPTS, m_software->equipmentIdTemplate().toLower() + ".bat", content);

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
		QString appDataPath = QDir::fromNativeSeparators(theSettings.buildOutputPath());

		if (appDataPath.endsWith("/") == true)
		{
			appDataPath.truncate(appDataPath.length() - 1);
		}

		QString buildDir = QString("%1-%2/build")
				.arg(m_dbController->currentProject().projectName())
				.arg(m_buildResultWriter->buildInfo().typeStr());

		content += " -b=" + appDataPath + "/" + buildDir;

		content += " -ip=" + m_software->propertyByCaption("ClientRequestIP")->value().toString() + "\n";

		BuildFile* buildFile = m_buildResultWriter->addFile(BuildResultWriter::RUN_SERVICE_SCRIPTS, m_software->equipmentIdTemplate().toLower() + ".sh", content);

		TEST_PTR_RETURN_FALSE(buildFile);

		return true;
	}

}
