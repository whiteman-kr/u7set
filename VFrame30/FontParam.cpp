#include "Stable.h"
#include "FontParam.h"
#include "../lib/ProtoSerialization.h"

namespace VFrame30
{
	FontParam::FontParam()
	{
	}

	FontParam::FontParam(const QString& name, double drawSize, bool bold, bool italic) :
		m_name(name),
		m_size(drawSize),
		m_bold(bold),
		m_italic(italic)
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
		Proto::Read(message.name(), &m_name);
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

	double FontParam::size(SchemaUnit unit) const
	{
		if (unit == SchemaUnit::Display)
		{
			return CUtils::RoundDisplayPoint(m_size);
		}
		else
		{
			double pt = CUtils::ConvertPoint(m_size, SchemaUnit::Inch, Settings::regionalUnit(), 0);
			return CUtils::RoundPoint(pt, Settings::regionalUnit());
		}
	}
	void FontParam::setSize(double value, SchemaUnit unit)
	{
		value = std::max(value, 0.0);
		if (unit == SchemaUnit::Display)
		{
			m_size = CUtils::RoundDisplayPoint(value);
		}
		else
		{
			double pt = CUtils::ConvertPoint(value, unit, SchemaUnit::Inch, 0);
			m_size = pt;
		}
	}

	double FontParam::drawSize() const
	{
		return m_size;
	}

	void FontParam::setDrawSize(double value)
	{
		m_size = value;
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
