#include "Stable.h"
#include "Settings.h"
#include "../lib/CUtils.h"

namespace VFrame30
{
	//SchemaUnit CSettings::m_regionalUnit = SchemaUnit::Inch;
	SchemaUnit Settings::m_regionalUnit = SchemaUnit::Millimeter;

	// The min grid size for schemas for Fbl pins positioning
	//
	const double Settings::m_defaultGridSizeIn = 0.03125;	// 1/32in
	const double Settings::m_defaultGridSizeMm = mm2in(1);	// 1mm
	const double Settings::m_defaultGridSizePx = 5;

	SchemaUnit Settings::regionalUnit(void)
	{
		return m_regionalUnit;
	}

	void Settings::setRegionalUnit(SchemaUnit value)
	{
		m_regionalUnit = value;
	}

	double Settings::defaultGridSize(SchemaUnit unit)
	{
		if (unit == SchemaUnit::Display)
		{
			return m_defaultGridSizePx;
		}

		assert(regionalUnit() == SchemaUnit::Inch || regionalUnit() == SchemaUnit::Millimeter);

		switch (regionalUnit())
		{
		case SchemaUnit::Inch:
			return m_defaultGridSizeIn;
		case SchemaUnit::Millimeter:
			return m_defaultGridSizeMm;
		default:
			return m_defaultGridSizeIn;
		}
	}
}
