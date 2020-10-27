#include "ProjectDiffGenerator.h"
#include "../lib/DbController.h"

#include "../lib/SignalProperties.h"
#include "../lib/PropertyEditor.h"
#include "../lib/Connection.h"
#include "../VFrame30/DrawParam.h"
#include "../lib/TypesAndEnums.h"

#include <QPrinter>

//
// ReportSchemaView
//

ReportSchemaView::ReportSchemaView(std::shared_ptr<VFrame30::Schema> schema, QWidget* parent):
	VFrame30::SchemaView(schema, parent)
{
}

void ReportSchemaView::adjust(QPainter* painter, double startX, double startY, double zoom) const
{
	Ajust(painter, startX, startY, zoom);
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

bool ReportObject::isNewPage() const
{
	return dynamic_cast<const ReportBreak*>(this) != nullptr;
}

bool ReportObject::isText() const
{
	return dynamic_cast<const ReportText*>(this) != nullptr;
}

bool ReportObject::isSchema() const
{
	return dynamic_cast<const ReportSchema*>(this) != nullptr;
}

bool ReportObject::isTable() const
{
	return dynamic_cast<const ReportTable*>(this) != nullptr;
}

//
// ReportBreak
//

ReportBreak::ReportBreak()
	:ReportObject()
{

}

void ReportBreak::render(ReportGenerator* reportGenerator, QTextCursor* textCursor) const
{
	Q_UNUSED(textCursor);

	reportGenerator->newPage();
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

void ReportTable::render(ReportGenerator* reportGenerator, QTextCursor* textCursor) const
{
	Q_UNUSED(reportGenerator);

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

	textCursor->insertHtml(html);

	textCursor->insertText("\n\n");
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

void ReportText::render(ReportGenerator* reportGenerator, QTextCursor* textCursor) const
{
	Q_UNUSED(reportGenerator);

	//textCursor->insertBlock();

	if (m_charFormat.isValid() == true)
	{
		textCursor->setCharFormat(m_charFormat);
	}
	if (m_blockCharFormat.isValid() == true)
	{
		textCursor->setBlockFormat(m_blockCharFormat);
	}

	textCursor->insertText(m_text);
}

//
// ReportSchema
//

ReportSchema::ReportSchema(std::shared_ptr<VFrame30::Schema> schema):
	m_schema(schema)
{
}

void ReportSchema::render(ReportGenerator* reportGenerator, QTextCursor* textCursor) const
{
	Q_UNUSED(textCursor);

	if (m_schema == nullptr)
	{
		Q_ASSERT(m_schema);
		return;
	}

	reportGenerator->printSchema(m_schema);
}

//
// ReportSection
//

ReportSection::ReportSection(ReportGenerator* reportGenerator, QTextCursor* textCursor):
	m_reportGenerator(reportGenerator),
	m_textCursor(textCursor)
{
	Q_ASSERT(m_reportGenerator);
	Q_ASSERT(m_textCursor);

	m_currentBlockCharFormat = m_textCursor->blockFormat();
	m_currentCharFormat = m_textCursor->charFormat();
}

ReportSection::~ReportSection()
{
}

bool ReportSection::isEmpty() const
{
	return m_objects.empty() == true;
}

void ReportSection::addText(const QString& text)
{
	m_objects.push_back(std::make_shared<ReportText>(text, m_currentCharFormat, m_currentBlockCharFormat));
}

void ReportSection::addSchema(std::shared_ptr<VFrame30::Schema> schema)
{
	m_objects.push_back(std::make_shared<ReportSchema>(schema));
}

void ReportSection::addTable(std::shared_ptr<ReportTable> table)
{
	m_objects.push_back(table);
}

std::shared_ptr<ReportTable> ReportSection::addTable(const QStringList& headerLabels)
{
	std::shared_ptr<ReportTable> table = std::make_shared<ReportTable>(headerLabels, m_currentCharFormat);
	m_objects.push_back(table);

	return table;
}

void ReportSection::addNewPage()
{
	m_objects.push_back(std::make_shared<ReportBreak>());
}

void ReportSection::saveFormat()
{
	m_currentCharFormatSaved = m_currentCharFormat;
	m_currentBlockCharFormatSaved = m_currentBlockCharFormat;
}

void ReportSection::restoreFormat()
{
	m_currentCharFormat = m_currentCharFormatSaved;
	m_currentBlockCharFormat = m_currentBlockCharFormatSaved;
}

void ReportSection::setFont(const QFont& font)
{
	m_currentCharFormat.setFont(font);
}

void ReportSection::setTextForeground(const QBrush& brush)
{
	m_currentCharFormat.setForeground(brush);
}

void ReportSection::setTextBackground(const QBrush& brush)
{
	m_currentCharFormat.setBackground(brush);
}

void ReportSection::setTextAlignment(Qt::Alignment alignment)
{
	m_currentBlockCharFormat.setAlignment(alignment);
}

void ReportSection::render() const
{
	// Other objects

	for (const std::shared_ptr<ReportObject> object : m_objects)
	{
		if (object == nullptr)
		{
			Q_ASSERT(object);
			return;
		}
		object->render(m_reportGenerator, m_textCursor);
	}
}

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

ReportGenerator::ReportGenerator(const QString& fileName, QWidget* parent):
	QObject(parent),
	m_parentWidget(parent),
	m_pdfWriter(fileName),
	m_textCursor(&m_textDocument)

{

}

QWidget* ReportGenerator::parentWidget() const
{
	return m_parentWidget;
}

void ReportGenerator::printSchema(std::shared_ptr<VFrame30::Schema> schema)
{
	// Calculate the upper schema offset

	const QRect pageRect = m_pdfWriter.pageLayout().paintRectPixels(m_pdfWriter.resolution());

	QRect contentRect = QRect(QPoint(0, 0), m_textDocument.size().toSize());

	int schemaLeft = 0;

	int schemaTop = contentRect.height() % pageRect.height();

	int schemaMaxHeight = pageRect.height() - schemaTop;

	// Print the rest of the document

	flushDocument();

	// Calculate draw parameters

	ReportSchemaView schemaView(schema, m_parentWidget);

	VFrame30::CDrawParam drawParam(&m_pdfPainter, schema.get(), &schemaView, schema->gridSize(), schema->pinGridStep());
	drawParam.setInfoMode(false);
	drawParam.session() = schemaView.session();

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

	m_pdfPainter.save();

	m_pdfPainter.setRenderHint(QPainter::Antialiasing);

	schemaView.adjust(&m_pdfPainter, schemaLeft, schemaTop, zoom * 100.0);		// Export 100% zoom

	// Draw Schema
	//
	QRectF clipRect(0, 0, schema->docWidth(), schema->docHeight());

	schema->Draw(&drawParam, clipRect);

	m_pdfPainter.restore();
}

void ReportGenerator::newPage()
{
	flushDocument();

	m_currentPage++;

	m_pdfWriter.newPage();
}


void ReportGenerator::flushDocument()
{
	if (m_textDocument.isEmpty() == true)
	{
		return;
	}

	const QRect pageRect = m_pdfWriter.pageLayout().paintRectPixels(m_pdfWriter.resolution());

	// The total extent of the content (there are no page margin in this)
	const QRect contentRect = QRect(QPoint(0, 0), m_textDocument.size().toSize());

	// This is the part of the content we will drop on a page.  It's a sliding window on the content.
	QRect currentRect(0, 0, pageRect.width(), pageRect.height());

	while (currentRect.intersects(contentRect) == true)
	{
			m_pdfPainter.save();
			m_pdfPainter.translate(0, -currentRect.y());
			m_textDocument.drawContents(&m_pdfPainter, currentRect);  // draws part of the document
			m_pdfPainter.restore();

			drawMarginItems(m_currentPage, m_pdfPainter);

			// Translate the current rectangle to the area to be printed for the next page
			currentRect.translate(0, currentRect.height());

			//Inserting a new page if there is still area left to be printed
			if (currentRect.intersects(contentRect))
			{
				m_currentPage++;

				m_pdfWriter.newPage();
			}
	}

	clearDocument();

	return;
}

void ReportGenerator::clearDocument()
{
	if (m_textDocument.isEmpty() == true)
	{
		return;
	}

	// Clear text document

	QTextCharFormat charFormat = m_textCursor.charFormat();
	m_textDocument.clear();
	m_textCursor.setCharFormat(charFormat);

	return;
}

void ReportGenerator::addMarginItem(const ReportMarginItem& item)
{
	m_marginItems.push_back(item);
}

void ReportGenerator::drawMarginItems(int page, QPainter& painter)
{
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

	painter.save();

	painter.translate(-pageRect.left(), -pageRect.top());

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

		painter.setFont(item.m_charFormat.font());

		QString text = item.m_text;

		if (text == "%PAGE%")
		{
			text = tr("Page %1").arg(page);
		}

		//painter.fillRect(topRect, Qt::green);
		//painter.fillRect(bottomRect, Qt::yellow);

		if (item.m_alignment & Qt::AlignTop)
		{
			int alignment = item.m_alignment & ~Qt::AlignTop;

			painter.drawText(topRect, alignment | Qt::AlignVCenter, text);
		}
		else
		{
			if (item.m_alignment & Qt::AlignBottom)
			{
				int alignment = item.m_alignment & ~Qt::AlignBottom;
				painter.drawText(bottomRect, alignment | Qt::AlignVCenter, text);
			}
		}
	}

	painter.restore();

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

void ProjectDiffGenerator::run(const CompareData& compareData, DbController* dbc, QWidget* parent)
{
	// Get filename
	//

	QString fileName = QFileDialog::getSaveFileName(parent, tr("Diff Report"),
													"./",
													tr("PDF documents (*.pdf)"));

	if (fileName.isNull() == true)
	{
		return;
	}

	ProjectDiffGenerator g(fileName, dbc, parent);

	g.compareProject(compareData);

}

ProjectDiffGenerator::ProjectDiffGenerator(const QString& fileName, DbController* dbc, QWidget* parent):
	ReportGenerator(fileName, parent),
	m_db(dbc)
{
	m_filesTypesNamesMap[m_db->busTypesFileId()] = tr("Busses");
	m_filesTypesNamesMap[m_db->schemaFileId()] = tr("Schemas");
	m_filesTypesNamesMap[m_db->afblFileId()] = tr("AFBL Descriptions ");
	m_filesTypesNamesMap[m_db->ufblFileId()] = tr("UFBL Descriptions");
	m_filesTypesNamesMap[m_db->alFileId()] = tr("Application Logic");
	m_filesTypesNamesMap[m_db->hcFileId()] = tr("Hardware Configuration");
	m_filesTypesNamesMap[m_db->hpFileId()] = tr("Hardware Presets");
	m_filesTypesNamesMap[m_db->mcFileId()] = tr("Module Configuration");
	m_filesTypesNamesMap[m_db->mvsFileId()] = tr("Monitor Schemas");
	m_filesTypesNamesMap[m_db->tvsFileId()] = tr("Tuning Schemas");
	m_filesTypesNamesMap[m_db->dvsFileId()] = tr("Diagnostics Schemas");
	m_filesTypesNamesMap[m_db->connectionsFileId()] = tr("Connections");
	m_filesTypesNamesMap[m_db->etcFileId()] = tr("Other Files");
	m_filesTypesNamesMap[m_db->testsFileId()] = tr("Tests");
	m_filesTypesNamesMap[m_db->simTestsFileId()] = tr("Simulator Tests");

	m_headerFont = QFont("Times", 72, QFont::Bold);
	m_normalFont = QFont("Times", 48);
	m_tableFont = QFont("Times", 24);
	m_marginFont = QFont("Times", 12);

	// Create headers/footers

	DbProject project = m_db->currentProject();

	QTextCharFormat charFormat = m_textCursor.charFormat();
	QTextBlockFormat blockFormat = m_textCursor.blockFormat();

	charFormat.setFont(m_marginFont);
	addMarginItem({tr("Project: ") + project.projectName(), 2, -1, Qt::AlignLeft | Qt::AlignTop, charFormat, blockFormat});

	QString appVersion = qApp->applicationName() +" v" + qApp->applicationVersion();
	addMarginItem({appVersion, 2, -1, Qt::AlignRight | Qt::AlignTop, charFormat, blockFormat});

	addMarginItem({tr("%PAGE%"), 2, -1, Qt::AlignRight | Qt::AlignBottom, charFormat, blockFormat});

	return;
}

ProjectDiffGenerator::~ProjectDiffGenerator()
{
}

void ProjectDiffGenerator::compareProject(const CompareData& compareData)
{

	// Get and count files
	//
	m_sourceDeviceObjectMap.clear();
	m_targetDeviceObjectMap.clear();

	std::vector<DbFileInfo> rootFiles;

	bool ok = m_db->getFileList(&rootFiles, m_db->rootFileId(), false, parentWidget());

	m_filesTotal = 0;
	m_currentPage = 1;

	std::vector<DbFileTree> filesTrees;

	filesTrees.resize(rootFiles.size());

	for (int i = 0; i < static_cast<int>(rootFiles.size()); i++)
	{
		const DbFileInfo& rootFile = rootFiles[i];

		DbFileTree* filesTree = &filesTrees[i];

		ok = m_db->getFileListTree(filesTree, rootFile.fileId(), false/*removeDeleted*/, parentWidget());
		if (ok == false)
		{
			Q_ASSERT(ok);
			return;
		}

		m_filesTotal += static_cast<int>(filesTree->files().size());
	}

	// Init document

	m_pdfWriter.setPageSize(QPageSize(QPageSize::A4));
	m_pdfWriter.setPageOrientation(QPageLayout::Portrait);
	m_pdfWriter.setPageMargins(QMargins(25, 15, 15, 15), QPageLayout::Unit::Millimeter);
	m_pdfWriter.setResolution(300);

	m_pdfPainter.begin(&m_pdfWriter);

	QRect pageRectPixels = m_pdfWriter.pageLayout().paintRectPixels(m_pdfWriter.resolution());
	m_textDocument.setPageSize(QSizeF(pageRectPixels.width(), pageRectPixels.height()));

	QTextCharFormat charFormat = m_textCursor.charFormat();
	charFormat.setFontPointSize(40);
	m_textCursor.setCharFormat(charFormat);

	// Generate title page

	generateTitlePage();

	// Process Files

	for (const DbFileTree& filesTree : filesTrees)
	{
		// Print section name

		QString sectionName;

		auto typeIt = m_filesTypesNamesMap.find(filesTree.rootFile()->fileId());
		if (typeIt == m_filesTypesNamesMap.end())
		{
			sectionName = filesTree.rootFile()->fileName();
		}
		else
		{
			sectionName = typeIt->second;
		}

		ReportSection dataSection(this, &m_textCursor);
		dataSection.setFont(m_normalFont);

		dataSection.saveFormat();
		dataSection.setTextAlignment(Qt::AlignHCenter);
		dataSection.setFont(m_headerFont);
		dataSection.addText(tr("%1\n\n").arg(sectionName));
		dataSection.restoreFormat();

		dataSection.addText(tr("Objects with Differences\n"));

		dataSection.saveFormat();
		dataSection.setFont(m_tableFont);
		std::shared_ptr<ReportTable> headerTable = dataSection.addTable({tr("Object"), tr("Status"), tr("Latest Changeset")});
		dataSection.restoreFormat();

		// Compare files

		if (compareFile(filesTree, filesTree.rootFile(), compareData, dataSection, headerTable.get()) == false)
		{
			dataSection.addText(tr("Could not compare file : %1").arg(filesTree.rootFile()->fileName()));
		}

		// Render results

		if (headerTable->rowCount() > 0)
		{
			newPage();

			dataSection.render();
		}
	}

	// Process signals
	//
	{
		ReportSection signalsSection(this, &m_textCursor);
		signalsSection.setFont(m_normalFont);

		signalsSection.saveFormat();
		signalsSection.setTextAlignment(Qt::AlignHCenter);
		signalsSection.setTextForeground(QBrush(Qt::red));
		signalsSection.setTextBackground(QBrush(Qt::black));
		signalsSection.setFont(m_headerFont);
		signalsSection.addText(tr("Application Signals\n\n"));
		signalsSection.restoreFormat();

		signalsSection.addText(tr("Signals with Differences\n"));

		signalsSection.saveFormat();
		signalsSection.setFont(m_tableFont);
		std::shared_ptr<ReportTable> headerTable = signalsSection.addTable({tr("Signal"), tr("Status"), tr("Latest Changeset")});
		signalsSection.restoreFormat();

		QVector<int> signalIDs;

		ok = m_db->getSignalsIDs(&signalIDs, parentWidget());

		for (int signalID : signalIDs)
		{
			compareSignal(signalID, compareData, signalsSection, headerTable.get());
		}

		if (headerTable->rowCount() > 0)
		{
			newPage();

			signalsSection.render();
		}
	}

	// Close file
	//

	flushDocument();

	m_pdfPainter.end();

	// Destroy document

	return;
}

bool ProjectDiffGenerator::compareFile(const DbFileTree& filesTree, const std::shared_ptr<DbFileInfo>& fi, const CompareData& compareData, ReportSection& diffDataSet, ReportTable* const headerTable)
{
	if (headerTable == nullptr)
	{
		Q_ASSERT(headerTable);
		return false;
	}

	if (fi == nullptr)
	{
		Q_ASSERT(fi);
		return false;
	}

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

	// Get file history
	//
	std::vector<DbChangeset> fileHistory;

	bool ok = m_db->getFileHistory(*fi, &fileHistory, parentWidget());
	if (ok == false)
	{
		return false;
	}

	// Get source file
	//
	std::shared_ptr<DbFile> sourceFile;

	if (compareData.sourceVersionType == CompareVersionType::LatestVersion)
	{
		ok = m_db->getLatestVersion(*fi, &sourceFile, parentWidget());
	}
	else
	{
		std::optional<DbChangeset> changesetOpt = getRecentChangeset(fileHistory, compareData.sourceVersionType, compareData.sourceChangeset, compareData.sourceDate);

		if (changesetOpt.has_value() == true)
		{
			ok = m_db->getSpecificCopy(*fi, changesetOpt.value().changeset(), &sourceFile, parentWidget());
		}
	}

	if (ok == false)
	{
		Q_ASSERT(false);
		return false;
	}

	// Get target file
	//
	std::shared_ptr<DbFile> targetFile;

	if (compareData.targetVersionType == CompareVersionType::LatestVersion)
	{
		ok = m_db->getLatestVersion(*fi, &targetFile, parentWidget());
	}
	else
	{
		std::optional<DbChangeset> changesetOpt = getRecentChangeset(fileHistory, compareData.targetVersionType, compareData.targetChangeset, compareData.targetDate);

		if (changesetOpt.has_value() == true)
		{
			ok = m_db->getSpecificCopy(*fi, changesetOpt.value().changeset(), &targetFile, parentWidget());
		}
	}

	if (ok == false)
	{
		Q_ASSERT(false);
		return false;
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

	compareFileContents(sourceFile, targetFile, fileName, diffDataSet, headerTable);

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
			return false;
		}

		if (compareFile(filesTree, fiChild, compareData, diffDataSet, headerTable) == false)
		{
			diffDataSet.addText(tr("Could not compare file : %1").arg(fiChild->fileName()));
		}
	}

	return true;
}

bool ProjectDiffGenerator::compareFileContents(const std::shared_ptr<DbFile>& sourceFile,
											   const std::shared_ptr<DbFile>& targetFile,
											   const QString& fileName,
											   ReportSection& diffDataSet,
											   ReportTable* const headerTable)
{
	if (headerTable == nullptr)
	{
		Q_ASSERT(headerTable);
		return false;
	}

	// No files at all
	//
	if (sourceFile == nullptr && targetFile == nullptr)
	{
		return false;
	}

	// Same changeset
	//
	if (sourceFile != nullptr &&
		targetFile != nullptr &&
		sourceFile->changeset() == targetFile->changeset())
	{
		return true;
	}

	// File was deleted
	//
	if (sourceFile != nullptr && sourceFile->deleted() == true)
	{
		headerTable->insertRow({fileName, tr("Deleted"), changesetString(sourceFile)});
		return false;
	}
	else
	{
		if (targetFile != nullptr && targetFile->deleted() == true)
		{
			headerTable->insertRow({fileName, tr("Deleted"), changesetString(targetFile)});
			return false;
		}
	}

	// Compare contents
	//
	if (isHardwareFile(fileName) == true)
	{
		compareDeviceObjects(sourceFile, targetFile, diffDataSet, headerTable);
		return true;
	}

	if (isConnectionFile(fileName) == true)
	{
		compareConnections(sourceFile, targetFile, diffDataSet, headerTable);
		return true;
	}

	if (isBusTypeFile(fileName) == true)
	{
		compareBusTypes(sourceFile, targetFile, diffDataSet, headerTable);
		return true;
	}

	if (isSchemaFile(fileName) == true)
	{
		compareSchemas(sourceFile, targetFile, diffDataSet, headerTable);
		return true;
	}

	compareFilesData(sourceFile, targetFile);
	return true;
}

std::optional<DbChangeset> ProjectDiffGenerator::getRecentChangeset(const std::vector<DbChangeset>& history,
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

std::shared_ptr<Hardware::DeviceObject> ProjectDiffGenerator::loadDeviceObject(const std::shared_ptr<DbFile>& file, std::map<int, std::shared_ptr<Hardware::DeviceObject>>* const deviceObjectMap)
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

	(*deviceObjectMap)[file->fileId()] = object;

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
			object->expandEquipmentId();
		}
	}

	return object;
}

void ProjectDiffGenerator::compareDeviceObjects(const std::shared_ptr<DbFile>& sourceFile,
												const std::shared_ptr<DbFile>& targetFile,
												ReportSection& diffDataSet,
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

	std::shared_ptr<Hardware::DeviceObject> sourceObject;
	std::shared_ptr<Hardware::DeviceObject> targetObject;

	// Load objects
	//
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

	// Single object
	//
	if ((sourceObject != nullptr && targetObject == nullptr) ||
		(sourceObject == nullptr && targetObject != nullptr))
	{
		auto singleFile = sourceFile != nullptr ? sourceFile : targetFile;
		auto singleObject = sourceObject != nullptr ? sourceObject : targetObject;

		headerTable->insertRow({singleObject->equipmentId(), tr("Added"),  changesetString(singleFile)});
		return;
	}

	// Both Objects
	//
	std::vector<PropertyDiff> diffs;

	comparePropertyObjects(*sourceObject, *targetObject, &diffs);

	if (diffs.empty() == false)
	{
		headerTable->insertRow({targetObject->equipmentId(), targetFile->action().text(),  changesetString(targetFile)});

		diffDataSet.addNewPage();

		diffDataSet.addText(targetObject->equipmentId());

		diffDataSet.saveFormat();
		diffDataSet.setFont(m_tableFont);
		std::shared_ptr<ReportTable> diffTable = diffDataSet.addTable({tr("Property"), tr("Status"), tr("Old Value"), tr("New Value")});
		diffDataSet.restoreFormat();

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
	}
}

void ProjectDiffGenerator::compareBusTypes(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile, ReportSection& diffDataSet, ReportTable* const headerTable)
{

}

void ProjectDiffGenerator::compareSchemas(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile, ReportSection& diffDataSet, ReportTable* const headerTable)
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
		diffDataSet.addText(tr("\n%1: source schema loading failed\n").arg(sourceFile->fileName()));
	}

	std::shared_ptr<VFrame30::Schema> targetSchema = VFrame30::Schema::Create(targetFile->data());
	if (targetSchema == nullptr)
	{
		Q_ASSERT(false);
		diffDataSet.addText(tr("\n%1: target schema loading failed\n").arg(targetFile->fileName()));
	}

	std::vector<PropertyDiff> diffs;

	comparePropertyObjects(*sourceSchema, *targetSchema, &diffs);

	if (diffs.empty() == false)
	{
		diffDataSet.addNewPage();

		diffDataSet.addText(targetFile->fileName());

		headerTable->insertRow({targetFile->fileName(), targetFile->action().text(),  changesetString(targetFile)});

		diffDataSet.saveFormat();
		diffDataSet.setFont(m_tableFont);
		std::shared_ptr<ReportTable> diffTable = diffDataSet.addTable({tr("Property"), tr("Status"), tr("Old Value"), tr("New Value")});
		diffDataSet.restoreFormat();

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

		diffDataSet.addNewPage();

		diffDataSet.addText("Source schema:\n");

		diffDataSet.addSchema(sourceSchema);

		diffDataSet.addNewPage();

		diffDataSet.addText("Target schema:\n");

		diffDataSet.addSchema(targetSchema);
	}

}

void ProjectDiffGenerator::compareConnections(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile, ReportSection& diffDataSet, ReportTable* const headerTable)
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
		diffDataSet.addNewPage();
		diffDataSet.addText(targetConnection.connectionID());

		headerTable->insertRow({targetConnection.connectionID(), targetFile->action().text(),  changesetString(targetFile)});

		diffDataSet.saveFormat();
		diffDataSet.setFont(m_tableFont);
		std::shared_ptr<ReportTable> diffTable = diffDataSet.addTable({tr("Property"), tr("Status"), tr("Old Value"), tr("New Value")});
		diffDataSet.restoreFormat();

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
	}
}

void ProjectDiffGenerator::compareFilesData(const std::shared_ptr<DbFile>& sourceFile, const std::shared_ptr<DbFile>& targetFile)
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

void ProjectDiffGenerator::compareSignal(const int signalID, const CompareData& compareData, ReportSection& diffDataSet, ReportTable* const headerTable)
{
	if (headerTable == nullptr)
	{
		Q_ASSERT(headerTable);
		return;
	}

	// Print signal ID
	//
	QString appSignalID;

	{
		Signal signal;
		bool ok = m_db->getLatestSignal(signalID, &signal, parentWidget());
		if (ok == true)
		{
			appSignalID = signal.appSignalID();
		}
	}

	// Get signals history
	//
	std::vector<DbChangeset> signalHistory;

	bool ok = m_db->getSignalHistory(signalID, &signalHistory, parentWidget());
	if (ok == false)
	{
		Q_ASSERT(false);
		return;
	}

	// Get source signal
	//

	std::optional<Signal> sourceSignal;

	if (compareData.sourceVersionType == CompareVersionType::LatestVersion)
	{
		Signal signal;
		ok = m_db->getLatestSignal(signalID, &signal, parentWidget());

		if (ok == true)
		{
			sourceSignal = signal;
		}
	}
	else
	{
		std::optional<DbChangeset> sourceChangesetOpt = getRecentChangeset(signalHistory, compareData.sourceVersionType, compareData.sourceChangeset, compareData.sourceDate);

		if (sourceChangesetOpt.has_value() == true)
		{
			std::vector<int> signalIDs;			// for getSpecificSignals
			signalIDs.push_back(signalID);
			std::vector<Signal> sourceSignals;	// for getSpecificSignals

			ok = m_db->getSpecificSignals(&signalIDs, sourceChangesetOpt.value().changeset(), &sourceSignals, parentWidget());
			if (ok == true)
			{
				if (sourceSignals.size() == 1)
				{
					sourceSignal = sourceSignals[0];
				}
				else
				{
					Q_ASSERT(sourceSignals.size() == 1);
				}
			}
		}
	}

	// Get target signal
	//
	std::optional<Signal> targetSignal;

	if (compareData.targetVersionType == CompareVersionType::LatestVersion)
	{
		Signal signal;
		ok = m_db->getLatestSignal(signalID, &signal, parentWidget());
		if (ok == true)
		{
			targetSignal = signal;
		}
	}
	else
	{
		std::optional<DbChangeset> targetChangesetOpt = getRecentChangeset(signalHistory, compareData.targetVersionType, compareData.targetChangeset, compareData.targetDate);

		if (targetChangesetOpt.has_value() == true)
		{
			std::vector<int> signalIDs;			// for getSpecificSignals
			signalIDs.push_back(signalID);
			std::vector<Signal> targetSignals;	// for getSpecificSignals

			ok = m_db->getSpecificSignals(&signalIDs, targetChangesetOpt.value().changeset(), &targetSignals, parentWidget());
			if (ok == true)
			{
				if (targetSignals.size() == 1)
				{
					targetSignal = targetSignals[0];
				}
				else
				{
					Q_ASSERT(targetSignals.size() == 1);
				}
			}
		}
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

				compareSignalContents(sourceSignal.value(), targetSignal.value(), diffDataSet, headerTable);			}
		}

	}

	return;
}

void ProjectDiffGenerator::compareSignalContents(const Signal& sourceSignal, const Signal& targetSignal, ReportSection& diffDataSet, ReportTable* const headerTable)
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
		diffDataSet.addNewPage();
		diffDataSet.addText(targetSignal.appSignalID());

		headerTable->insertRow({targetSignal.appSignalID(), targetSignal.instanceAction().text(), changesetString(targetSignal)});

		diffDataSet.saveFormat();
		diffDataSet.setFont(m_tableFont);
		std::shared_ptr<ReportTable> diffTable = diffDataSet.addTable({tr("Property"), tr("Status"), tr("Old Value"), tr("New Value")});
		diffDataSet.restoreFormat();

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
	}
}

void ProjectDiffGenerator::comparePropertyObjects(const PropertyObject& sourceObject, const PropertyObject& targetObject, std::vector<PropertyDiff>* const result) const
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

bool ProjectDiffGenerator::isHardwareFile(const QString& fileName) const
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

bool ProjectDiffGenerator::isBusTypeFile(const QString& fileName) const
{
	if (fileName.endsWith(".bus_type") == true)
	{
		return true;
	}

	return false;
}

bool ProjectDiffGenerator::isConnectionFile(const QString& fileName) const
{
	if (fileName.endsWith(".ocl") == true)
	{
		return true;
	}

	return false;
}

bool ProjectDiffGenerator::isTextFile(const QString& fileName) const
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

bool ProjectDiffGenerator::isSchemaFile(const QString& fileName) const
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

void ProjectDiffGenerator::generateTitlePage()
{
	DbProject project = m_db->currentProject();

	QTextBlockFormat headerCenterFormat = m_textCursor.blockFormat();
	headerCenterFormat.setAlignment(Qt::AlignHCenter);

	QTextBlockFormat regularFormat = m_textCursor.blockFormat();
	regularFormat.setAlignment(Qt::AlignLeft);

	//QTextCharFormat headerCharFormat = cursor.charFormat();
	//headerCharFormat.setFontWeight(static_cast<int>(QFont::Bold));
	//headerCharFormat.setFontPointSize(12.0);

	//QTextCharFormat regularCharFormat = cursor.charFormat();
	//headerCharFormat.setFontPointSize(10.0);

	m_textCursor.setBlockFormat(headerCenterFormat);
	//m_textCursor.setCharFormat(headerCharFormat);
	m_textCursor.insertText(QObject::tr("Project Differtences - %1\n").arg(project.projectName()));
	m_textCursor.insertText("\n");

	m_textCursor.setBlockFormat(regularFormat);
	//m_textCursor.setCharFormat(regularCharFormat);
	m_textCursor.insertText(tr("Generated: %1\n").arg(QDateTime::currentDateTime().toString("dd/MM/yyyy  HH:mm:ss")));
	//cursor.insertText(tr("RPCT: %1\n").arg(m_configuration->softwareEquipmentId));
	m_textCursor.insertText("\n");

	m_textCursor.insertText("\n");

	m_textCursor.insertText(tr("Total files to compare: %1\n").arg(m_filesTotal));

}

QString ProjectDiffGenerator::changesetString(const std::shared_ptr<DbFile>& file)
{
	return tr("#%1 (%2), %3").arg(file->changeset()).arg(file->lastCheckIn().toString("dd/MM/yyyy HH:mm:ss")).arg(m_db->username(file->userId()));
}

QString ProjectDiffGenerator::changesetString(const Signal& signal)
{
	return tr("#%1 (%2), %3").arg(signal.changesetID()).arg(signal.instanceCreated().toString("dd/MM/yyyy HH:mm:ss")).arg(m_db->username(signal.userID()));
}
