#pragma once

#include "ReportObject.h"
#include "ReportSchemaView.h"

#include <QPageLayout>

namespace ReportLib
{
	//
	// ReportSection
	//

	class ReportSection
	{
	public:
		static std::shared_ptr<ReportSection> create(const QString& caption, const QPageLayout& pageLayout);

		explicit ReportSection(const QString& caption);
		virtual ~ReportSection();

		const QString& caption() const;
		void setCaption(const QString& value);

		const QString& tag() const;
		void setTag(const QString& value);

		const QPageLayout& pageLayout() const;
		void setPageLayout(const QPageLayout& value);

		int startPage() const;
		void setStartPage(int page);

		// Add object functions
		//
        std::shared_ptr<ReportText> addText(const QString& text, const TextFormat& format);
		std::shared_ptr<ReportText> addText(std::shared_ptr<ReportText> object);

        std::shared_ptr<ReportTable> addTable(const TableFormat& format);
		std::shared_ptr<ReportTable> addTable(std::shared_ptr<ReportTable> object);

		std::shared_ptr<ReportSchema> addSchema(std::shared_ptr<ReportSchema> object);

		void addObject(std::shared_ptr<ReportObject> object);

		// Object access functions
		//
		size_t objectCount() const;
		std::shared_ptr<ReportObject> object(size_t index);

	private:
		QString m_caption;
		QString m_tag;	// Text printed in margin with %TAG% text
		int m_startPage;
		QPageLayout m_pageLayout = QPageLayout(QPageSize(QPageSize::A4),
											   QPageLayout::Orientation::Portrait,
											   QMarginsF(30, 20, 15, 20),
											   QPageLayout::Unit::Millimeter);
		std::vector<std::shared_ptr<ReportObject>> m_objects;
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
		const std::vector<ReportMarginItem>& marginItems() const;
		void addMarginItem(const ReportMarginItem& item);
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
		std::vector<ReportMarginItem> m_marginItems;
		ReportVariables m_variables;
		int m_startPage{1};						// Report start page number

		QString m_reportName;
		QString m_path;
	};

}

