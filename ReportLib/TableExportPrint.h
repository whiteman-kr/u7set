#pragma once

#include "ReportPrinter.h"

namespace ReportLib
{
	class ReportSection;
}

namespace ReportLib
{
	class ITableExportPrint
	{
	public:
		virtual void generateHeader(ReportLib::Report& report, ReportLib::ReportSection& mainSection) = 0;
	};

	class TableExportPrintModel : public QAbstractTableModel
	{
		Q_OBJECT

	public:
		TableExportPrintModel(QObject* parent = nullptr);

	signals:
		void signalRowCount(int& rowCount) const;
		void signalColumnCount(int& rowCount) const;
		void signalColumnsText(QStringList& text) const;
		void signalRowsText(int row, QStringList& text) const;

	public slots:
		int exportRowCount() const;
		int exportColumnCount() const;
		QStringList exportColumnsText() const;
		QStringList exportRowsText(int row) const;
	};

	class TableExportPrintPrivateWorker : public QObject
	{
		Q_OBJECT
	public:
		TableExportPrintPrivateWorker(ITableExportPrint& exportPrintInterface,
									  const TableExportPrintModel& model,
									  const std::vector<int>& visibleColumns,
									  const std::vector<int>& columnWidths,
									  const std::vector<int>& selectedRows);

		void setFileName(const QString& fileName);
		void setPrinterInfo(const QPrinterInfo& printerInfo);
		void setPageLayout(const QPageLayout& layout);
		void setExportSelected(bool value);
		void setMaxRows(int value);

	public slots:
		void printTable();
		void exportTable();

		void stop();
		void progressRequested();

		void getProgress(int* progress, int* progressMin, int* progressMax, QString* progressText);

	signals:
		void progressChanged(int progress, int progressMin, int progressMax, const QString& progressText);
		void finished(const QString& errorMessage);

	protected:
		void generateHeader(ReportLib::Report& report, ReportLib::ReportSection& mainSection);

	private:
		bool createReport(const TableExportPrintModel& model, const QString& fileName, QString& errorMsg);
		bool createText(const TableExportPrintModel& model, const QString& fileName, const QChar& separator, QString& errorMsg);

	public:
		static const int m_maxReportStatesForPrint = 1000;
		static const int m_maxReportStatesForCsv = 100000;

	private:
		// Export sources
		//
		ITableExportPrint& m_exportPrintInterface;
		const TableExportPrintModel& m_model;
		std::unique_ptr<ReportLib::ReportPrinter> m_printer;


		// Export parameters
		//
		std::vector<int> m_visibleColumns;
		const std::vector<int> m_columnWidths;
		std::vector<int> m_selectedRows;
		QPageLayout m_pageLayout{QPageSize(QPageSize::A4),
								 QPageLayout::Orientation::Portrait,
								 QMarginsF(25, 20, 15, 20),
								 QPageLayout::Unit::Millimeter};

		bool m_exportSelected = false;
		int m_maxRows = -1;
		QString m_fileName;
		QPrinterInfo m_printerInfo;

		std::atomic_bool m_stop = false; // Stop processing flag, set by stop()

		struct Statistics
		{
			bool printerIsActive = false;
			QString text;
			int value = 0;
			int maxValue = 0;
		};
		Statistics m_statistics;
		QMutex m_statisticsMutex;
	};

	//
	// ExportPrintPrivate
	//
	class TableExportPrintPrivate
	{
	public:
		TableExportPrintPrivate(ITableExportPrint& exportPrintInterface,
								QWidget* parent,
								const TableExportPrintModel& model,
								const std::vector<int>& visibleColumns,
								const std::vector<int>& columnWidths,
								const std::vector<int>& selectedRows,
								const QPageLayout& pageLayout);

		void printTable();
		void exportTable(const QString& fileName);
		void stop();

		const QPageLayout& pageLayout() const;
		void setPageLayout(const QPageLayout& layout);

	private:
		enum class TaskType
		{
			Export,
			Print
		};
		bool pageSetup();
		void run(TaskType task, const QString& fileName);

	private:
		QWidget* m_parent = nullptr;
		QPageLayout m_pageLayout;
		const TableExportPrintModel& m_model;
		TableExportPrintPrivateWorker* m_worker = nullptr;
		std::vector<int> m_selectedRows;
	};

	class TableExportPrint : public QObject,
							 public ITableExportPrint
	{
		Q_OBJECT
	public:
		TableExportPrint(QWidget* parent, const QTableView& table, const TableExportPrintModel& model, const QPageLayout& pageLayout);
		virtual ~TableExportPrint();

		void printTable();
		void exportTable(const QString& fileName);

		const QPageLayout& pageLayout() const;
		void setPageLayout(const QPageLayout& layout);

		static void savePageLayoutToSettings(const QPageLayout& pageLayout, const QString& groupName);
		static QPageLayout loadPageLayoutFromSettings(const QString& groupName, const QPageLayout& defaultPageLayout);

	public slots:
		void stop();

	protected:
		virtual void generateHeader(ReportLib::Report& report, ReportLib::ReportSection& mainSection) override;

	private:
		std::unique_ptr<TableExportPrintPrivate> m_impl;
	};
} // namespace ReportLib
