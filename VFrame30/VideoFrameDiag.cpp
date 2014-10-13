#include "Stable.h"
#include "VideoFrameDiag.h"

namespace VFrame30
{

	CVideoFrameDiag::CVideoFrameDiag(void)
	{
		setUnit(SchemeUnit::Display);

		setDocWidth(1000);
		setDocHeight(750);

		Layers.push_back(std::make_shared<SchemeLayer>("Drawing", true));
		Layers.push_back(std::make_shared<SchemeLayer>("Notes", false));

		return;
	}

	CVideoFrameDiag::~CVideoFrameDiag(void)
	{
	}

}
