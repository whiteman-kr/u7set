#include "SchemasReport.h"
#include "DialogSchemasReport.h"
#include "Settings.h"

#include "../UtilsLib/Ui/UiTools.h"

#include <VFrame30/SchemaDetails.h>
#include <UiLib/DialogProgress.h>

using namespace ReportLib;
using namespace Builder;

void SchemasAlbumGenerator::createSchemasAlbums(DbController* db, const AppSignalSet* signalSet, QWidget* parent)
{
	QString path = QSettings{}.value("SchemaEditor/Export/AlbumPath", QDir().toNativeSeparators(QDir::currentPath())).toString();

	std::vector<Builder::SchemaTypesParams> schemaTypesParams = Builder::SchemasReportGenerator::defaultFileTypesParams(db);
	for (auto& param : schemaTypesParams)
	{
		param.load(db);
	}

	// Get all schemas tags
	//
	std::set<QString> schemaTags;

	{
		// Approach 2 - parse all files and get ALL tags from details, slower
		DbFileTree tree;
		if (db->getFileListTree(&tree, db->systemFileId(DbDir::SchemasDir), true, parent) == false)
		{
			return;
		}
		auto filePointers = tree.toVectorOfSharedPointers(true);

		VFrame30::SchemaDetails details;
		for (const auto& file : filePointers)
		{
			bool ok = details.parseDetails(file->details());
			if (ok == true)
			{
				for (const QString& tag : details.schemaTags())
				{
					schemaTags.insert(tag);
				}
			}
		}
	}

	Builder::SchemasReportOptions options = Builder::SchemasReportOptions::optionsForSchemasAlbum(db);
	options.setSchemaTags(schemaTags);

	// Show dialog with report options
	//
	DialogSchemasReport d(path,
						  schemaTypesParams,
						  options,
						  db,
						  parent);
	
	int result = d.exec();

	if (d.optionsApplied() == true)
	{
		// Save options set in the dialog
		//
		schemaTypesParams = d.schemaTypesParams();
		for (auto& param : schemaTypesParams)
		{
			param.save(db);
		}
		
		options = d.options();
		options.save(db);

		path = d.path();
		QSettings{}.setValue("SchemaEditor/Export/AlbumPath", path);

	}

	if (result != QDialog::Accepted)
	{
		return;
	}
	
	SchemasReportGeneratorThread r(theSettings.serverHost(),
								   theSettings.serverPort(),
								   theSettings.serverUsername(),
								   theSettings.serverPassword(),
								   db->currentProject().projectName(),
								   db->currentUser().username(),
								   db->currentUser().password(),
								   signalSet,
								   parent,
								   options,
								   schemaTypesParams);

	r.exportAllSchemasToAlbum(path);
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
	QDir().mkpath(pdfPath);
	run(TaskType::ExportFilesToMultiplePdf, pdfPath, files);
}

void SchemasReportGeneratorThread::exportSchemasToSinglePdf(const QString& fileName, const std::vector<DbFileInfo>& files)
{
	QString pdfPath = QFileInfo(fileName).absolutePath();
	QDir().mkpath(pdfPath);
	run(TaskType::ExportFilesToSinglePdf, fileName, files);
}

void SchemasReportGeneratorThread::exportAllSchemasToAlbum(const QString& pdfPath)
{
	QDir().mkpath(pdfPath);
	run(TaskType::ExportAllSchemasToAlbum, pdfPath, {});
}

void SchemasReportGeneratorThread::run(TaskType task,
									   const QString& filePath,
									   const std::vector<DbFileInfo>& files)
{
	// Create View

	std::shared_ptr<ReportSchemaView> schemaView = std::make_shared<ReportSchemaView>(m_options.itemsLabels());

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
																false, /*generateToOutputData*/
																m_options,
																m_schemaTypesParams);

	// Create Progress Dialog

	UiLib::DialogProgress dialogProgress(QObject::tr("Exporting Schemas to PDF"), 1, m_parent);

	// Create thread

	QThread* thread = new QThread;

	worker->moveToThread(thread);

	switch(task)
	{
	case TaskType::ExportFilesToMultiplePdf:
		{
			QObject::connect(thread, &QThread::started, worker, &SchemasReportGenerator::exportSchemasToMultiplePdf);
		}
		break;
	case TaskType::ExportFilesToSinglePdf:
		{
			QObject::connect(thread, &QThread::started, worker, &SchemasReportGenerator::exportSchemasToSinglePdf);
		}
		break;
	case TaskType::ExportAllSchemasToAlbum:
		{
			Q_ASSERT(files.empty() == true);	// No files should be here
			QObject::connect(thread, &QThread::started, worker, &SchemasReportGenerator::exportSchemasToAlbums);
		}
		break;
	}

	QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);	// Schedule thread deleting

	QObject::connect(&dialogProgress, &UiLib::DialogProgress::getProgress, worker, &SchemasReportGenerator::progressRequested, Qt::DirectConnection);
	QObject::connect(&dialogProgress, &UiLib::DialogProgress::cancelClicked, worker, &SchemasReportGenerator::stop, Qt::DirectConnection);

	QObject::connect(worker, &SchemasReportGenerator::progressChanged, &dialogProgress, &UiLib::DialogProgress::setProgressSingle);

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

