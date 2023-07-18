#include "DialogSchemasExport.h"
#include "ui_DialogSchemasExport.h"

DialogSchemasExport::DialogSchemasExport(const Builder::SchemasReportOptions& options,
										 const QString& defaultPath,
										 const QString& defaultFile,
										 QWidget *parent) :
	QDialog(parent),
	ui(new Ui::DialogSchemasExport),
	m_options(options),
	m_pathName(defaultPath),
	m_fileName(defaultFile)
{
	ui->setupUi(this);

	ui->checkAddMargins->setChecked(m_options.addPageNumbers);
	ui->checkAddSchemaSignals->setChecked(m_options.addLogicSchemaDetails);
	ui->checkAddItemsLabels->setChecked(m_options.infoMode);

	ui->filePathEdit->setText(m_pathName);
	ui->fileNameEdit->setText(m_fileName);

	ui->filePathEdit->setFocus();
}

DialogSchemasExport::~DialogSchemasExport()
{
	delete ui;
}

const Builder::SchemasReportOptions& DialogSchemasExport::options() const
{
	return m_options;
}

bool DialogSchemasExport::isSingleFile() const
{
	return ui->tabWidget->currentIndex() == 1;
}

const QString& DialogSchemasExport::pathName() const
{
	return m_pathName;
}

const QString& DialogSchemasExport::fileName() const
{
	return m_fileName;
}

void DialogSchemasExport::accept()
{
	m_fileName = ui->fileNameEdit->text();
	m_pathName = ui->filePathEdit->text();

	m_options.addPageNumbers = ui->checkAddMargins->isChecked() == true;
	m_options.addLogicSchemaDetails = ui->checkAddSchemaSignals->isChecked() == true;
	m_options.infoMode = ui->checkAddItemsLabels->isChecked() == true;

	if (isSingleFile() == false)
	{
		if (m_pathName.isEmpty() == true || QDir(m_pathName).exists() == false)
		{
			QMessageBox::critical(this, qAppName(), tr("Selected path is incorrect, please choose an existing path."));
			ui->filePathEdit->setFocus();
			return;
		}
	}
	else
	{
		if (m_fileName.isEmpty() == true)
		{
			QMessageBox::critical(this, qAppName(), tr("File name can't be empty, please choose the file name."));
			ui->filePathEdit->setFocus();
			return;
		}
	}

	QDialog::accept();
}

void DialogSchemasExport::on_buttonPathBrowse_clicked()
{
	QString pdfDirectory = QFileDialog::getExistingDirectory(this,
															 QObject::tr("Select Directory"),
															 ui->filePathEdit->text(),
															 QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

	if (pdfDirectory.isNull() == true || pdfDirectory.isEmpty() == true)
	{
		return;
	}
	pdfDirectory = QDir::toNativeSeparators(pdfDirectory);
	ui->filePathEdit->setText(pdfDirectory);
}


void DialogSchemasExport::on_buttonFileBrowse_clicked()
{
	static QString path{"."};
	QString singleFileName = QFileDialog::getSaveFileName(this,
													tr("Export to PDF"),
													ui->fileNameEdit->text(),
													tr("Portable Documnet Format (*.pdf)"));
	if (singleFileName.isEmpty() == true)
	{
		return;
	}
	singleFileName = QDir::toNativeSeparators(singleFileName);
	path = QFileInfo(singleFileName).path(); // store path for next time
	ui->fileNameEdit->setText(singleFileName);
}

