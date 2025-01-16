#include <ReportLib/Report.h>
#include <ReportLib/ReportObject.h>

namespace ReportLib
{
	//
	// Statistics
	//
	void Statistics::fill(int* progress, int* progressMin, int* progressMax, QString* progressText) const
	{
		if (progress == nullptr || progressMin == nullptr || progressMax == nullptr || progressText == nullptr)
		{
			Q_ASSERT(progress);
			Q_ASSERT(progressMin);
			Q_ASSERT(progressMax);
			Q_ASSERT(progressText);
			return;
		}

		switch (status)
		{
		case None:
			*progressText = QObject::tr("Idle");
			*progress = 0;
			*progressMax = 0;
			break;
		case Preview:
			*progressText = QObject::tr("Generating the preview, section: %1/%2").arg(sectionIndex + 1).arg(sectionCount);
			*progress = sectionIndex + 1;
			*progressMax = sectionCount;
			break;
		case Rendering:
			*progressText = QObject::tr("Rendering the report, section: %1/%2").arg(sectionIndex + 1).arg(sectionCount);
			*progress = sectionIndex + 1;
			*progressMax = sectionCount;
			break;
		case Saving:
			*progressText = QObject::tr("Saving the report, page: %1/%2").arg(pageIndex + 1).arg(pagesCount);
			*progress = pageIndex + 1;
			*progressMax = pagesCount;
			break;
		case Printing:
			*progressText = QObject::tr("Printing the report, page: %1/%2").arg(pageIndex + 1).arg(pagesCount);
			*progress = pageIndex + 1;
			*progressMax = pagesCount;
			break;
		default:
			Q_ASSERT(false);
		}
	}


	//
	// ReportVariables
	//
	QStringList ReportVariables::viewVariables() const
	{
		QStringList result;
		result.reserve(m_variables.size());

		for (const auto& [name, value] : m_variables)
		{
			result.push_back(name);
		}

		return result;
	}

	bool ReportVariables::viewVariableExists(const QString& name) const
	{
		return m_variables.find(name) != m_variables.end();
	}
	
	QVariant ReportVariables::viewVariable(const QString& name) const
	{
		auto it = m_variables.find(name);
		if (it == m_variables.end())
		{
			return "!" + name + "!";
		}
		return it->second;
	}
	
	void ReportVariables::setViewVariable(const QString& name, const QVariant& value)
	{
		m_variables[name] = value.toString();
	}

	void ReportVariables::setVariables(const std::map<QString, QString>& variables)
	{
		for (const auto& [name, value] : variables)
		{
			m_variables[name] = value;
		}
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

	const std::vector<std::shared_ptr<ReportMarginItem>>& Report::marginItems() const
	{
		return m_marginItems;
	}

	void Report::addMarginItem(const ReportMarginItem& item) 
	{
		addMarginItem(std::make_shared<ReportMarginItem>(item));
	}

	void Report::addMarginItem(std::shared_ptr<ReportMarginItem> item)
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

