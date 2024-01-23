#include "Report.h"

namespace ReportLib
{
	//
	// ReportSection
	//

	std::shared_ptr<ReportSection> ReportSection::create(const QString& caption, const QPageLayout& pageLayout)
	{
		auto section = std::make_shared<ReportSection>(caption);
		section->setPageLayout(pageLayout);
		return section;
	}

	ReportSection::ReportSection(const QString& caption):
		m_caption(caption),
		m_startPage(0)
	{
	}

	ReportSection::~ReportSection()
	{
	}

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

	//
	// Report
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

	int Report::resolution() const
	{
		return m_pageResolution;
	}

	void Report::setResolution(int value)
	{
		m_pageResolution = value;
	}

	size_t Report::sectionsCount() const
	{
		return m_sections.size();
	}

	std::shared_ptr<ReportSection> Report::section(size_t index) const
	{
		if (index >= sectionsCount())
		{
			Q_ASSERT(false);
			return nullptr;
		}
		return m_sections[index];
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

	const ReportVariables& Report::reportVariables() const
	{
		return m_variables;
	}

	ReportVariables& Report::reportVariables()
	{
		return m_variables;
	}

	int Report::startPage() const
	{
		return m_startPage;
	}

	void Report::setStartPage(int value)
	{
		m_startPage = value;
	}

}

