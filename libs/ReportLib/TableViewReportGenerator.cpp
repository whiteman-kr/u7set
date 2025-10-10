#include <ReportLib/ReportPrinter.h>
#include "TableViewReportPrivate.h"
#include <ReportLib/Report.h>
#include <ReportLib/TableViewReportGenerator.h>
#include <UiLib/DialogProgress.h>
#include <UiLib/UiTools.h>

namespace ReportLib
{
	TableViewReportGenerator::TableViewReportGenerator(QWidget* parent,
													   const QTableView& table,
													   const ITableViewReportInfo& reportInfo,
													   const QPageLayout& pageLayout):
		QObject(parent)
	{
		std::vector<int> visibleColumns;
		std::vector<int> columnWidths;
		int count = table.model()->columnCount();
		for (int i = 0; i < count; i++)
		{
			if (table.horizontalHeader()->isSectionHidden(i) == false)
			{
				visibleColumns.push_back(i);
				columnWidths.push_back(table.columnWidth(i));
			}
		}

		std::vector<int> selectedRows;
		auto rowsIndexes = table.selectionModel()->selectedRows();
		for (const auto& ri : rowsIndexes)
		{
			selectedRows.push_back(ri.row());
		}

		m_impl = new TableViewReportPrivate(parent, reportInfo, table, visibleColumns, columnWidths, selectedRows, pageLayout);
	}

	void TableViewReportGenerator::printTable()
	{
		return m_impl->printTable();
	}

	void TableViewReportGenerator::exportTable(const QString& fileName)
	{
		return m_impl->exportTable(fileName);
	}

	const QPageLayout& TableViewReportGenerator::pageLayout() const
	{
		return m_impl->pageLayout();
	}

	void TableViewReportGenerator::setPageLayout(const QPageLayout& layout)
	{
		m_impl->setPageLayout(layout);
	}

	void TableViewReportGenerator::savePageLayoutToSettings(const QPageLayout& pageLayout, const QString& groupName)
	{
		QSettings settings;
		settings.beginGroup(groupName);
		settings.setValue("PageSize", pageLayout.pageSize().id());
		settings.setValue("Orientation", static_cast<int>(pageLayout.orientation()));
		settings.setValue("MarginsLeft", pageLayout.margins().left());
		settings.setValue("MarginsTop", pageLayout.margins().top());
		settings.setValue("MarginsRight", pageLayout.margins().right());
		settings.setValue("MarginsBottom", pageLayout.margins().bottom());
		settings.setValue("Units", static_cast<int>(pageLayout.units()));
		settings.endGroup();
	}

	QPageLayout TableViewReportGenerator::loadPageLayoutFromSettings(const QString& groupName, const QPageLayout& defaultPageLayout)
	{
		QSettings settings;
		settings.beginGroup(groupName);
		QPageSize::PageSizeId pageSizeId =
			static_cast<QPageSize::PageSizeId>(settings.value("PageSize", defaultPageLayout.pageSize().id()).toInt());
		QPageSize pageSize(pageSizeId);
		QPageLayout::Orientation orientation =
			static_cast<QPageLayout::Orientation>(settings.value("Orientation", defaultPageLayout.orientation()).toInt());
		QMarginsF margins(settings.value("MarginsLeft", defaultPageLayout.margins().left()).toDouble(),
						  settings.value("MarginsTop", defaultPageLayout.margins().top()).toDouble(),
						  settings.value("MarginsRight", defaultPageLayout.margins().right()).toDouble(),
						  settings.value("MarginsBottom", defaultPageLayout.margins().bottom()).toDouble());
		QPageLayout::Unit units = static_cast<QPageLayout::Unit>(settings.value("Units", defaultPageLayout.units()).toInt());
		settings.endGroup();

		QPageLayout pageLayout(pageSize, orientation, margins, units);
		return pageLayout;
	}

	void TableViewReportGenerator::stop()
	{
		return m_impl->stop();
	}
} // namespace ReportLib