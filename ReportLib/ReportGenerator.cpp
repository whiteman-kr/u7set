#include "ReportGenerator.h"

namespace ReportLib
{
	ReportGenerator::ReportGenerator(const ReportTemplate& reportTemplate):
		m_template(reportTemplate)
	{

	}

	bool ReportGenerator::generate(QBuffer& buffer, std::atomic_bool& stop)
	{
		Report report{"ProjectName", "ReportName"};
        report.setResolution(300);

        double kf = report.resolution() / 72;

		// Create report contents

        const std::vector<SectionTemplate>& sections = m_template.sections();

        for (const SectionTemplate& section : sections)
		{
            auto rs = ReportSection::create(section.caption);
            report.addSection(rs);

            for (const std::shared_ptr<ObjectTemplate>& object : section.objects)
			{
                if (object->objectType == ObjectTemplate::Type::Text)
                {
                    const TextTemplate* t = dynamic_cast<const TextTemplate*>(object.get());

                    rs->addText(text(object->tag), {t->fontName, t->fontSize * kf, t->alignment});
                }

                if (object->objectType == ObjectTemplate::Type::Table)
                {
                    TableTemplate* t = dynamic_cast<TableTemplate*>(object.get());

                    QStringList headerLabels;
                    std::vector<int> widths;
                    Qt::Alignment alignment;

                    for (const auto& col : t->columns)
                    {
                        int todo_every_column_has_alignment = 1;

                        headerLabels.push_back(col.caption);
                        widths.push_back(100 / t->columns.size());
                        alignment = col.alignment;
                    }

                    // Create table object

                    auto table = rs->addTable(headerLabels, widths, {t->fontName, t->fontSize * kf, alignment});

                    // Fill table with data

                    QStringList l;
                    for (int i = 0; i < t->columns.size(); i++)
                    {
                        l << tableText(t->tag, i);
                    }
                    table->insertRow(l);
                }
                //	Fill report data here
			}
		}

		return m_printer.print(report, buffer, stop);
	}

	ReportPrinter::Statistics ReportGenerator::statistics() const
	{
		return m_printer.statistics();
	}
}
