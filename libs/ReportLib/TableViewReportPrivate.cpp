#include "TableViewReportPrivate.h"
#include <ReportLib/Report.h>
#include <ReportLib/TableViewReportGenerator.h>
#include "ReportPrinterPrivate.h"
#include <UiLib/DialogProgress.h>
#include <UiLib/UiTools.h>

namespace ReportLib
{
	TableViewReportDataProvider::TableViewReportDataProvider(QAbstractItemModel& tableModel) :
		m_tableModel(tableModel)
	{
		connect(
			this,
			&TableViewReportDataProvider::signalRowCount,
			this,
			[this](int& result)
			{
				result = m_tableModel.rowCount();
			},
			Qt::BlockingQueuedConnection);

		connect(
			this,
			&TableViewReportDataProvider::signalColumnCount,
			this,
			[this](int& result)
			{
				result = m_tableModel.columnCount();
			},
			Qt::BlockingQueuedConnection);

		connect(
			this,
			&TableViewReportDataProvider::signalColumnsText,
			this,
			[this](QStringList& text)
			{
				text.clear();
				int count = m_tableModel.columnCount();
				for (int i = 0; i < count; i++)
				{
					text.push_back(m_tableModel.headerData(i, Qt::Horizontal, Qt::DisplayRole).toString());
				}
			},
			Qt::BlockingQueuedConnection);

		connect(
			this,
			&TableViewReportDataProvider::signalRowsText,
			this,
			[this](int row, QStringList& text)
			{
				text.clear();
				int count = m_tableModel.columnCount();
				for (int i = 0; i < count; i++)
				{
					text.push_back(m_tableModel.data(m_tableModel.index(row, i), Qt::DisplayRole).toString());
				}
			},
			Qt::BlockingQueuedConnection);
	}

	int TableViewReportDataProvider::exportRowCount() const
	{
		int result = 0;
		emit signalRowCount(result);
		return result;
	}

	int TableViewReportDataProvider::exportColumnCount() const
	{
		int result = 0;
		emit signalColumnCount(result);
		return result;
	}

	QStringList TableViewReportDataProvider::exportColumnsText() const
	{
		QStringList result;
		emit signalColumnsText(result);
		return result;
	}

	QStringList TableViewReportDataProvider::exportRowsText(int row) const
	{
		QStringList result;
		emit signalRowsText(row, result);
		return result;
	}

	//
	// --------------------------------------------- ExportPrintPrivateWorker --------------------------
	//
	TableViewReportWorker::TableViewReportWorker(const ITableViewReportInfo& reportInfo,
												 const QTableView& table,
												 const std::vector<int>& visibleColumns,
												 const std::vector<int>& columnWidths,
												 const std::vector<int>& selectedRows,
												 bool exportSelected) :
		m_reportInfo{reportInfo},
		m_modelDataProvider(*table.model()),
		m_visibleColumns(visibleColumns),
		m_columnWidths(columnWidths),
		m_selectedRows(selectedRows),
		m_exportSelected(m_selectedRows.size() > 0 ? exportSelected : false)
	{
		
	}

	void TableViewReportWorker::setFileName(const QString& fileName)
	{
		m_fileName = fileName;
	}

	void TableViewReportWorker::setPrinterInfo(const QPrinterInfo& printerInfo)
	{
		m_printerInfo = printerInfo;
	}

	void TableViewReportWorker::setPageLayout(const QPageLayout& layout)
	{
		m_pageLayout = layout;
	}

	void TableViewReportWorker::setExportSelected(bool value)
	{
		m_exportSelected = value;
	}

	void TableViewReportWorker::setMaxRows(int value)
	{
		m_maxRows = value;
	}

	void TableViewReportWorker::printTable()
	{
		QString errorMsg;
		createReport(QString(), errorMsg);
		emit finished(errorMsg);
	}

	void TableViewReportWorker::exportTable()
	{
		QFileInfo fileInfo(m_fileName);
		QString extension = fileInfo.completeSuffix();
		QString errorMsg;

		if (extension.compare(QLatin1String("csv"), Qt::CaseInsensitive) == 0)
		{
			createText(m_fileName, ';', errorMsg);
		}
		else
		{
			if (extension.compare(QLatin1String("txt"), Qt::CaseInsensitive) == 0)
			{
				createText(m_fileName, '\t', errorMsg);
			}
			else
			{
				if (extension.compare(QLatin1String("pdf"), Qt::CaseInsensitive) == 0)
				{
					createReport(m_fileName, errorMsg);
				}
				else
				{
					if (extension.compare(QLatin1String("html"), Qt::CaseInsensitive) == 0 || extension.compare(QLatin1String("htm"), Qt::CaseInsensitive) == 0)
					{
						createHtml(m_fileName, errorMsg);
					}
					else
					{
						errorMsg = tr("Unsupported file format!");
					}
				}
			}
		}

		emit finished(errorMsg);
	}

	void TableViewReportWorker::stop()
	{
		m_stop = true;
	}

	void TableViewReportWorker::progressRequested()
	{
		QString progressText;

		int progress = 0;
		int progressMin = 0;
		int progressMax = 0;

		getProgress(&progress, &progressMin, &progressMax, &progressText);

		emit progressChanged(progress, 0, progressMax, progressText);

		return;
	}

	void TableViewReportWorker::getProgress(int* progress, int* progressMin, int* progressMax, QString* progressText)
	{
		if (progress == nullptr || progressMin == nullptr || progressMax == nullptr || progressText == nullptr)
		{
			Q_ASSERT(progress);
			Q_ASSERT(progressMin);
			Q_ASSERT(progressMax);
			Q_ASSERT(progressText);
			return;
		}

		QMutexLocker l(&m_statisticsMutex);
		if (m_statistics.printerIsActive == true)
		{
			m_printer.statistics().fill(progress, progressMin, progressMax, progressText);
		}
		else
		{
			*progress = m_statistics.value;
			*progressMin = 0;
			*progressMax = m_statistics.maxValue;
			*progressText = m_statistics.text;
		}
	}

	void TableViewReportWorker::generateHeader(ReportLib::Report& report, ReportLib::ReportSection& mainSection)
	{
		m_reportInfo.generateHeader(report, mainSection);
	}

	bool TableViewReportWorker::createReport(const QString& fileName, QString& errorMsg)
	{
		QStringList columnsList = m_modelDataProvider.exportColumnsText();

		// Create report objects
		//
		ReportLib::Report report("Report", fileName);
		ReportLib::ReportFont font{"Arial", 10};
		int borderWidth = 0;

		// Create main section
		//
		std::shared_ptr<ReportLib::ReportSection> mainSection = std::make_shared<ReportLib::ReportSection>("Main");
		mainSection->setPageLayout(m_pageLayout);
		report.addSection(mainSection);

		// Create report header
		//
		generateHeader(report, *mainSection);

		// Create main table
		//
		int allWidth = 0;
		for (int i = 0; i < m_visibleColumns.size(); i++)
		{
			allWidth += m_columnWidths[i];
		}

		std::vector<ReportLib::TableFormat::ColumnFormat> columns;
		for (int i = 0; i < m_visibleColumns.size(); i++)
		{
			int columnWidth = static_cast<int>(static_cast<double>(m_columnWidths[i]) / allWidth * 100 + 0.5);
			columns.push_back({columnsList[m_visibleColumns[i]], columnWidth, Qt::AlignLeft});
		}

		ReportLib::TableFormat format{font, columns, borderWidth};
		std::shared_ptr<ReportLib::ReportTable> table = std::make_shared<ReportLib::ReportTable>(format);
		mainSection->addTable(table);

		// Fill the table
		//
		std::vector<int> rowsToProcess;
		int rowCount = m_modelDataProvider.exportRowCount();
		if (m_exportSelected == true)
		{
			rowsToProcess = m_selectedRows;
		}
		else
		{
			for (int row = 0; row < rowCount; row++)
			{
				rowsToProcess.push_back(row);
			}
		}

		{
			QMutexLocker l(&m_statisticsMutex);
			m_statistics.maxValue = m_maxRows > 0 ? m_maxRows : static_cast<int>(rowsToProcess.size());
			m_statistics.value = 0;
		}

		for (int row : rowsToProcess)
		{
			{
				QMutexLocker l(&m_statisticsMutex);
				if (m_statistics.value % 100 == 0)
				{
					m_statistics.text = tr("Processing data... %1/%2").arg(m_statistics.value + 1).arg(m_statistics.maxValue);
				}
				m_statistics.value++;

				if (m_maxRows > 0 && m_statistics.value > m_maxRows)
				{
					break;
				}
			}

			if (m_stop.load() == true)
			{
				errorMsg = QObject::tr("The process was interrupted.");
				return true;
			}

			QStringList rowsList = m_modelDataProvider.exportRowsText(row);
			QStringList rowStrings;
			for (int i = 0; i < m_visibleColumns.size(); i++)
			{
				const QString& cellText = rowsList[m_visibleColumns[i]];
				rowStrings.push_back(cellText);
			}
			table->insertRow(rowStrings);
		}

		{
			QMutexLocker l(&m_statisticsMutex);
			m_statistics.printerIsActive = true;
		}

		bool result = false;

		if (fileName.isEmpty() == true)
		{
			QPrinter printer(m_printerInfo, QPrinter::HighResolution);
			result = m_printer.print(report, printer, m_stop);
		}
		else
		{
			result = m_printer.save(report, fileName, m_stop);
		}

		if (result == false)
		{
			errorMsg = QObject::tr("An error has been occurred.");
		}
		else
		{
			if (m_stop.load() == true)
			{
				errorMsg = QObject::tr("The process was interrupted.");
			}
		}


		return result;
	}

	bool TableViewReportWorker::createText(const QString& fileName, const QChar& separator, QString& errorMsg)
	{
		if (fileName.isEmpty() == true)
		{
			Q_ASSERT(fileName.isEmpty() == false);
			return false;
		}

		// --
		//
		QFile file(fileName);

		bool ok = file.open(QIODevice::WriteOnly | QIODevice::Text);
		if (ok == false)
		{
			errorMsg = QObject::tr("Cannot open file %1 for writing.").arg(fileName);
			return false;
		}

		QTextStream out(&file);

		QStringList columnsList = m_modelDataProvider.exportColumnsText();

		// Fill header
		//
		QStringList headerStrings;
		for (int i = 0; i < m_visibleColumns.size(); i++)
		{
			headerStrings.push_back(columnsList[m_visibleColumns[i]]);
		}
		out << headerStrings.join(separator) << Qt::endl;

		// Fill table
		//
		std::vector<int> rowsToProcess;
		int rowCount = m_modelDataProvider.exportRowCount();
		if (m_exportSelected == true)
		{
			rowsToProcess = m_selectedRows;
		}
		else
		{
			for (int row = 0; row < rowCount; row++)
			{
				rowsToProcess.push_back(row);
			}
		}

		{
			QMutexLocker l(&m_statisticsMutex);
			m_statistics.maxValue = m_maxRows > 0 ? m_maxRows : static_cast<int>(rowsToProcess.size());
			m_statistics.value = 0;
		}

		for (int row : rowsToProcess)
		{
			{
				QMutexLocker l(&m_statisticsMutex);
				if (m_statistics.value % 100 == 0)
				{
					m_statistics.text = tr("Processing data... %1/%2").arg(m_statistics.value + 1).arg(m_statistics.maxValue);
				}
				m_statistics.value++;

				if (m_maxRows > 0 && m_statistics.value > m_maxRows)
				{
					break;
				}
			}

			if (m_stop.load() == true)
			{
				errorMsg = QObject::tr("The process was interrupted.");
				break;
			}

			QStringList rowsList = m_modelDataProvider.exportRowsText(row);
			QStringList rowStrings;
			for (int i = 0; i < m_visibleColumns.size(); i++)
			{
				const QString& cellText = rowsList[m_visibleColumns[i]];

				if (separator == ';' && cellText.contains(separator) == true)
				{
					// If cell contains semicolon it must be enclosed in quotes
					//
					rowStrings.push_back('\"' + cellText + '\"');
				}
				else
				{
					rowStrings.push_back(cellText);
				}
			}
			out << rowStrings.join(separator) << Qt::endl;
		}

		return true;
	}


	bool TableViewReportWorker::createHtml(const QString& fileName, QString& errorMsg)
	{
		if (fileName.isEmpty() == true)
		{
			Q_ASSERT(fileName.isEmpty() == false);
			return false;
		}

		// --
		//
		QFile file(fileName);

		bool ok = file.open(QIODevice::WriteOnly | QIODevice::Text);
		if (ok == false)
		{
			errorMsg = QObject::tr("Cannot open file %1 for writing.").arg(fileName);
			return false;
		}

		QTextStream out(&file);

		out << "<!doctype html>\n";
		out << "<html lang=\"en\">\n";
		out << "<head>\n";
		out << "  <meta charset=\"utf - 8\" />\n";
		out << "  <title>CSV as HTML</title>\n";
		out << "  <style>\n";
		out << "    body { font-family: Arial, sans-serif; padding: 16px; }\n";
		out << "    h1 { margin-bottom: 12px; }\n";
		out << "    table { border-collapse: collapse; width: 100%; }\n ";
		out << "    th, td { border: 1px solid #ccc; padding: 6px 8px; text-align: left; }\n";
		out << "    th { background: #f0f0f0; }\n";
		out << "  </style>\n";
		out << "</head>\n";
		out << "<body>\n";
		out << "  <table>\n";
		out << "    <thead>\n";
		out << "      <tr>\n";
		
		// Fill header
		//
		{
			QStringList columnsList = m_modelDataProvider.exportColumnsText();
			for (int i = 0; i < m_visibleColumns.size(); i++)
			{
				out << "        <th>" << columnsList[m_visibleColumns[i]] << "</ th>\n";
			}
		}
		
		out << "      </tr>\n";
		out << "    </thead>\n";
		out << "    <tbody>\n ";

		// Fill table
		//
		std::vector<int> rowsToProcess;
		int rowCount = m_modelDataProvider.exportRowCount();
		if (m_exportSelected == true)
		{
			rowsToProcess = m_selectedRows;
		}
		else
		{
			for (int row = 0; row < rowCount; row++)
			{
				rowsToProcess.push_back(row);
			}
		}

		{
			QMutexLocker l(&m_statisticsMutex);
			m_statistics.maxValue = m_maxRows > 0 ? m_maxRows : static_cast<int>(rowsToProcess.size());
			m_statistics.value = 0;
		}

		for (int row : rowsToProcess)
		{
			{
				QMutexLocker l(&m_statisticsMutex);
				if (m_statistics.value % 100 == 0)
				{
					m_statistics.text = tr("Processing data... %1/%2").arg(m_statistics.value + 1).arg(m_statistics.maxValue);
				}
				m_statistics.value++;

				if (m_maxRows > 0 && m_statistics.value > m_maxRows)
				{
					break;
				}
			}

			if (m_stop.load() == true)
			{
				errorMsg = QObject::tr("The process was interrupted.");
				break;
			}

			QStringList rowsList = m_modelDataProvider.exportRowsText(row);

			out << "      <tr>";
			for (int i = 0; i < m_visibleColumns.size(); i++)
			{
				const QString& cellText = rowsList[m_visibleColumns[i]];
				out << "<td> "<< cellText.toHtmlEscaped() << "</ td>";
			}
			out << "</ tr>\n ";
		}
		
		out << "    </tbody>\n";
		out << "  </table>\n";
		out << "</body>\n";
		out << "</html>\n";

		return true;
	}


	//
	// --------------------------------------------- ExportPrintPrivate --------------------------
	//
	TableViewReportPrivate::TableViewReportPrivate(QWidget* parent,
												   const ITableViewReportInfo& reportInfo,
												   const QTableView& table,
												   const std::vector<int>& visibleColumns,
												   const std::vector<int>& columnWidths,
												   const std::vector<int>& selectedRows,
												   bool exportSelected,
												   const QPageLayout& pageLayout) :
		QObject(parent),
		m_parent(parent),
		m_pageLayout(pageLayout),
		m_table(table),
		m_worker(new TableViewReportWorker(reportInfo, table, visibleColumns, columnWidths, selectedRows, exportSelected)),
		m_selectedRows(selectedRows),
		m_exportSelected(m_selectedRows.size() > 0 ? exportSelected : false)
	{
	}

	TableViewReportPrivate::~TableViewReportPrivate()
	{
		if (m_worker != nullptr)	
		{
			// Worker is deleted by desctructor in case if Worker thread was not executed.
			// If it was executed, worker is deleted by own thread.
			delete m_worker;
			m_worker = nullptr;
		}
	}
	
	void TableViewReportPrivate::printTable()
	{
		QPrintDialog dialog(m_parent);

		if (m_exportSelected == true)
		{
			dialog.setOption(QAbstractPrintDialog::PrintSelection);
			dialog.setPrintRange(QAbstractPrintDialog::Selection);
		}
		dialog.setOptions(dialog.options() & ~QAbstractPrintDialog::PrintPageRange);

		// Set page size and orientation to the printer
		//
		{
			QPageLayout l = dialog.printer()->pageLayout();
			auto id = QPageSize::id(pageLayout().pageSize().sizePoints(), QPageSize::FuzzyOrientationMatch);
			l.setPageSize(QPageSize(id));
			l.setOrientation(pageLayout().orientation());
			l.setMargins(pageLayout().margins());
			l.setUnits(pageLayout().units());
			dialog.printer()->setPageLayout(l);
		}

		int result = dialog.exec();
		if (result != QDialog::Accepted)
		{
			return;
		}

		QPrinter* printer = dialog.printer();
		if (printer == nullptr)
		{
			Q_ASSERT(printer);
			return;
		}

		// Set page size and orientation set in the dialog
		//
		{
			QPageLayout l = pageLayout();
			auto id = QPageSize::id(printer->pageLayout().pageSize().sizePoints(), QPageSize::FuzzyOrientationMatch);
			l.setPageSize(QPageSize(id));
			l.setOrientation(printer->pageLayout().orientation());
			l.setMargins(printer->pageLayout().margins());
			l.setUnits(printer->pageLayout().units());
			setPageLayout(l);
		}

		// Call the page setup dialog
		//
		if (pageSetup() == false)
		{
			return;
		}

		QPrinterInfo pi(*printer);
		m_worker->setPrinterInfo(pi);
		m_worker->setPageLayout(pageLayout());
		m_worker->setExportSelected(printer->printRange() == QPrinter::PrintRange::Selection);

		// Limit the row count
		//
		{
			int rowCount = (m_exportSelected == true && printer->printRange() == QPrinter::PrintRange::Selection) ?
							   static_cast<int>(m_selectedRows.size()) :
							   m_table.model()->rowCount();

			if (rowCount > TableViewReportWorker::m_maxReportStatesForPrint)
			{
				if (QMessageBox::warning(
						m_parent,
						qAppName(),
						QObject::tr(
							"Warning!\n\nThe report is too large (%1 records ).\nOnly first %2 records will pe printed.\nTo increase the "
							"report size, export the data to the CSV or TXT format.\n\nDo you wish to continue printing?")
							.arg(rowCount)
							.arg(TableViewReportWorker::m_maxReportStatesForPrint),
						QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
				{
					return;
				}

				m_worker->setMaxRows(TableViewReportWorker::m_maxReportStatesForPrint);
			}
		}
		//

		run(TaskType::Print, QString());
	}

	void TableViewReportPrivate::exportTable(const QString& fileName)
	{
		QFileInfo fileInfo(fileName);
		if (fileInfo.completeSuffix().compare(QLatin1String("pdf"), Qt::CaseInsensitive) == 0)
		{
			if (pageSetup() == false)
			{
				return;
			}
		}

		// Limit the row count
		//
		{
			int rowCount = m_exportSelected == true ? static_cast<int>(m_selectedRows.size()) : m_table.model()->rowCount();

			QString extension = fileInfo.completeSuffix();
			int maxRowCount = -1;

			if (extension.compare(QLatin1String("csv"), Qt::CaseInsensitive) == 0 ||
				extension.compare(QLatin1String("txt"), Qt::CaseInsensitive) == 0 || 
				extension.compare(QLatin1String("htm"), Qt::CaseInsensitive) == 0 ||
				extension.compare(QLatin1String("html"), Qt::CaseInsensitive) == 0)
			{
				maxRowCount = TableViewReportWorker::m_maxReportStatesForCsv;

				if (rowCount > maxRowCount)
				{
					if (QMessageBox::warning(
							m_parent,
							qAppName(),
							QObject::tr("Warning!\n\nThe report is too large (%1 records ).\nOnly first %2 records will pe "
										"exported.\n\nDo you wish to continue the exporting?")
								.arg(rowCount)
								.arg(maxRowCount),
							QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
					{
						return;
					}

					m_worker->setMaxRows(maxRowCount);
				}
			}
			else
			{
				maxRowCount = TableViewReportWorker::m_maxReportStatesForPrint;

				if (rowCount > maxRowCount)
				{
					if (QMessageBox::warning(
							m_parent,
							qAppName(),
							QObject::tr("Warning!\n\nThe report is too large (%1 records ).\nOnly first %2 records will pe exported.\nTo "
										"increase the "
										"report size, export the data to the CSV, TXT or HTML format.\n\nDo you wish to continue the exporting?")
								.arg(rowCount)
								.arg(maxRowCount),
							QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
					{
						return;
					}

					m_worker->setMaxRows(maxRowCount);
				}
			}
		}
		//

		m_worker->setPageLayout(pageLayout());

		run(TaskType::Export, fileName);
	}

	void TableViewReportPrivate::stop()
	{
		m_worker->stop();
	}

	const QPageLayout& TableViewReportPrivate::pageLayout() const
	{
		return m_pageLayout;
	}

	void TableViewReportPrivate::setPageLayout(const QPageLayout& layout)
	{
		m_pageLayout = layout;
	}

	bool TableViewReportPrivate::pageSetup()
	{
		QPrinter printer(QPrinter::HighResolution);

		QPageSize::PageSizeId id = QPageSize::id(pageLayout().pageSize().sizePoints(), QPageSize::FuzzyOrientationMatch);
		if (id == QPageSize::Custom)
		{
			id = QPageSize::A4;
		}

		printer.setFullPage(true);
		printer.setPageSize(QPageSize(id));
		printer.setPageOrientation(pageLayout().orientation());
		printer.setPageMargins(pageLayout().margins(), QPageLayout::Unit::Millimeter);

		QPageSetupDialog d(&printer, m_parent);
		if (d.exec() != QDialog::Accepted)
		{
			return false;
		}

		QPageLayout l = pageLayout();
		id = QPageSize::id(d.printer()->pageLayout().pageSize().sizePoints(), QPageSize::FuzzyOrientationMatch);
		l.setPageSize(QPageSize(id));
		l.setOrientation(d.printer()->pageLayout().orientation());
		l.setMargins(d.printer()->pageLayout().margins());
		setPageLayout(l);

		return true;
	}

	void TableViewReportPrivate::run(TaskType task, const QString& fileName)
	{
		// Create Progress Dialog

		UiLib::DialogProgress dialogProgress(QObject::tr("Exporting the data"), 1, m_parent);

		// Create thread

		QThread* thread = new QThread;

		m_worker->moveToThread(thread);

		switch (task)
		{
		case TaskType::Export:
			{
				m_worker->setFileName(fileName);
				QObject::connect(thread, &QThread::started, m_worker, &TableViewReportWorker::exportTable);
			}
			break;
		case TaskType::Print:
			{
				QObject::connect(thread, &QThread::started, m_worker, &TableViewReportWorker::printTable);
			}
			break;
		}

		QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater); // Schedule thread deleting

		QObject::connect(&dialogProgress,
						 &UiLib::DialogProgress::getProgress,
						 m_worker,
						 &TableViewReportWorker::progressRequested,
						 Qt::DirectConnection);
		QObject::connect(&dialogProgress,
						 &UiLib::DialogProgress::cancelClicked,
						 m_worker,
						 &TableViewReportWorker::stop,
						 Qt::DirectConnection);

		QObject::connect(m_worker, &TableViewReportWorker::progressChanged, &dialogProgress, &UiLib::DialogProgress::setProgressSingle);

		//  Schedule objects deleting

		QObject::connect(m_worker,
						 &TableViewReportWorker::finished,
						 m_worker,
						 [thread, &dialogProgress, this](const QString& errorMessage)
						 {
							 thread->quit();

							 if (errorMessage.isEmpty() == false)
							 {
								 dialogProgress.setErrorMessage(errorMessage);
							 }

							 dialogProgress.exit();

							 m_worker->deleteLater();
							 m_worker = nullptr;	// Worker will not be deleted by desctructor of TableViewReportPrivate as it lives on own thread
						 });

		// Start thread

		thread->start();

		dialogProgress.exec();

		if (dialogProgress.hasErrorMessage() == false)
		{
			if (task == TaskType::Export)
			{
				if (QMessageBox::question(m_parent, qAppName(), QObject::tr("Export is complete.\n\nDo you wish to open it?")) ==
					QMessageBox::Yes)
				{
					UiTools::openPdf(fileName, m_parent);
				}
			}
		}
		else
		{
			QMessageBox::critical(m_parent, qAppName(), dialogProgress.errorMessage());
		}

		return;
	}

} // namespace ReportLib
