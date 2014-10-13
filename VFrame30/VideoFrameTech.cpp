#include "Stable.h"
#include "VideoFrameTech.h"

namespace VFrame30
{

	CVideoFrameTech::CVideoFrameTech(void)
	{
		setUnit(SchemeUnit::Display);

		setDocWidth(1000);
		setDocHeight(750);

		Layers.push_back(std::make_shared<SchemeLayer>("Drawing", true));
		Layers.push_back(std::make_shared<SchemeLayer>("Notes", false));

		return;
	}
	
	CVideoFrameTech::~CVideoFrameTech(void)
	{
	}
}
