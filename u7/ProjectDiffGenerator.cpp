#include "ProjectDiffGenerator.h"

#include "../lib/SignalProperties.h"
#include "../lib/PropertyEditor.h"
#include "../lib/Connection.h"
#include "../VFrame30/DrawParam.h"
#include "../lib/TypesAndEnums.h"
#include "../VFrame30/Bus.h"

#include "Forms/DialogProjectDiffProgress.h"

#include <QPrinter>
#include "Settings.h"
//
// ReportSchemaView
//

ReportSchemaView::ReportSchemaView(QWidget* parent):
	VFrame30::SchemaView(parent)
{
}

void ReportSchemaView::adjust(QPainter* painter, double startX, double startY, double zoom) const
{
	Ajust(painter, startX, startY, zoom);
}

void ReportSchemaView::drawCompareOutlines(VFrame30::CDrawParam* drawParam, const QRectF& clipRect, const std::map<QUuid, CompareAction>& compareActions)
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

			CompareAction compareAction = actionIt->second;

			QColor color;

			switch (compareAction)
			{
			case CompareAction::Unmodified:
				color = QColor(0, 0, 0, 0);			// Full transparent, as is
				break;
			case CompareAction::Modified:
				color = QColor(0, 0, 0xC0, 128);
				break;
			case CompareAction::Added:
				color = QColor(0, 0xC0, 0, 128);
				break;
			case CompareAction::Deleted:
				color = QColor(0xC0, 0, 0, 128);
				break;
			default:
				assert(false);
			}

			if (compareAction != CompareAction::Unmodified &&
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

ReportTable::ReportTable(const QStringList& headerLabels, const QTextCharFormat& charFormat):
	m_headerLabels(headerLabels),
	m_charFormat(charFormat)
{
}

QStringList ReportTable::headerLabels() const
{
	return m_headerLabels;
}

void ReportTable::setHeaderLabels(const QStringList& headerLabels)
{
	m_headerLabels = headerLabels;
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

void ReportTable::render(const ReportObjectContext& context) const
{
	int cols = columnCount();
	int rows = rowCount();

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
	}\
	th {\
							padding: 3px;\
						}\
						td {\
							padding: 3px;\
						}\
						tr.d0 td {\
						  background-color: #dddddd;\
						  color: black;\
						}\
						tr.d1 td {\
						  background-color: #ffffff;\
						  color: black;\
						}\
						</style>\
					</head>\
				<body>\
					<table width=\"100%\">\
						<colgroup>\
							<col width=\"40%\">\
							<col width=\"20%\">\
							<col width=\"40%\">\
						</colgroup>").arg(m_charFormat.fontFamily()).arg(m_charFormat.fontPointSize());


	html += "<thead><tr>";
	for (int c = 0; c < cols; c++)
	{
		const QString& str = m_headerLabels[c];

		html += QObject::tr("<th>%1</th>").arg(str);
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
			const QString& str = row[c];

			html += QObject::tr("<td>%1</td>").arg(str);
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

ReportSection::ReportSection()
{
}

ReportSection::~ReportSection()
{
}

bool ReportSection::isEmpty() const
{
	return m_objects.empty() == true && m_schema == nullptr;
}

void ReportSection::addText(const QString& text, const QTextCharFormat& charFormat, const QTextBlockFormat& blockFormat)
{
	m_objects.push_back(std::make_shared<ReportText>(text, charFormat, blockFormat));
}

void ReportSection::addTable(std::shared_ptr<ReportTable> table)
{
	m_objects.push_back(table);
}

std::shared_ptr<ReportTable> ReportSection::addTable(const QStringList& headerLabels, const QTextCharFormat& charFormat)
{
	std::shared_ptr<ReportTable> table = std::make_shared<ReportTable>(headerLabels, charFormat);
	m_objects.push_back(table);

	return table;
}

void ReportSection::setSchema(std::shared_ptr<VFrame30::Schema> schema)
{
	m_schema = schema;
}

const std::map<QUuid, CompareAction>& ReportSection::compareItemActions() const
{
	return m_itemsActions;
}

void ReportSection::setCompareItemActions(const std::map<QUuid, CompareAction>& itemsActions)
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

ReportGenerator::ReportGenerator(const QString& fileName, std::shared_ptr<ReportSchemaView> schemaView):
	m_pdfWriter(fileName),
	m_schemaView(schemaView)
{

}

void ReportGenerator::addMarginItem(const ReportMarginItem& item)
{
	m_marginItems.push_back(item);
}

void ReportGenerator::printDocument(QPdfWriter* pdfWriter, QTextDocument* textDocument, QPainter* painter)
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

		{
			QMutexLocker l(&m_statisticsMutex);
			int page = m_pageIndex;
			l.unlock();

			drawMarginItems(page, painter);
		}

		// Translate the current rectangle to the area to be printed for the next page
		currentRect.translate(0, currentRect.height());

		//Inserting a new page if there is still area left to be printed
		if (currentRect.intersects(contentRect))
		{
			pdfWriter->newPage();

			{
				QMutexLocker l(&m_statisticsMutex);
				m_pageIndex++;
			}
		}
	}

	return;
}

void ReportGenerator::printSchema(QTextDocument* textDocument, QPainter* painter, ReportSchemaView* schemaView, std::shared_ptr<VFrame30::Schema> schema, const std::map<QUuid, CompareAction>& compareActions)
{
	if (textDocument == nullptr || painter == nullptr || schemaView == nullptr)
	{
		Q_ASSERT(textDocument);
		Q_ASSERT(painter);
		Q_ASSERT(schemaView);
		return;
	}

	if (schema == nullptr)
	{
		Q_ASSERT(schema);
		return;
	}

	// Calculate the upper schema offset

	const QRect pageRect = m_pdfWriter.pageLayout().paintRectPixels(m_pdfWriter.resolution());

	const QRect contentRect = QRect(QPoint(0, 0), textDocument->size().toSize());

	const int schemaTop = contentRect.height() % pageRect.height();

	int schemaLeft = 0;

	const int schemaMaxHeight = pageRect.height() - schemaTop;

	// Calculate draw parameters

	schemaView->setSchema(schema, true);

	VFrame30::CDrawParam drawParam(painter, schema.get(), schemaView, schema->gridSize(), schema->pinGridStep());
	drawParam.setInfoMode(false);
	drawParam.session() = schemaView->session();

	double schemaWidthInPixel = schema->GetDocumentWidth(m_pdfWriter.resolution(), 100.0);		// Export 100% zoom
	double schemaHeightInPixel = schema->GetDocumentHeight(m_pdfWriter.resolution(), 100.0);		// Export 100% zoom

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

	schemaView->adjust(painter, schemaLeft, schemaTop, zoom * 100.0);		// Export 100% zoom

	// Draw Schema
	//
	QRectF clipRect(0, 0, schema->docWidth(), schema->docHeight());

	schema->Draw(&drawParam, clipRect);

	drawParam.setControlBarSize(
		schema->unit() == VFrame30::SchemaUnit::Display ?
					(4 / zoom) : (mm2in(2.4) / zoom));

	schemaView->drawCompareOutlines(&drawParam, clipRect, compareActions);

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

void ReportGenerator::drawMarginItems(int page, QPainter* painter)
{
	if (painter == nullptr)
	{
		Q_ASSERT(painter);
	}

	const QRect fullPageRect = m_pdfWriter.pageLayout().fullRectPixels(m_pdfWriter.resolution());

	const QRect pageRect = m_pdfWriter.pageLayout().paintRectPixels(m_pdfWriter.resolution());

	QMargins margins = m_pdfWriter.pageLayout().marginsPixels(m_pdfWriter.resolution());

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
			{
				QMutexLocker l(&m_statisticsMutex);
				text = tr("Page %1 of %2").arg(page).arg(m_pagesCount);
			}
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

//
// FileDiff
//

bool FileDiff::diff(const QByteArray& sourceData, const QByteArray& targetData, std::vector<FileLine>* sourceDiff, std::vector<FileLine>* targetDiff)
{
	if (sourceDiff == nullptr || targetDiff == nullptr)
	{
		Q_ASSERT(sourceDiff);
		Q_ASSERT(targetDiff);
		return false;
	}

	std::vector<FileLine> fileLinesSource;
	std::vector<FileLine> fileLinesTarget;
	std::vector<FileLine> fileLinesCommon;

	loadFileData(sourceData, &fileLinesSource);
	loadFileData(targetData, &fileLinesTarget);

	fileLinesCommon.reserve(static_cast<int>(fileLinesSource.size()));

	calculateLcs(fileLinesSource, fileLinesTarget, &fileLinesCommon);

	for (const FileLine& fileLine : fileLinesSource)
	{
		bool commonLineExists = false;

		for (const FileLine& commonLine : fileLinesCommon)
		{
			if (fileLine.hash == commonLine.hash)
			{
				commonLineExists = true;
				break;
			}
		}

		if (commonLineExists == false)
		{
			sourceDiff->push_back(fileLine);
		}
	}

	for (const FileLine& fileLine : fileLinesTarget)
	{
		bool commonLineExists = false;

		for (const FileLine& commonLine : fileLinesCommon)
		{
			if (fileLine.hash == commonLine.hash)
			{
				commonLineExists = true;
				break;
			}
		}

		if (commonLineExists == false)
		{
			targetDiff->push_back(fileLine);
		}
	}

	return true;
}

void FileDiff::loadFileData(const QByteArray& fileData, std::vector<FileLine>* fileLines)
{
	if (fileLines == nullptr)
	{
		Q_ASSERT(fileLines);
		return;
	}

	QString string = fileData;

	QStringList strings = string.split(QChar::LineFeed);

	int count =  strings.size();

	fileLines->resize(count);

	for (int i = 0; i < count; i++)
	{
		FileLine& fileLine = (*fileLines)[i];

		fileLine.text = strings[i];
		fileLine.line = i + 1;
		fileLine.hash = ::calcHash(fileLine.text);
	}

	return;
}

template<typename T> void FileDiff::calculateLcs(const std::vector<T>& X, const std::vector<T>& Y, std::vector<T>* result)
{
	if (result == NULL)
	{
		Q_ASSERT(result);
		return;
	}

	int m = static_cast<int>(X.size());
	int n = static_cast<int>(Y.size());

	std::vector<std::vector<int>> L(m + 1, std::vector<int>(n + 1));

	/* Following steps build L[m+1][n+1] in bottom up fashion. Note
		that L[i][j] contains length of LCS of X[0..i-1] and Y[0..j-1] */
	for (int i = 0; i <= m; i++)
	{
		for (int j = 0; j <= n; j++)
		{
			if (i == 0 || j == 0)
			{
				L[i][j] = 0;
			}
			else
			{
				if (X[i - 1] == Y[j - 1])
				{
					L[i][j] = L[i - 1][j - 1] + 1;
				}
				else
				{
					L[i][j] = std::max<int>(L[i - 1][j], L[i][j - 1]);
				}
			}
		}
	}

	// Following code is used to print LCS
	int index = L[m][n];

	// Create a character array to store the lcs string
	result->resize(index/* + 1*/);
	//(*result)[index] = '\0'; // Set the terminating character

	// Start from the right-most-bottom-most corner and
	// one by one store characters in lcs[]
	int i = m;
	int j = n;
	while (i > 0 && j > 0)
	{
		// If current character in X[] and Y are same, then
		// current character is part of LCS
		if (X[i - 1] == Y[j - 1])
		{
			(*result)[index-1] = X[i-1]; // Put current character in result
			i--;
			j--;
			index--;     // reduce values of i, j and index
		}

		// If not same, then find the larger of two and
		// go in the direction of larger value
		else
		{
			if (L[i-1][j] > L[i][j-1])
			{
				i--;
			}
			else
			{
				j--;
			}
		}
	}
}

//
// ProjectDiffGenerator
//

void ProjectDiffGenerator::run(const CompareData& compareData, std::map<int, QString> fileTypesMap, const QString& projectName, const QString& userName, const QString& userPassword, QWidget* parent)
{
	// Get filename
	//

	QString fileName = QFileDialog::getSaveFileName(parent, QObject::tr("Diff Report"),
													"./",
													QObject::tr("PDF documents (*.pdf)"));

	if (fileName.isNull() == true)
	{
		return;
	}

	std::shared_ptr<ReportSchemaView> schemaView = std::make_shared<ReportSchemaView>(parent);

	// Create Worker

	std::shared_ptr<ProjectDiffWorker> worker = std::make_shared<ProjectDiffWorker>(fileName, compareData, fileTypesMap, schemaView, projectName, userName, userPassword);

	// Create Progress Dialog

	DialogProjectDiffProgress* dialogProgress = new DialogProjectDiffProgress(worker, parent);

	// Create thread

	QThread* thread = new QThread;

	worker->moveToThread(thread);

	QObject::connect(thread, &QThread::started, worker.get(), &ProjectDiffWorker::process);

	QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);	// Schedule thread deleting

	//  Schedule objects deleting

	QObject::connect(worker.get(), &ProjectDiffWorker::finished, [&thread, &dialogProgress, &worker, &schemaView]()
	{
		thread->quit();

		dialogProgress->deleteLater();	// worker will be destroyed here, because dialog contains shared pointer to worker

		//worker->deleteLater();
		//worker = nullptr;

		//schemaView->deleteLater();
		//schemaView = nullptr;
	});

	// Start thread

	thread->start();

	dialogProgress->exec();

	return;

}

std::map<int, QString> ProjectDiffGenerator::filesTypesNamesMap(DbController* db)
{
	std::map<int, QString> result;

	result[applicationSignalsTypeId()] = QObject::tr("Application Signals");

	result[db->busTypesFileId()] = QObject::tr("Busses");
	result[db->schemaFileId()] = QObject::tr("Schemas");
	result[db->afblFileId()] = QObject::tr("AFBL Descriptions ");
	result[db->ufblFileId()] = QObject::tr("UFBL Descriptions");
	result[db->alFileId()] = QObject::tr("Application Logic");
	result[db->hcFileId()] = QObject::tr("Hardware Configuration");
	result[db->hpFileId()] = QObject::tr("Hardware Presets");
	result[db->mcFileId()] = QObject::tr("Module Configuration");
	result[db->mvsFileId()] = QObject::tr("Monitor Schemas");
	result[db->tvsFileId()] = QObject::tr("Tuning Schemas");
	result[db->dvsFileId()] = QObject::tr("Diagnostics Schemas");
	result[db->connectionsFileId()] = QObject::tr("Connections");
	result[db->etcFileId()] = QObject::tr("Other Files");
	result[db->testsFileId()] = QObject::tr("Tests");
	result[db->simTestsFileId()] = QObject::tr("Simulator Tests");

	return result;
}

//
// ProjectDiffWorker
//

ProjectDiffWorker::ProjectDiffWorker(const QString& fileName,
									 const CompareData& compareData, std::map<int, QString> fileTypesMap,
									 std::shared_ptr<ReportSchemaView> schemaView,
									 const QString& projectName,
									 const QString& userName,
									 const QString& userPassword):
	ReportGenerator(fileName, schemaView),
	m_compareData(compareData),
	m_fileTypesNamesMap(fileTypesMap),
	m_projectName(projectName),
	m_userName(userName),
	m_userPassword(userPassword)
{
	return;
}

ProjectDiffWorker::~ProjectDiffWorker()
{
	qDebug() << "ProjectDiffWorker deleted";
}

void ProjectDiffWorker::process()
{
	compareProject();

	emit finished();
	return;
}

void ProjectDiffWorker::stop()
{
	m_stop = true;
}

ProjectDiffWorker::WorkerStatus ProjectDiffWorker::currentStatus() const
{
	QMutexLocker l(&m_statisticsMutex);
	return m_currentStatus;
}

int ProjectDiffWorker::signalsCount() const
{
	QMutexLocker l(&m_statisticsMutex);
	return m_signalsCount;
}

int ProjectDiffWorker::signalIndex() const
{
	QMutexLocker l(&m_statisticsMutex);
	return m_signalIndex;
}

int ProjectDiffWorker::filesCount() const
{
	QMutexLocker l(&m_statisticsMutex);
	return m_filesCount;
}

int ProjectDiffWorker::fileIndex() const
{
	QMutexLocker l(&m_statisticsMutex);
	return m_fileIndex;
}

int ProjectDiffWorker::sectionCount() const
{
	QMutexLocker l(&m_statisticsMutex);
	return m_sectionCount;
}

int ProjectDiffWorker::sectionIndex() const
{
	QMutexLocker l(&m_statisticsMutex);
	return m_sectionIndex;
}

int ProjectDiffWorker::pagesCount() const
{
	QMutexLocker l(&m_statisticsMutex);
	return m_pagesCount;
}

int ProjectDiffWorker::pageIndex() const
{
	QMutexLocker l(&m_statisticsMutex);
	return m_pageIndex;
}

QString ProjectDiffWorker::currentSection() const
{
	QMutexLocker l(&m_statisticsMutex);
	return m_currentSectionName;
}

QString ProjectDiffWorker::currentObjectName() const
{
	QMutexLocker l(&m_statisticsMutex);
	return m_currentObjectName;
}

DbController* const ProjectDiffWorker::db()
{
	return &m_db;
}

void ProjectDiffWorker::compareProject()
{
	{
		QMutexLocker l(&m_statisticsMutex);
		m_currentStatus = WorkerStatus::Analyzing;
	}

	db()->disableProgress();

	db()->setHost(theSettings.serverIpAddress());
	db()->setPort(theSettings.serverPort());
	db()->setServerUsername(theSettings.serverUsername());
	db()->setServerPassword(theSettings.serverPassword());

	bool ok = db()->openProject(m_projectName, m_userName, m_userPassword, nullptr);
	if (ok == false)
	{
		qDebug() << "Failed to open project!";
		Q_ASSERT(ok);
		return;
	}

	// Get and count files
	//
	m_sourceDeviceObjectMap.clear();
	m_targetDeviceObjectMap.clear();

	std::vector<DbFileInfo> rootFiles;

	ok = db()->getFileList(&rootFiles, db()->rootFileId(), false, nullptr);
	if (ok == false)
	{
		Q_ASSERT(ok);
		return;
	}

	{
		QMutexLocker l(&m_statisticsMutex);
		m_filesCount = 0;
		m_signalsCount = 0;
		m_currentSectionName.clear();
		m_currentObjectName.clear();
		m_pageIndex = 1;
	}


	int filesCount = 0;

	std::vector<DbFileTree> filesTrees;

	for (int i = 0; i < static_cast<int>(rootFiles.size()); i++)
	{
		const DbFileInfo& rootFile = rootFiles[i];

		auto typeIt = m_fileTypesNamesMap.find(rootFile.fileId());
		if (typeIt == m_fileTypesNamesMap.end())
		{
			continue;
		}

		{
			QMutexLocker l(&m_statisticsMutex);
			m_currentSectionName = rootFile.fileName();
		}

		filesTrees.push_back({});

		DbFileTree* filesTree = &filesTrees[filesTrees.size() - 1];

		ok = db()->getFileListTree(filesTree, rootFile.fileId(), false/*removeDeleted*/, nullptr);
		if (ok == false)
		{
			Q_ASSERT(ok);
			return;
		}

		filesCount += static_cast<int>(filesTree->files().size());

	}

	// Get and count signals

	QVector<int> signalIDs;

	{
		QMutexLocker l(&m_statisticsMutex);
		m_currentObjectName = tr("Application Signals");
	}

	ok = db()->getSignalsIDs(&signalIDs, nullptr);
	if (ok == false)
	{
		Q_ASSERT(ok);
		return;
	}

	{
		QMutexLocker l(&m_statisticsMutex);
		m_filesCount = filesCount;
		m_signalsCount = signalIDs.size();
	}

	// Init fonts

	m_headerFont = QFont("Times", 72, QFont::Bold);
	m_normalFont = QFont("Times", 48);
	m_tableFont = QFont("Times", 24);
	m_errorFont = QFont("Courier", 24);
	m_marginFont = QFont("Times", 12);

	// Init document

	m_pdfWriter.setPageSize(QPageSize(QPageSize::A4));
	m_pdfWriter.setPageOrientation(QPageLayout::Portrait);
	m_pdfWriter.setPageMargins(QMargins(25, 15, 15, 15), QPageLayout::Unit::Millimeter);
	m_pdfWriter.setResolution(300);

	QPainter painter(&m_pdfWriter);

	// Create title page and margins
	{
		QTextDocument textDocument;

		QRect pageRectPixels = m_pdfWriter.pageLayout().paintRectPixels(m_pdfWriter.resolution());
		textDocument.setPageSize(QSizeF(pageRectPixels.width(), pageRectPixels.height()));

		QTextCursor textCursor(&textDocument);

		m_currentCharFormat = textCursor.charFormat();
		m_currentBlockFormat = textCursor.blockFormat();

		QTextCharFormat charFormat = textCursor.charFormat();
		charFormat.setFontPointSize(40);
		textCursor.setCharFormat(charFormat);

		// Generate title page

		generateTitlePage(&textCursor);

		createMarginItems(&textCursor);

		{
			QMutexLocker l(&m_statisticsMutex);
			m_pageIndex = 1;
		}

		printDocument(&m_pdfWriter, &textDocument, &painter);
	}

	// Process Files

	for (const DbFileTree& filesTree : filesTrees)
	{
		if (m_stop == true)
		{
			return;
		}

		// Print section name

		QString sectionName;

		auto typeIt = m_fileTypesNamesMap.find(filesTree.rootFile()->fileId());
		if (typeIt == m_fileTypesNamesMap.end())
		{
			Q_ASSERT(false);
			sectionName = filesTree.rootFile()->fileName();
		}
		else
		{
			sectionName = typeIt->second;
		}

		{
			QMutexLocker l(&m_statisticsMutex);
			m_currentSectionName = sectionName;
		}

		std::shared_ptr<ReportSection> section = std::make_shared<ReportSection>();
		m_sections.push_back(section);

		setFont(m_normalFont);

		saveFormat();
		setTextAlignment(Qt::AlignHCenter);
		setFont(m_headerFont);
		section->addText(tr("%1\n\n").arg(sectionName), m_currentCharFormat, m_currentBlockFormat);
		restoreFormat();

		section->addText(tr("Objects with Differences\n"), m_currentCharFormat, m_currentBlockFormat);

		saveFormat();
		setFont(m_tableFont);
		std::shared_ptr<ReportTable> headerTable = section->addTable({tr("Object"), tr("Status"), tr("Latest Changeset")}, m_currentCharFormat);
		restoreFormat();

		// Compare files

		compareFilesRecursive(filesTree, filesTree.rootFile(), m_compareData, headerTable.get());

		// Remove header if no data

		if (headerTable->rowCount() == 0)
		{
			m_sections.pop_back();
		}
	}

	// Process signals
	//
	auto typeIt = m_fileTypesNamesMap.find(ProjectDiffGenerator::applicationSignalsTypeId());
	if (typeIt != m_fileTypesNamesMap.end())
	{

		{
			QMutexLocker l(&m_statisticsMutex);
			m_currentSectionName = tr("Application Signals");
			m_currentObjectName.clear();
		}

		std::shared_ptr<ReportSection> section = std::make_shared<ReportSection>();
		m_sections.push_back(section);

		setFont(m_normalFont);

		saveFormat();
		setTextAlignment(Qt::AlignHCenter);
		setTextForeground(QBrush(Qt::red));
		setTextBackground(QBrush(Qt::black));
		setFont(m_headerFont);
		section->addText(tr("Application Signals\n\n"), m_currentCharFormat, m_currentBlockFormat);
		restoreFormat();

		section->addText(tr("Signals with Differences\n"), m_currentCharFormat, m_currentBlockFormat);

		saveFormat();
		setFont(m_tableFont);
		std::shared_ptr<ReportTable> headerTable = section->addTable({tr("Signal"), tr("Status"), tr("Latest Changeset")}, m_currentCharFormat);
		restoreFormat();

		for (int signalID : signalIDs)
		{
			if (m_stop == true)
			{
				return;
			}

			compareSignal(signalID, m_compareData, headerTable.get());
		}

		if (headerTable->rowCount() == 0)
		{
			m_sections.pop_back();
		}
	}

	{
		QMutexLocker l(&m_statisticsMutex);
		m_pagesCount = 1;	// + title page
	}

	// Render all objects to documents
	//
	{
		QMutexLocker l(&m_statisticsMutex);
		m_currentStatus = WorkerStatus::Rendering;

		m_sectionCount = static_cast<int>(m_sections.size());
	}

	for (std::shared_ptr<ReportSection> section : m_sections)
	{
		{
			QMutexLocker l(&m_statisticsMutex);
			m_sectionIndex++;
		}

		QRect pageRectPixels = m_pdfWriter.pageLayout().paintRectPixels(m_pdfWriter.resolution());

		section->render(QSizeF(pageRectPixels.width(), pageRectPixels.height()));

		{
			QMutexLocker l(&m_statisticsMutex);
			m_pagesCount += section->pageCount();
		}
	}

	// Print PDF

	{
		QMutexLocker l(&m_statisticsMutex);
		m_currentStatus = WorkerStatus::Printing;

		m_sectionCount = m_pagesCount;
	}

	for (std::shared_ptr<ReportSection> section : m_sections)
	{
		// Print Text Document

		m_pdfWriter.newPage();

		{
			QMutexLocker l(&m_statisticsMutex);
			m_pageIndex++;
		}

		printDocument(&m_pdfWriter, section->textDocument(), &painter);

		// Print Schemas

		std::shared_ptr<VFrame30::Schema> schema = section->schema();
		if (schema != nullptr)
		{
			printSchema(section->textDocument(), &painter, m_schemaView.get(), schema, section->compareItemActions());
		}

		// Clear text document

		section->textDocument()->clear();

	}

	return;
}

void ProjectDiffWorker::compareFilesRecursive(const DbFileTree& filesTree, const std::shared_ptr<DbFileInfo>& fi, const CompareData& compareData, ReportTable* const headerTable)
{
	if (m_stop == true)
	{
		return;
	}

	if (headerTable == nullptr)
	{
		Q_ASSERT(headerTable);
		return;
	}

	if (fi == nullptr)
	{
		Q_ASSERT(fi);
		return;
	}

	compareFile(filesTree, fi, compareData, headerTable);

	// Process children
	//
	int fileId = fi->fileId();

	int childrenCount = filesTree.childrenCount(fileId);
	for (int i = 0; i < childrenCount; i++)
	{
		std::shared_ptr<DbFileInfo> fiChild = filesTree.child(fileId, i);
		if (fiChild == nullptr)
		{
			Q_ASSERT(fiChild);
			return;
		}

		compareFilesRecursive(filesTree, fiChild, compareData, headerTable);
	}

	return;
}

void ProjectDiffWorker::compareFile(const DbFileTree& filesTree,
				 const std::shared_ptr<DbFileInfo>& fi,
				 const CompareData& compareData,
				 ReportTable* const headerTable)
{
	// Print file name
	//
	QStringList pathList;
	std::shared_ptr<DbFileInfo> f = fi;

	while (f != nullptr)
	{
		pathList.push_front(f->fileName());

		if (f->fileId() == filesTree.rootFileId())
		{
			break;
		}

		f = filesTree.file(f->parentId());
	}

	QString fileName = QChar('/') + pathList.join(QChar('/'));

	{
		QMutexLocker l(&m_statisticsMutex);
		m_currentObjectName = fileName;
		m_fileIndex++;
	}

	// Get file history
	//
	std::vector<DbChangeset> fileHistory;

	bool ok = db()->getFileHistory(*fi, &fileHistory, nullptr);
	if (ok == false)
	{
		//diffDataSet.addText(tr("Get file history for %1 failed.").arg(fi->fileName()));
		return;
	}

	// Get source file
	//
	std::shared_ptr<DbFile> sourceFile;

	if (compareData.sourceVersionType == CompareVersionType::LatestVersion)
	{
		ok = db()->getLatestVersion(*fi, &sourceFile, nullptr);
	}
	else
	{
		std::optional<DbChangeset> changesetOpt = getRecentChangeset(fileHistory, compareData.sourceVersionType, compareData.sourceChangeset, compareData.sourceDate);

		if (changesetOpt.has_value() == true)
		{
			ok = db()->getSpecificCopy(*fi, changesetOpt.value().changeset(), &sourceFile, nullptr);
		}
		else
		{
			ok = true;
		}
	}

	if (ok == false)
	{
		headerTable->insertRow({fi->fileName(), tr("Get source copy failed"), QString()});
		return;
	}

	// Get target file
	//
	std::shared_ptr<DbFile> targetFile;

	if (compareData.targetVersionType == CompareVersionType::LatestVersion)
	{
		ok = db()->getLatestVersion(*fi, &targetFile, nullptr);
	}
	else
	{
		std::optional<DbChangeset> changesetOpt = getRecentChangeset(fileHistory, compareData.targetVersionType, compareData.targetChangeset, compareData.targetDate);

		if (changesetOpt.has_value() == true)
		{
			ok = db()->getSpecificCopy(*fi, changesetOpt.value().changeset(), &targetFile, nullptr);
		}
		else
		{
			ok = true;
		}
	}

	if (ok == false)
	{
		headerTable->insertRow({fi->fileName(), tr("Get target copy failed"), QString()});
		return;
	}

	// Target changeset should be later or checked-out - swap files if needed
	//
	if (sourceFile != nullptr && targetFile != nullptr)
	{
		if (sourceFile->changeset() == 0 || targetFile->changeset() == 0)
		{
			// One of files is checked out
			//
			if (sourceFile->changeset() == 0 && targetFile->changeset() != 0)
			{
				sourceFile.swap(targetFile);
			}
		}
		else
		{
			if (sourceFile->changeset() > targetFile->changeset())
			{
				sourceFile.swap(targetFile);
			}
		}
	}

	// Compare files

	compareFileContents(sourceFile, targetFile, fileName, headerTable);

	return;
}

void ProjectDiffWorker::compareFileContents(const std::shared_ptr<DbFile>& sourceFile,
											   const std::shared_ptr<DbFile>& targetFile,
											   const QString& fileName,
											   ReportTable* const headerTable)
{
	if (headerTable == nullptr)
	{
		Q_ASSERT(headerTable);
		return;
	}

	// No files at all
	//
	if (sourceFile == nullptr && targetFile == nullptr)
	{
		return;
	}

	// Load hardware objects
	//
	std::shared_ptr<Hardware::DeviceObject> sourceObject;
	std::shared_ptr<Hardware::DeviceObject> targetObject;

	bool hardwareObject = isHardwareFile(fileName);

	if (hardwareObject == true)
	{
		if (sourceFile != nullptr)
		{
			sourceObject = loadDeviceObject(sourceFile, &m_sourceDeviceObjectMap);
			if (sourceObject == nullptr)
			{
				Q_ASSERT(sourceObject);
				return;
			}
		}

		if (targetFile != nullptr)
		{
			targetObject = loadDeviceObject(targetFile, &m_targetDeviceObjectMap);
			if (targetObject == nullptr)
			{
				Q_ASSERT(targetObject);
				return;
			}
		}
	}

	// Same changeset
	//
	if (sourceFile != nullptr &&
		targetFile != nullptr &&
		sourceFile->changeset() == targetFile->changeset())
	{
		return;
	}

	// File was deleted
	//
	if (sourceFile != nullptr && sourceFile->deleted() == true)
	{
		if (hardwareObject == true)
		{
			headerTable->insertRow({sourceObject->equipmentId(), tr("Deleted"), changesetString(sourceFile)});
		}
		else
		{
			headerTable->insertRow({fileName, tr("Deleted"), changesetString(sourceFile)});
		}
		return;
	}
	else
	{
		if (targetFile != nullptr && targetFile->deleted() == true)
		{
			if (hardwareObject == true)
			{
				headerTable->insertRow({targetObject->equipmentId(), tr("Deleted"), changesetString(targetFile)});
			}
			else
			{
				headerTable->insertRow({fileName, tr("Deleted"), changesetString(targetFile)});
			}
			return;
		}
	}



	// Compare contents
	//
	if (hardwareObject == true)
	{
		compareDeviceObjects(sourceFile, targetFile, sourceObject, targetObject, headerTable);
		return;
	}

	if (isConnectionFile(fileName) == true)
	{
		compareConnections(sourceFile, targetFile, headerTable);
		return;
	}

	if (isBusTypeFile(fileName) == true)
	{
		compareBusTypes(sourceFile, targetFile, headerTable);
		return;
	}

	if (isSchemaFile(fileName) == true)
	{
		compareSchemas(sourceFile, targetFile, headerTable);
		return;
	}

	compareFilesData(sourceFile, targetFile);
	return;
}

std::optional<DbChangeset> ProjectDiffWorker::getRecentChangeset(const std::vector<DbChangeset>& history,
																	const CompareVersionType versionType,
																	const int compareChangeset,
																	const QDateTime& compareDate) const
{
	// Get source changeset and file contents
	//
	DbChangeset resultChangeset;
	bool changesetExists = false;

	switch (versionType)
	{
	case CompareVersionType::Changeset:
		{
			for (const DbChangeset& cs: history)
			{
				if (cs.changeset() > compareChangeset)
				{
					continue;
				}

				if (changesetExists == false || resultChangeset.changeset() < cs.changeset())
				{
					resultChangeset = cs;
					changesetExists = true;
				}
			}
		}
		break;
	case CompareVersionType::Date:
		{
			for (const DbChangeset& cs : history)
			{
				if (cs.date() > compareDate)
				{
					continue;
				}

				if (changesetExists == false || resultChangeset.date() < cs.date())
				{
					resultChangeset = cs;
					changesetExists = true;
				}
			}
		}
		break;
	case CompareVersionType::LatestVersion:
		{
			Q_ASSERT(false);
		}
		break;
	}

	if (changesetExists == true)
	{
		return resultChangeset;
	}

	return {};

}

std::shared_ptr<Hardware::DeviceObject> ProjectDiffWorker::loadDeviceObject(const std::shared_ptr<DbFile>& file, std::map<int, std::shared_ptr<Hardware::DeviceObject>>* const deviceObjectMap)
{
	if (deviceObjectMap == nullptr)
	{
		Q_ASSERT(deviceObjectMap);
		return nullptr;
	}

	std::shared_ptr<Hardware::DeviceObject> object = Hardware::DeviceObject::fromDbFile(*file);
	if (object == nullptr)
	{
		Q_ASSERT(object);
		return nullptr;
	}

	// Save object to the map

	(*deviceObjectMap)[object->fileId()] = object;

	//qDebug() << object->fileId() << object->equipmentIdTemplate() << object->equipmentId();

	// Get pointers to parent and expand Equipment ID

	auto it = deviceObjectMap->find(file->parentId());
	if (it != deviceObjectMap->end())
	{
		std::shared_ptr<Hardware::DeviceObject> parentObject = it->second;
		if (parentObject == nullptr)
		{
			Q_ASSERT(parentObject);
		}
		else
		{
			parentObject->addChild(object);

			//qDebug() << parentObject->fileId() << parentObject->equipmentIdTemplate() << parentObject->equipmentId();

			object->expandEquipmentId();

			//qDebug() << object->fileId() << object->equipmentIdTemplate() << object->equipmentId();
		}
	}
	/*
	else
	{
		qDebug() << "No parent file for " << object->equipmentIdTemplate();
	}
	*/

	return object;
}

void ProjectDiffWorker::compareDeviceObjects(const std::shared_ptr<DbFile>& sourceFile,
											 const std::shared_ptr<DbFile>& targetFile,
											 const std::shared_ptr<Hardware::DeviceObject>& sourceObject,
											 const std::shared_ptr<Hardware::DeviceObject>& targetObject,
											 ReportTable* const headerTable)
{
	if (headerTable == nullptr)
	{
		Q_ASSERT(headerTable);
		return;
	}

	// No Files
	//
	if (sourceFile == nullptr && targetFile == nullptr)
	{
		Q_ASSERT(sourceFile != nullptr || targetFile != nullptr);
		return;
	}

	if (sourceFile != nullptr && sourceObject == nullptr)
	{
		Q_ASSERT(sourceObject);
		return;
	}

	if (targetFile != nullptr && targetObject == nullptr)
	{
		Q_ASSERT(targetObject);
		return;
	}

	// Single object
	//
	if (sourceObject != nullptr && targetObject == nullptr)
	{
		headerTable->insertRow({tr("%1").arg(sourceObject->equipmentId()), tr("Added"),  changesetString(sourceFile)});
		return;
	}
	else
	{
		if (sourceObject == nullptr && targetObject != nullptr)
		{
			headerTable->insertRow({tr("%1").arg(targetObject->equipmentId()), tr("Added"),  changesetString(targetFile)});
			return;
		}
	}

	// Both Objects
	//
	std::vector<PropertyDiff> diffs;

	comparePropertyObjects(*sourceObject, *targetObject, &diffs);

	if (diffs.empty() == false)
	{
		headerTable->insertRow({tr("%1").arg(targetObject->equipmentId()), targetFile->action().text(),  changesetString(targetFile)});

		std::shared_ptr<ReportSection> section = std::make_shared<ReportSection>();
		m_sections.push_back(section);

		section->addText(targetObject->equipmentId(), m_currentCharFormat, m_currentBlockFormat);

		saveFormat();
		setFont(m_tableFont);
		std::shared_ptr<ReportTable> diffTable = section->addTable({tr("Property"), tr("Status"), tr("Old Value"), tr("New Value")}, m_currentCharFormat);
		restoreFormat();

		fillDiffTable(diffTable.get(), diffs);
	}

	return;
}

void ProjectDiffWorker::compareBusTypes(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile, ReportTable* const headerTable)
{
	if (headerTable == nullptr)
	{
		Q_ASSERT(headerTable);
		return;
	}

	// No Files
	if (sourceFile == nullptr && targetFile == nullptr)
	{
		Q_ASSERT(sourceFile != nullptr || targetFile != nullptr);
		return;
	}

	VFrame30::Bus sourceBus;
	VFrame30::Bus targetBus;

	bool ok = false;

	if (sourceFile != nullptr)
	{
		ok = sourceBus.Load(sourceFile->data());
		if (ok == false)
		{
			Q_ASSERT(ok);
			return;
		}
	}

	if (targetFile != nullptr)
	{
		ok = targetBus.Load(targetFile->data());
		if (ok == false)
		{
			Q_ASSERT(ok);
			return;
		}
	}

	// Single object
	//
	if ((sourceFile != nullptr && targetFile == nullptr) ||
		(sourceFile == nullptr && targetFile != nullptr))
	{
		auto singleFile = sourceFile != nullptr ? sourceFile : targetFile;
		auto* singleBus= sourceFile != nullptr ? &sourceBus : &targetBus;
		headerTable->insertRow({singleBus->busTypeId(), tr("Added"),  changesetString(singleFile)});
		return;
	}

	// Both Files
	//
	// Create tables

	saveFormat();
	setFont(m_tableFont);
	QStringList busHeaderLabels = {tr("Property"), tr("Status"), tr("Old Value"), tr("New Value")};
	std::shared_ptr<ReportTable> busDiffTable = std::make_shared<ReportTable>(busHeaderLabels, m_currentCharFormat);
	restoreFormat();

	saveFormat();
	setFont(m_tableFont);
	QStringList busSignalsHeaderLabels = {tr("SignalID"), tr("Caption"), tr("Status")};
	std::shared_ptr<ReportTable> busSignalsDiffTable = std::make_shared<ReportTable>(busSignalsHeaderLabels, m_currentCharFormat);
	restoreFormat();

	std::vector<PropertyDiff> busDiffs;

	// Compare bus properties

	comparePropertyObjects(sourceBus, targetBus, &busDiffs);

	if (busDiffs.empty() == false)
	{
		fillDiffTable(busDiffTable.get(), busDiffs);
	}

	std::map<QString, std::shared_ptr<ReportTable>> busSignalsPropertiesTables;

	// Compare bus signals

	for (const VFrame30::BusSignal& targetBusSignal : targetBus.busSignals())
	{
		bool busSignalFound = false;

		for (const VFrame30::BusSignal& sourceBusSignal : sourceBus.busSignals())
		{
			if (targetBusSignal.signalId() == sourceBusSignal.signalId())
			{
				std::vector<PropertyDiff> busSignalDiffs;

				comparePropertyObjects(sourceBusSignal, targetBusSignal, &busSignalDiffs);

				if (busSignalDiffs.empty() == false)
				{
					saveFormat();
					setFont(m_tableFont);
					QStringList busSignalsPropertiesHeaderLabels = {tr("Property"), tr("Status"), tr("Old Value"), tr("New Value")};
					std::shared_ptr<ReportTable> busSignalsPropertiesDiffTable = std::make_shared<ReportTable>(busSignalsPropertiesHeaderLabels, m_currentCharFormat);
					restoreFormat();

					busSignalsPropertiesTables[targetBusSignal.signalId()] = busSignalsPropertiesDiffTable;

					fillDiffTable(busSignalsPropertiesDiffTable.get(), busSignalDiffs);

					busSignalsDiffTable->insertRow({targetBusSignal.signalId(), targetBusSignal.caption(), tr("Modified")});
				}

				busSignalFound = true;
				break;
			}
		}

		if (busSignalFound == false)
		{
			// Bus signal was added
			busSignalsDiffTable->insertRow({targetBusSignal.signalId(), targetBusSignal.caption(), tr("Added")});
		}
	}

	for (const VFrame30::BusSignal& sourceBusSignal : sourceBus.busSignals())
	{
		bool busSignalFound = false;

		for (const VFrame30::BusSignal& targetBusSignal : targetBus.busSignals())
		{
			if (targetBusSignal.signalId() == sourceBusSignal.signalId())
			{
				busSignalFound = true;
				break;
			}
		}

		if (busSignalFound == false)
		{
			// Bus signal was deleted
			busSignalsDiffTable->insertRow({sourceBusSignal.signalId(), sourceBusSignal.caption(), tr("Deleted")});
		}
	}

	// Add tables to section

	if (busDiffTable->rowCount() > 0 || busSignalsDiffTable->rowCount() > 0 || busSignalsPropertiesTables.empty() == false)
	{
		headerTable->insertRow({targetBus.busTypeId(), targetFile->action().text(),  changesetString(targetFile)});

		// Add tables to section

		std::shared_ptr<ReportSection> busDiffSection = std::make_shared<ReportSection>();
		m_sections.push_back(busDiffSection);

		busDiffSection->addText(tr("Bus: %1\n").arg(targetBus.busTypeId()), m_currentCharFormat, m_currentBlockFormat);

		if (busDiffTable->rowCount() != 0)
		{
			busDiffSection->addTable(busDiffTable);
		}

		if (busSignalsDiffTable->rowCount() != 0)
		{
			busDiffSection->addText(tr("Bus %1 signals:\n").arg(targetBus.busTypeId()), m_currentCharFormat, m_currentBlockFormat);
			busDiffSection->addTable(busSignalsDiffTable);
		}

		for (auto it : busSignalsPropertiesTables)
		{
			const QString& signalId = it.first;
			const std::shared_ptr<ReportTable>& itemDiffTable = it.second;

			busDiffSection->addText(tr("Bus: %1, signal: %2\n").arg(targetBus.busTypeId()).arg(signalId), m_currentCharFormat, m_currentBlockFormat);
			busDiffSection->addTable(itemDiffTable);
		}
	}

	return;
}


void ProjectDiffWorker::compareSchemas(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile, ReportTable* const headerTable)
{
	if (headerTable == nullptr)
	{
		Q_ASSERT(headerTable);
		return;
	}

	// No Files
	if (sourceFile == nullptr && targetFile == nullptr)
	{
		Q_ASSERT(sourceFile != nullptr || targetFile != nullptr);
		return;
	}

	// Single File
	//
	if ((sourceFile != nullptr && targetFile == nullptr) ||
		(sourceFile == nullptr && targetFile != nullptr))
	{
		auto singleFile = sourceFile != nullptr ? sourceFile : targetFile;
		headerTable->insertRow({singleFile->fileName(), tr("Added"),  changesetString(singleFile)});
		return;
	}

	// Both Files
	//
	std::shared_ptr<VFrame30::Schema> sourceSchema = VFrame30::Schema::Create(sourceFile->data());
	if (sourceSchema == nullptr)
	{
		Q_ASSERT(false);
		headerTable->insertRow({sourceFile->fileName(), tr("Loading failed"),  QString()});
		return;
	}

	std::shared_ptr<VFrame30::Schema> targetSchema = VFrame30::Schema::Create(targetFile->data());
	if (targetSchema == nullptr)
	{
		Q_ASSERT(false);
		headerTable->insertRow({targetFile->fileName(), tr("Loading failed"),  QString()});
		return;
	}

	// Create tables

	saveFormat();
	setFont(m_tableFont);
	QStringList schemaHeaderLabels = {tr("Property"), tr("Status"), tr("Old Value"), tr("New Value")};
	std::shared_ptr<ReportTable> schemaDiffTable = std::make_shared<ReportTable>(schemaHeaderLabels, m_currentCharFormat);
	restoreFormat();

	saveFormat();
	setFont(m_tableFont);
	QStringList itemsHeaderLabels = {tr("Item"), tr("Layer"), tr("Status")};
	std::shared_ptr<ReportTable> itemsDiffTable = std::make_shared<ReportTable>(itemsHeaderLabels, m_currentCharFormat);
	restoreFormat();

	// Compare schemas properties

	std::vector<PropertyDiff> schemaDiffs;
	comparePropertyObjects(*sourceSchema, *targetSchema, &schemaDiffs);

	fillDiffTable(schemaDiffTable.get(), schemaDiffs);

	// Compare schemas items properties

	std::map<QUuid, CompareAction> itemsActions;

	std::map<std::shared_ptr<VFrame30::SchemaItem>, std::shared_ptr<ReportTable>> itemsTables;

	for (std::shared_ptr<VFrame30::SchemaLayer> targetLayer : targetSchema->Layers)
	{
		for (SchemaItemPtr targetItem : targetLayer->Items)
		{
			// Look for this item in source
			//
			SchemaItemPtr sourceItem = sourceSchema->getItemById(targetItem->guid());

			if (sourceItem != nullptr)
			{
				// Item is found, so it was modified
				//

				std::vector<PropertyDiff> itemDiffs;

				comparePropertyObjects(*sourceItem, *targetItem, &itemDiffs);

				// Check if properties where modified
				//
				if (itemDiffs.empty() == true)
				{
					// Check if position was changed
					//
					std::vector<VFrame30::SchemaPoint> sourcePoints = sourceItem->getPointList();
					std::vector<VFrame30::SchemaPoint> targetPoints = targetItem->getPointList();

					if (sourcePoints == targetPoints)
					{
						itemsActions[targetItem->guid()] = CompareAction::Unmodified;
					}
					else
					{
						itemsActions[targetItem->guid()] = CompareAction::Modified;
					}
				}
				else
				{
					itemsActions[targetItem->guid()] = CompareAction::Modified;
				}

				// Save properties to table

				if (itemDiffs.empty() == false)
				{
					const std::string& className = targetItem->metaObject()->className();
					itemsDiffTable->insertRow({QString(className.c_str()), targetLayer->name(), tr("Modified")});

					saveFormat();
					setFont(m_tableFont);
					QStringList itemHeaderLabels = {tr("Property"), tr("Status"), tr("Old Value"), tr("New Value")};
					std::shared_ptr<ReportTable> itemDiffTable = std::make_shared<ReportTable>(itemHeaderLabels, m_currentCharFormat);
					restoreFormat();

					fillDiffTable(itemDiffTable.get(), itemDiffs);

					itemsTables[targetItem] = itemDiffTable;
				}

				continue;
			}

			if (sourceItem == nullptr)
			{
				// Item was added to targer
				//
				itemsActions[targetItem->guid()] = CompareAction::Added;

				const std::string& className = targetItem->metaObject()->className();
				itemsDiffTable->insertRow({QString(className.c_str()), targetLayer->name(), tr("Added")});

				continue;
			}
		}
	}

	// Look for deteled items (in target)
	//
	for (std::shared_ptr<VFrame30::SchemaLayer> sourceLayer : sourceSchema->Layers)
	{
		for (SchemaItemPtr sourceItem : sourceLayer->Items)
		{
			// Look for this item in source
			//
			SchemaItemPtr targetItem = targetSchema->getItemById(sourceItem->guid());

			if (targetItem == nullptr)
			{
				// Item is found, so it was deleted in target
				//
				itemsActions[sourceItem->guid()] = CompareAction::Deleted;

				const std::string& className = sourceItem->metaObject()->className();
				itemsDiffTable->insertRow({QString(className.c_str()), sourceLayer->name(), tr("Deleted")});

				// Add item to target
				//
				bool layerFound = false;
				for (std::shared_ptr<VFrame30::SchemaLayer> targetLayer : targetSchema->Layers)
				{
					if (targetLayer->guid() == sourceLayer->guid())
					{
						targetLayer->Items.push_back(sourceItem);
						layerFound = true;
						break;
					}
				}

				Q_ASSERT(layerFound);
			}
		}
	}

	if (schemaDiffTable->rowCount() > 0 || itemsDiffTable->rowCount() > 0 || itemsTables.empty() == false)
	{
		headerTable->insertRow({targetFile->fileName(), targetFile->action().text(),  changesetString(targetFile)});

		// Add tables to section

		std::shared_ptr<ReportSection> schemaDiffSection = std::make_shared<ReportSection>();
		m_sections.push_back(schemaDiffSection);

		schemaDiffSection->addText(tr("Schema: %1\n").arg(targetFile->fileName()), m_currentCharFormat, m_currentBlockFormat);

		if (schemaDiffTable->rowCount() != 0)
		{
			schemaDiffSection->addTable(schemaDiffTable);
		}

		if (itemsDiffTable->rowCount() != 0)
		{
			schemaDiffSection->addText(tr("Items Differences:\n"), m_currentCharFormat, m_currentBlockFormat);
			schemaDiffSection->addTable(itemsDiffTable);
		}

		for (auto it : itemsTables)
		{
			const std::shared_ptr<VFrame30::SchemaItem>& item = it.first;
			const std::shared_ptr<ReportTable>& itemDiffTable = it.second;

			const std::string& className = item->metaObject()->className();
			schemaDiffSection->addText(QString(className.c_str()), m_currentCharFormat, m_currentBlockFormat);

			schemaDiffSection->addTable(itemDiffTable);
		}

		// Add schema differences drawing

		std::shared_ptr<ReportSection> schemaDrawingSection = std::make_shared<ReportSection>();
		m_sections.push_back(schemaDrawingSection);

		schemaDrawingSection->addText(tr("Drawing of Schema: %1\n").arg(targetFile->fileName()), m_currentCharFormat, m_currentBlockFormat);
		schemaDrawingSection->setSchema(targetSchema);
		schemaDrawingSection->setCompareItemActions(itemsActions);
	}

	/*
		std::shared_ptr<ReportSection> sourceSection = std::make_shared<ReportSection>();
		m_sections.push_back(sourceSection);

		sourceSection->addText("Source schema:\n", m_currentCharFormat, m_currentBlockFormat);
		sourceSection->setSchema(sourceSchema);

		std::shared_ptr<ReportSection> targetSection = std::make_shared<ReportSection>();
		m_sections.push_back(targetSection);

		targetSection->addText("Target schema:\n", m_currentCharFormat, m_currentBlockFormat);
		targetSection->setSchema(targetSchema);*/

}

void ProjectDiffWorker::compareConnections(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile, ReportTable* const headerTable)
{
	if (headerTable == nullptr)
	{
		Q_ASSERT(headerTable);
		return;
	}

	// No Files
	if (sourceFile == nullptr && targetFile == nullptr)
	{
		Q_ASSERT(sourceFile != nullptr || targetFile != nullptr);
		return;
	}

	Hardware::Connection sourceConnection;
	Hardware::Connection targetConnection;

	if (sourceFile != nullptr)
	{
		bool ok = sourceConnection.Load(sourceFile->data());
		if (ok == false)
		{
			Q_ASSERT(ok);
			return;
		}
	}
	if (targetFile != nullptr)
	{
		bool ok = targetConnection.Load(targetFile->data());
		if (ok == false)
		{
			Q_ASSERT(ok);
			return;
		}
	}

	// Single object
	//
	if ((sourceFile != nullptr && targetFile == nullptr) ||
		(sourceFile == nullptr && targetFile != nullptr))
	{
		auto singleFile = sourceFile != nullptr ? sourceFile : targetFile;
		auto* singleConnection = sourceFile != nullptr ? &sourceConnection : &targetConnection;
		headerTable->insertRow({singleConnection->connectionID(), tr("Added"),  changesetString(singleFile)});
		return;
	}

	// Both Files
	//
	std::vector<PropertyDiff> diffs;

	comparePropertyObjects(sourceConnection, targetConnection, &diffs);

	if (diffs.empty() == false)
	{
		std::shared_ptr<ReportSection> section = std::make_shared<ReportSection>();
		m_sections.push_back(section);

		section->addText(targetConnection.connectionID(), m_currentCharFormat, m_currentBlockFormat);

		headerTable->insertRow({targetConnection.connectionID(), targetFile->action().text(),  changesetString(targetFile)});

		saveFormat();
		setFont(m_tableFont);
		std::shared_ptr<ReportTable> diffTable = section->addTable({tr("Property"), tr("Status"), tr("Old Value"), tr("New Value")}, m_currentCharFormat);
		restoreFormat();

		fillDiffTable(diffTable.get(), diffs);
	}
}

void ProjectDiffWorker::compareFilesData(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile)
{
	/*// No Files
	if (sourceFile == nullptr && targetFile == nullptr)
	{
		Q_ASSERT(sourceFile != nullptr || targetFile != nullptr);
		return;
	}

	// Single File
	//
	if ((sourceFile != nullptr && targetFile == nullptr) ||
		(sourceFile == nullptr && targetFile != nullptr))
	{
		auto singleFile = sourceFile != nullptr ? sourceFile : targetFile;
		m_textCursor.insertText(tr("\n%1: %2\n").arg(singleFile->fileName()).arg(singleFile->action().text()));
		return;
	}

	// Both Files
	//
	if (isTextFile(targetFile->fileName()) == true)
	{
		std::vector<FileDiff::FileLine> sourceDiff;
		std::vector<FileDiff::FileLine> targetDiff;

		FileDiff::diff(sourceFile->data(), targetFile->data(), &sourceDiff, &targetDiff);

		if (sourceDiff.empty() == true && targetDiff.empty() == true)
		{
			return;
		}

		m_textCursor.insertText(tr("\n%1n").arg(targetFile->fileName()));
		if (sourceDiff.empty() == false)
		{
			m_textCursor.insertText(tr("Source differences:\n"));
		}

		for (const FileDiff::FileLine& fileLine : sourceDiff)
		{
			m_textCursor.insertText(tr("%1 %2\n").arg(fileLine.line).arg(fileLine.text));
		}

		if (targetDiff.empty() == false)
		{
			m_textCursor.insertText(tr("Target differences:\n"));
		}

		for (const FileDiff::FileLine& fileLine : targetDiff)
		{
			m_textCursor.insertText(tr("%1 %2\n").arg(fileLine.line).arg(fileLine.text));
		}
	}
	else
	{
		// Other file
		//
		if (sourceFile->data() != targetFile->data())
		{
			m_textCursor.insertText(tr("\n%1: Binary data changed\n").arg(targetFile->fileName()));
		}
	}

	if (sourceFile->data().size() != targetFile->data().size())
	{
		m_textCursor.insertText(tr("Size %1 bytes -> %2 bytes\n").arg(sourceFile->data().size()).arg(targetFile->data().size()));
	}*/
}

void ProjectDiffWorker::compareSignal(const int signalID, const CompareData& compareData, ReportTable* const headerTable)
{
	if (headerTable == nullptr)
	{
		Q_ASSERT(headerTable);
		return;
	}

	// Print signal ID
	//
	QString appSignalID;

	Signal latestSignal;

	{
		bool ok = db()->getLatestSignal(signalID, &latestSignal, nullptr);
		if (ok == true)
		{
			appSignalID = latestSignal.appSignalID();
		}
		else
		{
			headerTable->insertRow({QString::number(signalID), tr("getLatestSignal failed"),  QString()});
			return;
		}
	}

	{
		QMutexLocker l(&m_statisticsMutex);
		m_signalIndex++;
		m_currentObjectName = appSignalID;
	}

	// Get signals history
	//
	std::vector<DbChangeset> signalHistory;

	bool ok = db()->getSignalHistory(signalID, &signalHistory, nullptr);
	if (ok == false)
	{
		//diffDataSet.addText(tr("getSignalHistory for signal %1 failed.").arg(appSignalID));
		return;
	}

	// Get source signal
	//

	std::optional<Signal> sourceSignal;

	if (compareData.sourceVersionType == CompareVersionType::LatestVersion)
	{
		sourceSignal = latestSignal;
	}
	else
	{
		std::optional<DbChangeset> sourceChangesetOpt = getRecentChangeset(signalHistory, compareData.sourceVersionType, compareData.sourceChangeset, compareData.sourceDate);

		if (sourceChangesetOpt.has_value() == true)
		{
			std::vector<int> signalIDs;			// for getSpecificSignals
			signalIDs.push_back(signalID);
			std::vector<Signal> sourceSignals;	// for getSpecificSignals

			ok = db()->getSpecificSignals(&signalIDs, sourceChangesetOpt.value().changeset(), &sourceSignals, nullptr);
			if (ok == true)
			{
				if (sourceSignals.size() == 1)
				{
					sourceSignal = sourceSignals[0];
				}
				else
				{
					Q_ASSERT(sourceSignals.size() == 1);
					return;
				}
			}
		}
		else
		{
			ok = true;
		}
	}

	if (ok == false)
	{
		headerTable->insertRow({appSignalID, tr("getSpecificSignal for source failed"),  QString()});
		return;
	}


	// Get target signal
	//
	std::optional<Signal> targetSignal;

	if (compareData.targetVersionType == CompareVersionType::LatestVersion)
	{
		targetSignal = latestSignal;
	}
	else
	{
		std::optional<DbChangeset> targetChangesetOpt = getRecentChangeset(signalHistory, compareData.targetVersionType, compareData.targetChangeset, compareData.targetDate);

		if (targetChangesetOpt.has_value() == true)
		{
			std::vector<int> signalIDs;			// for getSpecificSignals
			signalIDs.push_back(signalID);
			std::vector<Signal> targetSignals;	// for getSpecificSignals

			ok = db()->getSpecificSignals(&signalIDs, targetChangesetOpt.value().changeset(), &targetSignals, nullptr);
			if (ok == true)
			{
				if (targetSignals.size() == 1)
				{
					targetSignal = targetSignals[0];
				}
				else
				{
					Q_ASSERT(targetSignals.size() == 1);
					return;
				}
			}
		}
		else
		{
			ok = true;
		}
	}

	if (ok == false)
	{
		headerTable->insertRow({appSignalID, tr("getSpecificSignal for target failed"),  QString()});
		return;
	}

	// Only source file exists
	//
	if (sourceSignal.has_value() == true && targetSignal.has_value() == false)
	{
		headerTable->insertRow({appSignalID, tr("Added"),  changesetString(sourceSignal.value())});
	}

	// Only target file exists
	//
	if (sourceSignal.has_value() == false && targetSignal.has_value() == true)
	{
		headerTable->insertRow({appSignalID, tr("Added"),  changesetString(targetSignal.value())});
	}

	// Both files exist
	//
	if (sourceSignal.has_value() == true &&
		targetSignal.has_value() == true &&
		sourceSignal.value().changesetID() != targetSignal.value().changesetID())
	{
		// Compare contents
		//
		if (sourceSignal.value().deleted() == true)
		{
			headerTable->insertRow({appSignalID, tr("Deleted"),  changesetString(sourceSignal.value())});
		}
		else
		{
			if (targetSignal.value().deleted() == true)
			{
				headerTable->insertRow({appSignalID, tr("Deleted"),  changesetString(targetSignal.value())});
			}
			else
			{
				bool swap = false;

				// Target changeset should be later or checked-out
				//
				if (sourceSignal.value().changesetID() == 0 ||  targetSignal.value().changesetID() == 0)
				{
					// One of files is checked out
					//
					if (sourceSignal.value().changesetID() == 0 &&  targetSignal.value().changesetID() != 0)
					{
						swap = true;
					}
				}
				else
				{
					if (sourceSignal.value().changesetID() >  targetSignal.value().changesetID())
					{
						swap = true;
					}
				}

				if (swap == true)
				{
					std::swap(sourceSignal, targetSignal);
				}

				compareSignalContents(sourceSignal.value(), targetSignal.value(), headerTable);
			}
		}
	}

	return;
}

void ProjectDiffWorker::compareSignalContents(const Signal& sourceSignal, const Signal& targetSignal, ReportTable* const headerTable)
{
	if (headerTable == nullptr)
	{
		Q_ASSERT(headerTable);
		return;
	}

	SignalProperties sourceProperties(sourceSignal);
	SignalProperties targetProperties(targetSignal);

	std::vector<PropertyDiff> diffs;

	comparePropertyObjects(sourceProperties, targetProperties, &diffs);

	if (diffs.empty() == false)
	{
		std::shared_ptr<ReportSection> section = std::make_shared<ReportSection>();
		m_sections.push_back(section);

		section->addText(targetSignal.appSignalID(), m_currentCharFormat, m_currentBlockFormat);

		headerTable->insertRow({targetSignal.appSignalID(), targetSignal.instanceAction().text(), changesetString(targetSignal)});

		saveFormat();
		setFont(m_tableFont);
		std::shared_ptr<ReportTable> diffTable = section->addTable({tr("Property"), tr("Status"), tr("Old Value"), tr("New Value")}, m_currentCharFormat);
		restoreFormat();

		fillDiffTable(diffTable.get(), diffs);
	}
}

void ProjectDiffWorker::comparePropertyObjects(const PropertyObject& sourceObject, const PropertyObject& targetObject, std::vector<PropertyDiff>* const result) const
{
	if (result == nullptr)
	{
		Q_ASSERT(result);
		return;
	}

	std::vector<std::shared_ptr<Property>> sourceProperties = sourceObject.properties();
	std::vector<std::shared_ptr<Property>> targetProperties = targetObject.properties();

	std::map<QString, int> propertyExistMap;

	for (std::shared_ptr<Property> p : sourceProperties)
	{
		propertyExistMap[p->caption()] = 1;
	}
	for (std::shared_ptr<Property> p : targetProperties)
	{
		propertyExistMap[p->caption()] |= 2;
	}

	result->reserve(propertyExistMap.size());

	for (auto it : propertyExistMap)
	{
		const QString& name = it.first;
		int existCode = it.second;

		PropertyDiff diff;
		diff.caption = name;

		if (existCode == 1)
		{
			// Exists only in source
			diff.action = PropertyDiff::Action::Removed;
			result->push_back(diff);
			continue;
		}


		if (existCode == 2)
		{
			// Exists only in target
			diff.action = PropertyDiff::Action::Added;
			result->push_back(diff);
			continue;
		}

		Q_ASSERT(existCode == 3);

		// Exists in both
		diff.action = PropertyDiff::Action::Modified;

		std::shared_ptr<Property> sp = sourceObject.propertyByCaption(name);
		std::shared_ptr<Property> tp = targetObject.propertyByCaption(name);

		if (sp == nullptr || tp == nullptr)
		{
			Q_ASSERT(sp);
			Q_ASSERT(tp);
			continue;
		}

		diff.oldValue = sp->value();
		diff.newValue = tp->value();

		diff.oldValueText = ExtWidgets::PropertyTools::propertyValueText(sp.get(), -1);
		diff.newValueText = ExtWidgets::PropertyTools::propertyValueText(tp.get(), -1);

		// Both are enums
		//
		if (sp->isEnum() == true && tp->isEnum() == true)
		{
			if (sp->value().toInt() != tp->value().toInt())
			{
				result->push_back(diff);
			}
			continue;
		}

		// Types are different
		//
		if (diff.oldValue.userType() != diff.oldValue.userType())
		{
			result->push_back(diff);
			continue;
		}

		// Special type TuningValue
		//
		if (diff.oldValue.userType() == TuningValue::tuningValueTypeId())
		{
			TuningValue tvOld;
			TuningValue tvNew;

			tvOld = diff.oldValue.value<TuningValue>();
			tvNew = diff.newValue.value<TuningValue>();

			if (tvOld != tvNew)
			{
				result->push_back(diff);
			}

			continue;
		}

		// General value
		//
		if (diff.oldValue != diff.newValue)
		{
			result->push_back(diff);
			continue;
		}
	}

	return;
}

bool ProjectDiffWorker::isHardwareFile(const QString& fileName) const
{
	for (const QString& ext : Hardware::DeviceObjectExtensions)
	{
		if (fileName.endsWith(ext) == true)
		{
			return true;
		}
	}

	return false;
}

bool ProjectDiffWorker::isBusTypeFile(const QString& fileName) const
{
	if (fileName.endsWith(".bus_type") == true)
	{
		return true;
	}

	return false;
}

bool ProjectDiffWorker::isConnectionFile(const QString& fileName) const
{
	if (fileName.endsWith(".ocl") == true)
	{
		return true;
	}

	return false;
}

bool ProjectDiffWorker::isTextFile(const QString& fileName) const
{
	const std::array<QString, 5> TextExtensions =
		{
			".js",			// Script
			".xml",			// Xml Document
			".xsd",			// Xml Schema
			".csv",			// Table
			".txt"			// Text
		};

	for (const QString& ext : TextExtensions)
	{
		if (fileName.endsWith(ext) == true)
		{
			return true;
		}
	}

	return false;
}

bool ProjectDiffWorker::isSchemaFile(const QString& fileName) const
{
	const std::array<QString, 10> TextExtensions =
	{
		Db::File::AlFileExtension,			// Script
		Db::File::AlTemplExtension,			// Xml Document
		Db::File::UfbFileExtension,			// Xml Schema
		Db::File::UfbTemplExtension,			// Table
		Db::File::MvsFileExtension,
		Db::File::MvsTemplExtension,
		Db::File::TvsFileExtension,
		Db::File::TvsTemplExtension,
		Db::File::DvsFileExtension,
		Db::File::DvsTemplExtension
	};

	for (const QString& ext : TextExtensions)
	{
		if (fileName.endsWith(ext) == true)
		{
			return true;
		}
	}

	return false;
}

void ProjectDiffWorker::generateTitlePage(QTextCursor* textCursor)
{
	if (textCursor == nullptr)
	{
		Q_ASSERT(textCursor);
		return;
	}

	DbProject project = db()->currentProject();

	QTextBlockFormat headerCenterFormat = textCursor->blockFormat();
	headerCenterFormat.setAlignment(Qt::AlignHCenter);

	QTextBlockFormat regularFormat = textCursor->blockFormat();
	regularFormat.setAlignment(Qt::AlignLeft);

	//QTextCharFormat headerCharFormat = cursor.charFormat();
	//headerCharFormat.setFontWeight(static_cast<int>(QFont::Bold));
	//headerCharFormat.setFontPointSize(12.0);

	//QTextCharFormat regularCharFormat = cursor.charFormat();
	//headerCharFormat.setFontPointSize(10.0);

	textCursor->setBlockFormat(headerCenterFormat);
	//m_textCursor.setCharFormat(headerCharFormat);
	textCursor->insertText(QObject::tr("Project Differrences - %1\n").arg(project.projectName()));
	textCursor->insertText("\n");

	textCursor->setBlockFormat(regularFormat);
	//m_textCursor.setCharFormat(regularCharFormat);
	textCursor->insertText(tr("Generated: %1\n").arg(QDateTime::currentDateTime().toString("dd/MM/yyyy  HH:mm:ss")));
	//cursor.insertText(tr("RPCT: %1\n").arg(m_configuration->softwareEquipmentId));
	textCursor->insertText("\n");

	textCursor->insertText("\n");

	textCursor->insertText(tr("Total files to compare: %1\n").arg(m_filesCount));
	textCursor->insertText(tr("Total signals to compare: %1\n").arg(m_signalsCount));

}

void ProjectDiffWorker::createMarginItems(QTextCursor* textCursor)
{
	if (textCursor == nullptr)
	{
		Q_ASSERT(textCursor);
		return;
	}

	// Create headers/footers

	DbProject project = db()->currentProject();

	QTextCharFormat charFormat = textCursor->charFormat();
	QTextBlockFormat blockFormat = textCursor->blockFormat();

	charFormat.setFont(m_marginFont);
	addMarginItem({tr("Project: ") + project.projectName(), 2, -1, Qt::AlignLeft | Qt::AlignTop, charFormat, blockFormat});

	QString appVersion = qApp->applicationName() +" v" + qApp->applicationVersion();
	addMarginItem({appVersion, 2, -1, Qt::AlignRight | Qt::AlignTop, charFormat, blockFormat});

	addMarginItem({tr("%PAGE%"), 2, -1, Qt::AlignRight | Qt::AlignBottom, charFormat, blockFormat});
}

void ProjectDiffWorker::fillDiffTable(ReportTable* diffTable, const std::vector<PropertyDiff>& diffs)
{
	if (diffTable == nullptr)
	{
		Q_ASSERT(diffTable);
		return;
	}

	for (const PropertyDiff& diff : diffs)
	{
		switch (diff.action)
		{

		case PropertyDiff::Action::Added:
			{
				diffTable->insertRow({diff.caption, tr("Added"), QString(), QString()});
			}
			break;
		case PropertyDiff::Action::Removed:
			{
				diffTable->insertRow({diff.caption, tr("Removed"), QString(), QString()});
			}
			break;
		case PropertyDiff::Action::Modified:
			{
				diffTable->insertRow({diff.caption, tr("Modified"), diff.oldValueText, diff.newValueText});
			}
			break;
		}
	}

	return;
}

QString ProjectDiffWorker::changesetString(const std::shared_ptr<DbFile>& file)
{
	return tr("#%1 (%2), %3").arg(file->changeset()).arg(file->lastCheckIn().toString("dd/MM/yyyy HH:mm:ss")).arg(db()->username(file->userId()));
}

QString ProjectDiffWorker::changesetString(const Signal& signal)
{
	return tr("#%1 (%2), %3").arg(signal.changesetID()).arg(signal.instanceCreated().toString("dd/MM/yyyy HH:mm:ss")).arg(db()->username(signal.userID()));
}
