#pragma once

#include "Report.h"
#include "ReportPrinter.h"
#include "ReportTemplate.h"

namespace ReportLib
{
	class ReportGenerator
	{
	public:
		ReportGenerator(const ReportTemplate& reportTemplate);

		bool generate(QBuffer& buffer, std::atomic_bool& stop);

		ReportPrinter::Statistics statistics() const;

	protected:
		virtual int count(const QString& tag) const = 0;

		virtual QString text(const QString& tag, int index) const = 0;
		virtual QString tableText(const QString& tag, int index) const = 0;

	private:
		bool generateSection(ReportSection& section, const SectionTemplate& sectionTemplate) const;

	private:
		const ReportTemplate& m_template;

		ReportPrinter m_printer;
	};
}
