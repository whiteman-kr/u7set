#include "Stable.h"
#include "WiringScheme.h"

namespace VFrame30
{

	WiringScheme::WiringScheme(void)
	{
		setUnit(SchemeUnit::Inch);

		setDocWidth(mm2in(420));
		setDocHeight(mm2in(297));

		Layers.push_back(std::make_shared<SchemeLayer>("Frame", false));
		Layers.push_back(std::make_shared<SchemeLayer>("Drawing", true));
		Layers.push_back(std::make_shared<SchemeLayer>("Notes", false));

		return;
	}
	
	WiringScheme::~WiringScheme(void)
	{
	}

}
