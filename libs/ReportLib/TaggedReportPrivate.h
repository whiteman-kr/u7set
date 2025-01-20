#pragma once

#include <ReportLib/ReportPrinter.h>

namespace ReportLib
{
	class ReportSection;
	class ReportTemplate;
	class SectionTemplate;
	struct Statistics;
	class ITaggedReportDataProvider;

	//
	// TaggedReportPrivate
	//
	class TaggedReportPrivate : public QObject
	{
	public:
		explicit TaggedReportPrivate(QObject* parent,
									 const ReportTemplate& reportTemplate,
									 const ITaggedReportDataProvider& reportDataProvider);
		virtual ~TaggedReportPrivate() = default;

		bool generate(QBuffer& buffer, std::atomic_bool& stop);

		Statistics statistics() const;

	private:
		bool generateSection(ReportSection& section, const SectionTemplate& sectionTemplate);

	private:
		const ReportTemplate& m_template;
		const ITaggedReportDataProvider& m_reportDataProvider;
		ReportPrinter m_printer;
	};
} // namespace ReportLib
