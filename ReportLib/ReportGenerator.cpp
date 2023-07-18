#include "ReportGenerator.h"

namespace ReportLib
{
	ReportGenerator::ReportGenerator(const ReportTemplate& reportTemplate):
		m_template(reportTemplate)
	{

	}

	bool ReportGenerator::generate(QBuffer& buffer, std::atomic_bool& stop)
	{
		Report report{"ProjectName", m_template.caption()};
		report.setResolution(m_template.resolution());

		// Add margins

		for (const MarginTemplate& mt : m_template.margins())
		{
			report.addMarginItem(mt.marginItem());
		}

		std::shared_ptr<ReportSection> rs;

		// Header

		const auto& header = m_template.header();
		if (header.empty() == false)
		{
			rs = ReportSection::create(header.caption(), m_template.pageLayout());
			report.addSection(rs);

			generateSection(*rs, header);
		}

		// Sections

		bool firstSection = true;

        const std::vector<SectionTemplate>& sections = m_template.sections();
        for (const SectionTemplate& section : sections)
		{
			if (rs == nullptr || firstSection == false)
			{
				rs = ReportSection::create(section.caption(), m_template.pageLayout());
				report.addSection(rs);
			}

			generateSection(*rs, section);

			if (firstSection == true)
			{
				firstSection = false;
			}
		}

		// Footer

		const auto& footer = m_template.footer();
		if (footer.empty() == false)
		{
			if (rs == nullptr)
			{
				rs = ReportSection::create(footer.caption(), m_template.pageLayout());
				report.addSection(rs);
			}

			generateSection(*rs, footer);
		}

		return m_printer.print(report, buffer, stop);
	}

	ReportPrinter::Statistics ReportGenerator::statistics() const
	{
		return m_printer.statistics();
	}

	bool ReportGenerator::generateSection(ReportSection& section, const SectionTemplate& sectionTemplate)
	{

		for (const std::shared_ptr<ObjectTemplate>& object : sectionTemplate.objects())
		{
			if (object->type() == ReportObject::Type::Text)
			{
				const TextTemplate* t = dynamic_cast<const TextTemplate*>(object.get());
				if (t == nullptr)
				{
					Q_ASSERT(t);
					return false;
				}

				if (t->tag().isEmpty() == false)
				{

					// Create text object
					bool ok = false;
					do
					{
						QString s = text(t->tag(), &ok);
						if (ok == true)
						{
							section.addText(s + "\n", t->format());
						}
					}while (ok == true);
				}
				else
				{
					QString s = t->text();
					s.replace("\\n", "\n");
					section.addText(s + "\n", t->format());
				}
			}

			if (object->type() == ReportObject::Type::Table)
			{
				TableTemplate* t = dynamic_cast<TableTemplate*>(object.get());

				// Create table object

				auto table = section.addTable(t->format());

				// Fill table with data
				bool ok = false;
				do
				{
					QString s = text(t->tag(), &ok);
					if (ok == true)
					{
						QStringList l;
						if (t->separator().isEmpty() == false)
						{
							l = s.split(t->separator(), Qt::SkipEmptyParts);
						}
						else
						{
							l << s;
						}
						table->insertRow(l);
					}
				}while (ok == true);
			}
		}

		return true;
	}
}

