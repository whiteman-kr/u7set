#include "TuningServiceCfgGenerator.h"

namespace Builder
{
	TuningServiceCfgGenerator::TuningServiceCfgGenerator(DbController* db,
														 Hardware::Software* software,
														 SignalSet* signalSet,
														 Hardware::EquipmentSet* equipment,
														 BuildResultWriter* buildResultWriter) :
		SoftwareCfgGenerator(db, software, signalSet, equipment, buildResultWriter)
	{
	}


	TuningServiceCfgGenerator::~TuningServiceCfgGenerator()
	{
	}


	bool TuningServiceCfgGenerator::generateConfiguration()
	{
		bool result = true;

		//

		return result;
	}

}
