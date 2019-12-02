#include "Indicator.h"

namespace VFrame30
{
	//
	// IndicatorComponent base class
	//
	Indicator::Indicator(SchemaUnit itemUnit) :
		m_itemUnit(itemUnit)
	{
		Q_ASSERT(m_itemUnit == SchemaUnit::Display || m_itemUnit == SchemaUnit::Inch);
	}

	void Indicator::setUnits(SchemaUnit itemUnit)
	{
		Q_ASSERT(m_itemUnit == SchemaUnit::Display || m_itemUnit == SchemaUnit::Inch);
		m_itemUnit = itemUnit;
	}


}
