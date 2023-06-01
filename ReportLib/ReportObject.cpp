#include "ReportObject.h"

namespace ReportLib
{
	//
	// ReportFormat
	//

    ReportObjectFormat::ReportObjectFormat(const QFont& font, Qt::Alignment alignment)
	{
		m_charFormat.setFont(font);
        m_blockFormat.setAlignment(alignment);
    }

    ReportObjectFormat::ReportObjectFormat(const QString& fontName, double fontPointSize, Qt::Alignment alignment)
	{
        m_charFormat.setFont(QFont(fontName, static_cast<int>(fontPointSize)));
        m_blockFormat.setAlignment(alignment);
    }

	void ReportObjectFormat::setFont(const QFont& font)
	{
		m_charFormat.setFont(font);
	}

	void ReportObjectFormat::setTextForeground(const QBrush& brush)
	{
		m_charFormat.setForeground(brush);
	}

	void ReportObjectFormat::setTextBackground(const QBrush& brush)
	{
		m_charFormat.setBackground(brush);
	}

    void ReportObjectFormat::setTextAlignment(Qt::Alignment alignment)
    {
        m_blockFormat.setAlignment(alignment);
    }

	const QTextCharFormat& ReportObjectFormat::charFormat() const
	{
		return m_charFormat;
	}

    const QTextBlockFormat& ReportObjectFormat::blockFormat() const
    {
        return m_blockFormat;
    }

	//
	// ReportMarginItem
	//

	ReportMarginItem::ReportMarginItem(const QString& text, int pageFrom, int pageTo, const ReportObjectFormat& format):
		text(text),
		pageFrom(pageFrom),
		pageTo(pageTo),
		format(format)
	{

	}

	ReportObject::ReportObject(const ReportObjectFormat& format, Type type):
		m_format(format),
		m_type(type)
	{
	}

	ReportObject::Type ReportObject::type() const
	{
		return m_type;
	}

	//
	// ReportSchema
	//
	std::shared_ptr<ReportSchema> ReportSchema::create(const QString& caption,
													   const ReportObjectFormat& format,
													   std::shared_ptr<VFrame30::Schema> schema,
													   const std::map<QUuid, ReportSchemaCompareAction>& compareActions)
	{
		auto result = std::make_shared<ReportSchema>(caption, format, schema, compareActions);
		return result;
	}

	ReportSchema::ReportSchema(const QString& caption,
							   const ReportObjectFormat& format,
							   std::shared_ptr<VFrame30::Schema> schema,
							   const std::map<QUuid, ReportSchemaCompareAction>& compareActions):
		ReportObject(format, ReportObject::Type::Schema),
		m_caption(caption),
		m_schema(schema),
		m_compareActions(compareActions)
	{

	}

	void ReportSchema::renderText(QTextCursor& /*cursor*/) const
	{
	}

	std::shared_ptr<VFrame30::Schema> ReportSchema::schema() const
	{
		return m_schema;
	}

	const std::map<QUuid, ReportSchemaCompareAction>& ReportSchema::compareActions() const
	{
		return m_compareActions;
	}

	//
	// ReportTable
	//

    ReportTable::ReportTable(const QStringList& headerLabels, const std::vector<int>& columnWidths, const ReportObjectFormat& format):
		ReportObject(format, ReportObject::Type::Table),
		m_headerLabels(headerLabels),
        m_columnWidths(columnWidths)
    {
	}

    std::shared_ptr<ReportTable> ReportTable::create(const QStringList& headerLabels, const std::vector<int>& columnWidths, const ReportObjectFormat& format)
	{
        auto result = std::make_shared<ReportTable>(headerLabels, columnWidths, format);
		return result;
	}

	int ReportTable::columnCount() const
	{
		return static_cast<int>(m_headerLabels.size());
	}

	int ReportTable::rowCount() const
	{
		return static_cast<int>(m_rows.size());
	}

	const QStringList& ReportTable::rowAt(int index) const
	{
		if (index < 0 || index >= rowCount())
		{
			Q_ASSERT(false);
			static QStringList errorsStrings;
			return errorsStrings;
		}

		return m_rows[index];
	}

	void ReportTable::insertRow(const QStringList& row)
	{
		if (row.size() != columnCount())
		{
			Q_ASSERT(false);
			return;
		}

		m_rows.push_back(row);
	}

	void ReportTable::sortByColumn(int column)
	{
		std::sort(m_rows.begin(), m_rows.end(), [column](const QStringList& a, const QStringList& b){

			if (column >= a.size() || column >= b.size())
			{
				Q_ASSERT(false);
				return false;
			}

			return a.at(column) < b.at(column);
		});
	}

	void ReportTable::renderText(QTextCursor& cursor) const
	{
		int cols = columnCount();
		int rows = rowCount();

		if (static_cast<int>(m_columnWidths.size()) != cols || m_headerLabels.size() != cols)
		{
			cursor.insertText("Table rendering error!");
			Q_ASSERT(false);
			return;
        }

		QString html = QObject::tr("<html>\
								   <head>\
								   <style>\
								   table, th, td {\
									   font-family: %1;\
									   font-size: %2pt;\
									   border-collapse: collapse;\
								   }\
								   th{\
									   border: 1px solid black;\
									   padding: 3px;\
								   }\
								   td {\
									   padding: 3px;\
								   }\
								   tr.d0 td {\
									   background-color: #e0e0e0;\
									   color: black;\
								   }\
								   tr.d1 td {\
									   background-color: #ffffff;\
									   color: black;\
								   }\
								   </style>\
								   </head>\
								   <body>\
								   <table width=\"100%\">")
				.arg(m_format.charFormat().font().family())
				.arg(m_format.charFormat().fontPointSize());

		html += "<thead><tr>";
		for (int c = 0; c < cols; c++)
		{
			const QString& str = m_headerLabels[c];

			html += QObject::tr("<th width=%1%>%2</th>").arg(m_columnWidths[c]).arg(str);
		}
		html += "</tr></thead>";

		html += "<tbody>";

		bool alternateRow = (rows & 1) != 0;	// odd number of rows, first is alternate

		for (int r = 0; r < rows; r++)
		{
			if (alternateRow == true)
			{
				html += "<tr class=\"d0\">";
			}
			else
			{
				html += "<tr class=\"d1\">";
			}
			alternateRow = !alternateRow;

			const QStringList& row = m_rows[r];

			for (int c = 0; c < cols; c++)
			{
				const QString str = row[c];

				html += QObject::tr("<td width=%1%>%2</td>").arg(m_columnWidths[c]).arg(str.toHtmlEscaped());
			}

			html += "</tr>";
		}

		html += "</tbody>";

		/* footer
html += "<tfoot style=\"background: #ffc\">><tr>";
for (int c = 0; c < cols; c++)
{
	const QString& str = m_headerLabels[c];

	html += QObject::tr("<th>%1</th>").arg(str);
}
html += "</tr></tfoot";
*/

		html += "</table>\
				</body>\
				</html>";

				cursor.insertHtml(html);

		cursor.insertText("\n\n");
	}

	//
	// ReportText
	//

    ReportText::ReportText(const QString& text, const ReportObjectFormat& format):
		ReportObject(format, ReportObject::Type::Text),
        m_text(text)
	{
	}

    std::shared_ptr<ReportText> ReportText::create(const QString& text, const ReportObjectFormat& format)
	{
        auto result = std::make_shared<ReportText>(text, format);
		return result;
	}

	void ReportText::renderText(QTextCursor& cursor) const
	{
		if (m_format.charFormat().isValid() == true)
		{
			cursor.setCharFormat(m_format.charFormat());
        }
        if (m_format.blockFormat().isValid() == true)
        {
            cursor.setBlockFormat(m_format.blockFormat());
        }

		cursor.insertText(m_text);
	}
}
