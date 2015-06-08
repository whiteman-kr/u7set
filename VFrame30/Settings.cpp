#include "Stable.h"
#include "Settings.h"
#include "../include/CUtils.h"

namespace VFrame30
{
	//SchemeUnit CSettings::m_regionalUnit = SchemeUnit::Inch;
	SchemeUnit Settings::m_regionalUnit = SchemeUnit::Millimeter;

	// The min grid size for schemes for Fbl pins positioning
	//
	const double Settings::m_minFblGridSizeIn = 0.02;
	const double Settings::m_minFblGridSizeMm = mm2in(0.5);
	const double Settings::m_minFblGridSizePx = 5;	

	SchemeUnit Settings::regionalUnit(void)
	{
		return m_regionalUnit;
	}

	void Settings::setRegionalUnit(SchemeUnit value)
	{
		m_regionalUnit = value;
	}

	double Settings::minFblGridSize(SchemeUnit unit)
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
