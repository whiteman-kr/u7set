#include "Stable.h"
#include "LogicScheme.h"

namespace VFrame30
{
	LogicScheme::LogicScheme(void)
	{
		setUnit(SchemeUnit::Inch);

		setDocWidth(mm2in(420));
		setDocHeight(mm2in(297));

		Layers.push_back(std::make_shared<SchemeLayer>("Frame", false));
		Layers.push_back(std::make_shared<SchemeLayer>("Drawing", true));
		Layers.push_back(std::make_shared<SchemeLayer>("Notes", false));

		return;
	}

	LogicScheme ::~LogicScheme (void)
	{
	}

	void LogicScheme::Draw(CDrawParam* pDrawParam, const QRectF& clipRect) const
	{
		BuildFblConnectionMap();

		Scheme::Draw(pDrawParam, clipRect);
		return;
    }

}
