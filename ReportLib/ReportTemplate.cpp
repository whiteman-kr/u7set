#include "ReportTemplate.h"

namespace ReportLib
{
	//
	// ObjectTemplate
	//
    ObjectTemplate::ObjectTemplate(Type type) :
        objectType(type)
    {
    }


    ObjectTemplate::~ObjectTemplate()
    {

    }

    bool ObjectTemplate::load(QXmlStreamReader& reader)
	{
        if (reader.attributes().hasAttribute("Tag"))
        {
            tag = reader.attributes().value("Tag").toString();
        }

        if (reader.attributes().hasAttribute("FontName"))
		{
            fontName = reader.attributes().value("FontName").toString();
		}

        if (reader.attributes().hasAttribute("FontSize"))
		{
			bool ok = false;
            fontSize = reader.attributes().value("FontSize").toInt(&ok);

			if (ok == false)
			{
				reader.raiseError(QObject::tr("Failed to load ObjectTemplate element - unknown font size."));
				return false;
			}
		}
		return true;
	}

    QString ObjectTemplate::typeStr() const
    {
        switch(objectType)
        {
        case Type::Text:    return QObject::tr("Text");  break;
        case Type::Table:   return QObject::tr("Table"); break;
        default:
            Q_ASSERT(false);
            return "???";
        }
    }

	//
	// TextTemplate
	//

    TextTemplate::TextTemplate():
        ObjectTemplate(Type::Text)
    {
    }

    bool TextTemplate::load(QXmlStreamReader& reader)
	{
		bool ok = ObjectTemplate::load(reader);
		if (ok == false)
		{
			return false;
		}

		if (reader.attributes().hasAttribute("Alignment"))
		{
			QString alignmentText = reader.attributes().value("Alignment").toString();

			if (alignmentText == "Left")
			{
				alignment = Qt::AlignLeft;
			}
			else
			{
				if (alignmentText == "Right")
				{
					alignment = Qt::AlignRight;
				}
				else
				{
					if (alignmentText == "Center")
					{
						alignment = Qt::AlignHCenter;
					}
					else
					{
						reader.raiseError(QObject::tr("Failed to load TextTemplate element - unknown alignment (%1)").arg(alignmentText));
						return false;
					}
				}
			}
        }

        QXmlStreamReader::TokenType tt = reader.readNext();
        Q_ASSERT(tt == QXmlStreamReader::TokenType::EndElement);

		return true;

	}

	//
	// TableTemplate
	//

    TableTemplate::TableTemplate():
        ObjectTemplate(Type::Table)
    {
    }

    bool TableTemplate::load(QXmlStreamReader& reader)
	{
        bool ok = ObjectTemplate::load(reader);
        if (ok == false)
        {
            return false;
        }

        if (reader.attributes().hasAttribute("Separator"))
        {
            separator = reader.attributes().value("Separator").toString();
        }

        while (reader.readNextStartElement())
        {
            if(reader.name() == QLatin1String("Column"))
            {
                Column c;

                if (reader.attributes().hasAttribute("Caption"))
                {
                    c.caption = reader.attributes().value("Caption").toString();
                }

                if (reader.attributes().hasAttribute("Alignment"))
                {
                    QString alignmentText = reader.attributes().value("Alignment").toString();

                    if (alignmentText == "Left")
                    {
                        c.alignment = Qt::AlignLeft;
                    }
                    else
                    {
                        if (alignmentText == "Right")
                        {
                            c.alignment = Qt::AlignRight;
                        }
                        else
                        {
                            if (alignmentText == "Center")
                            {
                                c.alignment = Qt::AlignHCenter;
                            }
                            else
                            {
                                reader.raiseError(QObject::tr("Failed to load TableTemplate element - unknown alignment (%1)").arg(alignmentText));
                                return false;
                            }
                        }
                    }
                }

                QXmlStreamReader::TokenType tt = reader.readNext();
                Q_ASSERT(tt == QXmlStreamReader::TokenType::EndElement);

                columns.push_back(c);
            }
            else
            {
                reader.raiseError(QObject::tr("Unknown tag: ") + reader.name().toString());
                return !reader.hasError();
            }
        }

        //QXmlStreamReader::TokenType tt = reader.readNext();
        //Q_ASSERT(tt == QXmlStreamReader::TokenType::EndElement);

        return true;
    }

	//
    // SectionTemplate
	//

    bool SectionTemplate::load(QXmlStreamReader& reader)
	{
        if (reader.attributes().hasAttribute("Caption"))
        {
            caption = reader.attributes().value("Caption").toString();
        }

        while (reader.readNextStartElement())
        {
            if(reader.name() == QLatin1String("TextTemplate"))
            {
                std::shared_ptr<TextTemplate> tt = std::make_shared<TextTemplate>();

                if (tt->load(reader) == true)
                {
                    objects.push_back(tt);
                }
                else
                {
                    return !reader.hasError();
                }
            }
            else
            {
                if(reader.name() == QLatin1String("TableTemplate"))
                {
                    std::shared_ptr<TableTemplate> tt = std::make_shared<TableTemplate>();

                    if (tt->load(reader) == true)
                    {
                        objects.push_back(tt);
                    }
                    else
                    {
                        return !reader.hasError();
                    }
                }
                else
                {
                    reader.raiseError(QObject::tr("Unknown tag: ") + reader.name().toString());
                    return !reader.hasError();
                }
            }
        }

		return true;
	}

	bool SectionTemplate::empty() const
	{
		return objects.empty();
	}

	//
	// ReportTemplate
	//

	ReportTemplate::ReportTemplate()
	{

	}

	bool ReportTemplate::load(QXmlStreamReader& reader)
	{
        if (reader.attributes().hasAttribute("Caption"))
        {
            m_caption = reader.attributes().value("Caption").toString();
        }

        while (reader.readNextStartElement())
        {
            if(reader.name() == QLatin1String("SectionTemplate"))
            {
                SectionTemplate st;

                if (st.load(reader) == true)
                {
                    m_sections.push_back(st);
                }
                else
                {
                    return !reader.hasError();
                }
            }
            else
            {
                reader.raiseError(QObject::tr("Unknown tag: ") + reader.name().toString());
                return !reader.hasError();
            }
        }

		return true;
	}

    const QString& ReportTemplate::caption() const
    {
        return m_caption;
    }

    const SectionTemplate& ReportTemplate::header() const
	{
		return m_reportHeader;
	}

	const SectionTemplate& ReportTemplate::footer() const
	{
		return m_reportFooter;
	}

	const SectionTemplate& ReportTemplate::pageHeader() const
	{
		return m_pageHeader;
	}

    const SectionTemplate& ReportTemplate::pageFooter() const
	{
		return m_pageFooter;
	}

    const std::vector<SectionTemplate>& ReportTemplate::sections() const
    {
        return m_sections;
    }

	//
	// ReportTemplateStorage
	//

	void ReportTemplateStorage::clear()
	{
		m_templates.clear();
	}

	bool ReportTemplateStorage::load(const QByteArray& data, QString* errorCode)
	{
		if (errorCode == nullptr)
		{
			Q_ASSERT(errorCode);
			return false;
		}

		clear();

		QXmlStreamReader reader(data);

		if (reader.readNextStartElement() == false)
		{
			reader.raiseError(QObject::tr("Failed to load root element."));
			*errorCode = reader.errorString();
			return !reader.hasError();
		}

		if (reader.name() != QLatin1String("ReportTemplateStorage"))
		{
			reader.raiseError(QObject::tr("The file is not an ReportTemplateStorage file."));
			*errorCode = reader.errorString();
			return !reader.hasError();
		}

		// Read signals
		//
		while (reader.readNextStartElement())
		{
			if(reader.name() == QLatin1String("ReportTemplate"))
			{
				ReportTemplate rt;

				if (rt.load(reader) == true)
				{
					m_templates.push_back(rt);
				}
				else
				{
					*errorCode = reader.errorString();
					return !reader.hasError();
				}
			}
			else
			{
				reader.raiseError(QObject::tr("Unknown tag: ") + reader.name().toString());
				*errorCode = reader.errorString();
				return !reader.hasError();
			}
		}

        if (reader.hasError() == true)
        {
            *errorCode = reader.errorString();
        }
		return !reader.hasError();
	}

	const std::vector<ReportTemplate>& ReportTemplateStorage::templates() const
	{
		return m_templates;
	}


}
