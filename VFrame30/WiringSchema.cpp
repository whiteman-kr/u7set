#include "Stable.h"
#include "WiringSchema.h"

namespace VFrame30
{

	WiringSchema::WiringSchema(void)
	{
		setUnit(SchemaUnit::Inch);

		setGridSize(Settings::defaultGridSize(unit()));
		setPinGridStep(4);

		setDocWidth(mm2in(420));
		setDocHeight(mm2in(297));

		Layers.push_back(std::make_shared<SchemeLayer>("Frame", false));
		Layers.push_back(std::make_shared<SchemeLayer>("Drawing", true));
		Layers.push_back(std::make_shared<SchemeLayer>("Notes", false));

		return;
	}
	
	WiringSchema::~WiringSchema(void)
	{
	}

}
