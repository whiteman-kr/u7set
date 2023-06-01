#pragma once

#include "ReportObject.h"
#include "ReportSchemaView.h"

#include <QPageLayout>

namespace ReportLib
{
	//
	// ReportFileTypeParams
	//

	class ReportFileTypeParams
	{
	public:
		ReportFileTypeParams(int fileId, const QString& caption, bool selected, QPageLayout pageLayout);

		int fileId() const;
		const QString& caption() const;

		bool selected() const;
		void setSelected(bool value);

		const QPageLayout& pageLayout() const;
		void setPageLayout(const QPageLayout& layout);

	private:
		int m_fileId = -1;
		QString m_caption;
		bool m_selected = false;

		// Multiple-file report section page options
		//
		QPageLayout m_pageLayout;
	};

	//
	// ReportSection
	//

	class ReportSection
	{
	public:
		static std::shared_ptr<ReportSection> create(const QString& caption);

		explicit ReportSection(const QString& caption);
		virtual ~ReportSection();

		const QString& caption() const;

		// Add object functions
		//
		std::shared_ptr<ReportText> addText(const QString& text, const ReportObjectFormat& format);
		std::shared_ptr<ReportText> addText(std::shared_ptr<ReportText> object);

        std::shared_ptr<ReportTable> addTable(const QStringList& headerLabels, const std::vector<int>& columnWidths, const ReportObjectFormat& format);
		std::shared_ptr<ReportTable> addTable(std::shared_ptr<ReportTable> object);

		std::shared_ptr<ReportSchema> addSchema(std::shared_ptr<ReportSchema> object);

		void addObject(std::shared_ptr<ReportObject> object);

		// Object access functions
		//
		size_t objectCount() const;
		std::shared_ptr<ReportObject> object(size_t index);

	private:
		QString m_caption;

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

		QPageLayout pageLayout() const;
		void setPageLayout(const QPageLayout& value);

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

	private:
		// Page options
		//
		QPageLayout m_pageLayout = QPageLayout(QPageSize(QPageSize::A4), QPageLayout::Orientation::Portrait, QMarginsF(15, 15, 15, 15));

		int m_pageResolution = 600;

		std::vector<std::shared_ptr<ReportSection>> m_sections;
		std::vector<ReportMarginItem> m_marginItems;

		QString m_reportName;
		QString m_path;
	};

}

