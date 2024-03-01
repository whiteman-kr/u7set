#include "../include/Behavior/TuningClientBehavior.h"

namespace Behavior
{
	//
	// TuningClientBehavior
	//
	TuningClientBehavior::TuningClientBehavior()
	{
		m_tagToColor["defaultMismatchBackColor"] = QColor(Qt::yellow);
		m_tagToColor["defaultMismatchTextColor"] = QColor(Qt::black);
		m_tagToColor["unappliedBackColor"] = QColor(Qt::gray);
		m_tagToColor["unappliedTextColor"] = QColor(Qt::black);
	}

	void TuningClientBehavior::saveToXml(QXmlStreamWriter& writer)
	{
		writer.writeStartElement("TagToColor");

		QHashIterator<QString, QColor> i(m_tagToColor);
		while (i.hasNext())
		{
			i.next();

			writer.writeStartElement("Item");
			writer.writeAttribute("tag", i.key());
			writer.writeAttribute("color", i.value().name());
			writer.writeEndElement();
		}

		writer.writeEndElement();
	}

	bool TuningClientBehavior::loadFromXml(QXmlStreamReader& reader)
	{
		m_tagToColor.clear();

		//
		while (reader.readNextStartElement())
		{
			// TagToColor tag
			//
			if (reader.name() == QLatin1String("TagToColor"))
			{
				reader.readNext();
				continue;
			}

			// Item tag
			//
			if (reader.name() == QLatin1String("Item"))
			{
				QString tag;
				if (reader.attributes().hasAttribute("tag"))
				{
					tag = reader.attributes().value("tag").toString();
				}

				QColor color;
				if (reader.attributes().hasAttribute("color"))
				{
					color = QColor(reader.attributes().value("color").toString());
				}

				if (tag.isEmpty() == false && color.isValid() == true)
				{
					m_tagToColor[tag] = color;
				}

				reader.readNext();
				continue;
			}

			// Unknown tag
			//
			Q_ASSERT(false);
			reader.raiseError(QObject::tr("Unknown tag: ") + reader.name().toString());
			return !reader.hasError();
		}

		reader.readNext();
		return !reader.hasError();
	}
} // namespace Behavior