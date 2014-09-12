#include "Stable.h"
#include "FblItemLine.h"

namespace VFrame30
{
	CFblItemLine::CFblItemLine(void)
	{
	}

	CFblItemLine::CFblItemLine(SchemeUnit unit) :
		m_weight(0),
		m_lineColor(qRgb(0x00, 0x00, 0x00))
	{
		setItemUnit(unit);
		m_static = false;
	}

	CFblItemLine::~CFblItemLine(void)
	{
	}

	// Serialization
	//
	bool CFblItemLine::SaveData(Proto::Envelope* message) const
	{
		bool result = CPosConnectionImpl::SaveData(message);
		if (result == false || message->has_videoitem() == false)
		{
			assert(result);
			assert(message->has_videoitem());
			return false;
		}

		result = CFblItem::SaveData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		Proto::FblItemLine* itemMessage = message->mutable_videoitem()->mutable_fblitemline();

		itemMessage->set_weight(m_weight);
		itemMessage->set_linecolor(m_lineColor);
		
		return true;
	}

	bool CFblItemLine::LoadData(const Proto::Envelope& message)
	{
		if (message.has_videoitem() == false)
		{
			assert(message.has_videoitem());
			return false;
		}

		// --
		//
		bool result = CPosConnectionImpl::LoadData(message);
		if (result == false)
		{
			return false;
		}

		result = CFblItem::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.videoitem().has_fblitemline() == false)
		{
			assert(message.videoitem().has_fblitemline());
			return false;
		}
		
		const Proto::FblItemLine& itemMessage = message.videoitem().fblitemline();

		m_weight = itemMessage.weight();
		m_lineColor = itemMessage.linecolor();

		return true;
	}

	// Properties and Data
	//
	bool CFblItemLine::IsFblItem() const
	{
		return true;
	}

	// Weight propertie
	//
	double CFblItemLine::weight() const
	{
		if (itemUnit() == SchemeUnit::Display)
		{
			return CUtils::RoundDisplayPoint(m_weight);
		}
		else
		{
			double pt = CUtils::ConvertPoint(m_weight, SchemeUnit::Inch, CSettings::regionalUnit(), ConvertDirection::Horz);
			return CUtils::RoundPoint(pt, CSettings::regionalUnit());
		}
	}

	void CFblItemLine::setWeight(double weight)
	{
		if (itemUnit() == SchemeUnit::Display)
		{
			m_weight = CUtils::RoundDisplayPoint(weight);
		}
		else
		{
			double pt = CUtils::ConvertPoint(weight, CSettings::regionalUnit(), SchemeUnit::Inch, ConvertDirection::Horz);
			m_weight = pt;
		}
	}

	// LineColor propertie
	//
	QRgb CFblItemLine::lineColor() const
	{
		return m_lineColor;
	}

	void CFblItemLine::setLineColor(QRgb color)
	{
		m_lineColor = color;
	}
}

