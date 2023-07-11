#include "SchemasReport.h"
#include "../../lib/Ui/DialogProgress.h"
#include <QPageSetupDialog>
#include <QPrinter>
#include "DialogReportFileTypeParams.h"
#include "../UtilsLib/Ui/UiTools.h"

//
// SchemasReportDialog
//

bool SchemasReportDialog::getReportFileName(QString* fileName,
						 QPageLayout* pageLayout,
						 QWidget *parent)
{
	if (fileName == nullptr || pageLayout == nullptr)
	{
		Q_ASSERT(fileName);
		Q_ASSERT(pageLayout);
		return false;
	}

	SchemasReportDialog d(fileName, pageLayout, parent);
	if (d.exec() == QDialog::Accepted)
	{
		return true;
	}
	return false;
}

bool SchemasReportDialog::getReportFilesPath(QString* path,
						 std::vector<ReportFileTypeParams>* reportFileTypeParams,
						 const std::vector<ReportFileTypeParams>& defaultFileTypeParams,
						 QWidget *parent)
{
	if (path == nullptr || reportFileTypeParams == nullptr)
	{
		Q_ASSERT(path);
		Q_ASSERT(reportFileTypeParams);
		return false;
	}

	SchemasReportDialog d(path, reportFileTypeParams, defaultFileTypeParams, parent);
	if (d.exec() == QDialog::Accepted)
	{
		return true;
	}
	return false;
}

SchemasReportDialog::SchemasReportDialog(Type type, QString* path, QWidget *parent):
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	m_type(type),
	m_reportPath(path)
{
	setWindowTitle(tr("Export Schemas to Album"));
	setMinimumWidth(500);

	QLabel* label = new QLabel(m_type == Type::SelectFile ? tr("Report file:") : tr("Report path:"));
	m_editReportPath = new QLineEdit(*path);

	QPushButton* browseButton = new QPushButton(tr("Browse..."));
	connect(browseButton, &QPushButton::clicked, this, &SchemasReportDialog::browseClicked);

	QHBoxLayout* reportPathLayout = new QHBoxLayout();
	reportPathLayout->addWidget(label);
	reportPathLayout->addWidget(m_editReportPath);
	reportPathLayout->addWidget(browseButton);

	QPushButton* okButton = new QPushButton(tr("OK"));
	connect(okButton, &QPushButton::clicked, this, &SchemasReportDialog::okClicked);

	QPushButton* cancelButton = new QPushButton(tr("Cancel"));
	connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

	QPushButton* pageSetupButton = new QPushButton(tr("Page Setup..."));
	connect(pageSetupButton, &QPushButton::clicked, this, &SchemasReportDialog::pageSetupClicked);

	QHBoxLayout* buttonsLayout = new QHBoxLayout();
	buttonsLayout->addWidget(pageSetupButton);
	buttonsLayout->addStretch();
	buttonsLayout->addWidget(okButton);
	buttonsLayout->addWidget(cancelButton);

	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->addLayout(reportPathLayout);
	mainLayout->addLayout(buttonsLayout);
	setLayout(mainLayout);
}

SchemasReportDialog::SchemasReportDialog(QString* path, std::vector<ReportFileTypeParams>* reportFileTypeParams, const std::vector<ReportFileTypeParams>& defaultFileTypeParams, QWidget *parent):
	SchemasReportDialog(Type::SelectPath, path, parent)
{
	m_reportFileTypeParams = reportFileTypeParams;
	m_defaultFileTypeParams = defaultFileTypeParams;

	return;
}

SchemasReportDialog::SchemasReportDialog(QString* fileName, QPageLayout* pageLayout, QWidget *parent):
	SchemasReportDialog(Type::SelectFile, fileName,  parent)
{
	m_pageLayout = pageLayout;

	return;
}

void SchemasReportDialog::okClicked()
{
	QString text = m_editReportPath->text();

	if (text.isEmpty() == true)
	{
		QMessageBox::critical(this, qAppName(), tr("Please enter the file name!"));
		m_editReportPath->setFocus();
		return;
	}

	*m_reportPath = text;

	QDialog::accept();
}

void SchemasReportDialog::browseClicked()
{
	if (m_type == Type::SelectFile)
	{
		static QString path{"."};
		QString fileName = QFileDialog::getSaveFileName(this, qAppName(), path + QDir::separator(), QObject::tr("PDF documents (*.pdf)"));

		if (fileName.isNull() == true || fileName.isEmpty() == true)
		{
			return;
		}
		path = QFileInfo(fileName).path(); // store path for next time

		m_editReportPath->setText(QDir::toNativeSeparators(fileName));
	}
	else
	{
		QString dir = QFileDialog::getExistingDirectory(this, tr("Select Directory"), QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
		if (dir.isEmpty() == true)
		{
			return;
		}

		m_editReportPath->setText(QDir::toNativeSeparators(dir));
	}

	return;
}

void SchemasReportDialog::pageSetupClicked()
{
	if (m_type == Type::SelectFile)
	{
		if (m_pageLayout == nullptr)
		{
			Q_ASSERT(m_pageLayout);
			return;
		}

		// Ask for page format

		QPageSize::PageSizeId id = QPageSize::id(m_pageLayout->pageSize().sizePoints(), QPageSize::FuzzyOrientationMatch);
		if (id == QPageSize::Custom)
		{
			id = QPageSize::A3;
		}

		QPrinter printer(QPrinter::HighResolution);
		printer.setFullPage(true);
		printer.setPageSize(QPageSize(id));
		printer.setPageOrientation(m_pageLayout->orientation());
		printer.setPageMargins(m_pageLayout->margins(), QPageLayout::Unit::Millimeter);

		QPageSetupDialog d(&printer, this);
		if (d.exec() != QDialog::Accepted)
		{
			return;
		}

		id = QPageSize::id(d.printer()->pageLayout().pageSize().sizePoints(), QPageSize::FuzzyOrientationMatch);

		m_pageLayout->setPageSize(QPageSize(id));
		m_pageLayout->setOrientation(d.printer()->pageLayout().orientation());
		m_pageLayout->setMargins(d.printer()->pageLayout().margins());
	}
	else
	{
		if (m_reportFileTypeParams == nullptr)
		{
			Q_ASSERT(m_reportFileTypeParams);
			return;
		}

		DialogReportFileTypeParams d(*m_reportFileTypeParams, m_defaultFileTypeParams, this);
		if (d.exec() == QDialog::Accepted)
		{
			*m_reportFileTypeParams = d.fileTypeParams();
		}
	}

	return;
}

//
// SchemasReportGeneratorThread
//

SchemasReportGeneratorThread::SchemasReportGeneratorThread(const QString& serverIp,
														   int serverPort,
														   const QString& serverUserName,
														   const QString& serverPassword,
														   const QString& projectName,
														   const QString& userName,
														   const QString& userPassword,
														   const AppSignalSet *signalSet,
														   QWidget *parent):
	m_serverIp(serverIp),
	m_serverPort(serverPort),
	m_serverUserName(serverUserName),
	m_serverPassword(serverPassword),
	m_projectName(projectName),
	m_userName(userName),
	m_userPassword(userPassword),
	m_signalSet(signalSet),
	m_parent(parent)
{

}

void SchemasReportGeneratorThread::exportSchemasToPdf(const QString& pdfPath, const std::vector<DbFileInfo>& files)
{
	run(TaskType::ExportFilesToPdf, pdfPath, files, QPageLayout(), {});
}

void SchemasReportGeneratorThread::exportSchemasToAlbum(const QString& albumPath, const std::vector<DbFileInfo>& files, const QPageLayout& pageLayout)
{
	run(TaskType::ExportFilesToAlbum, albumPath, files, pageLayout, {});
}

void SchemasReportGeneratorThread::exportAllSchemasToAlbum(const QString& albumPath, const std::vector<ReportFileTypeParams>& reportFileTypeParams)
{
	run(TaskType::ExportAllSchemasToAlbum, albumPath, {}, QPageLayout(), reportFileTypeParams);
}

void SchemasReportGeneratorThread::run(TaskType task,
									   const QString& filePath,
									   const std::vector<DbFileInfo>& files,
									   const QPageLayout& albumPageLayout,	// Used in ExportFilesToAlbum
									   const std::vector<ReportFileTypeParams>& albumsFileTypeParams)// Used in ExportAllSchemasToAlbum
{
	// Create View

	std::shared_ptr<ReportSchemaView> schemaView = std::make_shared<ReportSchemaView>();

	schemaView->session().setProject(m_projectName);
	schemaView->session().setUsername(m_userName);
	schemaView->session().setHost(QHostInfo::localHostName());

	// Create Worker

	SchemasReportGenerator* worker = new SchemasReportGenerator(schemaView,
																m_signalSet,
																m_serverIp,
																m_serverPort,
																m_serverUserName,
																m_serverPassword,
																m_projectName,
																m_userName,
																m_userPassword,
																files,
																filePath);

	if (task == TaskType::ExportFilesToAlbum)
	{
		worker->setPageLayout(albumPageLayout);
	}
	if (task == TaskType::ExportAllSchemasToAlbum)
	{
		worker->setReportFileTypeParams(albumsFileTypeParams);
	}

	// Create Progress Dialog

	DialogProgress dialogProgress(QObject::tr("Exporting Schemas to PDF"), 1, m_parent);

	// Create thread

	QThread* thread = new QThread;

	worker->moveToThread(thread);

	switch(task)
	{
	case TaskType::ExportFilesToPdf:
		{
			QObject::connect(thread, &QThread::started, worker, &SchemasReportGenerator::exportFilesToPdf);
		}
		break;
	case TaskType::ExportFilesToAlbum:
		{
			QObject::connect(thread, &QThread::started, worker, &SchemasReportGenerator::exportFilesToAlbum);
		}
		break;
	case TaskType::ExportAllSchemasToAlbum:
		{
			Q_ASSERT(files.empty() == true);	// No files should be here
			QObject::connect(thread, &QThread::started, worker, &SchemasReportGenerator::exportAllSchemasToAlbums);
		}
		break;
	}

	QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);	// Schedule thread deleting

	QObject::connect(&dialogProgress, &DialogProgress::getProgress, worker, &SchemasReportGenerator::progressRequested, Qt::DirectConnection);
	QObject::connect(&dialogProgress, &DialogProgress::cancelClicked, worker, &SchemasReportGenerator::stop, Qt::DirectConnection);

	QObject::connect(worker, &SchemasReportGenerator::progressChanged, &dialogProgress, &DialogProgress::setProgressSingle);

	//  Schedule objects deleting

	QObject::connect(worker, &SchemasReportGenerator::finished, worker, [thread, &dialogProgress, worker, schemaView](const QString& errorMessage)
	{
		thread->quit();

		if (errorMessage.isEmpty() == false)
		{
			dialogProgress.setErrorMessage(errorMessage);
		}

		dialogProgress.exit();

		worker->deleteLater();
	});

	// Start thread

	thread->start();

	dialogProgress.exec();

	if (dialogProgress.hasErrorMessage() == false)
	{
		if (task == TaskType::ExportFilesToAlbum)
		{
			if (QMessageBox::question(m_parent, qAppName(), QObject::tr("Album generating has been finished.\n\nDo you with to open it?")) == QMessageBox::Yes)
			{
				UiTools::openHelp(filePath, m_parent);
			}
		}
		else
		{
			if (task == TaskType::ExportFilesToPdf || task == TaskType::ExportAllSchemasToAlbum)
			{
				if (QMessageBox::question(m_parent, qAppName(), QObject::tr("Album generating has been finished.\n\nDo you with to open the containing folder?")) == QMessageBox::Yes)
				{
					QUrl url = QUrl::fromLocalFile(filePath);
					QDesktopServices::openUrl(url);
				}
			}
		}
	}
	else
	{
		QMessageBox::critical(m_parent, qAppName(), dialogProgress.errorMessage());
	}

	return;
}

