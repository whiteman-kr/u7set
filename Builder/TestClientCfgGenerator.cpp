#include "TestClientCfgGenerator.h"
#include "../lib/ServiceSettings.h".h"

namespace Builder
{
	TestClientCfgGenerator::TestClientCfgGenerator(DbController* db,
												   Hardware::Software* software,
												   SignalSet* signalSet,
												   Hardware::EquipmentSet* equipment,
												   BuildResultWriter* buildResultWriter) :
		SoftwareCfgGenerator(db, software, signalSet, equipment, buildResultWriter)
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

		return result;
	}

	bool TestClientCfgGenerator::writeBatFile()
	{
		bool result = true;
		return result;
	}

	bool TestClientCfgGenerator::writeShFile()
	{
		bool result = true;
		return result;
	}
}
