#include "ExportData.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QFile>
#include <QtConcurrent>
#include <exception>

#include "Options.h"

// -------------------------------------------------------------------------------------------------------------------

ExportData::ExportData(QTableView *pView, const QString& fileName) :
	QObject(pView),
	m_pView(pView),
	m_fileName(fileName)
{
	createProgressDialog(pView);
}

// -------------------------------------------------------------------------------------------------------------------

ExportData::~ExportData()
{
}

// -------------------------------------------------------------------------------------------------------------------

void ExportData::createProgressDialog(QTableView *pView)
{
	if (pView == nullptr)
	{
		return;
	}

	m_pProgressDialog = new QDialog(pView->parentWidget());

	m_pProgressDialog->setWindowFlags(Qt::Drawer);
	m_pProgressDialog->setFixedSize(300, 70);
	m_pProgressDialog->setWindowTitle(EXPORT_WINDOW_TITLE);
	m_pProgressDialog->setWindowIcon(QIcon(":/icons/Export.png"));

		m_progress = new QProgressBar;
		m_progress->setTextVisible(false);
		m_progress->setRange(0, 100);
		m_progress->setFixedHeight(10);

		m_cancelButton = new QPushButton;
		m_cancelButton->setText(tr("Cancel"));

		QHBoxLayout *buttonLayout = new QHBoxLayout ;

		buttonLayout->addStretch();
		buttonLayout->addWidget(m_cancelButton);
		buttonLayout->addStretch();

		QVBoxLayout *mainLayout = new QVBoxLayout ;

		mainLayout->addWidget(m_progress);
		mainLayout->addLayout(buttonLayout);

	m_pProgressDialog->setLayout(mainLayout);

	connect(this, &ExportData::setValue, m_progress, &QProgressBar::setValue);
	connect(this, &ExportData::setRange, m_progress, &QProgressBar::setRange);

	connect(this, &ExportData::exportThreadFinish, m_pProgressDialog, &QDialog::reject);
	connect(this, &ExportData::exportThreadFinish, this, &ExportData::exportComplited);
	connect(m_cancelButton, &QPushButton::clicked, m_pProgressDialog, &QDialog::reject);
	connect(m_pProgressDialog, &QDialog::rejected, this, &ExportData::exportCancel);
}

// -------------------------------------------------------------------------------------------------------------------

void ExportData::exec()
{
	if (m_pProgressDialog == nullptr)
	{
		return;
	}

	if (m_pView == nullptr)
	{
		return;
	}

	if (m_fileName.isEmpty() == true)
	{
		m_fileName = tr("Export");
	}

	if (m_pView->model()->rowCount() == 0)
	{
		QMessageBox::information(m_pProgressDialog, EXPORT_WINDOW_TITLE, tr("Data is absent!"));
		return;
	}

	//QString filter = tr("Excel files (*.xlsx);;CSV files (*.csv)");
	QString filter = tr("CSV files (*.csv)");

	QString fileName = QFileDialog::getSaveFileName(m_pProgressDialog, EXPORT_WINDOW_TITLE, m_fileName, filter);
	if (fileName.isEmpty() == true)
	{
		return;
	}

	m_pProgressDialog->show();
	QtConcurrent::run(ExportData::startExportThread, this, fileName);

	//QFuture<void> result = QtConcurrent::run(ExportData::startExportThread, this, fileName);
	//result.waitForFinished();
}

// -------------------------------------------------------------------------------------------------------------------

void ExportData::startExportThread(ExportData* pThis, const QString& fileName)
{
	if (pThis == nullptr)
	{
		return;
	}

	QString fileExt = fileName.right(fileName.count() - fileName.lastIndexOf(".") - 1);
	if (fileExt.isEmpty() == true)
	{
		return;
	}

	if (fileExt == "xlsx")
	{
		pThis->saveExcelFile(fileName);
	}

	if (fileExt == "csv")
	{
		pThis->saveCsvFile(fileName);
	}

	emit pThis->exportThreadFinish();
}

// -------------------------------------------------------------------------------------------------------------------

bool ExportData::saveExcelFile(const QString& fileName)
{
	Q_UNUSED(fileName);
    /*if (m_pView == nullptr)
	{
		return false;
	}

	if (fileName.isEmpty() == true)
	{
		return false;
	}

	m_exportCancel = false;

	ExcelExportHelper helper;

	int columnCount = m_pView->model()->columnCount();
	for(int column = 0; column < columnCount; column++)
	{
		if (m_pView->isColumnHidden(column) == true)
		{
			continue;
		}

		helper.setCellValue(1, column, m_pView->model()->headerData(column, Qt::Horizontal).toString().toLocal8Bit());
	}

	int rowCount = m_pView->model()->rowCount();
	rowCount = 10;

	setRange(0, rowCount);

	for(int row = 0; row < rowCount; row++)
	{
		if (m_exportCancel == true)
		{
			break;
		}

		for(int column = 0; column < columnCount; column++)
		{
			if (m_pView->isColumnHidden(column) == true)
			{
				continue;
			}

			helper.setCellValue(row + 2, column, m_pView->model()->data(m_pView->model()->index(row, column)).toString().toLocal8Bit());
		}

		setValue(row);
	}

	if (m_exportCancel == true)
	{
		return false;
	}

    helper.saveAs(fileName);*/

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool ExportData::saveCsvFile(const QString &fileName)
{
	if (m_pView == nullptr)
	{
		return false;
	}

	if (fileName.isEmpty() == true)
	{
		return false;
	}

	m_exportCancel = false;

	QFile file;
	file.setFileName(fileName);
	if (file.open(QIODevice::WriteOnly) == false)
	{
		return false;
	}

	int columnCount = m_pView->model()->columnCount();
	for(int column = 0; column < columnCount; column++)
	{
		if (m_pView->isColumnHidden(column) == true)
		{
			continue;
		}

		file.write(m_pView->model()->headerData(column, Qt::Horizontal).toString().toLocal8Bit());
		file.write(";");
	}

	file.write("\n");

	int rowCount = m_pView->model()->rowCount();

	setRange(0, rowCount);

	for(int row = 0; row < rowCount; row++)
	{
		if (m_exportCancel == true)
		{
			break;
		}

		for(int column = 0; column < columnCount; column++)
		{
			if (m_pView->isColumnHidden(column) == true)
			{
				continue;
			}

			file.write(m_pView->model()->data(m_pView->model()->index(row, column)).toString().toLocal8Bit());
			file.write(";");
		}

		file.write("\n");
		file.flush();

		setValue(row);
	}

	file.close();

	return true;
}

// -------------------------------------------------------------------------------------------------------------------
