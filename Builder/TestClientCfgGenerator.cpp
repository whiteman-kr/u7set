#include "TestClientCfgGenerator.h"

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
			if (linkAppSignalsFile() == false) break;
			if (writeBatFile() == false) break;
			if (writeShFile() == false) break;

			result = true;
		}
		while(false);

		return result;
	}

	bool TestClientCfgGenerator::getSettingsXml(QXmlStreamWriter& xmlWriter)
	{
		XmlWriteHelper xml(xmlWriter);

		return m_settings.writeToXml(xml);
	}

	bool TestClientCfgGenerator::writeSettings()
	{
		bool result = m_settings.readFromDevice(m_equipment, m_software, m_log);

		RETURN_IF_FALSE(result);

		return getSettingsXml(m_cfgXml->xmlWriter());
	}

	bool TestClientCfgGenerator::linkAppSignalsFile()
	{
		bool res = m_cfgXml->addLinkToFile(Directory::COMMON, File::APP_SIGNALS_ASGS);

		if (res == false)
		{
			// Can't link build file %1 into /%2/configuration.xml.
			//
			m_log->errCMN0018(QString("%1\\%2").arg(Directory::COMMON).arg(File::APP_SIGNALS_ASGS), equipmentID());
			return false;
		}

		return true;
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

		BuildFile* buildFile = m_buildResultWriter->addFile(Directory::RUN_SERVICE_SCRIPTS, m_software->equipmentIdTemplate().toLower() + ".bat", content);

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

		BuildFile* buildFile = m_buildResultWriter->addFile(Directory::RUN_SERVICE_SCRIPTS, m_software->equipmentIdTemplate().toLower() + ".sh", content);

		TEST_PTR_RETURN_FALSE(buildFile);

		return true;
	}
}
