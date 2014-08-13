#include "Stable.h"
#include "Settings.h"
#include "../include/VFrameUtils.h"

namespace VFrame30
{
	SchemeUnit CSettings::m_regionalUnit = SchemeUnit::Inch;

	// ћинимальный грид, дл€ схем, используетс€ дл€ позиционировани€ (выравнивание€) пинов в Fbl едементах
	//
	const double CSettings::m_minFblGridSizeIn = 0.02;
	const double CSettings::m_minFblGridSizeMm = mm2in(0.5);
	const double CSettings::m_minFblGridSizePx = 5;	

	SchemeUnit CSettings::regionalUnit(void)
	{
		return m_regionalUnit;
	}

	void CSettings::setRegionalUnit(SchemeUnit value)
	{
		m_regionalUnit = value;
	}

	double CSettings::minFblGridSize(SchemeUnit unit)
	{
		switch (unit)
		{
		case SchemeUnit::Inch:
			return m_minFblGridSizeIn;
		case SchemeUnit::Millimeter:
			return m_minFblGridSizeMm;
		case SchemeUnit::Display:
			return m_minFblGridSizePx;
		default:
			assert(false);
		}

		return m_minFblGridSizeIn;
	}
}
