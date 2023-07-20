#include "SchemasReport.h"
#include "../../lib/Ui/DialogProgress.h"
#include <QPageSetupDialog>
#include <QPrinter>
#include "DialogSchemasReport.h"
#include "../UtilsLib/Ui/UiTools.h"

using namespace ReportLib;
using namespace Builder;

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
														   QWidget *parent,
														   const Builder::SchemasReportOptions& options,
														   const std::vector<SchemaTypesParams>& schemaTypesParams):
	m_serverIp(serverIp),
	m_serverPort(serverPort),
	m_serverUserName(serverUserName),
	m_serverPassword(serverPassword),
	m_projectName(projectName),
	m_userName(userName),
	m_userPassword(userPassword),
	m_signalSet(signalSet),
	m_parent(parent),
	m_options(options),
	m_schemaTypesParams(schemaTypesParams)
{

}

void SchemasReportGeneratorThread::exportSchemasToMultiplePdf(const QString& pdfPath, const std::vector<DbFileInfo>& files)
{
	run(TaskType::ExportFilesToMultiplePdf, pdfPath, files);
}

void SchemasReportGeneratorThread::exportSchemasToSinglePdf(const QString& albumPath, const std::vector<DbFileInfo>& files)
{
	QDir().mkpath(albumPath);
	run(TaskType::ExportFilesToSinglePdf, albumPath, files);
}

void SchemasReportGeneratorThread::exportAllSchemasToAlbum(const QString& albumPath)
{
	QDir().mkpath(albumPath);
	run(TaskType::ExportAllSchemasToAlbum, albumPath, {});
}

void SchemasReportGeneratorThread::run(TaskType task,
									   const QString& filePath,
									   const std::vector<DbFileInfo>& files)
{
	// Create View

	std::shared_ptr<ReportSchemaView> schemaView = std::make_shared<ReportSchemaView>(m_options.itemsLabels);

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
																filePath,
																m_options,
																m_schemaTypesParams);

	// Create Progress Dialog

	DialogProgress dialogProgress(QObject::tr("Exporting Schemas to PDF"), 1, m_parent);

	// Create thread

	QThread* thread = new QThread;

	worker->moveToThread(thread);

	switch(task)
	{
	case TaskType::ExportFilesToMultiplePdf:
		{
			QObject::connect(thread, &QThread::started, worker, &SchemasReportGenerator::exportFilesToMultiplePdf);
		}
		break;
	case TaskType::ExportFilesToSinglePdf:
		{
			QObject::connect(thread, &QThread::started, worker, &SchemasReportGenerator::exportFilesToSinglePdf);
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
		if (task == TaskType::ExportFilesToSinglePdf)
		{
			if (QMessageBox::question(m_parent, qAppName(), QObject::tr("Album generating has been finished.\n\nDo you with to open it?")) == QMessageBox::Yes)
			{
				UiTools::openPdf(filePath, m_parent);
			}
		}
		else
		{
			if (task == TaskType::ExportFilesToMultiplePdf || task == TaskType::ExportAllSchemasToAlbum)
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

