#include "ReportGenerator.h"
#include "../VFrame30/Schema.h"

#include <QPageSetupDialog>
#include <QPrinter>

#include "../VFrame30/VFrameTools.h"
#include "../VFrame30/Schema.h"
#include "../VFrame30/DrawParam.h"

namespace  Builder
{

//
// ReportSchemaView
//

ReportSchemaView::ReportSchemaView():
	VFrame30::SchemaView()
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

	if (schema() == nullptr)
	{
		assert(schema() != nullptr);
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

	if (static_cast<int>(m_columnWidths.size()) != cols || m_headerLabels.size() != cols)
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
					<table width=\"100%\">").arg(m_charFormat.font().family()).arg(m_charFormat.fontPointSize());

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

	for (const std::shared_ptr<ReportObject>& object : m_objects)
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

ReportMarginItem::ReportMarginItem(const QString& text, int pageFrom, int pageTo, const QFont& font, Qt::Alignment alignment):
	m_text(text),
	m_pageFrom(pageFrom),
	m_pageTo(pageTo),
	m_font(font),
	m_alignment(alignment)
{

}

//
// ReportGenerator
//

ReportGenerator::ReportGenerator(std::shared_ptr<ReportSchemaView> schemaView, const AppSignalSet* signalSet):
	m_schemaView(schemaView),
	m_appSignalProvider(signalSet),
	m_appSignalController(&m_appSignalProvider, nullptr)
{
	Q_ASSERT(m_currentCharFormat.isValid());
	Q_ASSERT(m_currentBlockFormat.isValid());
}

QPageLayout ReportGenerator::pageLayout() const
{
	return m_pageLayout;
}

void ReportGenerator::setPageLayout(const QPageLayout& value)
{
	m_pageLayout = value;
}

int ReportGenerator::resolution() const
{
	return m_pageResolution;
}

void ReportGenerator::setResolution(int value)
{
	m_pageResolution = value;
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
									const QString& objectName, int* pageIndex, QMutex* pageIndexMutex, int pageCount) const
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

		drawMarginItems(objectName, page, pageCount, pdfWriter, painter);

		return;
	}

	const QRect pageRect = pdfWriter->pageLayout().paintRectPixels(pdfWriter->resolution());

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

		drawMarginItems(objectName, page, pageCount, pdfWriter, painter);

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
								  QPainter* painter,
								  std::shared_ptr<VFrame30::Schema> schema,
								  std::optional<const QTextDocument* const> textDocument,
								  std::optional<const std::map<QUuid, ReportSchemaCompareAction>* const> compareActions)
{
	if ( m_schemaView == nullptr || pdfWriter == nullptr || painter == nullptr || schema == nullptr)
	{
		Q_ASSERT(m_schemaView);
		Q_ASSERT(pdfWriter);
		Q_ASSERT(painter);
		Q_ASSERT(schema);
		return;
	}

	// Calculate the upper schema offset
	//
	const QRect pageRect = pdfWriter->pageLayout().paintRectPixels(pdfWriter->resolution());

	int schemaTop = 0;
	int schemaLeft = 0;

	if (textDocument.has_value() == true && textDocument.value()->isEmpty() == false)
	{
		const QRect contentRect = QRect(QPoint(0, 0), textDocument.value()->size().toSize());
		schemaTop =  contentRect.height() % pageRect.height();
	}

	const int schemaMaxHeight = pageRect.height() - schemaTop;

	// Calculate draw parameters
	//
	double schemaWidthInPixel = schema->GetDocumentWidth(pdfWriter->physicalDpiX(), 100.0);		// Export 100% zoom
	double schemaHeightInPixel = schema->GetDocumentHeight(pdfWriter->physicalDpiY(), 100.0);		// Export 100% zoom

	double zoom = pageRect.width() / schemaWidthInPixel;

	double schemaHeightInPixelWZoomed = schemaHeightInPixel * zoom;

	if (schemaHeightInPixelWZoomed > schemaMaxHeight)
	{
		// Reduce schema's height, it does not fit vertically
		//
		double yZoom =  schemaMaxHeight / schemaHeightInPixelWZoomed;

		zoom *= yZoom;

		// Center schema horizontally
		//
		int schemaWidthInPixelZoomed = static_cast<int>(schemaWidthInPixel * zoom + 0.5);

		schemaLeft =  (pageRect.width() - schemaWidthInPixelZoomed) / 2;
	}

	// Draw Schema
	//
	painter->save();
	painter->setRenderHint(QPainter::Antialiasing);

	VFrame30::CDrawParam drawParam(painter, schema.get(), m_schemaView.get(), schema->gridSize(), schema->pinGridStep());
	drawParam.setInfoMode(false);
	drawParam.setPdfMode(true);
	drawParam.session() = m_schemaView->session();
	drawParam.setAppSignalController(&m_appSignalController);

	m_schemaView->setSchemaInternal(schema);
	m_schemaView->adjust(painter, schemaLeft, schemaTop, zoom * 100.0);		// Export 100% zoom

	QRectF clipRect(0, 0, schema->docWidth(), schema->docHeight());

	schema->Draw(&drawParam, clipRect);

	if (compareActions.has_value() == true &&
		compareActions.value() != nullptr &&
		compareActions.value()->empty() == false)
	{
		drawParam.setControlBarSize(CONTROL_BAR(schema->unit(), drawParam.device()->devicePixelRatioF(), zoom * 100.0));
		m_schemaView->drawCompareOutlines(&drawParam, clipRect, *(compareActions.value()));
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

void ReportGenerator::drawMarginItems(const QString& objectName, int page, int totalPages, QPdfWriter* pdfWriter, QPainter* painter) const
{
	if (pdfWriter == nullptr || painter == nullptr)
	{
		Q_ASSERT(pdfWriter);
		Q_ASSERT(painter);
		return;
	}

	const QRect fullPageRect = pdfWriter->pageLayout().fullRectPixels(pdfWriter->resolution());

	const QRect pageRect = pdfWriter->pageLayout().paintRectPixels(pdfWriter->resolution());

	QMargins margins = pdfWriter->pageLayout().marginsPixels(pdfWriter->resolution());

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

		painter->setFont(item.m_font);

		QString text = item.m_text;

		if (text == "%PAGE%")
		{
			text = tr("Page %1 of %2").arg(page).arg(totalPages);
		}

		if (text == "%OBJECT%")
		{
			text = objectName;
		}

		//painter.fillRect(topRect, Qt::green);
		//painter.fillRect(bottomRect, Qt::yellow);

		QFontMetrics fm(item.m_font);
		QRect textBoundingRect = fm.boundingRect(text);

		if (item.m_alignment & Qt::AlignTop)
		{
			if (topRect.width() >= textBoundingRect.width() && topRect.height() >= textBoundingRect.height())
			{
				int alignment = item.m_alignment & ~Qt::AlignTop;
				painter->drawText(topRect, alignment | Qt::AlignVCenter, text);
			}
		}
		else
		{
			if (item.m_alignment & Qt::AlignBottom)
			{
				if (bottomRect.width() >= textBoundingRect.width() && bottomRect.height() >= textBoundingRect.height())
				{
					int alignment = item.m_alignment & ~Qt::AlignBottom;
					painter->drawText(bottomRect, alignment | Qt::AlignVCenter, text);
				}
			}
		}
	}

	painter->restore();

}

//
// SchemasReportGenerator
//

SchemasReportGenerator::SchemasReportGenerator(std::shared_ptr<ReportSchemaView> schemaView,
											   const AppSignalSet *signalSet,
											   const QString& serverIp,
											   int serverPort,
											   const QString& serverUserName,
											   const QString& serverPassword,
											   const QString& projectName,
											   const QString& userName,
											   const QString& userPassword,
											   std::vector<DbFileInfo> files,
											   const QString& filePath):
	ReportGenerator(schemaView, signalSet),
	m_inputFiles(files),
	m_filePath(filePath),
	m_serverIp(serverIp),
	m_serverPort(serverPort),
	m_serverUserName(serverUserName),
	m_serverPassword(serverPassword),
	m_projectName(projectName),
	m_userName(userName),
	m_userPassword(userPassword)
{
	m_marginFont = QFont("Arial", 8);

	return;
}

SchemasReportGenerator::~SchemasReportGenerator()
{
	qDebug() << "SchemasReportWorker deleted";
}

void SchemasReportGenerator::setReportFileTypeParams(const std::vector<ReportFileTypeParams>& reportFileTypeParams)
{
	m_reportFileTypeParams = reportFileTypeParams;
}

std::vector<ReportFileTypeParams> SchemasReportGenerator::defaultFileTypeParams(DbController* db)
{
	std::vector<ReportFileTypeParams> result;

	if (db == nullptr || db->isProjectOpened() == false)
	{
		Q_ASSERT(false);
		return result;
	}

	result.push_back({db->systemFileId(DbDir::MonitorSchemasDir), QObject::tr("Monitor Schemas"),			true, QPageLayout(QPageSize(QPageSize::A3), QPageLayout::Orientation::Landscape, QMarginsF(15, 15, 15, 15))});
	result.push_back({db->systemFileId(DbDir::TuningSchemasDir), QObject::tr("Tuning Schemas"),			true, QPageLayout(QPageSize(QPageSize::A3), QPageLayout::Orientation::Landscape, QMarginsF(15, 15, 15, 15))});
	result.push_back({db->systemFileId(DbDir::DiagnosticsSchemasDir), QObject::tr("Diagnostics Schemas"),		true, QPageLayout(QPageSize(QPageSize::A3), QPageLayout::Orientation::Landscape, QMarginsF(15, 15, 15, 15))});
	result.push_back({db->systemFileId(DbDir::AppLogicDir), QObject::tr("Application Logic"),		true, QPageLayout(QPageSize(QPageSize::A3), QPageLayout::Orientation::Landscape, QMarginsF(15, 15, 15, 15))});
	result.push_back({db->systemFileId(DbDir::UfblDir), QObject::tr("UFBL Descriptions"),		true, QPageLayout(QPageSize(QPageSize::A3), QPageLayout::Orientation::Landscape, QMarginsF(15, 15, 15, 15))});

	return result;
}

void SchemasReportGenerator::exportFilesToPdf()
{
	std::map<QString, std::shared_ptr<VFrame30::Schema>> schemas;	// Key is full path to schema file

	try
	{
		openProject();

		loadSchemas(m_inputFiles, &schemas);

		closeProject();
	}
	catch (QString errorMessage)
	{
		closeProject();

		emit finished(errorMessage);
	}

	{
		QMutexLocker l(&m_statisticsMutex);
		m_currentStatus = WorkerStatus::Rendering;

		m_schemaIndex = 0;
	}

	// Save schemas to PDF
	//
	for (auto it = schemas.begin(); it != schemas.end(); it++)
	{
		if (m_stop == true)
		{
			break;
		}

		const std::shared_ptr<VFrame30::Schema> schema = it->second;
		const QString& schemaId = schema->schemaId();

		{
			QMutexLocker l(&m_statisticsMutex);
			m_schemaIndex++;
			m_currentSchemaId = schemaId;
		}

		QString fileName = it->first;

		fileName.replace('/', '_');

		qsizetype pos = fileName.lastIndexOf('.');
		if (pos != -1)
		{
			fileName = fileName.left(pos);
		}
		fileName += tr(".pdf");

		QBuffer buffer;
		buffer.open(QIODevice::WriteOnly);

		{
			// Create PDF writer
			//
			std::shared_ptr<QPdfWriter> pdfWriter = std::make_shared<QPdfWriter>(&buffer);
			pdfWriter->setTitle(schema->caption());

			QPageSize pageSize;
			double pageWidth = schema->docWidth();
			double pageHeight = schema->docHeight();

			if (schema->unit() == SchemaUnit::Inch)
			{
				pageSize = QPageSize(QSizeF(pageWidth, pageHeight), QPageSize::Inch);
				pdfWriter->setResolution(resolution());
			}
			else
			{
				assert(schema->unit() == SchemaUnit::Display);
				pageSize = QPageSize(QSize(static_cast<int>(pageWidth), static_cast<int>(pageHeight)));

				pdfWriter->setResolution(72);	// 72 is from enum QPageLayout::Unit help,
				// QPageLayout::Point	1	1/!!! 72th !!!! of an inch
			}

			pdfWriter->setPageSize(pageSize);
			pdfWriter->setPageMargins(QMarginsF(0, 0, 0, 0));

			// Create painter

			QPainter painter(pdfWriter.get());

			printSchema(pdfWriter.get(), &painter, schema, {}, {});
		}

		buffer.close();

		if (filePath().isEmpty() == false)
		{
			// Write buffer to a file
			//
			QFile f(filePath() + "//" + fileName);
			if (f.open(QIODevice::WriteOnly|QIODevice::Truncate) == false)
			{
				emit finished(tr("Can't save file: ") + f.fileName());
				return;
			}
			f.write(buffer.data());
		}
		else
		{
			// Move buffer to output data map
			//
			m_outputData[fileName] = std::move(buffer.data());
		}
	}

	emit finished(QString());

	return;
}

void SchemasReportGenerator::exportFilesToAlbum()
{
	std::map<QString, std::shared_ptr<VFrame30::Schema>> schemas;	// Key is full path to schema file

	try
	{
		openProject();

		loadSchemas(m_inputFiles, &schemas);

		closeProject();
	}

	catch (QString errorMessage)
	{
		closeProject();

		emit finished(errorMessage);
	}

	{
		QMutexLocker l(&m_statisticsMutex);
		m_currentStatus = WorkerStatus::Rendering;

		m_schemaIndex = 0;
	}

	// Init margins
	clearMarginItems();	// Just for fun
	addMarginItem({tr("Project: %1").arg(m_projectName), -1, -1, m_marginFont, Qt::AlignLeft | Qt::AlignTop});
	addMarginItem({tr("%OBJECT%"), -1, -1, m_marginFont, Qt::AlignRight | Qt::AlignTop});

	QBuffer buffer;
	buffer.open(QIODevice::WriteOnly);

	{
		// Create PDF writer
		//
		std::shared_ptr<QPdfWriter> pdfWriter = std::make_shared<QPdfWriter>(&buffer);
		pdfWriter->setTitle(m_projectName);
		pdfWriter->setPageLayout(pageLayout());
		pdfWriter->setResolution(resolution());

		bool firstSchema = true;

		QPainter painter(pdfWriter.get());

		for (auto it = schemas.begin(); it != schemas.end(); it++)
		{
			if (m_stop == true)
			{
				break;
			}

			const std::shared_ptr<VFrame30::Schema> schema = it->second;
			const QString& schemaId = schema->schemaId();

			{
				QMutexLocker l(&m_statisticsMutex);
				m_schemaIndex++;
				m_currentSchemaId = schemaId;
			}

			if (firstSchema == true)
			{
				firstSchema = false;
			}
			else
			{
				pdfWriter->newPage();
			}

			QTextDocument emptyTextDocument;
			printDocument(pdfWriter.get(), &emptyTextDocument, &painter, tr("Schema: %1").arg(schemaId), nullptr, nullptr, 0);

			printSchema(pdfWriter.get(), &painter, schema, {}, {});
		}
	}

	buffer.close();

	if (filePath().isEmpty() == false)
	{
		// Write buffer to a file
		//
		QFile f(filePath());
		if (f.open(QIODevice::WriteOnly|QIODevice::Truncate) == false)
		{
			emit finished(tr("Can't save file: ") + f.fileName());
			return;
		}
		f.write(buffer.data());
	}
	else
	{
		// Move buffer to output data map
		//
		m_outputData[filePath()] = std::move(buffer.data());
	}

	emit finished(QString());

	return;
}

void SchemasReportGenerator::exportAllSchemasToAlbums()
{
	std::vector<SchemaFilesInfo> schemaFilesInfo;

	try
	{
		openProject();

		schemaFilesInfo.push_back({db()->systemFileId(DbDir::AppLogicDir), tr("ApplicationLogic")});
		schemaFilesInfo.push_back({db()->systemFileId(DbDir::MonitorSchemasDir), tr("MonitorSchemas")});
		schemaFilesInfo.push_back({db()->systemFileId(DbDir::TuningSchemasDir), tr("TuningSchemas")});
		schemaFilesInfo.push_back({db()->systemFileId(DbDir::DiagnosticsSchemasDir), tr("DiagnosticsSchemas")});
		schemaFilesInfo.push_back({db()->systemFileId(DbDir::UfblDir), tr("UFBSchemas")});

		for (SchemaFilesInfo& sfi : schemaFilesInfo)
		{
			if (m_stop == true)
			{
				break;
			}

			DbFileTree fileTree;

			{
				QMutexLocker l(&m_statisticsMutex);
				m_currentSchemaType = sfi.caption;
			}

			bool ok = db()->getFileListTree(&fileTree, sfi.fileId, true/*removeDeleted*/, nullptr);
			if (ok == false)
			{
				throw(tr("DbController::getFileListTree failed on fileId = %1").arg(db()->systemFileId(DbDir::SchemasDir)));
			}

			const std::map<int, std::shared_ptr<DbFileInfo>>  files = fileTree.files();

			for (auto it = files.begin(); it != files.end(); it++)
			{
				const std::shared_ptr<DbFileInfo>& fi = it->second;

				if (fi->fileName().endsWith("." + QString(Db::File::AlFileExtension)) == false &&
					fi->fileName().endsWith("." + QString(Db::File::UfbFileExtension)) == false &&
					fi->fileName().endsWith("." + QString(Db::File::MvsFileExtension)) == false &&
					fi->fileName().endsWith("." + QString(Db::File::TvsFileExtension)) == false &&
					fi->fileName().endsWith("." + QString(Db::File::DvsFileExtension)) == false)
				{
					continue;
				}

				sfi.schemasFiles.push_back(*fi);
			}

			loadSchemas(sfi.schemasFiles, &sfi.schemas);

		}

		closeProject();
	}

	catch (QString errorMessage)
	{
		closeProject();

		emit finished(errorMessage);
	}

	// Init margins
	clearMarginItems();	// Just for fun
	addMarginItem({tr("Project: %1").arg(m_projectName), -1, -1, m_marginFont, Qt::AlignLeft | Qt::AlignTop});
	addMarginItem({tr("%OBJECT%"), -1, -1, m_marginFont, Qt::AlignRight | Qt::AlignTop});

	for (SchemaFilesInfo& sfi : schemaFilesInfo)
	{
		if (m_stop == true)
		{
			break;
		}

		if (sfi.schemas.empty() == true)
		{
			continue;
		}

		{
			QMutexLocker l(&m_statisticsMutex);
			m_currentStatus = WorkerStatus::Rendering;
			m_currentSchemaType = sfi.caption;
			m_schemaIndex = 0;
			m_schemasCount = static_cast<int>(sfi.schemas.size());
		}

		QBuffer buffer;
		buffer.open(QIODevice::WriteOnly);

		{
			// Create PDF writer
			//
			std::shared_ptr<QPdfWriter> pdfWriter = std::make_shared<QPdfWriter>(&buffer);
			pdfWriter->setTitle(m_projectName);

			// Find page layout

			QPageLayout pl = pageLayout();

			bool plFound = false;

			for (const ReportFileTypeParams& rp : m_reportFileTypeParams)
			{
				if (rp.fileId == sfi.fileId)
				{
					pl = rp.pageLayout;

					plFound = true;
					break;
				}
			}

			if (plFound == false)
			{
				// File type was not found
				Q_ASSERT(false);
			}

			pdfWriter->setPageLayout(pl);
			pdfWriter->setResolution(resolution());

			// Render schemas

			bool firstSchema = true;

			QPainter painter(pdfWriter.get());

			for (auto it = sfi.schemas.begin(); it != sfi.schemas.end(); it++)
			{
				if (m_stop == true)
				{
					break;
				}

				const std::shared_ptr<VFrame30::Schema> schema = it->second;
				const QString& schemaId = schema->schemaId();

				{
					QMutexLocker l(&m_statisticsMutex);
					m_schemaIndex++;
					m_currentSchemaId = schemaId;
				}

				if (firstSchema == true)
				{
					firstSchema = false;
				}
				else
				{
					pdfWriter->newPage();
				}

				QTextDocument emptyTextDocument;
				printDocument(pdfWriter.get(), &emptyTextDocument, &painter, tr("Schema: %1").arg(schemaId), nullptr, nullptr, 0);

				printSchema(pdfWriter.get(), &painter, schema, {}, {});
			}
		}

		buffer.close();

		if (filePath().isEmpty() == false)
		{
			// Write buffer to a file
			//
			QString fileName = tr("%1/%2_%3.pdf").arg(filePath()).arg(m_projectName).arg(sfi.caption);
			QFile f(fileName);
			if (f.open(QIODevice::WriteOnly|QIODevice::Truncate) == false)
			{
				emit finished(tr("Can't save file: ") + f.fileName());
				return;
			}
			f.write(buffer.data());
		}
		else
		{
			// Move buffer to output data map
			//
			m_outputData[sfi.caption + ".pdf"] = std::move(buffer.data());
		}
	}

	emit finished(QString());

	return;
}

void SchemasReportGenerator::stop()
{
	m_stop = true;
}

void SchemasReportGenerator::progressRequested()
{
	QString progressText;

	int progress = 0;
	int progressMin = 0;
	int progressMax = 0;

	getProgress(&progress, &progressMin, &progressMax, &progressText);

	emit progressChanged(progress, 0, progressMax, progressText);

	return;
}

void SchemasReportGenerator::getProgress(int* progress, int* progressMin, int* progressMax, QString* progressText)
{
	if (progress == nullptr || progressMin == nullptr || progressMax == nullptr || progressText == nullptr)
	{
		Q_ASSERT(progress);
		Q_ASSERT(progressMin);
		Q_ASSERT(progressMax);
		Q_ASSERT(progressText);
		return;
	}

	QMutexLocker l(&m_statisticsMutex);

	SchemasReportGenerator::WorkerStatus status = currentStatus();

	*progressMin = 0;

	switch (status)
	{
	case SchemasReportGenerator::WorkerStatus::Idle:
		{
			*progressText = tr("Idle");
		}
		break;
	case SchemasReportGenerator::WorkerStatus::Loading:
		{
			if (m_currentSchemaType.isEmpty() == false)
			{
				*progressText = tr("Loading schema: %1/%2").arg(currentSchemaType()).arg(currentSchemaId());
			}
			else
			{
				*progressText = tr("Loading schema: %1").arg(currentSchemaId());
			}
			*progress = schemaIndex();
			*progressMax = schemasCount();
		}
		break;
	case SchemasReportGenerator::WorkerStatus::Parsing:
		{
			if (m_currentSchemaType.isEmpty() == false)
			{
				*progressText = tr("Parsing schema: %1/%2").arg(currentSchemaType()).arg(currentSchemaId());
			}
			else
			{
				*progressText = tr("Parsing schema: %1").arg(currentSchemaId());
			}
			*progress = schemaIndex();
			*progressMax = schemasCount();
		}
		break;
	case SchemasReportGenerator::WorkerStatus::Rendering:
		{
			if (m_currentSchemaType.isEmpty() == false)
			{
				*progressText = tr("Rendering schema: %1/%2").arg(currentSchemaType()).arg(currentSchemaId());
			}
			else
			{
				*progressText = tr("Rendering schema: %1").arg(currentSchemaId());
			}
			*progress = schemaIndex();
			*progressMax = schemasCount();
		}
		break;
	}

	l.unlock();
}

QStringList SchemasReportGenerator::outputFilesList() const
{
	QStringList result;
	for (auto it : m_outputData)
	{
		result.push_back(it.first);
	}
	return result;
}

const QByteArray& SchemasReportGenerator::outputData(const QString& fileName)
{
	auto it = m_outputData.find(fileName);
	if (it == m_outputData.end())
	{
		Q_ASSERT(false);
		static QByteArray e;
		return e;
	}

	return it->second;
}


SchemasReportGenerator::WorkerStatus SchemasReportGenerator::currentStatus() const
{
	return m_currentStatus;
}

int SchemasReportGenerator::schemasCount() const
{
	return m_schemasCount;
}

int SchemasReportGenerator::schemaIndex() const
{
	return m_schemaIndex;
}

QString SchemasReportGenerator::currentSchemaType() const
{
	return m_currentSchemaType;
}

QString SchemasReportGenerator::currentSchemaId() const
{
	return m_currentSchemaId;
}

DbController* SchemasReportGenerator::db()
{
	return &m_db;
}

const QString& SchemasReportGenerator::filePath() const
{
	return m_filePath;
}

void SchemasReportGenerator::openProject()
{
	if (db()->isProjectOpened() == true)
	{
		Q_ASSERT(false);
		throw(tr("Failed to open project - it is open!"));
	}

	db()->disableProgress();

	db()->setHost(m_serverIp);
	db()->setPort(m_serverPort);
	db()->setServerUsername(m_serverUserName);
	db()->setServerPassword(m_serverPassword);

	bool ok = db()->openProject(m_projectName, m_userName, m_userPassword, nullptr);
	if (ok == false)
	{
		throw(tr("Failed to open project!"));
	}

	return;
}

void SchemasReportGenerator::closeProject()
{
	if (db()->isProjectOpened() == false)
	{
		return;
	}

	db()->closeProject(nullptr);

	return;
}


void SchemasReportGenerator::loadSchemas(const std::vector<DbFileInfo>& files, std::map<QString, std::shared_ptr<VFrame30::Schema>>* schemas)
{
	if (schemas == nullptr)
	{
		Q_ASSERT(schemas);
		throw(tr("internal error: schemas is nullptr"));
	}

	schemas->clear();

	// Load schemas from files
	{
		QMutexLocker l(&m_statisticsMutex);
		m_currentStatus = WorkerStatus::Loading;

		m_schemasCount = static_cast<int>(files.size());
		m_schemaIndex = 0;
	}

	// Get files from the database

	std::vector<std::shared_ptr<DbFile>> out;

	for (const DbFileInfo& fi : files)
	{
		if (m_stop == true)
		{
			break;
		}

		std::shared_ptr<DbFile> f;

		bool ok = db()->getLatestVersion(fi, &f, nullptr);
		if (ok == false)
		{
			throw(tr("Failed to load file %1").arg(fi.fileName()));
		}

		{
			QMutexLocker l(&m_statisticsMutex);
			m_schemaIndex++;
			m_currentSchemaId = f->fileName();
		}

		out.push_back(f);
	}

	// Parse schemas

	{
		QMutexLocker l(&m_statisticsMutex);
		m_currentStatus = WorkerStatus::Parsing;
		m_schemaIndex = 0;
	}

	// Calculate if selected files have different parent
	//
	bool differentParentId = false;
	int firstParentId = -1;

	for (std::shared_ptr<DbFile> dbFile : out)
	{
		if (firstParentId == -1)
		{
			firstParentId = dbFile->parentId();
			continue;
		}

		if (firstParentId != dbFile->parentId())
		{
			differentParentId = true;
			break;
		}
	}

	// Load schemas from files
	//
	for (std::shared_ptr<DbFile> dbFile : out)
	{
		if (m_stop == true)
		{
			break;
		}

		std::shared_ptr<VFrame30::Schema> schema = VFrame30::Schema::Create(dbFile->data());
		if (schema == nullptr)
		{
			throw(tr("Failed to load schema from '%1'!").arg(dbFile->fileName()));
		}

		{
			QMutexLocker l(&m_statisticsMutex);
			m_schemaIndex++;
			m_currentSchemaId = schema->schemaId();
		}

		QString fileName = dbFile->fileName();

		if (differentParentId == true)
		{
			// Include full file path
			//
			int parentId = dbFile->parentId();
			while(true)
			{
				DbFileInfo parentFileInfo;

				bool result = db()->getFileInfo(parentId, &parentFileInfo, nullptr);
				if (result == false || parentFileInfo.parentId() == 0)
				{
					break;
				}

				fileName = parentFileInfo.fileName() + '/' + fileName;
				parentId = parentFileInfo.parentId();
			};
		}

		(*schemas)[fileName] = schema;
	}

	return;
}

}
