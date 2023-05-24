#include "ReportPrinter.h"
#include "../VFrame30/VFrameTools.h"

namespace ReportLib
{
	//
	// PrintText
	//

	PrintText::PrintText(QSizeF pageSize):
		m_textCusror(&m_textDocument)
	{
		m_type = Type::Text;
		m_textDocument.setPageSize(pageSize);
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

	void PrintText::print(QPdfWriter& pdfWriter, QPainter& painter, int /*vOffset*/)
	{
		if (m_textDocument.isEmpty() == true)
		{
			return;

		}
		// Page contains text

		const QRect pageRect = pdfWriter.pageLayout().paintRectPixels(pdfWriter.resolution());

		// The total extent of the content (there are no page margin in this)
		const QRect contentRect = QRect(QPoint(0, 0), m_textDocument.size().toSize());

		// This is the part of the content we will drop on a page.  It's a sliding window on the content.
		QRect currentRect(0, 0, pageRect.width(), pageRect.height());

		while (currentRect.intersects(contentRect) == true)
		{
			// Increase the page number
			/*{
				QMutexLocker l(&m_statisticsMutex);
				m_statistics.pageIndex++;
			}*/

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
			}
		}
	}

	PrintNewPage::PrintNewPage()
	{
		m_type = Type::NewPage;
	}

	void PrintNewPage::print(QPdfWriter& pdfWriter, QPainter& /*painter*/, int /*vOffset*/)
	{
		pdfWriter.newPage();
	}

	QRect PrintNewPage::contentRect() const
	{
		return QRect{0, 0, 0, 0};
	}

	//
	// PrintSchema
	//

	PrintSchema::PrintSchema(const std::shared_ptr<ReportSchemaView>& schemaView,
							 const std::shared_ptr<VFrame30::Schema>& schema,
							 const std::map<QUuid, ReportSchemaCompareAction>& compareActions):
		m_schemaView(schemaView),
		m_schema(schema),
		m_compareActions(compareActions)
	{
		m_type = Type::Schema;
	}

	QRect PrintSchema::contentRect() const
	{
		return QRect{0, 0, 0, 0};
	}

	void PrintSchema::print(QPdfWriter& pdfWriter, QPainter& painter, int vOffset)
	{
		if (m_schemaView == nullptr || m_schema == nullptr)
		{
			Q_ASSERT(m_schemaView);
			Q_ASSERT(m_schema);
			return;
		}

		// Calculate the upper schema offset
		//
		const QRect pageRect = pdfWriter.pageLayout().paintRectPixels(pdfWriter.resolution());

		int schemaTop = vOffset;
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
		drawParam.setInfoMode(false);
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

		return;
	}

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
		int add_margins_and_page_counter_print = 1;

		m_printObjects.clear();

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

		const QRect pageRectPixels = pdfWriter.pageLayout().paintRectPixels(pdfWriter.resolution());

		QPainter painter(&pdfWriter);

		bool firstSection = true;

		// Render all objects to print objects
		//
		for (const std::shared_ptr<ReportSection>& section : report.sections())
		{
			if (stop == true)
			{
				return true;
			}

			if (firstSection == true)
			{
				firstSection = false;
			}
			else
			{
				// New section starts from new page
				auto pnp = std::make_shared<PrintNewPage>();
				m_printObjects.push_back(pnp);
			}

			{
				QMutexLocker l(&m_statisticsMutex);
				m_statistics.sectionIndex++;
			}

			size_t count = section->objectCount();

			bool firstObject = true;

			std::shared_ptr<PrintText> pt;
			std::shared_ptr<PrintSchema> ps;

			ReportObject::Type lastObjectType = ReportObject::Type::Undefined;

			for (size_t i = 0; i < count; i++)
			{
				std::shared_ptr<ReportObject> object = section->object(i);

				if (firstObject == true || lastObjectType == ReportObject::Type::Schema)
				{
					firstObject = false;
					lastObjectType = object->type();

					// Create new text print object
					//
					if (object->type() == ReportObject::Type::Text || object->type() == ReportObject::Type::Table)
					{
						pt = std::make_shared<PrintText>(pageRectPixels.size());
						m_printObjects.push_back(pt);
					}
				}

				// Create new schema print object
				//
				if (object->type() == ReportObject::Type::Schema)
				{
					ReportSchema* rs = dynamic_cast<ReportSchema*>(object.get());
					if (rs == nullptr)
					{
						Q_ASSERT(rs);
						continue;
					}

					ps = std::make_shared<PrintSchema>(m_schemaView,
													   rs->schema(),
													   rs->compareActions());
					m_printObjects.push_back(ps);

					// Insert new page after schema if this object is not last
					//
					if (i != count -1)
					{
						auto pnp = std::make_shared<PrintNewPage>();
						m_printObjects.push_back(pnp);
					}
				}

				// Create new page object
				//
				if (object->type() == ReportObject::Type::NewPage)
				{
					auto pnp = std::make_shared<PrintNewPage>();
					m_printObjects.push_back(pnp);
				}

				// Render contents to print object
				//
				if (object->type() == ReportObject::Type::Text || object->type() == ReportObject::Type::Table)
				{
					if (pt == nullptr)
					{
						Q_ASSERT(pt);
						return false;
					}

					object->renderText(pt->textCursor());
				}

				/*
				if (m_textDocument.isEmpty() == true)
				{
					m_pageCount = m_schemaObject == nullptr ? 0 : 1;
				}
				else
				{
					m_pageCount = m_textDocument.pageCount();
				}
				*/
			}

			//sectionsPagesCount += section->pageCount();
		}

		// Print PDF

		{
			QMutexLocker l(&m_statisticsMutex);
			m_statistics.status = Statistics::Status::Printing;
			m_statistics.pagesCount = sectionsPagesCount;
		}

		int textBottomOffset = 0;

		for (const std::shared_ptr<PrintObject>& po : m_printObjects)
		{
			if (stop == true)
			{
				return true;
			}

			po->print(pdfWriter, painter, textBottomOffset);

			textBottomOffset = po->contentRect().height() % pageRectPixels.height();
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

	/*void ReportPrinter::printMarginItems(QPdfWriter& pdfWriter, QPainter& painter, const QString& objectName, const std::vector<ReportMarginItem>& marginItems) const
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

		QMargins margins = pdfWriter.pageLayout().marginsPixels(resolution);

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


			QFont font{item.format.charFormat().font()};

			{
				QFontMetrics coefMetrics{font};

				double fontCoef = static_cast<double>(coefMetrics.height()) / font.pointSize();

				int pointSize = font.pointSize() * 72 / resolution/*translate points to pixels*//*;

				font.setPointSize(static_cast<int>(pointSize * fontCoef));
			}

			painter.setFont(font);

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

			QFontMetrics fm(font);
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
	}*/

	/*
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
	}*/
}
