#include <ReportLib/Report.h>
#include <ReportLib/ReportObject.h>

namespace ReportLib
{
	//
	// TextFormat
	//

	TextFormat::TextFormat(const ReportFont& font, Qt::Alignment alignment) :
		m_font(font),
		m_alignment(alignment)
	{
	}

	const ReportFont& TextFormat::font() const
	{
		return m_font;
	}

	Qt::Alignment TextFormat::alignment() const
	{
		return m_alignment;
	}

	/*void TextFormat::setTextForeground(const QBrush& brush)
	{
		m_charFormat.setForeground(brush);
	}

	void TextFormat::setTextBackground(const QBrush& brush)
	{
		m_charFormat.setBackground(brush);
	}*/

	//
	// TableFormat
	//

	TableFormat::TableFormat(const ReportFont& font,
							 const QStringList& headerLabels,
							 const std::vector<int> columnWidths,
							 Qt::Alignment alignment,
							 int borderWidth) :
		m_font(font),
		m_borderWidth(borderWidth)
	{
		if (static_cast<int>(headerLabels.size()) != columnWidths.size())
		{
			Q_ASSERT(static_cast<int>(headerLabels.size()) != columnWidths.size());
			return;
		}

		int index = 0;
		for (const QString& hl : headerLabels)
		{
			m_columnsFormat.push_back({hl, columnWidths[index++], alignment});
		}
	}

	TableFormat::TableFormat(const ReportFont& font, const std::vector<ColumnFormat>& columnsFormat, int borderWidth) :
		m_font(font),
		m_columnsFormat(columnsFormat),
		m_borderWidth(borderWidth)
	{
	}

	const ReportFont& TableFormat::font() const
	{
		return m_font;
	}

	const std::vector<TableFormat::ColumnFormat>& TableFormat::columnsFormat() const
	{
		return m_columnsFormat;
	}

	int TableFormat::borderWidth() const
	{ 
		return m_borderWidth; 
	}

	//
	// ReportMarginItem
	//

	ReportMarginItem::ReportMarginItem(const QString& text, int pageFrom, int pageTo, const TextFormat& format) :
		text(text),
		pageFrom(pageFrom),
		pageTo(pageTo),
		format(format)
	{
	}

	//
	// ReportTagStorage
	//

	ReportTagStorage::ReportTagStorage(const std::map<QString, std::shared_ptr<ReportSection>>& allSections) :
		m_allSections(allSections)
	{
	}

	QString ReportTagStorage::processTags(const QString& str) const
	{
		QString result{str};
		while (true)
		{
			qsizetype pos = result.indexOf(tagSectionStartPage);
			if (pos == -1)
			{
				break;
			}
			qsizetype openPos = result.indexOf('(', pos + 1);
			qsizetype closePos = result.indexOf(')', pos + 1);
			if (openPos != -1 && closePos != -1 && openPos < closePos)
			{
				QString schemaId = result.mid(openPos + 1, closePos - openPos - 1);

				auto it = m_allSections.find(schemaId);
				if (it != m_allSections.end())
				{
					result.remove(pos, closePos - pos + 1);
					result.insert(pos, QString::number(it->second->startPage()));
				}
				else
				{
					result.remove(pos, closePos - pos + 1);
					result.insert(pos, "?");
				}
			}
		}
		return result;
	}

	//
	// ReportObject
	//
	ReportObject::ReportObject(ReportObject::Type type) :
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
													   const SchemaFormat& format,
													   std::shared_ptr<VFrame30::Schema> schema,
													   const std::map<QUuid, ReportSchemaCompareAction>& compareActions)
	{
		auto result = std::make_shared<ReportSchema>(caption, format, schema, compareActions);
		return result;
	}

	ReportSchema::ReportSchema(const QString& caption,
							   const SchemaFormat& format,
							   std::shared_ptr<VFrame30::Schema> schema,
							   const std::map<QUuid, ReportSchemaCompareAction>& compareActions) :
		ReportObject(Type::Schema),
		m_caption(caption),
		m_format(format),
		m_schema(schema),
		m_compareActions(compareActions)
	{
	}

	void ReportSchema::renderText(QTextCursor& /*cursor*/,
								  double /*fontScaling*/,
								  const ReportTagStorage& /*tagStorage*/,
								  QMutex& /*statisticsMutex*/,
								  Statistics& /*statistics*/,
								  std::atomic_bool& /*stop*/) const
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

	ReportTable::ReportTable(const TableFormat& format) :
		ReportObject(Type::Table),
		m_format(format)
	{
	}

	std::shared_ptr<ReportTable> ReportTable::create(const TableFormat& format)
	{
		auto result = std::make_shared<ReportTable>(format);
		return result;
	}

	bool ReportTable::htmlEscaped() const
	{
		return m_htmlEscaped;
	}

	void ReportTable::setHtmlEscaped(bool value)
	{
		m_htmlEscaped = value;
	}

	int ReportTable::columnCount() const
	{
		return static_cast<int>(m_format.columnsFormat().size());
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
		if (row.size() > columnCount())
		{
			Q_ASSERT(false);
			return;
		}

		m_rows.push_back(row);
	}

	void ReportTable::sortByColumn(int column)
	{
		std::sort(m_rows.begin(),
				  m_rows.end(),
				  [column](const QStringList& a, const QStringList& b)
				  {
					  if (column >= a.size() || column >= b.size())
					  {
						  Q_ASSERT(false);
						  return false;
					  }

					  return a.at(column) < b.at(column);
				  });
	}

	void ReportTable::renderText(QTextCursor& cursor,
								 double fontScaling,
								 const ReportTagStorage& tagStorage,
								 QMutex& /*statisticsMutex*/,
								 Statistics& /*statistics*/,
								 std::atomic_bool& stop) const
	{
		int cols = columnCount();
		int rows = rowCount();

		QString borderFormat;
		{
			int borderWidth = m_format.borderWidth();
			if (borderWidth > 0)
			{
				borderFormat = QObject::tr("border : %1px solid black;").arg(borderWidth);
			}
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
									   %3\
									   background-color: #e0e0e0;\
									   color: black;\
								   }\
								   tr.d1 td {\
									   %3\
									   background-color: #ffffff;\
									   color: black;\
								   }\
								   </style>\
								   </head>\
								   <body>\
								   <table width=\"100%\">")
						   .arg(m_format.font().family)
						   .arg(m_format.font().pointSize * fontScaling)
						   .arg(borderFormat);

		html += "<thead><tr>";
		for (int c = 0; c < cols; c++)
		{
			const TableFormat::ColumnFormat& colFormat = m_format.columnsFormat()[c];

			const QString& str = colFormat.caption;

			html += QObject::tr("<th width=%1% align=>%2</th>").arg(colFormat.width).arg(str);
		}
		html += "</tr></thead>";

		html += "<tbody>";

		bool alternateRow = (rows & 1) != 0; // odd number of rows, first is alternate

		for (int r = 0; r < rows; r++)
		{
			if (stop.load() == true)
			{
				break;
			}

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

			int c = 0;
			for (const QString& str : row)
			{
				QString text = tagStorage.processTags(str);

				const TableFormat::ColumnFormat& colFormat = m_format.columnsFormat()[c];

				QString alignStr;
				switch (colFormat.alignment)
				{
				case Qt::AlignLeft:
					alignStr = "Left";
					break;
				case Qt::AlignHCenter:
					alignStr = "Center";
					break;
				case Qt::AlignRight:
					alignStr = "Right";
					break;
				default:
					alignStr = "Left";
					Q_ASSERT(false);
				}

				html += QObject::tr("<td width=%1% align=%2>%3</td>")
							.arg(colFormat.width)
							.arg(alignStr)
							.arg(htmlEscaped() ? text.toHtmlEscaped() : text);

				if (c++ >= cols)
				{
					break;
				}
			}

			html += "</tr>";
		}

		html += "</tbody>";

		html += "</table>\
				</body>\
				</html>";

		cursor.insertHtml(html);

		cursor.insertText("\n\n");
	}

	//
	// ReportText
	//

	ReportText::ReportText(const QString& text, const TextFormat& format) :
		ReportObject(Type::Text),
		m_format(format),
		m_text(text)
	{
	}

	std::shared_ptr<ReportText> ReportText::create(const QString& text, const TextFormat& format)
	{
		auto result = std::make_shared<ReportText>(text, format);
		return result;
	}

	void ReportText::renderText(QTextCursor& cursor,
								double fontScaling,
								const ReportTagStorage& tagStorage,
								QMutex& /*statisticsMutex*/,
								Statistics& /*statistics*/,
								std::atomic_bool& /*stop*/) const
	{
		QTextCharFormat cf = cursor.charFormat();
		cf.setFont(m_format.font()(fontScaling));
		cursor.setCharFormat(cf);

		QTextBlockFormat bf = cursor.blockFormat();
		bf.setAlignment(m_format.alignment());
		cursor.setBlockFormat(bf);

		cursor.insertText(tagStorage.processTags(m_text));
	}

	//
	// ReportSection
	//

	std::shared_ptr<ReportSection> ReportSection::create(const QString& caption, const QPageLayout& pageLayout)
	{
		auto section = std::make_shared<ReportSection>(caption);
		section->setPageLayout(pageLayout);
		return section;
	}

	ReportSection::ReportSection(const QString& caption) :
		m_caption(caption),
		m_startPage(0)
	{
	}

	ReportSection::~ReportSection() {}

	const QString& ReportSection::caption() const
	{
		return m_caption;
	}

	void ReportSection::setCaption(const QString& value)
	{
		m_caption = value;
	}

	const QString& ReportSection::tag() const
	{
		return m_tag;
	}

	void ReportSection::setTag(const QString& value)
	{
		m_tag = value;
	}

	const QPageLayout& ReportSection::pageLayout() const
	{
		return m_pageLayout;
	}

	void ReportSection::setPageLayout(const QPageLayout& value)
	{
		m_pageLayout = value;
	}

	int ReportSection::startPage() const
	{
		return m_startPage;
	}

	void ReportSection::setStartPage(int page)
	{
		m_startPage = page;
	}

	std::shared_ptr<ReportLib::ReportText> ReportSection::addText(const QString& text, const TextFormat& format)
	{
		std::shared_ptr<ReportLib::ReportText> object = std::make_shared<ReportLib::ReportText>(text, format);
		addObject(object);
		return object;
	}

	std::shared_ptr<ReportText> ReportSection::addText(std::shared_ptr<ReportText> object)
	{
		addObject(object);
		return object;
	}

	std::shared_ptr<ReportTable> ReportSection::addTable(const TableFormat& format)
	{
		std::shared_ptr<ReportTable> object = std::make_shared<ReportLib::ReportTable>(format);
		addObject(object);
		return object;
	}

	std::shared_ptr<ReportTable> ReportSection::addTable(std::shared_ptr<ReportTable> object)
	{
		addObject(object);
		return object;
	}

	std::shared_ptr<ReportSchema> ReportSection::addSchema(std::shared_ptr<ReportSchema> object)
	{
		addObject(object);
		return object;
	}

	void ReportSection::addObject(std::shared_ptr<ReportObject> object)
	{
		m_objects.push_back(object);
	}

	size_t ReportSection::objectCount() const
	{
		return m_objects.size();
	}

	std::shared_ptr<ReportObject> ReportSection::object(size_t index)
	{
		if (index >= objectCount())
		{
			Q_ASSERT(false);
			return nullptr;
		}
		return m_objects[index];
	}

} // namespace ReportLib
