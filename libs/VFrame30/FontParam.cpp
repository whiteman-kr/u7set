#include <VFrame30/FontParam.h>
#include <VFrame30/Settings.h>

namespace VFrame30
{
	FontParam::FontParam()
	{
	}

	FontParam::FontParam(const QString& name, double drawSize, bool bold, bool italic, bool underline /*= false*/) :
		m_name(name),
		m_size(drawSize),
		m_bold(bold),
		m_italic(italic),
		m_underline(underline)
	{
	}

	bool FontParam::SaveData(Proto::FontParam* message) const
	{
		message->mutable_name_obsolete()->set_text("");				// This field is marked as REQUIRED in proto file, so it must be written in any case
		message->set_name(m_name.toStdString());
		message->set_size(m_size);
		message->set_bold(m_bold);
		message->set_italic(m_italic);
		message->set_underline(m_underline);
		return true;
	}

	FontParam::operator QString() const
	{
		return QString{"FontParam{name: %1, size(in): %2, size(px): %3, drawSize: %4, bold: %5, italic: %6, underline: %7}"}
				.arg(name())
				.arg(size(SchemaUnit::Inch))
				.arg(size(SchemaUnit::Display))
				.arg(drawSize())
				.arg(bold())
				.arg(italic())
				.arg(underline());
	}

	bool FontParam::LoadData(const Proto::FontParam& message)
	{
		if (message.has_name() == true)
		{
			m_name = QString::fromStdString(message.name());
		}
		else
		{
			Proto::Read(message.name_obsolete(), &m_name);
		}

		m_size = message.size();
		m_bold = message.bold();
		m_italic = message.italic();
		m_underline = message.underline();
		return true;
	}

	QFont FontParam::qfont(SchemaUnit unit, double dpiY) const
	{
		QFont f{m_name};

		f.setBold(m_bold);
		f.setItalic(m_italic);
		f.setUnderline(m_underline);

		if (unit == SchemaUnit::Display)
		{
			f.setPixelSize(static_cast<int>(drawSize()));
		}
		else
		{
			int pixelSize = static_cast<int>(drawSize() * dpiY);
			f.setPixelSize(pixelSize > 0 ? pixelSize : 1);
		}

		return f;
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
			return VFrame30::RoundDisplayPoint(m_size);
		}
		else
		{
			double pt = VFrame30::ConvertPoint(m_size, SchemaUnit::Inch, Settings::regionalUnit(), 0);
			return VFrame30::RoundPoint(pt, Settings::regionalUnit());
		}
	}
	void FontParam::setSize(double value, SchemaUnit unit)
	{
		value = std::max(value, 0.0);
		if (unit == SchemaUnit::Display)
		{
			m_size = VFrame30::RoundDisplayPoint(value);
		}
		else
		{
			double pt = VFrame30::ConvertPoint(value, unit, SchemaUnit::Inch, 0);
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

	bool FontParam::underline() const
	{
		return m_underline;
	}

	void FontParam::setUnderline(bool value)
	{
		m_underline = value;
	}
}
