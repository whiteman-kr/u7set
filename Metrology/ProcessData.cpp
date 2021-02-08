#include "ProcessData.h"

#include <QApplication>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStringListModel>
#include <QClipboard>
#include <QSettings>
#include <QtConcurrent>

#include "Options.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

CompleterData::CompleterData(QObject* parent) :
	QObject(parent)
{
	if (parent == nullptr)
	{
		return;
	}

	create(parent);
}

// -------------------------------------------------------------------------------------------------------------------

CompleterData::~CompleterData()
{
}

// -------------------------------------------------------------------------------------------------------------------

bool CompleterData::setFilterCount(int count)
{
	if (count <= 0)
	{
		return false;
	}

	m_count = count;

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void CompleterData::setFilterList(const QStringList& list)
{
	if (m_filterCompleter == nullptr)
	{
		return;
	}

	QStringListModel* completerModel = dynamic_cast<QStringListModel*>(m_filterCompleter->model());
	if (completerModel == nullptr)
	{
		return;
	}

	m_filterCompleterList = list;

	completerModel->setStringList(m_filterCompleterList);
}

// -------------------------------------------------------------------------------------------------------------------

bool CompleterData::create(QObject* parent)
{
	if (parent == nullptr)
	{
		assert(0);
		return false;
	}

	m_filterCompleter = new QCompleter(m_filterCompleterList, parent);
	if (m_filterCompleter == nullptr)
	{
		return false;
	}

	m_filterCompleter->setCaseSensitivity(Qt::CaseInsensitive);

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool CompleterData::appendFilter(const QString& text)
{
	if (m_filterCompleter == nullptr)
	{
		return false;
	}

	if (m_filterCompleterList.contains(text) == true)
	{
		return false;
	}

	m_filterCompleterList.append(text);

	if (m_filterCompleterList.count() > m_count)
	{
		m_filterCompleterList.removeAt(0);
	}

	QStringListModel* completerModel = dynamic_cast<QStringListModel*>(m_filterCompleter->model());
	if (completerModel == nullptr)
	{
		return false;
	}

	completerModel->setStringList(m_filterCompleterList);

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void CompleterData::load(const QString& optionsKey)
{
	if (optionsKey.isEmpty() == true)
	{
		assert(0);
		return;
	}

	QSettings s;

	for(int i = 0; i < m_count; i++)
	{
		QString recentFindText = s.value(QString("%1/%2/Text%3").
										 arg(optionsKey).
										 arg(COMPLETER_OPTIONS_KEY).
										 arg(i),
										 QString()).toString();
		if (recentFindText.isEmpty() == true)
		{
			continue;
		}

		if (m_filterCompleterList.contains(recentFindText) == true)
		{
			continue;
		}

		m_filterCompleterList.append(recentFindText);
	}

	setFilterList(m_filterCompleterList);
}

// -------------------------------------------------------------------------------------------------------------------

void CompleterData::save(const QString& optionsKey)
{
	if (optionsKey.isEmpty() == true)
	{
		assert(0);
		return;
	}

	QSettings s;

	int count = m_filterCompleterList.count();
	for(int i = 0; i < count; i++)
	{
		s.setValue(QString("%1/%2/Text%3").
				   arg(optionsKey).
				   arg(COMPLETER_OPTIONS_KEY).
				   arg(i),
				   m_filterCompleterList[i]);
	}
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

CopyData::CopyData(QTableView* pView, bool copyHiddenColumn) :
	QObject(pView),
	m_pView(pView),
	m_copyHiddenColumn(copyHiddenColumn)
{
}

// -------------------------------------------------------------------------------------------------------------------

CopyData::~CopyData()
{
}

// -------------------------------------------------------------------------------------------------------------------

void CopyData::exec()
{
	if (m_pView == nullptr)
	{
		return;
	}

	copyToMemory();
}

// -------------------------------------------------------------------------------------------------------------------

bool CopyData::copyToMemory()
{
	if (m_pView == nullptr)
	{
		return false;
	}

	QString textClipboard;

	m_copyCancel = false;

	int columnCount = m_pView->model()->columnCount();
	for(int column = 0; column < columnCount; column++)
	{
		if (m_copyHiddenColumn == false)
		{
			if (m_pView->isColumnHidden(column) == true)
			{
				continue;
			}
		}

		textClipboard.append(m_pView->model()->headerData(column, Qt::Horizontal).toString());
		textClipboard.append("\t");
	}

	textClipboard.append("\n");

	int rowCount = m_pView->model()->rowCount();

	for(int row = 0; row < rowCount; row++)
	{
		if (m_copyCancel == true)
		{
			break;
		}

		if (m_pView->selectionModel()->isRowSelected(row, QModelIndex()) == false)
		{
			continue;
		}

		for(int column = 0; column < columnCount; column++)
		{
			if (m_copyHiddenColumn == false)
			{
				if (m_pView->isColumnHidden(column) == true)
				{
					continue;
				}
			}

			textClipboard.append(m_pView->model()->data(m_pView->model()->index(row, column)).toString());
			textClipboard.append("\t");
		}

		textClipboard.append("\n");
	}

	QClipboard* clipboard = QApplication::clipboard();
	clipboard->setText(textClipboard);

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void CopyData::copyCancel()
{
	m_copyCancel = true;
}

// -------------------------------------------------------------------------------------------------------------------

void CopyData::copyComplited()
{
	qDebug() << "Copy is complited!";
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

FindData::FindData(QTableView* pView) :
	QDialog(pView->parentWidget()),
	m_pView(pView)
{
	loadSettings();
	createInterface(pView);
}

// -------------------------------------------------------------------------------------------------------------------

FindData::~FindData()
{
}

// -------------------------------------------------------------------------------------------------------------------

void FindData::createInterface(QTableView* pView)
{
	if (pView == nullptr)
	{
		return;
	}

	setWindowFlags(Qt::Dialog | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
	setWindowTitle(tr("Find"));
	setWindowIcon(QIcon(":/icons/Find.png"));

	m_findCompleter.create(this);

	m_pFindTextEdit = new QLineEdit(m_findText, this);
	m_pFindTextEdit->setPlaceholderText(tr("Search Text"));
	m_pFindTextEdit->setCompleter(m_findCompleter.completer());
	//m_pFindTextEdit->setClearButtonEnabled(true);

	m_findNextButton = new QPushButton(tr(" Find next ..."), this);

	QHBoxLayout* mainLayout = new QHBoxLayout ;

	mainLayout->addWidget(m_pFindTextEdit);
	mainLayout->addWidget(m_findNextButton);

	setLayout(mainLayout);

	connect(m_pFindTextEdit, &QLineEdit::textChanged, this, &FindData::findTextChanged);
	connect(m_findNextButton, &QPushButton::clicked, this, &FindData::findNext);

	emit findTextChanged();
}

// -------------------------------------------------------------------------------------------------------------------

void FindData::findTextChanged()
{
	if (m_pView == nullptr)
	{
		return;
	}

	m_pView->clearSelection();

	m_findText = m_pFindTextEdit->text();
	if (m_findText.isEmpty() == true)
	{
		return;
	}

	int foundRow = find(-1);
	if (foundRow != -1)
	{
		m_pView->setCurrentIndex(m_pView->model()->index(foundRow, 0));
	}

	enableFindNextButton(foundRow);
}

// -------------------------------------------------------------------------------------------------------------------

void FindData::findNext()
{
	if (m_pView == nullptr)
	{
		return;
	}

	int startRow = m_pView->currentIndex().row();

	int foundRow = find(startRow);
	if (foundRow != -1)
	{
		m_pView->setCurrentIndex(m_pView->model()->index(foundRow, 0));
	}

	enableFindNextButton(foundRow);
}

// -------------------------------------------------------------------------------------------------------------------

int FindData::find(int start)
{
	if (m_pView == nullptr)
	{
		return - 1;
	}

	int rowCount = m_pView->model()->rowCount();
	int columnCount = m_pView->model()->columnCount();

	if (rowCount == 0 || columnCount == 0)
	{
		return -1;
	}

	m_findText = m_pFindTextEdit->text();
	if (m_findText.isEmpty() == true)
	{
		return -1;
	}

	int foundRow = -1;

	for(int row = start + 1; row < rowCount; row++)
	{
		for(int column = 0; column < columnCount; column++)
		{
			if (m_pView->isColumnHidden(column) == true)
			{
				continue;
			}

			QString text = m_pView->model()->data(m_pView->model()->index(row, column)).toString();

			if (text.contains(m_findText, Qt::CaseInsensitive) == false)
			{
				continue;
			}

			foundRow = row;

			break;
		}

		if (foundRow != -1)
		{
			break;
		}
	}

	return foundRow;
}

// -------------------------------------------------------------------------------------------------------------------

void FindData::enableFindNextButton(int foundRow)
{
	if (foundRow == -1)
	{
		m_findNextButton->setEnabled(false);
		return;
	}

	int foundNextRow = find(foundRow);
	if (foundNextRow != -1)
	{
		m_findNextButton->setEnabled(true);
	}
	else
	{
		m_findNextButton->setEnabled(false);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void FindData::loadSettings()
{
	QSettings s;

	m_findText = s.value(QString("%1/FindDataText").arg(FIND_DATA_OPTIONS_KEY), QString()).toString();

	m_findCompleter.load(FIND_DATA_OPTIONS_KEY);
}

// -------------------------------------------------------------------------------------------------------------------

void FindData::saveSettings()
{
	QSettings s;

	s.setValue(QString("%1/FindDataText").arg(FIND_DATA_OPTIONS_KEY), m_findText);
}

// -------------------------------------------------------------------------------------------------------------------

void FindData::reject()
{
	saveSettings();

	QDialog::reject();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

ExportData::ExportData(QTableView* pView, bool writeHiddenColumn, const QString& fileName) :
	QObject(pView),
	m_pView(pView),
	m_writeHiddenColumn(writeHiddenColumn),
	m_fileName(fileName)
{
	createProgressDialog(pView);
}

// -------------------------------------------------------------------------------------------------------------------

ExportData::~ExportData()
{
}

// -------------------------------------------------------------------------------------------------------------------

void ExportData::createProgressDialog(QTableView* pView)
{
	if (pView == nullptr)
	{
		return;
	}

	m_pProgressDialog = new QDialog(pView->parentWidget());

	m_pProgressDialog->setWindowFlags(Qt::Drawer);
	m_pProgressDialog->setFixedSize(300, 70);
	m_pProgressDialog->setWindowTitle(qApp->translate("ExportData.h", EXPORT_WINDOW_TITLE));
	m_pProgressDialog->setWindowIcon(QIcon(":/icons/Export.png"));

		m_progress = new QProgressBar;
		m_progress->setTextVisible(false);
		m_progress->setRange(0, 100);
		m_progress->setFixedHeight(10);

		m_cancelButton = new QPushButton;
		m_cancelButton->setText(tr("Cancel"));

		QHBoxLayout* buttonLayout = new QHBoxLayout ;

		buttonLayout->addStretch();
		buttonLayout->addWidget(m_cancelButton);
		buttonLayout->addStretch();

		QVBoxLayout* mainLayout = new QVBoxLayout ;

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
		QMessageBox::information(m_pProgressDialog,
								 qApp->translate("ExportData.h", EXPORT_WINDOW_TITLE),
								 tr("Data is absent!"));
		return;
	}

	//QString filter = tr("Excel files (*.xlsx);;CSV files (*.csv)");
	QString filter = tr("CSV files (*.csv)");

	QString fileName = QFileDialog::getSaveFileName(m_pProgressDialog,
													qApp->translate("ExportData.h", EXPORT_WINDOW_TITLE),
													m_fileName,
													filter);
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
	Q_UNUSED(fileName)
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
		if (m_writeHiddenColumn == false)
		{
			if (m_pView->isColumnHidden(column) == true)
			{
				continue;
			}
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
			if (m_writeHiddenColumn == false)
			{
				if (m_pView->isColumnHidden(column) == true)
				{
					continue;
				}
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

void ExportData::exportCancel()
{
	m_exportCancel = true;
}

// -------------------------------------------------------------------------------------------------------------------

void ExportData::exportComplited()
{
	QMessageBox::information(m_pProgressDialog,
							 qApp->translate("ExportData.h", EXPORT_WINDOW_TITLE),
							 tr("Export is complited!"));
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
