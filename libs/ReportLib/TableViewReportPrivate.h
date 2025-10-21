#pragma once

#include <ReportLib/ReportPrinter.h>

namespace ReportLib
{
	class Report;
	class ReportSection;
	class ITableViewReportInfo;

	class TableViewReportDataProvider : public QObject
	{
		Q_OBJECT

	public:
		explicit TableViewReportDataProvider(QAbstractItemModel& tableModel);

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

	private:
		const QAbstractItemModel& m_tableModel;
	};
	
	class TableViewReportWorker : public QObject
	{
		Q_OBJECT
	public:
		TableViewReportWorker(const ITableViewReportInfo& reportInfo,
							  const QTableView& table,
							  const std::vector<int>& visibleColumns,
							  const std::vector<int>& columnWidths,
							  const std::vector<int>& selectedRows,
							  bool exportSelected);

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
		bool createReport(const QString& fileName, QString& errorMsg);
		bool createText(const QString& fileName, const QChar& separator, QString& errorMsg);
		bool createHtml(const QString& fileName, QString& errorMsg);

	public:
		static const int m_maxReportStatesForPrint = 1000;
		static const int m_maxReportStatesForCsv = 100000;

	private:
		// Export sources
		//
		const ITableViewReportInfo& m_reportInfo;
		TableViewReportDataProvider m_modelDataProvider;
		ReportPrinter m_printer;


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
	// TableViewReportPrivate
	//
	class TableViewReportPrivate: public QObject
	{
	public:
		TableViewReportPrivate(QWidget* parent,
							   const ITableViewReportInfo& reportInfo,
							   const QTableView& table,
							   const std::vector<int>& visibleColumns,
							   const std::vector<int>& columnWidths,
							   const std::vector<int>& selectedRows,
							   bool exportSelected,
							   const QPageLayout& pageLayout);
		~TableViewReportPrivate();

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
		const QTableView& m_table;
		TableViewReportWorker* m_worker = nullptr;
		std::vector<int> m_selectedRows;
		bool m_exportSelected = false;
	};
} // namespace ReportLib
