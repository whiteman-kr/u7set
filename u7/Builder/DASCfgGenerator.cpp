#include "DASCfgGenerator.h"


namespace Builder
{
	DASCfgGenerator::DASCfgGenerator(DbController* db, Hardware::Software* software, SignalSet* signalSet, Hardware::EquipmentSet* equipment, BuildResultWriter* buildResultWriter) :
		SoftwareCfgGenerator(db, software, signalSet, equipment, buildResultWriter)
	{
	}


	DASCfgGenerator::~DASCfgGenerator()
	{
	}


	bool DASCfgGenerator::generateConfiguration()
	{
		return true;
	}
}
