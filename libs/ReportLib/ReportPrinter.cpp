#include <ReportLib/ReportPrinter.h>
#include "ReportPrinterPrivate.h"

#include <ReportLib/Report.h>

namespace ReportLib
{
	//
	// RenderedSection
	//
	RenderedSection::RenderedSection(std::shared_ptr<ReportSection> section) :
		m_section(section)
	{
	}

	int RenderedSection::pagesCount() const
	{
		int result = 0;
		for (const std::shared_ptr<PrintObject>& po : m_printObjects)
		{
			result += po->pageCount();
		}
		return result;
	}

	std::shared_ptr<ReportSection>& RenderedSection::section()
	{
		return m_section;
	}

	const std::shared_ptr<ReportSection>& RenderedSection::section() const
	{
		return m_section;
	}

	const QPageLayout& RenderedSection::pageLayout() const
	{
		return m_section->pageLayout();
	}

	std::vector<std::shared_ptr<PrintObject>>& RenderedSection::printObjects()
	{
		return m_printObjects;
	}

	const std::vector<std::shared_ptr<PrintObject>>& RenderedSection::printObjects() const
	{
		return m_printObjects;
	}


	//
	// ReportPrinter
	//

	ReportPrinter::ReportPrinter() :
		m_impl{std::make_unique<ReportPrinterPrivate>()}
	{
	}

	ReportPrinter::ReportPrinter(std::shared_ptr<ReportSchemaView> reportSchemaView) :
		m_impl{std::make_unique<ReportPrinterPrivate>(reportSchemaView)}
	{
	}

	ReportPrinter::~ReportPrinter() = default;

	bool ReportPrinter::preview(const Report& report, std::vector<RenderedSection>& renderedSections, std::atomic_bool& stop)
	{
		return m_impl->preview(report, renderedSections, stop);
	}

	bool ReportPrinter::save(Report& report, const QString& fileName, std::atomic_bool& stop)
	{
		return m_impl->save(report, fileName, stop);
	}

	bool ReportPrinter::save(Report& report, QBuffer& buffer, std::atomic_bool& stop)
	{
		return m_impl->save(report, buffer, stop);
	}

	bool ReportPrinter::print(Report& report, QPrinter& printer, std::atomic_bool& stop)
	{
		return m_impl->print(report, printer, stop);
	}

	ReportLib::Statistics ReportPrinter::statistics() const
	{
		return m_impl->statistics();
	}

}
