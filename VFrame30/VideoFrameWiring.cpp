#include "Stable.h"
#include "VideoFrameWiring.h"

namespace VFrame30
{

	CVideoFrameWiring::CVideoFrameWiring(void)
	{
		setUnit(SchemeUnit::Inch);

		setDocWidth(mm2in(420));
		setDocHeight(mm2in(297));

		Layers.push_back(std::make_shared<CVideoLayer>("Frame", false));
		Layers.push_back(std::make_shared<CVideoLayer>("Drawing", true));
		Layers.push_back(std::make_shared<CVideoLayer>("Notes", false));

		return;
	}
	
	CVideoFrameWiring::~CVideoFrameWiring(void)
	{
	}

}
