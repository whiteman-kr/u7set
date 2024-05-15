#include "ReportGenerator.h"
#include "ReportTemplate.h"

namespace ReportLib
{
	ReportGenerator::ReportGenerator(const ReportTemplate& reportTemplate) :
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
			rs = ReportSection::create(header.caption(), header.pageLayout());
			rs->setTag(header.tag());
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
				rs = ReportSection::create(section.caption(), section.pageLayout());
				rs->setTag(section.tag());
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
				rs = ReportSection::create(footer.caption(), footer.pageLayout());
				rs->setTag(footer.tag());
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
		
		static QString firstTag = "$FIRST(";
		static QString lastTag = "$LAST(";
		static QString nextTag = "$NEXT(";

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

				const QString& tag = t->tag();

				if (tag.isEmpty() == false)
				{
					if (tag.startsWith(firstTag) && tag.endsWith(")"))
					{
						// Show only first tag
						//
						QString tagValue = tag;
						tagValue = tagValue.remove(0, firstTag.length());
						tagValue.chop(1);
						
						bool ok = false;
						QString s = text(tagValue, &ok);
						if (ok == true)
						{
							section.addText(s + "\n", t->format());
						}
					}
					else
					{
						if (tag.startsWith(lastTag) && tag.endsWith(")"))
						{
							// Show only last tag
							//
							QString tagValue = tag;
							tagValue = tagValue.remove(0, lastTag.length());
							tagValue.chop(1);

							bool ok = false;
							bool tagFound = false;
							QString s;
							do
							{
								QString tx = text(tagValue, &ok);
								if (ok == true)
								{
									s = tx;
									tagFound = true;
								}
							} while (ok == true);

							if (tagFound == true)
							{
								section.addText(s + "\n", t->format());
							}
						}
						else
						{
							if (tag.startsWith(nextTag) == false)
							{
								// Show all text with all all other tag instances EXCEPT %NEXT tag
								//
								bool ok = false;
								do
								{
									QString s = text(t->tag(), &ok);
									if (ok == true)
									{
										section.addText(s + "\n", t->format());
									}
								} while (ok == true);
							}
						}
					}
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

		{
			// Place all text with $NEXT(Tag) tag
			//
			QString tag;
			int itemsCount = count();
			for (int i = 0; i < itemsCount; i++) 
			{
				QString msg = text(i, &tag);

				QString templateTag = QObject::tr("%1%2)").arg(nextTag).arg(tag);

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

						if (t->tag() == templateTag)
						{
							msg.replace("\\n", "\n");
							section.addText(msg + "\n", t->format());
						}
					}
				}
			}
		}

		return true;
	}
}

