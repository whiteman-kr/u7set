#include "Stable.h"
#include "VideoFrameLogic.h"

namespace VFrame30
{
	CVideoFrameLogic::CVideoFrameLogic(void)
	{
		setUnit(SchemeUnit::Inch);

		setDocWidth(mm2in(420));
		setDocHeight(mm2in(297));

		Layers.push_back(std::make_shared<SchemeLayer>("Frame", false));
		Layers.push_back(std::make_shared<SchemeLayer>("Drawing", true));
		Layers.push_back(std::make_shared<SchemeLayer>("Notes", false));

		return;
	}

	CVideoFrameLogic::~CVideoFrameLogic(void)
	{
	}

	void CVideoFrameLogic::Draw(CDrawParam* pDrawParam, const QRectF& clipRect) const
	{
		BuildFblConnectionMap();

		Scheme::Draw(pDrawParam, clipRect);
		return;
    }

}
