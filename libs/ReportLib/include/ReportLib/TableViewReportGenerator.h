#pragma once

#include <ReportLib/Report.h>

namespace ReportLib
{
	class TableViewReportPrivate;

	//
	// ITableViewReportInfo
	//
	class ITableViewReportInfo
	{
	public:
		virtual void generateHeader(ReportLib::Report& report, ReportLib::ReportSection& mainSection) const = 0;
	};

	//
	// TableViewReportGenerator
	//
	class TableViewReportGenerator : public QObject
	{
		Q_OBJECT
	public:
		TableViewReportGenerator(QWidget* parent,
								 const QTableView& table,
								 const ITableViewReportInfo& reportInfo,
								 const QPageLayout& pageLayout,
								 bool exportSelected);

		void printTable();
		void exportTable(const QString& fileName);

		const QPageLayout& pageLayout() const;
		void setPageLayout(const QPageLayout& layout);

		static void savePageLayoutToSettings(const QPageLayout& pageLayout, const QString& groupName);
		static QPageLayout loadPageLayoutFromSettings(const QString& groupName, const QPageLayout& defaultPageLayout);

	public slots:
		void stop();

	private:
		TableViewReportPrivate* m_impl = nullptr;
	};
} // namespace ReportLib
