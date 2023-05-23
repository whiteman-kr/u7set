#include "Report.h"

namespace ReportLib
{

	//
	// ReportFileTypeParams
	//
	ReportFileTypeParams::ReportFileTypeParams(int fileId, const QString& caption, bool selected, QPageLayout pageLayout):
		m_fileId(fileId),
		m_caption(caption),
		m_selected(selected),
		m_pageLayout(pageLayout)
	{
	}

	int ReportFileTypeParams::fileId() const
	{
		return m_fileId;
	}

	const QString& ReportFileTypeParams::caption() const
	{
		return m_caption;
	}

	bool ReportFileTypeParams::selected() const
	{
		return m_selected;
	}

	void ReportFileTypeParams::setSelected(bool value)
	{
		m_selected = value;
	}

	const QPageLayout& ReportFileTypeParams::pageLayout() const
	{
		return m_pageLayout;
	}

	void ReportFileTypeParams::setPageLayout(const QPageLayout& layout)
	{
		m_pageLayout = layout;
	}

	//
	// ReportSection
	//

	std::shared_ptr<ReportSection> ReportSection::create(const QString& caption)
	{
		auto section = std::make_shared<ReportSection>(caption);
		return section;
	}

	ReportSection::ReportSection(const QString& caption):
		m_caption(caption)
	{
	}

	ReportSection::~ReportSection()
	{
	}

	QTextDocument& ReportSection::textDocument()
	{
		return m_textDocument;
	}

	std::shared_ptr<ReportSchema> ReportSection::schemaObject() const
	{
		return m_schemaObject;
	}

	const QString& ReportSection::caption() const
	{
		return m_caption;
	}

	std::shared_ptr<ReportLib::ReportText> ReportSection::addText(const QString& text, const ReportObjectFormat& format)
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

	std::shared_ptr<ReportTable> ReportSection::addTable(const QStringList& headerLabels,
																	const std::vector<int>& columnWidths,
																	const ReportObjectFormat& format)
	{
		std::shared_ptr<ReportTable> object = std::make_shared<ReportLib::ReportTable>(headerLabels,
																								  columnWidths,
																								  format);
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
		if (m_schemaObject != nullptr)
		{
			Q_ASSERT(false);
			return {};
		}
		m_schemaObject = object;
		return object;
	}

	void ReportSection::render(QSizeF pageSize)
	{
		// Document has content

		m_textDocument.setPageSize(pageSize);

		QTextCursor textCursor(&m_textDocument);

		for (const std::shared_ptr<ReportObject>& object : m_textObjects)
		{
			if (object == nullptr)
			{
				Q_ASSERT(object);
				return;
			}

			object->renderText(textCursor);
		}

		if (m_textDocument.isEmpty() == true)
		{
			m_pageCount = m_schemaObject == nullptr ? 0 : 1;
		}
		else
		{
			m_pageCount = m_textDocument.pageCount();
		}

		return;
	}

	int ReportSection::pageCount() const
	{
		return m_pageCount;
	}

	void ReportSection::addObject(std::shared_ptr<ReportObject> object)
	{
		m_textObjects.push_back(object);
	}


	//
	// ReportGenerator
	//

	Report::Report(const QString& reportName, const QString& path):
		m_reportName(reportName),
		m_path(path)
	{
	}

	QString Report::projectName() const
	{
		return m_reportName;
	}

	QString Report::path() const
	{
		return m_path;
	}

	QPageLayout Report::pageLayout() const
	{
		return m_pageLayout;
	}

	void Report::setPageLayout(const QPageLayout& value)
	{
		m_pageLayout = value;
	}

	int Report::resolution() const
	{
		return m_pageResolution;
	}

	void Report::setResolution(int value)
	{
		m_pageResolution = value;
	}

	const std::vector<std::shared_ptr<ReportSection>>& Report::sections() const
	{
		return m_sections;
	}

	std::shared_ptr<ReportSection> Report::addHeaderSection(std::shared_ptr<ReportSection> section)
	{
		m_sections.insert(m_sections.begin(), section);
		return section;
	}

	std::shared_ptr<ReportSection> Report::insertSection(int index, std::shared_ptr<ReportSection> section)
	{
		if (index < 0 || index > m_sections.size())
		{
			Q_ASSERT(false);
			return nullptr;
		}

		m_sections.insert(m_sections.begin() + index, section);
		return section;
	}

	std::shared_ptr<ReportSection> Report::addSection(std::shared_ptr<ReportSection> section)
	{
		m_sections.push_back(section);
		return section;
	}

	const std::vector<ReportMarginItem>& Report::marginItems() const
	{
		return m_marginItems;
	}

	void Report::addMarginItem(const ReportMarginItem& item)
	{
		m_marginItems.push_back(item);
	}

	void Report::clearMarginItems()
	{
		m_marginItems.clear();
	}
}

