#include <UiLib/ExportPrint.h>
#include <UiLib/DialogProgress.h>
#include "../UtilsLib/Ui/UiTools.h"
#include "../ReportLib/Report.h"

namespace UiLib
{
	TableExportPrintModel::TableExportPrintModel(QObject* parent):
		QAbstractTableModel(parent)
	{
		connect(
			this,
			&TableExportPrintModel::signalRowCount,
			this,
			[this](int& result)
			{
				result = rowCount();
			},
			Qt::BlockingQueuedConnection);

		connect(
			this,
			&TableExportPrintModel::signalColumnCount,
			this,
			[this](int& result)
			{
				result = columnCount();
			},
			Qt::BlockingQueuedConnection);

		connect(
			this,
			&TableExportPrintModel::signalColumnsText,
			this,
			[this](QStringList& text)
			{
				text.clear();
				int count = columnCount();
				for (int i = 0; i < count; i++)
				{
					text.push_back(headerData(i, Qt::Horizontal, Qt::DisplayRole).toString());
				}
			},
			Qt::BlockingQueuedConnection);

		connect(
			this,
			&TableExportPrintModel::signalRowsText,
			this,
			[this](int row, QStringList& text)
			{
				text.clear();
				int count = columnCount();
				for (int i = 0; i < count; i++)
				{
					text.push_back(data(index(row, i), Qt::DisplayRole).toString());
				}
			},
			Qt::BlockingQueuedConnection);
	}

	int TableExportPrintModel::exportRowCount() const
	{
		int result = 0;
		emit signalRowCount(result);
		return result;
	}
	
	int TableExportPrintModel::exportColumnCount() const
	{
		int result = 0;
		emit signalColumnCount(result);
		return result;
	}
	
	QStringList TableExportPrintModel::exportColumnsText() const
	{
		QStringList result;
		emit signalColumnsText(result);
		return result;
	}
	
	QStringList TableExportPrintModel::exportRowsText(int row) const
	{
		QStringList result;
		emit signalRowsText(row, result);
		return result;
	}

	//
	// --------------------------------------------- ExportPrintPrivateWorker --------------------------
	//
	TableExportPrintPrivateWorker::TableExportPrintPrivateWorker(ITableExportPrint& exportPrintInterface,
													   const TableExportPrintModel& model,
													   const std::vector<int>& visibleColumns,
													   const std::vector<int>& columnWidths,
													   const std::vector<int>& selectedRows) :
		m_exportPrintInterface{exportPrintInterface},
		m_model(model),
		m_printer(std::make_unique<ReportLib::ReportPrinter>()),
		m_visibleColumns(visibleColumns),
		m_columnWidths(columnWidths),
		m_selectedRows(selectedRows),
		m_exportSelected(m_selectedRows.size() > 1)
	{
	}

	void TableExportPrintPrivateWorker::setFileName(const QString& fileName)
	{
		m_fileName = fileName;
	}

	void TableExportPrintPrivateWorker::setPrinterInfo(const QPrinterInfo& printerInfo) 
	{
		m_printerInfo = printerInfo;
	}

	void TableExportPrintPrivateWorker::setPageLayout(const QPageLayout& layout)
	{
		m_pageLayout = layout;
	}
	
	void TableExportPrintPrivateWorker::setExportSelected(bool value)
	{
		m_exportSelected = value;
	}

	void TableExportPrintPrivateWorker::setMaxRows(int value) 
	{
		m_maxRows = value;
	}

	void TableExportPrintPrivateWorker::printTable()
	{
		QString errorMsg;
		createReport(m_model, QString(), errorMsg);
		emit finished(errorMsg);
	}

	void TableExportPrintPrivateWorker::exportTable()
	{
		QFileInfo fileInfo(m_fileName);
		QString extension = fileInfo.completeSuffix();
		QString errorMsg;

		if (extension.compare(QLatin1String("csv"), Qt::CaseInsensitive) == 0)
		{
			createText(m_model, m_fileName, ';', errorMsg);
		}
		else
		{
			if (extension.compare(QLatin1String("txt"), Qt::CaseInsensitive) == 0)
			{
				createText(m_model, m_fileName, '\t', errorMsg);
			}
			else
			{
				if (extension.compare(QLatin1String("pdf"), Qt::CaseInsensitive) == 0)
				{
					createReport(m_model, m_fileName, errorMsg);
				}
				else
				{
					errorMsg = tr("Unsupported file format!");
				}
			}
		}

		emit finished(errorMsg);
	}

	void TableExportPrintPrivateWorker::stop()
	{
		m_stop = true;
	}

	void TableExportPrintPrivateWorker::progressRequested()
	{
		QString progressText;

		int progress = 0;
		int progressMin = 0;
		int progressMax = 0;

		getProgress(&progress, &progressMin, &progressMax, &progressText);

		emit progressChanged(progress, 0, progressMax, progressText);

		return;
	}

	void TableExportPrintPrivateWorker::getProgress(int* progress, int* progressMin, int* progressMax, QString* progressText)
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
			m_printer->statistics().fill(progress, progressMin, progressMax, progressText);
		}
		else
		{
			*progress = m_statistics.value;
			*progressMin = 0;
			*progressMax = m_statistics.maxValue;
			*progressText = m_statistics.text;
		}
	}
	
	void TableExportPrintPrivateWorker::generateHeader(ReportLib::Report& report, ReportLib::ReportSection& mainSection)
	{
		m_exportPrintInterface.generateHeader(report, mainSection);
	}

	bool TableExportPrintPrivateWorker::createReport(const TableExportPrintModel& model, const QString& fileName, QString& errorMsg)
	{
		QStringList columnsList = model.exportColumnsText();

		// Create report objects
		//
		ReportLib::Report report("Report", fileName);
		ReportLib::ReportFont font{"Arial", 10};

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
		
		ReportLib::TableFormat format{font, columns};
		std::shared_ptr<ReportLib::ReportTable> table = std::make_shared<ReportLib::ReportTable>(format);
		mainSection->addTable(table);

		// Fill the table
		//
		std::vector<int> rowsToProcess;
		int rowCount = model.exportRowCount();
		if (m_exportSelected == true) // If more than 1 row is selected - export only them, otherwise export all rows
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

			QStringList rowsList = model.exportRowsText(row);
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
			result = m_printer->print(report, printer, m_stop);
		}
		else
		{
			result = m_printer->save(report, fileName, m_stop);
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

	bool TableExportPrintPrivateWorker::createText(const TableExportPrintModel& model, const QString& fileName, const QChar& separator, QString& errorMsg)
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

		QStringList columnsList = model.exportColumnsText();

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
		int rowCount = model.exportRowCount();
		std::vector<int> rowsToProcess = m_selectedRows;
		if (rowsToProcess.size() <= 1)		// If more than 1 row is selected - export only them, otherwise export all rows
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

		for (int row: rowsToProcess)
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

			QStringList rowsList = model.exportRowsText(row);
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

	//
	// --------------------------------------------- ExportPrintPrivate --------------------------
	//
	TableExportPrintPrivate::TableExportPrintPrivate(ITableExportPrint& exportPrintInterface,
										   QWidget* parent,
										   const TableExportPrintModel& model,
										   const std::vector<int>& visibleColumns,
										   const std::vector<int>& columnWidths,
										   const std::vector<int>& selectedRows,
										   const QPageLayout& pageLayout) :
		m_parent(parent),
		m_pageLayout(pageLayout),
		m_model(model),
		m_worker(new TableExportPrintPrivateWorker(exportPrintInterface, model, visibleColumns, columnWidths, selectedRows)),
		m_selectedRows(selectedRows)
	{
	}

	void TableExportPrintPrivate::printTable()
	{
		QPrintDialog dialog(m_parent);

		if (m_selectedRows.size() > 1)
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
			int rowCount = (m_selectedRows.size() > 1 && printer->printRange() == QPrinter::PrintRange::Selection) ?
							   static_cast<int>(m_selectedRows.size()) :
							   m_model.rowCount();

			if (rowCount > TableExportPrintPrivateWorker::m_maxReportStatesForPrint)
			{
				if (QMessageBox::warning(
						m_parent,
						qAppName(),
						QObject::tr(
							"Warning!\n\nThe report is too large (%1 records ).\nOnly first %2 records will pe printed.\nTo increase the "
							"report size, export the data to the CSV or TXT format.\n\nDo you wish to continue printing?")
							.arg(rowCount)
							.arg(TableExportPrintPrivateWorker::m_maxReportStatesForPrint),
						QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
				{
					return;
				}

				m_worker->setMaxRows(TableExportPrintPrivateWorker::m_maxReportStatesForPrint);
			}
		}
		//

		run(TaskType::Print, QString());
	}
	
	void TableExportPrintPrivate::exportTable(const QString& fileName)
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
			int rowCount = m_selectedRows.size() > 1 ? static_cast<int>(m_selectedRows.size()) : m_model.rowCount();

			QString extension = fileInfo.completeSuffix();
			int maxRowCount = -1;

			if (extension.compare(QLatin1String("csv"), Qt::CaseInsensitive) == 0 ||
				extension.compare(QLatin1String("txt"), Qt::CaseInsensitive) == 0)
			{
				maxRowCount = TableExportPrintPrivateWorker::m_maxReportStatesForCsv;

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
				maxRowCount = TableExportPrintPrivateWorker::m_maxReportStatesForPrint;

				if (rowCount > maxRowCount)
				{
					if (QMessageBox::warning(
							m_parent,
							qAppName(),
							QObject::tr("Warning!\n\nThe report is too large (%1 records ).\nOnly first %2 records will pe exported.\nTo "
										"increase the "
										"report size, export the data to the CSV or TXT format.\n\nDo you wish to continue the exporting?")
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

	void TableExportPrintPrivate::stop()
	{
		m_worker->stop();
	}
	
	const QPageLayout& TableExportPrintPrivate::pageLayout() const
	{
		return m_pageLayout;
	}

	void TableExportPrintPrivate::setPageLayout(const QPageLayout& layout)
	{
		m_pageLayout = layout;
	}

	bool TableExportPrintPrivate::pageSetup() 
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
		
	void TableExportPrintPrivate::run(TaskType task, const QString& fileName) 
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
				QObject::connect(thread, &QThread::started, m_worker, &TableExportPrintPrivateWorker::exportTable);
			}
			break;
		case TaskType::Print:
			{
				QObject::connect(thread, &QThread::started, m_worker, &TableExportPrintPrivateWorker::printTable);
			}
			break;
		}

		QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater); // Schedule thread deleting

		QObject::connect(&dialogProgress,
						 &UiLib::DialogProgress::getProgress,
						 m_worker,
						 &TableExportPrintPrivateWorker::progressRequested,
						 Qt::DirectConnection);
		QObject::connect(&dialogProgress,
						 &UiLib::DialogProgress::cancelClicked,
						 m_worker,
						 &TableExportPrintPrivateWorker::stop,
						 Qt::DirectConnection);

		QObject::connect(m_worker, &TableExportPrintPrivateWorker::progressChanged, &dialogProgress, &UiLib::DialogProgress::setProgressSingle);

		//  Schedule objects deleting

		QObject::connect(m_worker,
						 &TableExportPrintPrivateWorker::finished,
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

} // namespace UiLib

namespace UiLib
{
	TableExportPrint::TableExportPrint(QWidget* parent,
							 const QTableView& table,
							 const TableExportPrintModel& model,
							 const QPageLayout& pageLayout)
	{

		std::vector<int> visibleColumns;
		std::vector<int> columnWidths;
		int count = model.columnCount();
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

		m_impl = std::make_unique<TableExportPrintPrivate>(*this, parent, model, visibleColumns, columnWidths, selectedRows, pageLayout);
	}

	TableExportPrint::~TableExportPrint() = default;

	void TableExportPrint::printTable()
	{
		return m_impl->printTable();
	}

	void TableExportPrint::exportTable(const QString& fileName)
	{
		return m_impl->exportTable(fileName);
	}

	const QPageLayout& TableExportPrint::pageLayout() const
	{
		return m_impl->pageLayout();
	}

	void TableExportPrint::setPageLayout(const QPageLayout& layout)
	{
		m_impl->setPageLayout(layout);
	}

	void TableExportPrint::savePageLayoutToSettings(const QPageLayout& pageLayout, const QString& groupName)
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

	QPageLayout TableExportPrint::loadPageLayoutFromSettings(const QString& groupName, const QPageLayout& defaultPageLayout)
	{
		QSettings settings;
		settings.beginGroup(groupName);
		QPageSize::PageSizeId pageSizeId = static_cast<QPageSize::PageSizeId>(settings.value("PageSize", defaultPageLayout.pageSize().id()).toInt());
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

	void TableExportPrint::stop()
	{
		return m_impl->stop();
	}

	void TableExportPrint::generateHeader(ReportLib::Report& report, ReportLib::ReportSection& mainSection)
	{
		Q_UNUSED(report);
		Q_UNUSED(mainSection);
	}
} // namespace UiLib