#include "FblItemLine.h"
#include "PropertyNames.h"

namespace VFrame30
{
	FblItemLine::FblItemLine(void)
	{
		ADD_PROPERTY_GET_SET_CAT(double, PropertyNames::lineWeight, PropertyNames::appearanceCategory, true, FblItemLine::weight, FblItemLine::setWeight);
		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::lineColor, PropertyNames::appearanceCategory, true, FblItemLine::lineColor, FblItemLine::setLineColor);
	}

	FblItemLine::FblItemLine(SchemaUnit unit) :
		m_weight(0),
		m_lineColor(qRgb(0x00, 0x00, 0xC0))
	{
		ADD_PROPERTY_GET_SET_CAT(double, PropertyNames::lineWeight, PropertyNames::appearanceCategory, true, FblItemLine::weight, FblItemLine::setWeight);
		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::lineColor, PropertyNames::appearanceCategory, true, FblItemLine::lineColor, FblItemLine::setLineColor);

		setItemUnit(unit);
		m_static = false;
	}

	FblItemLine::~FblItemLine(void)
	{
	}

	// Serialization
	//
	bool FblItemLine::SaveData(Proto::Envelope* message) const
	{
		bool result = PosConnectionImpl::SaveData(message);
		if (result == false || message->has_schemaitem() == false)
		{
			assert(result);
			assert(message->has_schemaitem());
			return false;
		}

		result = FblItem::SaveData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		Proto::FblItemLine* itemMessage = message->mutable_schemaitem()->mutable_fblitemline();

		itemMessage->set_weight(m_weight);
		itemMessage->set_linecolor(m_lineColor.rgba());
		
		return true;
	}

	bool FblItemLine::LoadData(const Proto::Envelope& message)
	{
		if (message.has_schemaitem() == false)
		{
			assert(message.has_schemaitem());
			return false;
		}

		// --
		//
		bool result = PosConnectionImpl::LoadData(message);
		if (result == false)
		{
			return false;
		}

		result = FblItem::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.schemaitem().has_fblitemline() == false)
		{
			assert(message.schemaitem().has_fblitemline());
			return false;
		}
		
		const Proto::FblItemLine& itemMessage = message.schemaitem().fblitemline();

		m_weight = itemMessage.weight();
		m_lineColor = itemMessage.linecolor();

		return true;
	}

	// Properties and Data
	//

	// Weight propertie
	//
	double FblItemLine::weight() const
	{
		if (itemUnit() == SchemaUnit::Display)
		{
			return CUtils::RoundDisplayPoint(m_weight);
		}
		else
		{
			double pt = CUtils::ConvertPoint(m_weight, SchemaUnit::Inch, Settings::regionalUnit(), 0);
			return CUtils::RoundPoint(pt, Settings::regionalUnit());
		}
	}

	void FblItemLine::setWeight(double weight)
	{
		if (itemUnit() == SchemaUnit::Display)
		{
			m_weight = CUtils::RoundDisplayPoint(weight);
		}
		else
		{
			double pt = CUtils::ConvertPoint(weight, Settings::regionalUnit(), SchemaUnit::Inch, 0);
			m_weight = pt;
		}
	}

	// LineColor propertie
	//
	QColor FblItemLine::lineColor() const
	{
		return m_lineColor;
	}

	void FblItemLine::setLineColor(QColor color)
	{
		m_lineColor = color;
	}
}

