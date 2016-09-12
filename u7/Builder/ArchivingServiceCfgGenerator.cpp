#include "ArchivingServiceCfgGenerator.h"
#include "../lib/ServiceSettings.h"

namespace Builder
{

	ArchivingServiceCfgGenerator::ArchivingServiceCfgGenerator(DbController* db,
															   Hardware::Software* software,
															   SignalSet* signalSet,
															   Hardware::EquipmentSet* equipment,
															   BuildResultWriter* buildResultWriter) :
		SoftwareCfgGenerator(db, software, signalSet, equipment, buildResultWriter)
	{
	}


	ArchivingServiceCfgGenerator::~ArchivingServiceCfgGenerator()
	{
	}


	bool ArchivingServiceCfgGenerator::generateConfiguration()
	{
		bool result = true;

		result = writeSettings();

		return result;
	}


	bool ArchivingServiceCfgGenerator::writeSettings()
	{
		ArchivingServiceSettings settings;

		bool result = true;

		result &= settings.readFromDevice(m_software, m_log);

		XmlWriteHelper xml(m_cfgXml->xmlWriter());

		result &= settings.writeToXml(xml);

		return result;
	}

}
