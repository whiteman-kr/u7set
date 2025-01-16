#pragma once

#include <ReportLib/Report.h>

class QPrinter;

namespace ReportLib
{
	class ReportPrinterPrivate;
	class ReportSchemaView;
	class PrintObject;

	//
	// RenderedSection
	//
	struct RenderedSection
	{
		RenderedSection(std::shared_ptr<ReportSection> section);

		int pagesCount() const;

		// ReportSection access
		//
		std::shared_ptr<ReportSection>& section();
		const std::shared_ptr<ReportSection>& section() const;

		// Data access
		//
		const QPageLayout& pageLayout() const;

		// Rendered objects access
		//
		std::vector<std::shared_ptr<PrintObject>>& printObjects();
		const std::vector<std::shared_ptr<PrintObject>>& printObjects() const;

	private:
		std::shared_ptr<ReportSection> m_section;
		std::vector<std::shared_ptr<PrintObject>> m_printObjects;
	};

	//
	// ReportPrinter
	//

	class ReportPrinter            // : public QObject
	{
	public:
		ReportPrinter();
		explicit ReportPrinter(std::shared_ptr<ReportSchemaView> reportSchemaView); // Call this constructor if your report contains schemas
		virtual ~ReportPrinter();

		bool preview(const Report& report, std::vector<RenderedSection>& renderedSections, std::atomic_bool& stop);

		bool save(Report& report, const QString& fileName, std::atomic_bool& stop);
		bool save(Report& report, QBuffer& buffer, std::atomic_bool& stop);

		bool print(Report& report, QPrinter& printer, std::atomic_bool& stop);

		Statistics statistics() const;

	private:
		std::unique_ptr<ReportLib::ReportPrinterPrivate> m_impl;
	};
} // namespace ReportLib
