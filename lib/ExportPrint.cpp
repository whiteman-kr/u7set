#include "ExportPrint.h"
#include <QPrintDialog>
#include <QPageSetupDialog>

ExportPrint::ExportPrint(QWidget* parent):
	QObject(parent),
	m_parent(parent)
{

}

void ExportPrint::printTable(QTableView* tableView)
{
	if (tableView == nullptr)
	{
		Q_ASSERT(tableView);
		return;
	}

	QPrintDialog dialog(m_parent);

	if (tableView->selectionModel()->hasSelection() == true)
	{
		dialog.setOption(QAbstractPrintDialog::PrintSelection);
		dialog.setOptions(dialog.options() & ~QAbstractPrintDialog::PrintPageRange);

		//dialog.printer()->setPrintRange(QPrinter::PrintRange::Selection);				// Set Selection option by default
	}

	int result = dialog.exec();

	if (result == QDialog::Accepted)
	{
		QPrinter* printer = dialog.printer();

		if (printer == nullptr)
		{
			Q_ASSERT(printer);
			return;
		}

		QTextDocument doc;

		QSize pageSize = printer->pageLayout().paintRectPixels(printer->resolution()).size();
		doc.setPageSize(pageSize);

		exportToTextDocument(tableView, &doc, printer->printRange() == QPrinter::PrintRange::Selection);

		doc.print(printer);
	}

//	QPrintPreviewDialog printDialog(this);

//	printDialog.setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
//	printDialog.setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);

//	if (m_view->selectionModel()->hasSelection() == true)
//	{
//		qDebug() << "Print selection enabled";
//		printDialog.printer()->setPrintRange(QPrinter::PrintRange::Selection);
//	}

	//connect(&printDialog, &QPrintPreviewDialog::paintRequested, this, &MonitorArchiveWidget::printRequested);

//	printDialog.exec();
}

void ExportPrint::exportTable(QTableView* tableView, QString fileName, QString extension)
{
	if (extension.compare(QLatin1String("csv"), Qt::CaseInsensitive) == 0)
	{
		saveArchiveToCsv(tableView, fileName);
		return;
	}

	if (extension.compare(QLatin1String("pdf"), Qt::CaseInsensitive) == 0 ||
		extension.compare(QLatin1String("htm"), Qt::CaseInsensitive) == 0 ||
		extension.compare(QLatin1String("html"), Qt::CaseInsensitive) == 0 ||
		extension.compare(QLatin1String("txt"), Qt::CaseInsensitive) == 0/* ||
		extension.compare(QLatin1String("odt"), Qt::CaseInsensitive) == 0*/)
	{
		saveArchiveWithDocWriter(tableView, fileName, extension);
		return;
	}

}

void ExportPrint::generateHeader(QTextCursor& cursor)
{
	Q_UNUSED(cursor);
}

bool ExportPrint::exportToTextDocument(QTableView* tableView, QTextDocument* doc, bool onlySelectedRows)
{
	if (tableView == nullptr || doc == nullptr)
	{
		Q_ASSERT(tableView);
		Q_ASSERT(doc);
		return false;
	}

	QAbstractItemModel* model = tableView->model();
	if (model == nullptr)
	{
		Q_ASSERT(model);
		return false;
	}

	QTextCursor cursor(doc);

	// Generate Header
	//
	generateHeader(cursor);

	// States
	//
	int rowCount = qBound(0, model->rowCount(), m_maxReportStates);		// Limit row count to m_maxReportStates

	// reinit rowCount if printing only selected rows
	//
	QModelIndexList selectedIndexes;
	if (onlySelectedRows == true)
	{
		selectedIndexes = tableView->selectionModel()->selectedRows();
		std::sort(selectedIndexes.begin(), selectedIndexes.end());

		rowCount = selectedIndexes.size();
	}

	int columnCount = model->columnCount();

	std::vector<int> shownColums;
	shownColums.reserve(columnCount);

	int shownColumnCount = 0;
	for (int column = 0; column < columnCount; column++)
	{
		if (tableView->isColumnHidden(column) == false)
		{
			shownColumnCount ++;
			shownColums.push_back(column);
		}
	}

	QProgressDialog progressDialog(tr("Generating report..."), tr("Cancel"), 0, rowCount, m_parent);
	progressDialog.setMinimumDuration(100);

	// Add table
	//
	QTextTableFormat tableFormat;

	tableFormat.setHeaderRowCount(1);
	tableFormat.setBorderStyle(QTextFrameFormat::BorderStyle_None);
	tableFormat.setBorder(0);
	tableFormat.setBorderBrush(Qt::NoBrush);
	tableFormat.setWidth(QTextLength(QTextLength::PercentageLength, 100));

	cursor.insertTable(rowCount + 1, shownColumnCount, tableFormat);	// VERY HEAVY OPERATION

	// Fill table header
	//
	for (int column : shownColums)
	{
		QString columnHeader = model->headerData(column, Qt::Horizontal, Qt::DisplayRole).toString();

		cursor.insertText(columnHeader);
		cursor.movePosition(QTextCursor::NextCell);
	}

	// Fill table
	//
	if (onlySelectedRows == false)
	{
		QString cellText;
		for (int row = 0; row < rowCount; row++)
		{
			progressDialog.setValue(row);
			if (progressDialog.wasCanceled() == true)
			{
				break;
			}

			QApplication::processEvents();

			for (int column : shownColums)
			{
				cellText = model->data(model->index(row, column), Qt::DisplayRole).toString();

				cursor.insertText(cellText);
				cursor.movePosition(QTextCursor::NextCell);
			}
		}
	}
	else
	{
		QString cellText;
		for (const QModelIndex mi : selectedIndexes)
		{
			int row = mi.row();

			progressDialog.setValue(row);
			if (progressDialog.wasCanceled() == true)
			{
				break;
			}

			QApplication::processEvents();

			for (int column : shownColums)
			{
				cellText = model->data(model->index(row, column), Qt::DisplayRole).toString();

				cursor.insertText(cellText);
				cursor.movePosition(QTextCursor::NextCell);
			}
		}
	}

	cursor.movePosition(QTextCursor::End);

	cursor.insertText("\n\n");

	if (model->rowCount() > m_maxReportStates)
	{
		cursor.insertText(tr("Warning: Only first %1 of %2 records present in generated report.\n").arg(rowCount).arg(tableView->model()->rowCount()));
	}

	progressDialog.setValue(rowCount);

	return !progressDialog.wasCanceled();
}

bool ExportPrint::saveArchiveWithDocWriter(QTableView* tableView, QString fileName, QString format)
{
	if (tableView == nullptr ||
		fileName.isEmpty() == true)
	{
		Q_ASSERT(tableView);
		Q_ASSERT(fileName.isEmpty() == false);
		return false;
	}

	QAbstractItemModel* model = tableView->model();
	if (model == nullptr)
	{
		Q_ASSERT(model);
		return false;
	}

	if (model->rowCount() > m_maxReportStates)
	{
		QMessageBox::warning(m_parent, qAppName(), tr("Too many archive records, only first %1 states will appear in the generated report.").arg(m_maxReportStates));
	}

	bool isPdf = format.compare(QLatin1String("pdf"), Qt::CaseInsensitive) == 0;
	bool isOdt = format.compare(QLatin1String("odt"), Qt::CaseInsensitive) == 0;

	if (format.compare(QLatin1String("pdf"), Qt::CaseInsensitive) == 0 &&
		format.compare(QLatin1String("htm"), Qt::CaseInsensitive) == 0 &&
		format.compare(QLatin1String("html"), Qt::CaseInsensitive) == 0 &&
		format.compare(QLatin1String("txt"), Qt::CaseInsensitive) == 0 &&
		format.compare(QLatin1String("odt"), Qt::CaseInsensitive) == 0)
	{
		QMessageBox::critical(m_parent, qAppName(), tr("Unsupported file format."));
		return false;
	}

	// Get page size, margins and orientation
	//
	std::unique_ptr<QPageSetupDialog> pageSetupDialog;

	if (isPdf == true || isOdt == true)
	{
		pageSetupDialog = std::make_unique<QPageSetupDialog>(m_parent);

		int pageSetupResult = pageSetupDialog->exec();

		if (pageSetupResult != QDialog::Accepted)
		{
			return false;
		}
	}

	// Set wait cursor
	//
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	QApplication::processEvents();

	// --
	//
	QPrinter printer(QPrinter::PrinterResolution);
	printer.setOutputFormat(QPrinter::PdfFormat);

	if (pageSetupDialog != nullptr)
	{
		printer.setPageLayout(pageSetupDialog->printer()->pageLayout());
	}

	// Generate doc
	//
	QTextDocument doc;

	if (pageSetupDialog != nullptr)
	{
		QSizeF pageSize = pageSetupDialog->printer()->pageLayout().paintRectPixels(pageSetupDialog->printer()->resolution()).size();
		doc.setPageSize(pageSize);
	}

	 exportToTextDocument(tableView, &doc, false);

	// --
	//
	if (format.compare(QLatin1String("pdf"), Qt::CaseInsensitive) == 0)
	{
		printer.setOutputFileName(fileName);
		doc.print(&printer);
	}
	else
	{
		QByteArray docFormat;

		if (format.compare(QLatin1String("htm"), Qt::CaseInsensitive) == 0 ||
			format.compare(QLatin1String("html"), Qt::CaseInsensitive) == 0)
		{
			docFormat = "html";
		}

		if (format.compare(QLatin1String("txt"), Qt::CaseInsensitive) == 0)
		{
			docFormat = "plaintext";
		}

		if (format.compare(QLatin1String("odt"), Qt::CaseInsensitive) == 0)
		{
			docFormat = "odf";
		}

		QTextDocumentWriter writer(fileName);
		writer.setFormat(docFormat);

		writer.write(&doc);
	}

	// Restore cursor from wait
	//
	QApplication::restoreOverrideCursor();
	QApplication::processEvents();

	return true;
}

bool ExportPrint::saveArchiveToCsv(QTableView* tableView, QString fileName)
{
	if (tableView == nullptr ||
		fileName.isEmpty() == true)
	{
		Q_ASSERT(tableView);
		Q_ASSERT(fileName.isEmpty() == false);
		return false;
	}

	QAbstractItemModel* model = tableView->model();
	if (model == nullptr)
	{
		Q_ASSERT(model);
		return false;
	}

	if (model->rowCount() > m_maxReportStatesForCsv )
	{
		QMessageBox::warning(m_parent, qAppName(), tr("Too many archive records, only first %1 states will appear in the generated report.").arg(m_maxReportStatesForCsv ));
	}

	// --
	//
	QFile file(fileName);

	bool ok = file.open(QIODevice::WriteOnly | QIODevice::Text);
	if (ok == false)
	{
		QMessageBox::critical(m_parent, qAppName(), tr("Cannot open file %1 for writing.").arg(fileName));
		return false;
	}

	QTextStream out(&file);

	// Set wait cursor
	//
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	QApplication::processEvents();

	// States
	//
	int rowCount = qBound(0, model->rowCount(), m_maxReportStatesForCsv);		// Limit row count to m_maxReportStates
	int columnCount = model->columnCount();

	std::vector<int> shownColums;
	shownColums.reserve(columnCount);

	int shownColumnCount = 0;
	for (int column = 0; column < columnCount; column++)
	{
		if (tableView->isColumnHidden(column) == false)
		{
			shownColumnCount ++;
			shownColums.push_back(column);
		}
	}

	QProgressDialog progressDialog(tr("Generating report..."), tr("Cancel"), 0, rowCount, m_parent);
	progressDialog.setMinimumDuration(100);

	// Fill header
	//
	for (int column : shownColums)
	{
		QString columnHeader = model->headerData(column, Qt::Horizontal, Qt::DisplayRole).toString();
		out << columnHeader << ";";
	}
	out << Qt::endl;

	// Fill table
	//
	QString cellText;
	for (int row = 0; row < rowCount; row++)
	{
		progressDialog.setValue(row);
		if (progressDialog.wasCanceled() == true)
		{
			break;
		}

		QApplication::processEvents();

		if (row % 10000 == 0)
		{
			progressDialog.setLabelText(tr("Generating report... %1/%2").arg(row).arg(rowCount));
		}

		for (int column : shownColums)
		{
			cellText = model->data(model->index(row, column), Qt::DisplayRole).toString();

			if (cellText.contains(';') == true)
			{
				// If cell contains semicolon it must be enclosed in quotes
				//
				cellText.prepend('"');
				cellText.append('"');
			}

			out << cellText << ";";
		}

		out << Qt::endl;
	}

	progressDialog.setValue(rowCount);

	// Restore cursor from wait
	//
	QApplication::restoreOverrideCursor();
	QApplication::processEvents();

	return true;
}
