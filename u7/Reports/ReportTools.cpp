#include "ReportTools.h"

//#include <QPrinter>

#include "../VFrame30/Schema.h"
#include "../VFrame30/DrawParam.h"

//
// ReportSchemaView
//

ReportSchemaView::ReportSchemaView(QWidget* parent):
	VFrame30::SchemaView(parent)
{
}

ReportSchemaView::~ReportSchemaView()
{
	qDebug() << "ReportSchemaView deleted";
}

void ReportSchemaView::adjust(QPainter* painter, double startX, double startY, double zoom) const
{
	Ajust(painter, startX, startY, zoom);
}

void ReportSchemaView::drawCompareOutlines(VFrame30::CDrawParam* drawParam, const QRectF& clipRect, const std::map<QUuid, ReportSchemaCompareAction>& compareActions)
{
	if (drawParam == nullptr)
	{
		assert(drawParam != nullptr);
		return;
	}

	// Draw items by layers which has Show flag
	//
	double clipX = clipRect.left();
	double clipY = clipRect.top();
	double clipWidth = clipRect.width();
	double clipHeight = clipRect.height();

	// Find compile layer
	//
	for (auto layer = schema()->Layers.cbegin(); layer != schema()->Layers.cend(); ++layer)
	{
		const VFrame30::SchemaLayer* pLayer = layer->get();

		if (pLayer->show() == false)
		{
			continue;
		}

		for (auto vi = pLayer->Items.cbegin(); vi != pLayer->Items.cend(); ++vi)
		{
			const SchemaItemPtr& item = *vi;

			auto actionIt = compareActions.find(item->guid());
			if (actionIt == compareActions.end())
			{
				assert(actionIt != compareActions.end());
				continue;
			}

			ReportSchemaCompareAction compareAction = actionIt->second;

			QColor color;

			switch (compareAction)
			{
			case ReportSchemaCompareAction::Unmodified:
				color = QColor(0, 0, 0, 0);			// Full transparent, as is
				break;
			case ReportSchemaCompareAction::Modified:
				color = QColor(0, 0, 0xC0, 128);
				break;
			case ReportSchemaCompareAction::Added:
				color = QColor(0, 0xC0, 0, 128);
				break;
			case ReportSchemaCompareAction::Deleted:
				color = QColor(0xC0, 0, 0, 128);
				break;
			default:
				assert(false);
			}

			if (compareAction != ReportSchemaCompareAction::Unmodified &&
				item->isIntersectRect(clipX, clipY, clipWidth, clipHeight) == true)
			{
				// Draw item issue
				//
				item->drawCompareAction(drawParam, color);
			}
		}
	}

}

//
// ReportObject
//

ReportObject::ReportObject()
{
}

ReportObject::~ReportObject()
{
}

bool ReportObject::isText() const
{
	return dynamic_cast<const ReportText*>(this) != nullptr;
}

bool ReportObject::isTable() const
{
	return dynamic_cast<const ReportTable*>(this) != nullptr;
}

//
// ReportTable
//

ReportTable::ReportTable(const QStringList& headerLabels, const std::vector<int>& columnWidths, const QTextCharFormat& charFormat):
	m_headerLabels(headerLabels),
	m_columnWidths(columnWidths),
	m_charFormat(charFormat)
{
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

void ReportTable::render(const ReportObjectContext& context) const
{
	int cols = columnCount();
	int rows = rowCount();

	if (m_columnWidths.size() != cols || m_headerLabels.size() != cols)
	{
		context.textCursor->insertText("Table rendering error!");
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
					<table width=\"100%\">").arg(m_charFormat.fontFamily()).arg(m_charFormat.fontPointSize());

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

	context.textCursor->insertHtml(html);

	context.textCursor->insertText("\n\n");
}

//
// ReportText
//

ReportText::ReportText(const QString& text, const QTextCharFormat& charFormat, const QTextBlockFormat& blockFormat):
	m_text(text),
	m_charFormat(charFormat),
	m_blockCharFormat(blockFormat)
{
}

void ReportText::render(const ReportObjectContext& context) const
{
	if (m_charFormat.isValid() == true)
	{
		context.textCursor->setCharFormat(m_charFormat);
	}
	if (m_blockCharFormat.isValid() == true)
	{
		context.textCursor->setBlockFormat(m_blockCharFormat);
	}

	context.textCursor->insertText(m_text);
}

//
// ReportSection
//

ReportSection::ReportSection(const QString& caption):
	m_caption(caption)
{
}

ReportSection::~ReportSection()
{
}

bool ReportSection::isEmpty() const
{
	return m_objects.empty() == true && m_schema == nullptr;
}

const QString& ReportSection::caption() const
{
	return m_caption;
}

void ReportSection::addText(const QString& text, const QTextCharFormat& charFormat, const QTextBlockFormat& blockFormat)
{
	m_objects.push_back(std::make_shared<ReportText>(text, charFormat, blockFormat));
}

void ReportSection::addTable(std::shared_ptr<ReportTable> table)
{
	m_objects.push_back(table);
}

std::shared_ptr<ReportTable> ReportSection::addTable(const QStringList& headerLabels, const std::vector<int>& columnWidths, const QTextCharFormat& charFormat)
{
	std::shared_ptr<ReportTable> table = std::make_shared<ReportTable>(headerLabels, columnWidths, charFormat);
	m_objects.push_back(table);

	return table;
}

std::shared_ptr<ReportTable> ReportSection::createTable(const QStringList& headerLabels, const std::vector<int>& columnWidths, const QTextCharFormat& charFormat)
{
	std::shared_ptr<ReportTable> table = std::make_shared<ReportTable>(headerLabels, columnWidths, charFormat);

	return table;
}

void ReportSection::setSchema(std::shared_ptr<VFrame30::Schema> schema)
{
	m_schema = schema;
}

const std::map<QUuid, ReportSchemaCompareAction>& ReportSection::compareItemActions() const
{
	return m_itemsActions;
}

void ReportSection::setCompareItemActions(const std::map<QUuid, ReportSchemaCompareAction>& itemsActions)
{
	m_itemsActions = itemsActions;
}

std::shared_ptr<VFrame30::Schema> ReportSection::schema() const
{
	return m_schema;
}

void ReportSection::render(QSizeF pageSize)
{
	if (m_objects.empty() == true)
	{
		if (m_schema != nullptr)
		{
			// Document has only schema
			m_pageCount = 1;
		}

		return;
	}

	// Document has content

	m_textDocument.setPageSize(pageSize);

	QTextCursor textCursor(&m_textDocument);

	//int remove_caption = 1;
	//textCursor.insertText(m_caption);

	QTextCharFormat charFormat = textCursor.charFormat();
	charFormat.setFontPointSize(40);
	textCursor.setCharFormat(charFormat);

	for (const std::shared_ptr<ReportObject> object : m_objects)
	{
		if (object == nullptr)
		{
			Q_ASSERT(object);
			return;
		}

		ReportObjectContext context;
		context.textDocument = &m_textDocument;
		context.textCursor = &textCursor;

		object->render(context);
	}

	m_pageCount = m_textDocument.pageCount();

	return;
}

int ReportSection::pageCount() const
{
	return m_pageCount;
}

QTextDocument* ReportSection::textDocument()
{
	return &m_textDocument;
}

//
// ReportMarginItem
//

ReportMarginItem::ReportMarginItem(const QString& text, int pageFrom, int pageTo, Qt::Alignment alignment, const QTextCharFormat& charFormat, const QTextBlockFormat& blockFormat):
	m_text(text),
	m_pageFrom(pageFrom),
	m_pageTo(pageTo),
	m_alignment(alignment),
	m_charFormat(charFormat),
	m_blockFormat(blockFormat)

{

}

//
// ReportGenerator
//

ReportGenerator::ReportGenerator(ReportSchemaView* schemaView):
	m_schemaView(schemaView)
{
	Q_ASSERT(m_currentCharFormat.isValid());
	Q_ASSERT(m_currentBlockFormat.isValid());
}

int ReportGenerator::resolution() const
{
	return m_resolution;
}

void ReportGenerator::addMarginItem(const ReportMarginItem& item)
{
	m_marginItems.push_back(item);
}

void ReportGenerator::clearMarginItems()
{
	m_marginItems.clear();
}

void ReportGenerator::printDocument(QPdfWriter* pdfWriter, QTextDocument* textDocument, QPainter* painter,
									int* pageIndex, QMutex* pageIndexMutex, int pageCount)
{
	if (pdfWriter == nullptr || textDocument == nullptr || painter == nullptr)
	{
		Q_ASSERT(pdfWriter);
		Q_ASSERT(textDocument);
		Q_ASSERT(painter);
		return;
	}

	if (textDocument->isEmpty() == true)
	{
		return;
	}

	const QRect pageRect = pdfWriter->pageLayout().paintRectPixels(m_resolution);

	// The total extent of the content (there are no page margin in this)
	const QRect contentRect = QRect(QPoint(0, 0), textDocument->size().toSize());

	// This is the part of the content we will drop on a page.  It's a sliding window on the content.
	QRect currentRect(0, 0, pageRect.width(), pageRect.height());

	while (currentRect.intersects(contentRect) == true)
	{
		painter->save();
		painter->translate(0, -currentRect.y());
		textDocument->drawContents(painter, currentRect);  // draws part of the document
		painter->restore();

		// Get the page number

		int page = 0;

		if (pageIndex != nullptr)
		{
			if (pageIndexMutex != nullptr)
			{
				pageIndexMutex->lock();
			}

			page = *pageIndex;

			if (pageIndexMutex != nullptr)
			{
				pageIndexMutex->unlock();
			}
		}

		drawMarginItems(page, pageCount, pdfWriter, painter);

		// Translate the current rectangle to the area to be printed for the next page
		currentRect.translate(0, currentRect.height());

		//Inserting a new page if there is still area left to be printed
		if (currentRect.intersects(contentRect))
		{
			pdfWriter->newPage();

			// Increase the page number

			if (pageIndex != nullptr)
			{
				if (pageIndexMutex != nullptr)
				{
					pageIndexMutex->lock();
				}

				(*pageIndex)++;

				if (pageIndexMutex != nullptr)
				{
					pageIndexMutex->unlock();
				}
			}
		}
	}

	return;
}

void ReportGenerator::printSchema(QPdfWriter* pdfWriter,
								  QTextDocument* textDocument,
								  QPainter* painter,
								  std::shared_ptr<VFrame30::Schema> schema,
								  const std::map<QUuid, ReportSchemaCompareAction>& compareActions)
{
	if (pdfWriter == nullptr || textDocument == nullptr || painter == nullptr || m_schemaView == nullptr)
	{
		Q_ASSERT(pdfWriter);
		Q_ASSERT(textDocument);
		Q_ASSERT(painter);
		Q_ASSERT(m_schemaView);
		return;
	}

	if (schema == nullptr)
	{
		Q_ASSERT(schema);
		return;
	}

	// Calculate the upper schema offset

	const QRect pageRect = pdfWriter->pageLayout().paintRectPixels(m_resolution);

	const QRect contentRect = QRect(QPoint(0, 0), textDocument->size().toSize());

	const int schemaTop = contentRect.height() % pageRect.height();

	int schemaLeft = 0;

	const int schemaMaxHeight = pageRect.height() - schemaTop;

	// Calculate draw parameters

	m_schemaView->setSchema(schema, true);

	VFrame30::CDrawParam drawParam(painter, schema.get(), m_schemaView, schema->gridSize(), schema->pinGridStep());
	drawParam.setInfoMode(false);
	drawParam.session() = m_schemaView->session();

	double schemaWidthInPixel = schema->GetDocumentWidth(m_resolution, 100.0);		// Export 100% zoom
	double schemaHeightInPixel = schema->GetDocumentHeight(m_resolution, 100.0);		// Export 100% zoom

	double zoom = pageRect.width() / schemaWidthInPixel;

	double schemaHeightInPixelWZoomed = schemaHeightInPixel * zoom;

	if (schemaHeightInPixelWZoomed > schemaMaxHeight)
	{
		// Reduce schema's height, it does not fit vertically

		double yZoom =  schemaMaxHeight / schemaHeightInPixelWZoomed;

		zoom *= yZoom;

		// Center schema horizontally

		int schemaWidthInPixelZoomed = static_cast<int>(schemaHeightInPixel * zoom + 0.5);

		schemaLeft =  (pageRect.width() - schemaWidthInPixelZoomed) / 2;
	}

	// Draw rect

	//m_pdfPainter.fillRect(QRectF(0, schemaTop, pageRect.width(), pageRect.height() - schemaTop), QColor(0xB0, 0xB0, 0xB0));

	// Ajust QPainter
	//

	painter->save();

	painter->setRenderHint(QPainter::Antialiasing);

	m_schemaView->adjust(painter, schemaLeft, schemaTop, zoom * 100.0);		// Export 100% zoom

	// Draw Schema
	//
	QRectF clipRect(0, 0, schema->docWidth(), schema->docHeight());

	schema->Draw(&drawParam, clipRect);

	if (compareActions.empty() == false)
	{
		drawParam.setControlBarSize(
			schema->unit() == VFrame30::SchemaUnit::Display ?
						(4 / zoom) : (mm2in(2.4) / zoom));

		m_schemaView->drawCompareOutlines(&drawParam, clipRect, compareActions);
	}

	painter->restore();

	return;
}

void ReportGenerator::saveFormat()
{
	m_currentCharFormatSaved = m_currentCharFormat;
	m_currentBlockFormatSaved = m_currentBlockFormat;
}

void ReportGenerator::restoreFormat()
{
	m_currentCharFormat = m_currentCharFormatSaved;
	m_currentBlockFormat = m_currentBlockFormatSaved;
}

void ReportGenerator::setFont(const QFont& font)
{
	m_currentCharFormat.setFont(font);
}

void ReportGenerator::setTextForeground(const QBrush& brush)
{
	m_currentCharFormat.setForeground(brush);
}

void ReportGenerator::setTextBackground(const QBrush& brush)
{
	m_currentCharFormat.setBackground(brush);
}

void ReportGenerator::setTextAlignment(Qt::Alignment alignment)
{
	m_currentBlockFormat.setAlignment(alignment);
}

const QTextCharFormat& ReportGenerator::currentCharFormat() const
{
	return m_currentCharFormat;
}

const QTextBlockFormat& ReportGenerator::currentBlockFormat() const
{
	return m_currentBlockFormat;
}

void ReportGenerator::drawMarginItems(int page, int totalPages, QPdfWriter* pdfWriter, QPainter* painter)
{
	if (pdfWriter == nullptr || painter == nullptr)
	{
		Q_ASSERT(pdfWriter);
		Q_ASSERT(painter);
		return;
	}

	const QRect fullPageRect = pdfWriter->pageLayout().fullRectPixels(m_resolution);

	const QRect pageRect = pdfWriter->pageLayout().paintRectPixels(m_resolution);

	QMargins margins = pdfWriter->pageLayout().marginsPixels(m_resolution);

	QRect topRect(fullPageRect.left() + margins.left() / 2,
				  fullPageRect.top(),
				  pageRect.width() + (margins.left() + margins.right()) / 2,
				  abs(pageRect.top() - fullPageRect.top()));

	QRect bottomRect(fullPageRect.left() + margins.left() / 2,
					 pageRect.bottom(),
					 pageRect.width() + (margins.left() + margins.right()) / 2,
					 abs(pageRect.bottom() - fullPageRect.bottom()));

	painter->save();

	painter->translate(-pageRect.left(), -pageRect.top());

	for (const ReportMarginItem& item : m_marginItems)
	{
		if (item.m_pageFrom != -1 && item.m_pageFrom > page)
		{
			continue;
		}
		if (item.m_pageTo != -1 && item.m_pageTo < page)
		{
			continue;
		}

		painter->setFont(item.m_charFormat.font());

		QString text = item.m_text;

		if (text == "%PAGE%")
		{
			text = tr("Page %1 of %2").arg(page).arg(totalPages);
		}

		//painter.fillRect(topRect, Qt::green);
		//painter.fillRect(bottomRect, Qt::yellow);

		if (item.m_alignment & Qt::AlignTop)
		{
			int alignment = item.m_alignment & ~Qt::AlignTop;

			painter->drawText(topRect, alignment | Qt::AlignVCenter, text);
		}
		else
		{
			if (item.m_alignment & Qt::AlignBottom)
			{
				int alignment = item.m_alignment & ~Qt::AlignBottom;
				painter->drawText(bottomRect, alignment | Qt::AlignVCenter, text);
			}
		}
	}

	painter->restore();

}
