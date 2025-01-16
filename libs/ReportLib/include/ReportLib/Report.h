#pragma once

#include <VFrame30/IViewVariables.h>

namespace ReportLib
{
	class ReportSection;
	struct ReportMarginItem;

	//
	// Statistics
	//
	struct Statistics
	{
		enum Status
		{
			None,
			Preview,
			Rendering,
			Saving,
			Printing
		};

		int sectionCount = 0;
		int sectionIndex = 0;

		int pagesCount = 0; // Calculated after text rendering
		int pageIndex = 0;

		Status status{None};

		void fill(int* progress, int* progressMin, int* progressMax, QString* progressText) const;
	};

	//
	// ReportVariables
	//
	class ReportVariables : public VFrame30::IViewVariables
	{
	public:
		// IViewVariables implementation
		//
		[[nodiscard]] QStringList viewVariables() const override;
		[[nodiscard]] bool viewVariableExists(const QString& name) const override;
		[[nodiscard]] QVariant viewVariable(const QString& name) const override;
		void setViewVariable(const QString& name, const QVariant& value) override;

		//
		void setVariables(const std::map<QString, QString>& variables);

	private:
		std::map<QString, QString> m_variables;
	};

	//
	// Report
	//

	class Report
	{
	public:
		Report(const QString& projectName, const QString& path);

	public:
		QString projectName() const;
		QString path() const;

		int resolution() const;
		void setResolution(int value);

		// Contents access functions
		//
		size_t sectionsCount() const;
		std::shared_ptr<ReportSection> section(size_t index) const;

		const std::vector<std::shared_ptr<ReportSection>>& sections() const;

		// Add sectinons functions
		//
		std::shared_ptr<ReportSection> addHeaderSection(std::shared_ptr<ReportSection> section);
		std::shared_ptr<ReportSection> insertSection(int index, std::shared_ptr<ReportSection> section);
		std::shared_ptr<ReportSection> addSection(std::shared_ptr<ReportSection> section);

		// Margins functions
		//
		const std::vector<std::shared_ptr<ReportMarginItem>>& marginItems() const;
		void addMarginItem(const ReportMarginItem& item);
		void addMarginItem(std::shared_ptr<ReportMarginItem> item);
		void clearMarginItems();

		const ReportVariables& reportVariables() const;
		ReportVariables& reportVariables();

		int startPage() const;
		void setStartPage(int value);

	private:
		// Page options
		//
		int m_pageResolution = 600;

		std::vector<std::shared_ptr<ReportSection>> m_sections;
		std::vector <std::shared_ptr<ReportMarginItem>> m_marginItems;
		ReportVariables m_variables;
		int m_startPage{1}; // Report start page number

		QString m_reportName;
		QString m_path;
	};

} // namespace ReportLib
