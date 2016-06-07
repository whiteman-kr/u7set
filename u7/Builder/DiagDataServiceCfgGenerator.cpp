#include "DiagDataServiceCfgGenerator.h"
#include "../lib/ServiceSettings.h"


namespace Builder
{
	DiagDataServiceCfgGenerator::DiagDataServiceCfgGenerator(DbController* db, Hardware::Software* software, SignalSet* signalSet, Hardware::EquipmentSet* equipment, BuildResultWriter* buildResultWriter) :
		SoftwareCfgGenerator(db, software, signalSet, equipment, buildResultWriter)
	{
	}


	DiagDataServiceCfgGenerator::~DiagDataServiceCfgGenerator()
	{
	}


	bool DiagDataServiceCfgGenerator::generateConfiguration()
	{
		bool result = true;

/*		result &= writeSettings();
		result &= writeAppDataSourcesXml();
		result &= writeAppSignalsXml();
		result &= writeEquipmentXml();*/

		return result;
	}
}
