#include "Stable.h"
#include "Settings.h"
#include "../include/CUtils.h"

namespace VFrame30
{
	//SchemeUnit CSettings::m_regionalUnit = SchemeUnit::Inch;
	SchemeUnit Settings::m_regionalUnit = SchemeUnit::Millimeter;

	// The min grid size for schemes for Fbl pins positioning
	//
	const double Settings::m_defaultGridSizeIn = 0.03125;	// 1/32in
	const double Settings::m_defaultGridSizeMm = mm2in(1);	// 1mm
	const double Settings::m_defaultGridSizePx = 5;

	SchemeUnit Settings::regionalUnit(void)
	{
		return m_regionalUnit;
	}

	void Settings::setRegionalUnit(SchemeUnit value)
	{
		m_regionalUnit = value;
	}

	double Settings::defaultGridSize(SchemeUnit unit)
	{
		if (unit == SchemeUnit::Display)
		{
			return m_defaultGridSizePx;
		}

		assert(regionalUnit() == SchemeUnit::Inch || regionalUnit() == SchemeUnit::Millimeter);

		switch (regionalUnit())
		{
		case SchemeUnit::Inch:
			return m_defaultGridSizeIn;
		case SchemeUnit::Millimeter:
			return m_defaultGridSizeMm;
		default:
			return m_defaultGridSizeIn;
		}
	}
}
