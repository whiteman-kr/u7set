#include "Stable.h"
#include "FontParam.h"
#include "../include/ProtoSerialization.h"

namespace VFrame30
{
	FontParam::FontParam()
	{
	}

	bool FontParam::SaveData(Proto::FontParam* message) const
	{
		Proto::Write(message->mutable_name(), m_name);
		message->set_size(m_size);
		message->set_bold(m_bold);
		message->set_italic(m_italic);
		return true;
	}

	bool FontParam::LoadData(const Proto::FontParam& message)
	{
		m_name = Proto::Read(message.name());
		m_size = message.size();
		m_bold = message.bold();
		m_italic = message.italic();
		return true;
	}

	const QString& FontParam::name() const
	{
		return m_name;
	}

	void FontParam::setName(const QString& value)
	{
		m_name = value;
	}

	double FontParam::size(SchemeUnit unit) const
	{
		if (unit == SchemeUnit::Display)
		{
			return CUtils::RoundDisplayPoint(m_size);
		}
		else
		{
			double pt = CUtils::ConvertPoint(m_size, SchemeUnit::Inch, CSettings::regionalUnit(), ConvertDirection::Horz);
			return CUtils::RoundPoint(pt, CSettings::regionalUnit());
		}
	}
	void FontParam::setSize(double value, SchemeUnit unit)
	{
		value = std::max(value, 0.0);
		if (unit == SchemeUnit::Display)
		{
			m_size = CUtils::RoundDisplayPoint(value);
		}
		else
		{
			double pt = CUtils::ConvertPoint(value, CSettings::regionalUnit(), SchemeUnit::Inch, ConvertDirection::Horz);
			m_size = pt;
		}
	}

	double FontParam::drawSize() const
	{
		return m_size;
	}

	bool FontParam::bold() const
	{
		return m_bold;
	}

	void FontParam::setBold(bool value)
	{
		m_bold = value;
	}

	bool FontParam::italic() const
	{
		return m_italic;
	}

	void FontParam::setItalic(bool value)
	{
		m_italic = value;
	}

}
