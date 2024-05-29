#include <VFrame30/Indicator.h>

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

	SchemaUnit Indicator::itemUnit() const
	{
		return m_itemUnit;
	}

	void Indicator::setUnits(SchemaUnit itemUnit)
	{
		Q_ASSERT(m_itemUnit == SchemaUnit::Display || m_itemUnit == SchemaUnit::Inch);
		m_itemUnit = itemUnit;
	}


}
