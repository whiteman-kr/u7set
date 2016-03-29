#include "Stable.h"
#include "MonitorSchema.h"

namespace VFrame30
{

	MonitorSchema::MonitorSchema(void)
	{
		setUnit(SchemaUnit::Display);

		setGridSize(Settings::defaultGridSize(unit()));
		setPinGridStep(20);

		setDocWidth(1000);
		setDocHeight(750);

		Layers.push_back(std::make_shared<SchemeLayer>("Drawing", true));
		Layers.push_back(std::make_shared<SchemeLayer>("Notes", false));

		return;
	}
	
	MonitorSchema::~MonitorSchema(void)
	{
	}
}
