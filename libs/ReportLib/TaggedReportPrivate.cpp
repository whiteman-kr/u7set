#include "TaggedReportPrivate.h"
#include "ReportPrinterPrivate.h"
#include <ReportLib/ReportPrinter.h>
#include <ReportLib/Report.h>
#include <ReportLib/ReportTemplate.h>
#include <ReportLib/TaggedReportGenerator.h>

namespace ReportLib
{
	TaggedReportPrivate::TaggedReportPrivate(QObject* parent,
											 const ReportTemplate& reportTemplate,
											 const ITaggedReportDataProvider& reportDataProvider) :
		QObject(parent),
		m_template(reportTemplate),
		m_reportDataProvider(reportDataProvider)
	{
	}

	bool TaggedReportPrivate::generate(QBuffer& buffer, std::atomic_bool& stop)
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

		return m_printer.save(report, buffer, stop);
	}

	Statistics TaggedReportPrivate::statistics() const
	{
		return m_printer.statistics();
	}

	bool TaggedReportPrivate::generateSection(ReportSection& section, const SectionTemplate& sectionTemplate)
	{
		static QString firstTag = "$FIRST(";
		static QString lastTag = "$LAST(";
		static QString repeatIndexTag = "$(REPEATINDEX)";
		//static QString nextTag = "$NEXT(";

		// Find the repeat count for section by its tag. If this value is zero, no repeats will be used.
		//
		int repeatCount = 1;
		{
			QString repeatCountRecordTag = tr("SECTION_REPEAT_COUNT(%1)").arg(sectionTemplate.tag());

			bool ok = false;
			QString repeatCountString = m_reportDataProvider.text(repeatCountRecordTag, &ok);
			if (ok == true)
			{
				int result = repeatCountString.toInt(&ok);
				if (ok == true) 
				{
					repeatCount = result;
				}
			}
		}
		
		for (int i = 0; i < repeatCount; i++)
		{
			// Process section objects
			//
			for (const std::shared_ptr<ObjectTemplate>& object : sectionTemplate.objects())
			{
				QString tag = object->tag();

				// Replace $(REPEATINDEX) by iteration number
				//
				if (tag.isEmpty() == false && tag.contains(repeatIndexTag) == true)
				{
					tag = tag.replace(repeatIndexTag, tr("%1").arg(i));
				}

				if (object->type() == ReportObject::Type::Text)
				{
					const TextTemplate* t = dynamic_cast<const TextTemplate*>(object.get());
					if (t == nullptr)
					{
						Q_ASSERT(t);
						return false;
					}

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
							QString s = m_reportDataProvider.text(tagValue, &ok);
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
									QString tx = m_reportDataProvider.text(tagValue, &ok);
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
								// if (tag.startsWith(nextTag) == false)
								//{
								//   Show all text with all all other tag instances /*EXCEPT %NEXT tag*/
								//
								bool ok = false;
								do
								{
									QString s = m_reportDataProvider.text(tag, &ok);
									if (ok == true)
									{
										section.addText(s + "\n", t->format());
									}
								} while (ok == true);
								//}
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
				else if (object->type() == ReportObject::Type::Table)
				{
					TableTemplate* t = dynamic_cast<TableTemplate*>(object.get());
					if (t == nullptr)
					{
						Q_ASSERT(t);
						return false;
					}

					if (tag.isEmpty() == false)
					{
						// Create table object

						auto table = section.addTable(t->format());

						// Fill table with data
						bool ok = false;
						do
						{
							QString s = m_reportDataProvider.text(tag, &ok);
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
						} while (ok == true);
					}
					else
					{
						// Do nothing, table shall always have a tag
					}
				}
			}
		}// repeatCount

		/*
		{
			// Place all text with $NEXT(Tag) tag
			//
			QString tag;
			int itemsCount = m_reportDataProvider.count();
			for (int i = 0; i < itemsCount; i++)
			{
				QString msg = m_reportDataProvider.text(i, &tag);

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
		}*/

		return true;
	}
} // namespace ReportLib
