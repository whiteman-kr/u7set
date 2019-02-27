#include "TestClientCfgGenerator.h"
#include "../lib/ServiceSettings.h"

namespace Builder
{
	TestClientCfgGenerator::TestClientCfgGenerator(Context* context, Hardware::Software* software) :
		SoftwareCfgGenerator(context, software)
	{
	}

	bool TestClientCfgGenerator::generateConfiguration()
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

	bool TestClientCfgGenerator::writeSettings()
	{
		TEST_PTR_RETURN_FALSE(m_log);
		TEST_PTR_LOG_RETURN_FALSE(m_equipment, m_log);
		TEST_PTR_LOG_RETURN_FALSE(m_software, m_log);
		TEST_PTR_LOG_RETURN_FALSE(m_cfgXml, m_log);

		bool result = true;

		TestClientSettings settings;

		result = settings.readFromDevice(m_equipment, m_software, m_log);

		if (result == true)
		{
			XmlWriteHelper xml(m_cfgXml->xmlWriter());

			result = settings.writeToXml(xml);
		}

		bool res = m_cfgXml->addLinkToFile(DIR_COMMON, FILE_APP_SIGNALS_ASGS);

		if (res == false)
		{
			// Can't link build file %1 into /%2/configuration.xml.
			//
			m_log->errCMN0018(QString("%1\\%2").arg(DIR_COMMON).arg(FILE_APP_SIGNALS_ASGS), equipmentID());
			result = false;
		}

		return result;
	}

	bool TestClientCfgGenerator::writeBatFile()
	{
		TEST_PTR_RETURN_FALSE(m_software);

		QString content = getBuildInfoCommentsForBat();

		content += "TestAppDataSrv.exe";

		QString parameters;
		if (getServiceParameters(parameters) == false)
		{
			return false;
		}
		content += parameters.mid(3);	// Skip -e parameter

		BuildFile* buildFile = m_buildResultWriter->addFile(Builder::DIR_RUN_SERVICE_SCRIPTS, m_software->equipmentIdTemplate().toLower() + ".bat", content);

		TEST_PTR_RETURN_FALSE(buildFile);

		return true;
	}

	bool TestClientCfgGenerator::writeShFile()
	{
		TEST_PTR_RETURN_FALSE(m_software);

		QString content = getBuildInfoCommentsForSh();

		content += "./TestAppDataSrv";

		QString parameters;

		if (getServiceParameters(parameters) == false)
		{
			return false;
		}

		content += parameters.mid(3);	// Skip -e parameter

		BuildFile* buildFile = m_buildResultWriter->addFile(Builder::DIR_RUN_SERVICE_SCRIPTS, m_software->equipmentIdTemplate().toLower() + ".sh", content);

		TEST_PTR_RETURN_FALSE(buildFile);

		return true;
	}
}
