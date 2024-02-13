#include "ReportPrinter.h"
#include "../VFrame30/Schema.h"
#include "../VFrame30/DrawParam.h"

namespace ReportLib
{
	//
	// PrintText
	//

	PrintText::PrintText(QSizeF pageSize, int verticalOffset, bool newPageBefore, const QString& tag):
		m_textCusror(&m_textDocument)
	{
		m_type = Type::Text;
		m_verticalOffset = verticalOffset;
		m_newPageBefore = newPageBefore;
		m_textDocument.setPageSize(pageSize);
		m_tag = tag;
	}

	QTextCursor& PrintText::textCursor()
	{
		return m_textCusror;
	}

	QRect PrintText::contentRect() const
	{
		if (m_textDocument.isEmpty() == true)
		{
			return QRect{0, 0, 0, 0};
		}

		return QRect(QPoint(0, 0), m_textDocument.size().toSize());
	}

	void PrintText::print(Report& report,
						  ReportPrinter& printer,
						  QPdfWriter& pdfWriter,
						  QPainter& painter,
						  int& pageIndex,
						  QMutex& pageCounterMutex)
	{
		if (m_newPageBefore == true)
		{
			pdfWriter.newPage();

			QMutexLocker l(&pageCounterMutex);
			pageIndex++;
		}

		report.reportVariables().setVariable("REPORT_PAGE", pageIndex + report.startPage());

		if (m_textDocument.isEmpty() == true)
		{
			return;
		}

		if (m_verticalOffset == 0)
		{
			printer.printMarginItems(report, pdfWriter, painter, tag());
		}

		// Page contains text

		const QRect pageRect = pdfWriter.pageLayout().paintRectPixels(pdfWriter.resolution());

		// The total extent of the content (there are no page margin in this)
		const QRect contentRect = QRect(QPoint(0, 0), m_textDocument.size().toSize());

		// This is the part of the content we will drop on a page.  It's a sliding window on the content.
		QRect currentRect(0, 0, pageRect.width(), pageRect.height());

		while (currentRect.intersects(contentRect) == true)
		{
			// Print document part

			painter.save();
			painter.translate(0, -currentRect.y());
			m_textDocument.drawContents(&painter, currentRect);  // draws part of the document
			painter.restore();

			// Print margins

			//printMarginItems(pdfWriter, painter, section.caption(), marginItems);

			// Translate the current rectangle to the area to be printed for the next page

			currentRect.translate(0, currentRect.height());

			//Inserting a new page if there is still area left to be printed

			if (currentRect.intersects(contentRect))
			{
				pdfWriter.newPage();

				{
					QMutexLocker l(&pageCounterMutex);
					pageIndex++;
				}

				report.reportVariables().setVariable("REPORT_PAGE", pageIndex + report.startPage());

				printer.printMarginItems(report, pdfWriter, painter, tag());
			}
		}
	}

	int PrintText::pageCount() const
	{
		return m_textDocument.pageCount();
	}

	//
	// PrintSchema
	//

	PrintSchema::PrintSchema(const std::shared_ptr<ReportSchemaView>& schemaView,
							 const std::shared_ptr<VFrame30::Schema>& schema,
							 const std::map<QUuid, ReportSchemaCompareAction>& compareActions,
							 int verticalOffset,
							 bool newPageBefore,
							 const QString& tag):
		m_schemaView(schemaView),
		m_schema(schema),
		m_compareActions(compareActions)
	{
		m_type = Type::Schema;
		m_verticalOffset = verticalOffset;
		m_newPageBefore = newPageBefore;
		m_tag = tag;
	}

	QRect PrintSchema::contentRect() const
	{
		return QRect{0, 0, 0, 0};
	}

	void PrintSchema::print(Report& report,
							ReportPrinter& printer,
							QPdfWriter& pdfWriter,
							QPainter& painter,
							int& pageIndex,
							QMutex& pageCounterMutex)
	{
		if (m_schemaView == nullptr || m_schema == nullptr)
		{
			Q_ASSERT(m_schemaView);
			Q_ASSERT(m_schema);
			return;
		}

		if (m_newPageBefore == true)
		{
			pdfWriter.newPage();
			{
				QMutexLocker l(&pageCounterMutex);
				pageIndex++;
			}
		}

		report.reportVariables().setVariable("REPORT_PAGE", pageIndex + report.startPage());

		// Calculate the upper schema offset
		//
		QRect pageRect = pdfWriter.pageLayout().paintRectPixels(pdfWriter.resolution());

		// Set margins area as 1% of area height
		//
		int schemaTop = m_verticalOffset;
		int schemaLeft = 0;

		const int schemaMaxHeight = pageRect.height() - schemaTop;

		// Calculate draw parameters
		//
		double schemaWidthInPixel = m_schema->GetDocumentWidth(pdfWriter.physicalDpiX(), 100.0);		// Export 100% zoom
		double schemaHeightInPixel = m_schema->GetDocumentHeight(pdfWriter.physicalDpiY(), 100.0);		// Export 100% zoom

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
		painter.save();
		painter.setRenderHint(QPainter::Antialiasing);

		VFrame30::CDrawParam drawParam(&painter, m_schemaView.get(), m_schema->gridSize(), m_schema->pinGridStep(), m_schema->unit());
		drawParam.setInfoMode(m_schemaView->infoMode());
		drawParam.setPdfMode(true);

		m_schemaView->setSchemaInternal(m_schema);
		m_schemaView->adjust(&painter, schemaLeft, schemaTop, zoom * 100.0);		// Export 100% zoom

		QRectF clipRect(0, 0, m_schema->docWidth(), m_schema->docHeight());

		m_schema->Draw(&drawParam, clipRect);

		if (m_compareActions.empty() == false)
		{
			drawParam.setControlBarSize(CONTROL_BAR_MM);
			m_schemaView->drawCompareOutlines(&drawParam, clipRect, m_compareActions);
		}

		painter.restore();

		// Print footer
		//
		if (m_verticalOffset == 0)
		{
			printer.printMarginItems(report, pdfWriter, painter, tag());
		}


		return;
	}

	int PrintSchema::pageCount() const
	{
		if (m_verticalOffset == 0)
			return 1;

		return 0;
	}

	//
	// ReportPrinter
	//
	ReportPrinter::ReportPrinter(std::shared_ptr<ReportSchemaView> reportSchemaView):
		m_schemaView(reportSchemaView)
	{

	}

	bool ReportPrinter::preview(const Report& report, std::vector<RenderedSection>& renderedSections, std::atomic_bool& stop)
	{
		if (createRenderedSections(report, renderedSections, Statistics::Preview, stop) == false)
		{
			return false;
		}

		{
			QMutexLocker l(&m_statisticsMutex);
			m_statistics.status = Statistics::Status::None;
		}

		return true;
	}

	bool ReportPrinter::print(Report& report, const QString& fileName, std::atomic_bool& stop)
	{
		QBuffer buffer;

		if (print(report, buffer, stop) == false)
		{
			return false;
		}

		QFile f(fileName);
		if (f.open(QIODevice::WriteOnly|QIODevice::Truncate) == false)
		{
			return false;
		}
		f.write(buffer.data());

		return true;
	}

	bool ReportPrinter::print(Report& report, QBuffer& buffer, std::atomic_bool& stop)
	{
		std::vector<RenderedSection> renderedSections;

		if (createRenderedSections(report, renderedSections, Statistics::Rendering, stop) == false)
		{
			return false;
		}

		if (printRenderedSections(report, renderedSections, buffer, stop) == false)
		{
			return false;
		}

		{
			QMutexLocker l(&m_statisticsMutex);
			m_statistics.status = Statistics::Status::None;
		}

		return true;
	}

	ReportPrinter::Statistics ReportPrinter::statistics() const
	{
		QMutexLocker l(&m_statisticsMutex);
		return m_statistics;
	}

	void ReportPrinter::printMarginItems(const Report& report,
										 QPdfWriter& pdfWriter,
										 QPainter& painter,
										 const QString& tag) const
	{
		int page = 0;
		int pagesCount = 0;

		{
			QMutexLocker l(&m_statisticsMutex);
			page = m_statistics.pageIndex;
			pagesCount = m_statistics.pagesCount;
		}

		int resolution = pdfWriter.resolution();

		const QRect fullPageRect = pdfWriter.pageLayout().fullRectPixels(resolution);

		const QRect pageRect = pdfWriter.pageLayout().paintRectPixels(resolution);

		QRect topRect(pageRect.left(),
					  fullPageRect.top(),
					  pageRect.width(),
					  abs(pageRect.top() - fullPageRect.top()));

		QRect bottomRect(pageRect.left(),
						 pageRect.bottom(),
						 pageRect.width(),
						 abs(pageRect.bottom() - fullPageRect.bottom()));

		painter.save();

		painter.translate(-pageRect.left(), -pageRect.top());

#ifdef DEBUG_PRINT_PAGE_RECT
		bool first = true;
#endif
		for (const ReportMarginItem& item : report.marginItems())
		{
			if (item.pageFrom != -1 && item.pageFrom > page)
			{
				continue;
			}
			if (item.pageTo != -1 && item.pageTo < page)
			{
				continue;
			}

			QString text = item.text;

			if (text == "%PAGE%")
			{
				text = QObject::tr("Page %1 of %2").arg(page + report.startPage()).arg(pagesCount + report.startPage() - 1);
			}

			if (text == "%TAG%")
			{
				text = tag;
			}

#ifdef DEBUG_PRINT_PAGE_RECT
			if (first == true)
			{
				painter.fillRect(topRect, Qt::green);
				painter.fillRect(bottomRect, Qt::yellow);
				first = false;
			}
#endif

			if (text.isEmpty() == false)
			{
				painter.setFont(item.format.font());
				QFontMetrics fm(item.format.font());
				QRect textBoundingRect = fm.boundingRect(text);

				auto itemAlignment = item.format.alignment();
				if (itemAlignment & Qt::AlignTop)
				{
					if (topRect.width() >= textBoundingRect.width() && topRect.height() >= textBoundingRect.height())
					{
						QRect textRect{topRect};
						textRect.setBottom(textRect.bottom() - fm.height() / 2);
						
						int alignment = itemAlignment & ~Qt::AlignTop;
						painter.drawText(textRect, alignment | Qt::AlignBottom, text);
						
						QPen pen = painter.pen();
						painter.setPen(QPen(Qt::gray, 1.5));
						painter.drawLine(topRect.bottomLeft(), topRect.bottomRight());
						painter.setPen(pen);
					}
				}
				else
				{
					if (itemAlignment & Qt::AlignBottom)
					{
						if (bottomRect.width() >= textBoundingRect.width() && bottomRect.height() >= textBoundingRect.height())
						{
							QRect textRect{bottomRect};
							textRect.setTop(textRect.top() + fm.height() / 2);

							int alignment = itemAlignment & ~Qt::AlignBottom;
							painter.drawText(textRect, alignment | Qt::AlignTop, text);

							QPen pen = painter.pen();
							painter.setPen(QPen(Qt::gray, 1.5));
							painter.drawLine(bottomRect.topLeft(), bottomRect.topRight());
							painter.setPen(pen);
						}
					}
				}
			}
		}

		painter.restore();
	}

	bool ReportPrinter::createRenderedSections(const Report& report, std::vector<RenderedSection>& renderedSections, Statistics::Status status, std::atomic_bool& stop)
	{
		{
			QMutexLocker l(&m_statisticsMutex);
			m_statistics.sectionCount = static_cast<int>(report.sections().size());
			m_statistics.sectionIndex = 0;
			m_statistics.status = status;
		}

		std::map<QString, std::shared_ptr<ReportSection>> allSectionsMap;
		for (const std::shared_ptr<ReportSection>& section : report.sections())
		{
			allSectionsMap[section->caption()] = section;
		}
		ReportTagStorage tagStorage{allSectionsMap};

		double fontScaling = report.resolution() / 72.0;

		{
			QFont font{"Arial", 72, QFont::Normal};

			QFontMetrics coefMetrics{font};

			double kFont = font.pointSize() / static_cast<double>(coefMetrics.height());

			fontScaling *= kFont;
		}

		// Render all objects to print objects
		//

		bool firstSection = true;

		for (const std::shared_ptr<ReportSection>& section : report.sections())
		{
			renderedSections.push_back(RenderedSection{section});

			std::vector<std::shared_ptr<PrintObject>>& printObjects = renderedSections.back().printObjects();

			const QRect pageRectPixels = section->pageLayout().paintRectPixels(report.resolution());

			if (stop == true)
			{
				return true;
			}

			{
				QMutexLocker l(&m_statisticsMutex);
				m_statistics.sectionIndex++;
			}

			size_t count = section->objectCount();

			std::shared_ptr<PrintText> printText;
			std::shared_ptr<PrintSchema> ps;

			ReportObject::Type lastObjectType = ReportObject::Type::Undefined;

			int lastDocumentTextPageHeight = 0; // Height of the text on the last page of QTextDocument

			bool firstObject = true;

			for (size_t i = 0; i < count; i++)
			{
				std::shared_ptr<ReportObject> object = section->object(i);

				switch (object->type())
				{
				case ReportObject::Type::Text:
				case ReportObject::Type::Table:
					{
						// Create new text print object in cases:
						// 1. This is first text object
						// 2. New page has been started
						// 3. Schema has been drawn
						//
						if (printText == nullptr ||
								lastObjectType == ReportObject::Type::Schema)
						{
							printText = std::make_shared<PrintText>(pageRectPixels.size(),
																	lastDocumentTextPageHeight,
																	lastObjectType == ReportObject::Type::Schema ||
																	(firstObject == true && firstSection == false),
																	section->tag());
							printObjects.push_back(printText);
						}
						break;
					}
				case ReportObject::Type::Schema:
					{
						// Create new schema print object
						//
						ReportSchema* rs = dynamic_cast<ReportSchema*>(object.get());
						if (rs == nullptr)
						{
							Q_ASSERT(rs);
							continue;
						}

						if (m_schemaView == nullptr)
						{
							Q_ASSERT(m_schemaView);
							return false;
						}

						ps = std::make_shared<PrintSchema>(m_schemaView,
														   rs->schema(),
														   rs->compareActions(),
														   lastDocumentTextPageHeight,
														   lastObjectType == ReportObject::Type::Schema ||
														   (firstObject == true && firstSection == false),
														   section->tag());
						printObjects.push_back(ps);


					break;
					}
				default:
					Q_ASSERT(false);
				}

				// Render text to print object
				//
				if (object->type() == ReportObject::Type::Text || object->type() == ReportObject::Type::Table)
				{
					if (printText == nullptr)
					{
						Q_ASSERT(printText);
						return false;
					}

					object->renderText(printText->textCursor(), fontScaling, tagStorage);

					lastDocumentTextPageHeight = printText->contentRect().height() % pageRectPixels.height();
				}
				else
				{
					lastDocumentTextPageHeight = 0;
				}

				lastObjectType = object->type();

				if (firstObject == true)
				{
					firstObject = false;
				}
			}

			if (firstSection == true)
			{
				firstSection = false;
			}
		}
		return true;
	}

	bool ReportPrinter::printRenderedSections(Report& report, const std::vector<RenderedSection>& renderedSections, QBuffer& buffer, std::atomic_bool& stop)
	{
		int pagesCount = 0;

		for (const auto& rs : renderedSections)
		{
			pagesCount += rs.pagesCount();
		}

		// Print PDF

		{
			QMutexLocker l(&m_statisticsMutex);
			m_statistics.status = Statistics::Status::Printing;
			m_statistics.pagesCount = pagesCount;
			m_statistics.pageIndex = 0;
		}

		buffer.open(QIODevice::WriteOnly);

		QPdfWriter pdfWriter(&buffer);
		pdfWriter.setTitle(report.path());
		if (renderedSections.empty() == false)
		{
			pdfWriter.setPageLayout(renderedSections[0].pageLayout());
		}
		pdfWriter.setResolution(report.resolution());

		QPainter painter(&pdfWriter);

		bool firstRenderedSection = true;

		for (const auto& rs : renderedSections)
		{
			if (firstRenderedSection == false)
			{
				pdfWriter.setPageLayout(rs.pageLayout());
			}
			else
			{
//#define DEBUG_PRINT_PAGE_RECT	//	Uncomment this for debug
#ifdef DEBUG_PRINT_PAGE_RECT
				if (firstRenderedSection == true)
				{
					auto fullRect = rs.pageLayout().fullRectPixels(report.resolution());

					auto pageRect = rs.pageLayout().paintRectPixels(report.resolution());

					painter.save();

					painter.translate(-pageRect.left(), -pageRect.top());

					painter.fillRect(fullRect, Qt::lightGray);

					painter.restore();

					painter.save();

					painter.translate(-pageRect.left(), -pageRect.top());

					painter.fillRect(pageRect, Qt::gray);

					painter.restore();
				}
#endif
				firstRenderedSection = false;
			}

			for (const std::shared_ptr<PrintObject>& po : rs.printObjects())
			{
				if (stop == true)
				{
					return true;
				}

				po->print(report, *this, pdfWriter, painter, m_statistics.pageIndex, m_statisticsMutex);
			}
		}
		return true;
	}
}
