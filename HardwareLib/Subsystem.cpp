#ifndef HARDWARE_LIB_DOMAIN
#error Don't include this file in the project! Link HardwareLib instead.
#endif

#include "Subsystem.h"

namespace Hardware
{
	//
	// Subsystem
	//
	Subsystem::Subsystem(QObject* parent) :
		QObject(parent)
	{
	}

	Subsystem::Subsystem(int index, int key, const QString& subsystemId, const QString& caption, QObject* parent) :
		QObject(parent),
		m_index(index),
		m_key(key),
		m_subsystemId(std::move(subsystemId)),
		m_caption(caption)
	{

	}

	bool Subsystem::save(QXmlStreamWriter& writer)
	{
		writer.writeAttribute("Index", QString::number(index()));
		writer.writeAttribute("Key", QString::number(key()));
		writer.writeAttribute("SubsystemID", subsystemId());
		writer.writeAttribute("Caption", caption());
		return true;
	}


	bool Subsystem::load(QXmlStreamReader& reader)
	{
		if (reader.attributes().hasAttribute(QLatin1String("Index")) == true)
		{
			setIndex(reader.attributes().value(QLatin1String("Index")).toInt());
		}

		if (reader.attributes().hasAttribute(QLatin1String("Key")))
		{
			setKey(reader.attributes().value(QLatin1String("Key")).toInt());
		}

		if (reader.attributes().hasAttribute(QLatin1String("SubsystemID")))
		{
			setSubsystemId(reader.attributes().value(QLatin1String("SubsystemID")).toString());
		}
		else
		{
			// The old file format, before renaming StrID->SubsytemID (RPCT-744)
			//
			if (reader.attributes().hasAttribute(QLatin1String("StrID")))
			{
				setSubsystemId(reader.attributes().value(QLatin1String("StrID")).toString());
			}
		}

		if (reader.attributes().hasAttribute(QLatin1String("Caption")))
		{
			setCaption(reader.attributes().value(QLatin1String("Caption")).toString());
		}

		QXmlStreamReader::TokenType endToken = reader.readNext();
		Q_ASSERT(endToken == QXmlStreamReader::EndElement || endToken == QXmlStreamReader::Invalid);

		return true;
	}


	const QString& Subsystem::subsystemId() const
	{
		return m_subsystemId;
	}

	void Subsystem::setSubsystemId(const QString& value)
	{
		m_subsystemId = value;
	}

	const QString& Subsystem::caption() const
	{
		return m_caption;
	}

	void Subsystem::setCaption(const QString& value)
	{
		m_caption = value;
	}

	int Subsystem::index() const
	{
		return m_index;
	}

	void Subsystem::setIndex(int value)
	{
		m_index = value;
	}

	int Subsystem::key() const
	{
		return m_key;
	}

	void Subsystem::setKey(int value)
	{
		m_key = value;
	}
}
