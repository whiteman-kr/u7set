#include "ReportPrinter.h"
#include "../VFrame30/VFrameTools.h"

namespace ReportLib
{
	//
	// ReportPrinter
	//
	ReportPrinter::ReportPrinter(std::shared_ptr<ReportSchemaView> reportSchemaView):
		m_schemaView(reportSchemaView)
	{

	}

	bool ReportPrinter::print(const Report& report, const QString& fileName, std::atomic_bool& stop)
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

	bool ReportPrinter::print(const Report& report, QBuffer& buffer, std::atomic_bool& stop)
	{
		buffer.open(QIODevice::WriteOnly);

		int sectionsPagesCount = 0;

		{
			QMutexLocker l(&m_statisticsMutex);
			m_statistics.sectionCount = static_cast<int>(report.sections().size());
			m_statistics.sectionIndex = 0;
			m_statistics.pagesCount = 0;
			m_statistics.pageIndex = 0;
			m_statistics.status = Statistics::Status::Rendering;
		}

		// Create PDF writer

		QPdfWriter pdfWriter(&buffer);
		pdfWriter.setTitle(report.path());
		pdfWriter.setPageLayout(report.pageLayout());
		pdfWriter.setResolution(report.resolution());

		QRect pageRectPixels = pdfWriter.pageLayout().paintRectPixels(pdfWriter.resolution());

		QPainter painter(&pdfWriter);

		// Render sections pages
		//
		for (const std::shared_ptr<ReportSection>& section : report.sections())
		{
			if (stop == true)
			{
				return true;
			}

			{
				QMutexLocker l(&m_statisticsMutex);
				m_statistics.sectionIndex++;
			}

			section->render(QSizeF(pageRectPixels.width(), pageRectPixels.height()));

			sectionsPagesCount += section->pageCount();
		}

		// Print PDF

		{
			QMutexLocker l(&m_statisticsMutex);
			m_statistics.status = Statistics::Status::Printing;
			m_statistics.pagesCount = sectionsPagesCount;
		}

		bool firstPage = true;

		for (const std::shared_ptr<ReportSection>& section : report.sections())
		{
			if (stop == true)
			{
				return true;
			}

			if (firstPage == true)
			{
				firstPage = false;
			}
			else
			{
				pdfWriter.newPage();
			}

			// Print text
			//
			printSection(pdfWriter, painter, *section, report.marginItems());

			// Clear document after printing
			//
			section->textDocument().clear();
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

	void ReportPrinter::printMarginItems(QPdfWriter& pdfWriter, QPainter& painter, const QString& objectName, const std::vector<ReportMarginItem>& marginItems) const
	{
		int page = 0;
		int pagesCount = 0;

		{
			QMutexLocker l(&m_statisticsMutex);
			page = m_statistics.pageIndex;
			pagesCount = m_statistics.pagesCount;
		}

		const QRect fullPageRect = pdfWriter.pageLayout().fullRectPixels(pdfWriter.resolution());

		const QRect pageRect = pdfWriter.pageLayout().paintRectPixels(pdfWriter.resolution());

		QMargins margins = pdfWriter.pageLayout().marginsPixels(pdfWriter.resolution());

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

		for (const ReportMarginItem& item : marginItems)
		{
			if (item.pageFrom != -1 && item.pageFrom > page)
			{
				continue;
			}
			if (item.pageTo != -1 && item.pageTo < page)
			{
				continue;
			}

			painter.setFont(item.format.charFormat().font());

			QString text = item.text;

			if (text == "%PAGE%")
			{
				text = QObject::tr("Page %1 of %2").arg(page).arg(pagesCount);
			}

			if (text == "%OBJECT%")
			{
				text = objectName;
			}

			//painter.fillRect(topRect, Qt::green);
			//painter.fillRect(bottomRect, Qt::yellow);

			QFontMetrics fm(item.format.charFormat().font());
			QRect textBoundingRect = fm.boundingRect(text);

			auto itemAlignment = item.format.blockFormat().alignment();

			if (itemAlignment & Qt::AlignTop)
			{
				if (topRect.width() >= textBoundingRect.width() && topRect.height() >= textBoundingRect.height())
				{
					int alignment = itemAlignment & ~Qt::AlignTop;
					painter.drawText(topRect, alignment | Qt::AlignVCenter, text);
				}
			}
			else
			{
				if (itemAlignment & Qt::AlignBottom)
				{
					if (bottomRect.width() >= textBoundingRect.width() && bottomRect.height() >= textBoundingRect.height())
					{
						int alignment = itemAlignment & ~Qt::AlignBottom;
						painter.drawText(bottomRect, alignment | Qt::AlignVCenter, text);
					}
				}
			}
		}
		painter.restore();
	}

	void ReportPrinter::printSection(QPdfWriter& pdfWriter,
									 QPainter& painter,
									 ReportSection& section,
									 const std::vector<ReportMarginItem>& marginItems) const
	{
		QTextDocument& textDocument = section.textDocument();

		auto schemaObject = section.schemaObject();

		if (textDocument.isEmpty() == true)
		{
			if (schemaObject != nullptr)
			{
				// Page contains no text, but contains schema

				// Increase the page number
				{
					QMutexLocker l(&m_statisticsMutex);
					m_statistics.pageIndex++;
				}

				// Print margins

				printMarginItems(pdfWriter, painter, section.caption(), marginItems);
			}
		}
		else
		{
			// Page contains text

			const QRect pageRect = pdfWriter.pageLayout().paintRectPixels(pdfWriter.resolution());

			// The total extent of the content (there are no page margin in this)
			const QRect contentRect = QRect(QPoint(0, 0), textDocument.size().toSize());

			// This is the part of the content we will drop on a page.  It's a sliding window on the content.
			QRect currentRect(0, 0, pageRect.width(), pageRect.height());

			while (currentRect.intersects(contentRect) == true)
			{
				// Increase the page number
				{
					QMutexLocker l(&m_statisticsMutex);
					m_statistics.pageIndex++;
				}

				// Print document part

				painter.save();
				painter.translate(0, -currentRect.y());
				textDocument.drawContents(&painter, currentRect);  // draws part of the document
				painter.restore();

				// Print margins

				printMarginItems(pdfWriter, painter, section.caption(), marginItems);

				// Translate the current rectangle to the area to be printed for the next page

				currentRect.translate(0, currentRect.height());

				//Inserting a new page if there is still area left to be printed

				if (currentRect.intersects(contentRect))
				{
					pdfWriter.newPage();
				}
			}
		}

		// Print schema
		//
		if (schemaObject != nullptr)
		{
			printSectionSchema(pdfWriter, painter, section);
		}

		return;
	}

	void ReportPrinter::printSectionSchema(QPdfWriter& pdfWriter,
										   QPainter& painter,
										   ReportSection& section) const
	{
		auto schemaObject = section.schemaObject();
		if (schemaObject == nullptr)
		{
			return;
		}

		const QTextDocument& textDocument = section.textDocument();

		const std::shared_ptr<VFrame30::Schema>& schema = schemaObject->schema();
		const std::map<QUuid, ReportSchemaCompareAction>& compareActions = schemaObject->compareActions();

		if (m_schemaView == nullptr)
		{
			Q_ASSERT(m_schemaView);
			Q_ASSERT(schema);
			return;
		}

		// Calculate the upper schema offset
		//
		const QRect pageRect = pdfWriter.pageLayout().paintRectPixels(pdfWriter.resolution());

		int schemaTop = 0;
		int schemaLeft = 0;

		if (textDocument.isEmpty() == false)
		{
			const QRect contentRect = QRect(QPoint(0, 0), textDocument.size().toSize());
			schemaTop =  contentRect.height() % pageRect.height();
		}

		const int schemaMaxHeight = pageRect.height() - schemaTop;

		// Calculate draw parameters
		//
		double schemaWidthInPixel = schema->GetDocumentWidth(pdfWriter.physicalDpiX(), 100.0);		// Export 100% zoom
		double schemaHeightInPixel = schema->GetDocumentHeight(pdfWriter.physicalDpiY(), 100.0);		// Export 100% zoom

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

		VFrame30::CDrawParam drawParam(&painter, m_schemaView.get(), schema->gridSize(), schema->pinGridStep(), schema->unit());
		drawParam.setInfoMode(false);
		drawParam.setPdfMode(true);

		m_schemaView->setSchemaInternal(schema);
		m_schemaView->adjust(&painter, schemaLeft, schemaTop, zoom * 100.0);		// Export 100% zoom

		QRectF clipRect(0, 0, schema->docWidth(), schema->docHeight());

		schema->Draw(&drawParam, clipRect);

		if (compareActions.empty() == false)
		{
			drawParam.setControlBarSize(CONTROL_BAR_MM);
			m_schemaView->drawCompareOutlines(&drawParam, clipRect, compareActions);
		}

		painter.restore();

		return;
	}
}
