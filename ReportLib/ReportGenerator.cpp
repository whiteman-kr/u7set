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
        report.setResolution(300);

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
			rs = ReportSection::create(header.caption());
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
				rs = ReportSection::create(section.caption());
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
				rs = ReportSection::create(footer.caption());
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

	bool ReportGenerator::generateSection(ReportSection& section, const SectionTemplate& sectionTemplate) const
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

				// Create text object

				int tagCount = count(t->tag());

				for (int i = 0; i < tagCount; i++)
				{
					section.addText(text(t->tag(), i), t->format());
				}
			}

			if (object->type() == ReportObject::Type::Table)
			{
				TableTemplate* t = dynamic_cast<TableTemplate*>(object.get());

				// Create table object

				auto table = section.addTable(t->format());

				// Fill table with data

				int tagCount = count(t->tag());

				for (int i = 0; i < tagCount; i++)
				{
					QString s = tableText(t->tag(), i);

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

			}
		}

		return true;
	}
}

