#include "ReportTemplate.h"

namespace ReportLib
{
	//
	// ObjectTemplate
	//
    ObjectTemplate::ObjectTemplate(ReportObject::Type type) :
        m_type(type)
    {
    }


    ObjectTemplate::~ObjectTemplate()
    {

    }

    bool ObjectTemplate::load(QXmlStreamReader& reader)
	{
        if (reader.attributes().hasAttribute("Tag"))
        {
            m_tag = reader.attributes().value("Tag").toString();
        }

		if (reader.attributes().hasAttribute("Text"))
		{
			m_text = reader.attributes().value("Text").toString();
		}

		return true;
	}

    ReportObject::Type ObjectTemplate::type() const
    {
        return m_type;
    }

    QString ObjectTemplate::typeStr() const
    {
        switch(m_type)
        {
        case ReportObject::Type::Text:    return QObject::tr("Text");  break;
        case ReportObject::Type::Table:   return QObject::tr("Table"); break;
        case ReportObject::Type::Schema:  return QObject::tr("Schema"); break;
        default:
            Q_ASSERT(false);
            return "???";
        }
    }

	const QString& ObjectTemplate::text() const
	{
		return m_text;
	}

	const QString& ObjectTemplate::tag() const
    {
        return m_tag;
    }

	//
	// TextTemplate
	//

    TextTemplate::TextTemplate():
        ObjectTemplate(ReportObject::Type::Text)
    {
    }

    bool TextTemplate::load(QXmlStreamReader& reader)
	{
		bool ok = ObjectTemplate::load(reader);
		if (ok == false)
		{
			return false;
		}

        QString fontName{"Arial"};
        int fontSize{12};
		bool fontBold{false};
        Qt::Alignment alignment{Qt::AlignLeft};

        if (reader.attributes().hasAttribute("FontName"))
        {
            fontName = reader.attributes().value("FontName").toString();
        }

        if (reader.attributes().hasAttribute("FontSize"))
        {
            fontSize = reader.attributes().value("FontSize").toInt(&ok);

            if (ok == false)
            {
				reader.raiseError(QObject::tr("Failed to load Text element - unknown font size."));
                return false;
            }
        }

        if (reader.attributes().hasAttribute("FontBold"))
        {
			fontBold = reader.attributes().value("FontBold").toString().compare("True", Qt::CaseInsensitive) == 0;
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

		m_format = {ReportFont{fontName, fontSize, fontBold ? QFont::Bold : QFont::Normal}, alignment};

		return true;

	}

    const TextFormat& TextTemplate::format() const
    {
        return m_format;
    }

	QString TextTemplate::propToText() const
	{
		QString alStr;
		switch (m_format.alignment())
		{
		case Qt::AlignLeft:
			alStr = "Left";
			break;
		case Qt::AlignRight:
			alStr = "Right";
			break;
		case Qt::AlignHCenter:
			alStr = "Center";
			break;
		default:
			alStr = "???";
		}

		QString format = QObject::tr("[%1, %2, %3, %4]")
						  .arg(m_format.font().family)
						  .arg(m_format.font().pointSize)
						  .arg(m_format.font().weight == QFont::Bold ? "Bold" : "Normal")
						  .arg(alStr);

		if (tag().isEmpty() == true)
		{
			return text() + format;
		}

		return tag() + format;
	}

	//
	// TableTemplate
	//

    TableTemplate::TableTemplate():
        ObjectTemplate(ReportObject::Type::Table)
    {
    }

    bool TableTemplate::load(QXmlStreamReader& reader)
	{
        bool ok = ObjectTemplate::load(reader);
        if (ok == false)
        {
            return false;
        }

        QString fontName{"Arial"};
        int fontSize{12};
		bool fontBold{false};
        std::vector<TableFormat::ColumnFormat> columns;

        if (reader.attributes().hasAttribute("FontName"))
        {
            fontName = reader.attributes().value("FontName").toString();
        }

        if (reader.attributes().hasAttribute("FontSize"))
        {
            fontSize = reader.attributes().value("FontSize").toInt(&ok);

            if (ok == false)
            {
				reader.raiseError(QObject::tr("Failed to load Table element - unknown font size."));
                return false;
            }
        }

        if (reader.attributes().hasAttribute("FontBold"))
        {
			fontBold = reader.attributes().value("FontBold").toString().compare("True", Qt::CaseInsensitive) == 0;
        }

		if (reader.attributes().hasAttribute("Separator"))
        {
            m_separator = reader.attributes().value("Separator").toString();
        }

        while (reader.readNextStartElement())
        {
            if(reader.name() == QLatin1String("Column"))
            {
                TableFormat::ColumnFormat c;

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

                if (reader.attributes().hasAttribute("Width"))
                {
                    c.width = reader.attributes().value("Width").toInt(&ok);
                    if (ok == false)
                    {
                        reader.raiseError(QObject::tr("Failed to load TableTemplate element - unknown column width."));
                        return false;
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

		m_format = {ReportFont{fontName, fontSize, fontBold ? QFont::Bold : QFont::Normal}, columns};

        //QXmlStreamReader::TokenType tt = reader.readNext();
        //Q_ASSERT(tt == QXmlStreamReader::TokenType::EndElement);

        return true;
    }

    const TableFormat& TableTemplate::format() const
    {
        return m_format;
    }

    const QString& TableTemplate::separator() const
    {
        return m_separator;
    }

	QString TableTemplate::propToText() const
	{
		return QString("Tag: '%1', Sep: '%2', Cols: %3").arg(tag()).arg(separator()).arg(m_format.columnsFormat().size());
	}

	//
	// MarginTemplate
	//

	bool MarginTemplate::load(QXmlStreamReader& reader)
	{
		QString text;
		int pageFrom = -1;
		int pageTo = -1;

		QString fontName{"Arial"};
		int fontSize{12};
		Qt::Alignment alignment{Qt::AlignLeft};

		//

		bool ok = false;

		if (reader.attributes().hasAttribute("FontName"))
		{
			fontName = reader.attributes().value("FontName").toString();
		}

		if (reader.attributes().hasAttribute("FontSize"))
		{
			fontSize = reader.attributes().value("FontSize").toInt(&ok);

			if (ok == false)
			{
				reader.raiseError(QObject::tr("Failed to load MarginItem element - unknown font size."));
				return false;
			}
		}

		if (reader.attributes().hasAttribute("HorzPosition"))
		{
			QString horzPositionText = reader.attributes().value("HorzPosition").toString();

			if (horzPositionText == "Left")
			{
				alignment = Qt::AlignLeft;
			}
			else
			{
				if (horzPositionText == "Right")
				{
					alignment = Qt::AlignRight;
				}
				else
				{
					if (horzPositionText == "Center")
					{
						alignment = Qt::AlignHCenter;
					}
					else
					{
						reader.raiseError(QObject::tr("Failed to load MarginItem element - unknown HorzPosition (%1)").arg(horzPositionText));
						return false;
					}
				}
			}
		}
		else
		{
			alignment = Qt::AlignLeft;
		}

		if (reader.attributes().hasAttribute("VertPosition"))
		{
			QString vertPositionText = reader.attributes().value("VertPosition").toString();

			if (vertPositionText == "Top")
			{
				alignment |= Qt::AlignTop;
			}
			else
			{
				if (vertPositionText == "Bottom")
				{
					alignment |= Qt::AlignBottom;
				}
				else
				{
					reader.raiseError(QObject::tr("Failed to load MarginItem element - unknown VertPosition (%1)").arg(vertPositionText));
					return false;
				}
			}
		}
		else
		{
			alignment |= Qt::AlignTop;
		}

		if (reader.attributes().hasAttribute("PageFrom"))
		{
			pageFrom = reader.attributes().value("PageFrom").toInt(&ok);
			if (ok == false)
			{
				reader.raiseError(QObject::tr("Failed to load MarginItem element - unknown PageFrom."));
				return false;
			}
		}

		if (reader.attributes().hasAttribute("PageTo"))
		{
			pageTo = reader.attributes().value("PageTo").toInt(&ok);
			if (ok == false)
			{
				reader.raiseError(QObject::tr("Failed to load MarginItem element - unknown PageTo."));
				return false;
			}
		}

		if (reader.attributes().hasAttribute("Text"))
		{
			text = reader.attributes().value("Text").toString();
		}

		QXmlStreamReader::TokenType tt = reader.readNext();
		Q_ASSERT(tt == QXmlStreamReader::TokenType::EndElement);

		m_marginItem = {text, pageFrom, pageTo, {ReportFont{fontName, fontSize, QFont::Normal}, alignment}};

		return true;
	}

	const ReportMarginItem& MarginTemplate::marginItem() const
	{
		return m_marginItem;
	}

	//
    // SectionTemplate
	//

    bool SectionTemplate::load(QXmlStreamReader& reader)
	{
        if (reader.attributes().hasAttribute("Caption"))
        {
            m_caption = reader.attributes().value("Caption").toString();
        }

		if (reader.attributes().hasAttribute("Tag"))
		{
			m_tag = reader.attributes().value("Tag").toString();
		}

		// Load page layout

		QPageSize pageSize(QPageSize::A4);
		QPageLayout::Orientation orientation(QPageLayout::Portrait);
		QMarginsF margins(30, 20, 15, 20);

		if (reader.attributes().hasAttribute("PageSize"))
		{
			QString strPageSize = reader.attributes().value("PageSize").toString();

			if (strPageSize == "A4")
			{
				pageSize = QPageSize::A4;
			}
			else
			{
				if (strPageSize == "A3")
				{
					pageSize = QPageSize::A3;
				}
				else
				{
					reader.raiseError(QObject::tr("Failed to load SectionTemplate element - unknown PageSize (A4 or A3 expected)."));
					return false;
				}
			}

		}
		if (reader.attributes().hasAttribute("Orientation"))
		{
			if (reader.attributes().value("Orientation").toString().compare("Landscape", Qt::CaseInsensitive) == 0)
			{
				orientation = QPageLayout::Landscape;
			}
		}

		if (reader.attributes().hasAttribute("Margins"))
		{
			QStringList marginsList = reader.attributes().value("Margins").toString().split(',', Qt::SkipEmptyParts);
			if (marginsList.size() != 4)
			{
				reader.raiseError(QObject::tr("Failed to load SectionTemplate element - Margins should have 4 numbers (e.g. \"left,top,right,bottom\")."));
				return false;
			}
			bool ok[4] = {false};
			int left = marginsList[0].toInt(&ok[0]);
			int top = marginsList[1].toInt(&ok[1]);
			int right = marginsList[2].toInt(&ok[2]);
			int bottom = marginsList[3].toInt(&ok[3]);
			if ((ok[0] && ok[1] && ok[2] && ok[3]) != true)
			{
				reader.raiseError(QObject::tr("Failed to load SectionTemplate element - incorrect Margins format (e.g. \"30,20,15,20\")."));
				return false;
			}

			margins = QMarginsF(left, top, right, bottom);
		}

		m_pageLayout = QPageLayout(pageSize, orientation, margins, QPageLayout::Unit::Millimeter);

		//

		while (reader.readNextStartElement())
        {
			if(reader.name() == QLatin1String("Text"))
            {
                std::shared_ptr<TextTemplate> tt = std::make_shared<TextTemplate>();

                if (tt->load(reader) == true)
                {
                    m_objects.push_back(tt);
                }
                else
                {
                    return !reader.hasError();
                }
            }
            else
            {
				if(reader.name() == QLatin1String("Table"))
                {
                    std::shared_ptr<TableTemplate> tt = std::make_shared<TableTemplate>();

                    if (tt->load(reader) == true)
                    {
                        m_objects.push_back(tt);
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
        return m_objects.empty();
	}

	const QPageLayout& SectionTemplate::pageLayout() const
	{
		return m_pageLayout;
	}


	const QString& SectionTemplate::caption() const
    {
        return m_caption;
    }

	const QString& SectionTemplate::tag() const
	{
		return m_tag;
	}

	const std::vector<std::shared_ptr<ObjectTemplate>>& SectionTemplate::objects() const
    {
        return m_objects;
    }

	//
	// ReportTemplate
	//

	ReportTemplate::ReportTemplate()
	{

	}

	bool ReportTemplate::load(QXmlStreamReader& reader)
	{
		// Load basic attributes

        if (reader.attributes().hasAttribute("Caption"))
        {
            m_caption = reader.attributes().value("Caption").toString();
        }

		if (reader.attributes().hasAttribute("Resolution"))
		{
			bool ok = false;
			m_resolution = reader.attributes().value("Resolution").toInt(&ok);
			if (ok == false)
			{
				reader.raiseError(QObject::tr("Failed to load ReportTemplate element - unknown Resolution."));
				return false;
			}
		}

		// Load contents

		while (reader.readNextStartElement())
        {
			if(reader.name() == QLatin1String("Section"))
			{
				SectionTemplate st;

				if (st.load(reader) == false)
				{
					return !reader.hasError();
				}

				m_sections.push_back(st);
				continue;
			}

			if(reader.name() == QLatin1String("Header"))
			{
				if (m_reportHeader.load(reader) == false)
				{
					return !reader.hasError();
				}
				continue;
			}

			if(reader.name() == QLatin1String("Footer"))
			{
				if (m_reportFooter.load(reader) == false)
				{
					return !reader.hasError();
				}
				continue;
			}

			if(reader.name() == QLatin1String("MarginItem"))
			{
				MarginTemplate mt;

				if (mt.load(reader) == false)
				{
					return !reader.hasError();
				}

				m_margins.push_back(mt);
				continue;
			}

			reader.raiseError(QObject::tr("Unknown tag: ") + reader.name().toString());
			return !reader.hasError();
        }

		return true;
	}

	int ReportTemplate::resolution() const
	{
		return m_resolution;
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

	/*const SectionTemplate& ReportTemplate::pageHeader() const
	{
		return m_pageHeader;
	}

    const SectionTemplate& ReportTemplate::pageFooter() const
	{
		return m_pageFooter;
	}*/

    const std::vector<SectionTemplate>& ReportTemplate::sections() const
    {
        return m_sections;
    }

	const std::vector<MarginTemplate>& ReportTemplate::margins() const
	{
		return m_margins;
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
			if(reader.name() == QLatin1String("Report"))
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

	const ReportTemplate& ReportTemplateStorage::templateByCaption(const QString& caption, bool* found) const
	{
		auto templ = std::find_if(m_templates.begin(),
								  m_templates.end(),
								  [&caption](const ReportLib::ReportTemplate& t){
			return t.caption() == caption;
		});

		if (templ == m_templates.end())
		{
			Q_ASSERT(false);
			if (found != nullptr)
			{
				*found = false;
			}
			static ReportTemplate err;
			return err;
		}

		if (found != nullptr)
		{
			*found = true;
		}
		return *templ;

	}


}
