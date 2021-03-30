#include "SchemasReportGenerator.h"
#include "../VFrame30/Schema.h"
#include "../../lib/Ui/DialogProgress.h"
#include <QPageSetupDialog>
#include <QPrinter>

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
	m_reportPath(path),
	m_type(type)
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
		QString path = QFileDialog::getSaveFileName(this, qAppName(), "./", QObject::tr("PDF documents (*.pdf)"));

		if (path.isNull() == true || path.isEmpty() == true)
		{
			return;
		}

		m_editReportPath->setText(QDir::toNativeSeparators(path));
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
											   QWidget* parent):
	m_serverIp(serverIp),
	m_serverPort(serverPort),
	m_serverUserName(serverUserName),
	m_serverPassword(serverPassword),
	m_projectName(projectName),
	m_userName(userName),
	m_userPassword(userPassword),
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
									   const std::vector<ReportFileTypeParams>& albumsFileTypeParams)	// Used in ExportAllSchemasToAlbum
{
	// Create View

	ReportSchemaView* schemaView = new ReportSchemaView(m_parent);

	schemaView->session().setProject(m_projectName);
	schemaView->session().setUsername(m_userName);
	schemaView->session().setHost(QHostInfo::localHostName());

	// Create Worker

	SchemasReportGenerator* worker = new SchemasReportGenerator(schemaView,
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

	DialogProgress* dialogProgress = new DialogProgress(QObject::tr("Exporting Schemas to PDF"), 1, m_parent);

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
			QObject::connect(thread, &QThread::started, worker, &SchemasReportGenerator::exportAllSchemasToAlbum);
		}
		break;
	}

	QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);	// Schedule thread deleting

	QObject::connect(dialogProgress, &DialogProgress::getProgress, worker, &SchemasReportGenerator::progressRequested, Qt::DirectConnection);

	QObject::connect(dialogProgress, &DialogProgress::cancelClicked, worker, &SchemasReportGenerator::stop, Qt::DirectConnection);

	QObject::connect(worker, &SchemasReportGenerator::progressChanged, dialogProgress, &DialogProgress::setProgressSingle);

	//  Schedule objects deleting

	QObject::connect(worker, &SchemasReportGenerator::finished, [thread, dialogProgress, worker, schemaView](const QString& errorMessage)
	{
		thread->quit();

		if (errorMessage.isEmpty() == false)
		{
			dialogProgress->setErrorMessage(errorMessage);
		}
		else
		{
			dialogProgress->deleteLater();
		}

		worker->deleteLater();

		schemaView->deleteLater();
	});

	// Start thread

	thread->start();

	dialogProgress->exec();

	return;
}

//
// SchemasReportGenerator
//

SchemasReportGenerator::SchemasReportGenerator(ReportSchemaView* schemaView,
										 const QString& serverIp,
										 int serverPort,
										 const QString& serverUserName,
										 const QString& serverPassword,
										 const QString& projectName,
										 const QString& userName,
										 const QString& userPassword,
										 std::vector<DbFileInfo> files,
										 const QString& filePath):
	ReportGenerator(schemaView),
	m_serverIp(serverIp),
	m_serverPort(serverPort),
	m_serverUserName(serverUserName),
	m_serverPassword(serverPassword),
	m_projectName(projectName),
	m_userName(userName),
	m_userPassword(userPassword),
	m_filePath(filePath),
	m_inputFiles(files)
{
	m_font = QFont("Times", 24);
	m_marginFont = QFont("Times", static_cast<int>(24 * (96.0 / resolution())));

	return;
}

SchemasReportGenerator::~SchemasReportGenerator()
{
	qDebug() << "SchemasReportWorker deleted";
}

void SchemasReportGenerator::setReportFileTypeParams(const std::vector<ReportFileTypeParams>& reportFileTypeParams)
{
	m_reportFileTypeParams = reportFileTypeParams;
}

std::vector<ReportFileTypeParams> SchemasReportGenerator::defaultFileTypeParams(DbController* db)
{
	std::vector<ReportFileTypeParams> result;

	if (db == nullptr || db->isProjectOpened() == false)
	{
		Q_ASSERT(false);
		return result;
	}

	result.push_back({db->systemFileId(DbDir::MonitorSchemasDir), QObject::tr("Monitor Schemas"),			true, QPageLayout(QPageSize(QPageSize::A3), QPageLayout::Orientation::Landscape, QMarginsF(15, 15, 15, 15))});
	result.push_back({db->systemFileId(DbDir::TuningSchemasDir), QObject::tr("Tuning Schemas"),			true, QPageLayout(QPageSize(QPageSize::A3), QPageLayout::Orientation::Landscape, QMarginsF(15, 15, 15, 15))});
	result.push_back({db->systemFileId(DbDir::DiagnosticsSchemasDir), QObject::tr("Diagnostics Schemas"),		true, QPageLayout(QPageSize(QPageSize::A3), QPageLayout::Orientation::Landscape, QMarginsF(15, 15, 15, 15))});
	result.push_back({db->systemFileId(DbDir::AppLogicDir), QObject::tr("Application Logic"),		true, QPageLayout(QPageSize(QPageSize::A3), QPageLayout::Orientation::Landscape, QMarginsF(15, 15, 15, 15))});
	result.push_back({db->systemFileId(DbDir::UfblDir), QObject::tr("UFBL Descriptions"),		true, QPageLayout(QPageSize(QPageSize::A3), QPageLayout::Orientation::Landscape, QMarginsF(15, 15, 15, 15))});

	return result;
}

void SchemasReportGenerator::exportFilesToPdf()
{
	std::map<QString, std::shared_ptr<VFrame30::Schema>> schemas;	// Key is full path to schema file

	try
	{
		openProject();

		loadSchemas(m_inputFiles, &schemas);

		closeProject();
	}

	catch (QString errorMessage)
	{
		closeProject();

		emit finished(errorMessage);
	}

	{
		QMutexLocker l(&m_statisticsMutex);
		m_currentStatus = WorkerStatus::Rendering;

		m_schemaIndex = 0;
	}

	// Save schemas to PDF

	for (auto it = schemas.begin(); it != schemas.end(); it++)
	{
		if (m_stop == true)
		{
			break;
		}

		const std::shared_ptr<VFrame30::Schema> schema = it->second;
		const QString& schemaId = schema->schemaId();

		{
			QMutexLocker l(&m_statisticsMutex);
			m_schemaIndex++;
			m_currentSchemaId = schemaId;
		}

		// Create separate PDF writer

		QString fileName = it->first;

		fileName.replace('/', '_');

		int pos = fileName.lastIndexOf('.');
		if (pos != -1)
		{
			fileName = fileName.left(pos);
		}
		fileName += tr(".pdf");

		std::shared_ptr<QPdfWriter> pdfWriter = std::make_shared<QPdfWriter>(filePath() + "//" + fileName);

		pdfWriter->setTitle(schema->caption());

		QPageSize pageSize;
		double pageWidth = schema->docWidth();
		double pageHeight = schema->docHeight();

		if (schema->unit() == VFrame30::SchemaUnit::Inch)
		{
			pageSize = QPageSize(QSizeF(pageWidth, pageHeight), QPageSize::Inch);
			pdfWriter->setResolution(resolution());
		}
		else
		{
			assert(schema->unit() == VFrame30::SchemaUnit::Display);
			pageSize = QPageSize(QSize(static_cast<int>(pageWidth), static_cast<int>(pageHeight)));

			pdfWriter->setResolution(72);	// 72 is from enum QPageLayout::Unit help,
			// QPageLayout::Point	1	1/!!! 72th !!!! of an inch
		}

		pdfWriter->setPageSize(pageSize);
		pdfWriter->setPageMargins(QMarginsF(0, 0, 0, 0));

		// Create painter

		QPainter painter(pdfWriter.get());

		printSchema(pdfWriter.get(), &painter, schema, {}, {});
	}

	emit finished(QString());

	return;
}

void SchemasReportGenerator::exportFilesToAlbum()
{
	std::map<QString, std::shared_ptr<VFrame30::Schema>> schemas;	// Key is full path to schema file

	try
	{
		openProject();

		loadSchemas(m_inputFiles, &schemas);

		closeProject();
	}

	catch (QString errorMessage)
	{
		closeProject();

		emit finished(errorMessage);
	}

	{
		QMutexLocker l(&m_statisticsMutex);
		m_currentStatus = WorkerStatus::Rendering;

		m_schemaIndex = 0;
	}

	// Init margins
	clearMarginItems();	// Just for fun
	addMarginItem({tr("Project: %1").arg(m_projectName), -1, -1, m_marginFont, Qt::AlignLeft | Qt::AlignTop});
	addMarginItem({tr("%OBJECT%"), -1, -1, m_marginFont, Qt::AlignRight | Qt::AlignTop});

	// Create PDF writer

	std::shared_ptr<QPdfWriter> pdfWriter = std::make_shared<QPdfWriter>(filePath());
	pdfWriter->setTitle(m_projectName);
	pdfWriter->setPageLayout(pageLayout());
	pdfWriter->setResolution(resolution());

	bool firstSchema = true;

	QPainter painter(pdfWriter.get());

	for (auto it = schemas.begin(); it != schemas.end(); it++)
	{
		if (m_stop == true)
		{
			break;
		}

		const std::shared_ptr<VFrame30::Schema> schema = it->second;
		const QString& schemaId = schema->schemaId();

		{
			QMutexLocker l(&m_statisticsMutex);
			m_schemaIndex++;
			m_currentSchemaId = schemaId;
		}

		if (firstSchema == true)
		{
			firstSchema = false;
		}
		else
		{
			pdfWriter->newPage();
		}

		QTextDocument emptyTextDocument;
		printDocument(pdfWriter.get(), &emptyTextDocument, &painter, tr("Schema: %1").arg(schemaId), nullptr, nullptr, 0);

		printSchema(pdfWriter.get(), &painter, schema, {}, {});
	}

	emit finished(QString());

	return;
}

void SchemasReportGenerator::exportAllSchemasToAlbum()
{
	std::vector<SchemaFilesInfo> schemaFilesInfo;

	try
	{
		openProject();

		schemaFilesInfo.push_back({db()->systemFileId(DbDir::AppLogicDir), tr("ApplicationLogic")});
		schemaFilesInfo.push_back({db()->systemFileId(DbDir::MonitorSchemasDir), tr("MonitorSchemas")});
		schemaFilesInfo.push_back({db()->systemFileId(DbDir::TuningSchemasDir), tr("TunungSchemas")});
		schemaFilesInfo.push_back({db()->systemFileId(DbDir::DiagnosticsSchemasDir), tr("DiagnosticsSchemas")});
		schemaFilesInfo.push_back({db()->systemFileId(DbDir::UfblDir), tr("UFBSchemas")});

		for (SchemaFilesInfo& sfi : schemaFilesInfo)
		{
			if (m_stop == true)
			{
				break;
			}

			DbFileTree fileTree;

			{
				QMutexLocker l(&m_statisticsMutex);
				m_currentSchemaType = sfi.caption;
			}

			bool ok = db()->getFileListTree(&fileTree, sfi.fileId, true/*removeDeleted*/, nullptr);
			if (ok == false)
			{
				throw(tr("DbController::getFileListTree failed on fileId = %1").arg(db()->systemFileId(DbDir::SchemasDir)));
			}

			const std::map<int, std::shared_ptr<DbFileInfo>>  files = fileTree.files();

			for (auto it = files.begin(); it != files.end(); it++)
			{
				const std::shared_ptr<DbFileInfo>& fi = it->second;

				if (fi->fileName().endsWith("." + QString(Db::File::AlFileExtension)) == false &&
					fi->fileName().endsWith("." + QString(Db::File::UfbFileExtension)) == false &&
					fi->fileName().endsWith("." + QString(Db::File::MvsFileExtension)) == false &&
					fi->fileName().endsWith("." + QString(Db::File::DvsFileExtension)) == false)
				{
					continue;
				}

				sfi.schemasFiles.push_back(*fi);
			}

			loadSchemas(sfi.schemasFiles, &sfi.schemas);

		}

		closeProject();
	}

	catch (QString errorMessage)
	{
		closeProject();

		emit finished(errorMessage);
	}

	// Init margins
	clearMarginItems();	// Just for fun
	addMarginItem({tr("Project: %1").arg(m_projectName), -1, -1, m_marginFont, Qt::AlignLeft | Qt::AlignTop});
	addMarginItem({tr("%OBJECT%"), -1, -1, m_marginFont, Qt::AlignRight | Qt::AlignTop});

	for (SchemaFilesInfo& sfi : schemaFilesInfo)
	{
		if (m_stop == true)
		{
			break;
		}

		if (sfi.schemas.empty() == true)
		{
			continue;
		}

		{
			QMutexLocker l(&m_statisticsMutex);
			m_currentStatus = WorkerStatus::Rendering;
			m_currentSchemaType = sfi.caption;
			m_schemaIndex = 0;
			m_schemasCount = static_cast<int>(sfi.schemas.size());
		}

		// Save schemas to PDF

		// Create single PDF writer

		QString fileName = tr("%1/%2_%3.pdf").arg(filePath()).arg(m_projectName).arg(sfi.caption);

		std::shared_ptr<QPdfWriter> pdfWriter = std::make_shared<QPdfWriter>(fileName);
		pdfWriter->setTitle(m_projectName);

		// Find page layout

		QPageLayout pl = pageLayout();

		bool plFound = false;

		for (const ReportFileTypeParams& rp : m_reportFileTypeParams)
		{
			if (rp.fileId == sfi.fileId)
			{
				pl = rp.pageLayout;

				plFound = true;
				break;
			}
		}

		if (plFound == false)
		{
			// File type was not found
			Q_ASSERT(false);
		}

		pdfWriter->setPageLayout(pl);

		pdfWriter->setResolution(resolution());

		// Render schemas

		bool firstSchema = true;

		QPainter painter(pdfWriter.get());

		for (auto it = sfi.schemas.begin(); it != sfi.schemas.end(); it++)
		{
			if (m_stop == true)
			{
				break;
			}

			const std::shared_ptr<VFrame30::Schema> schema = it->second;
			const QString& schemaId = schema->schemaId();

			{
				QMutexLocker l(&m_statisticsMutex);
				m_schemaIndex++;
				m_currentSchemaId = schemaId;
			}

			if (firstSchema == true)
			{
				firstSchema = false;
			}
			else
			{
				pdfWriter->newPage();
			}

			QTextDocument emptyTextDocument;
			printDocument(pdfWriter.get(), &emptyTextDocument, &painter, tr("Schema: %1").arg(schemaId), nullptr, nullptr, 0);

			printSchema(pdfWriter.get(), &painter, schema, {}, {});
		}
	}

	emit finished(QString());

	return;
}

void SchemasReportGenerator::stop()
{
	m_stop = true;
}

void SchemasReportGenerator::progressRequested()
{
	QString progressText;

	int progress = 0;
	int progressMax = 0;

	QMutexLocker l(&m_statisticsMutex);

	SchemasReportGenerator::WorkerStatus status = currentStatus();

	switch (status)
	{
	case SchemasReportGenerator::WorkerStatus::Idle:
		{
			progressText = tr("Idle");
		}
		break;
	case SchemasReportGenerator::WorkerStatus::Loading:
		{
			if (m_currentSchemaType.isEmpty() == false)
			{
				progressText = tr("Loading schema: %1/%2").arg(currentSchemaType()).arg(currentSchemaId());
			}
			else
			{
				progressText = tr("Loading schema: %1").arg(currentSchemaId());
			}
			progress = schemaIndex();
			progressMax = schemasCount();
		}
		break;
	case SchemasReportGenerator::WorkerStatus::Parsing:
		{
			if (m_currentSchemaType.isEmpty() == false)
			{
				progressText = tr("Parsing schema: %1/%2").arg(currentSchemaType()).arg(currentSchemaId());
			}
			else
			{
				progressText = tr("Parsing schema: %1").arg(currentSchemaId());
			}
			progress = schemaIndex();
			progressMax = schemasCount();
		}
		break;
	case SchemasReportGenerator::WorkerStatus::Rendering:
		{
			if (m_currentSchemaType.isEmpty() == false)
			{
				progressText = tr("Rendering schema: %1/%2").arg(currentSchemaType()).arg(currentSchemaId());
			}
			else
			{
				progressText = tr("Rendering schema: %1").arg(currentSchemaId());
			}
			progress = schemaIndex();
			progressMax = schemasCount();
		}
		break;
	}

	l.unlock();

	emit progressChanged(progress, 0, progressMax, progressText);

	return;
}

SchemasReportGenerator::WorkerStatus SchemasReportGenerator::currentStatus() const
{
	return m_currentStatus;
}

int SchemasReportGenerator::schemasCount() const
{
	return m_schemasCount;
}

int SchemasReportGenerator::schemaIndex() const
{
	return m_schemaIndex;
}

QString SchemasReportGenerator::currentSchemaType() const
{
	return m_currentSchemaType;
}

QString SchemasReportGenerator::currentSchemaId() const
{
	return m_currentSchemaId;
}

DbController* SchemasReportGenerator::db()
{
	return &m_db;
}

const QString& SchemasReportGenerator::filePath() const
{
	return m_filePath;
}

void SchemasReportGenerator::openProject()
{
	if (db()->isProjectOpened() == true)
	{
		Q_ASSERT(false);
		throw(tr("Failed to open project - it is open!"));
	}

	db()->disableProgress();

	db()->setHost(m_serverIp);
	db()->setPort(m_serverPort);
	db()->setServerUsername(m_serverUserName);
	db()->setServerPassword(m_serverPassword);

	bool ok = db()->openProject(m_projectName, m_userName, m_userPassword, nullptr);
	if (ok == false)
	{
		throw(tr("Failed to open project!"));
	}

	return;
}

void SchemasReportGenerator::closeProject()
{
	if (db()->isProjectOpened() == false)
	{
		return;
	}

	db()->closeProject(nullptr);

	return;
}


void SchemasReportGenerator::loadSchemas(const std::vector<DbFileInfo>& files, std::map<QString, std::shared_ptr<VFrame30::Schema>>* schemas)
{
	if (schemas == nullptr)
	{
		Q_ASSERT(schemas);
		throw(tr("internal error: schemas is nullptr"));
	}

	schemas->clear();

	// Load schemas from files
	{
		QMutexLocker l(&m_statisticsMutex);
		m_currentStatus = WorkerStatus::Loading;

		m_schemasCount = static_cast<int>(files.size());
		m_schemaIndex = 0;
	}

	// Get files from the database

	std::vector<std::shared_ptr<DbFile>> out;

	for (const DbFileInfo& fi : files)
	{
		if (m_stop == true)
		{
			break;
		}

		std::shared_ptr<DbFile> f;

		bool ok = db()->getLatestVersion(fi, &f, nullptr);
		if (ok == false)
		{
			throw(tr("Failed to load file %1").arg(fi.fileName()));
		}

		{
			QMutexLocker l(&m_statisticsMutex);
			m_schemaIndex++;
			m_currentSchemaId = f->fileName();
		}

		out.push_back(f);
	}

	// Parse schemas

	{
		QMutexLocker l(&m_statisticsMutex);
		m_currentStatus = WorkerStatus::Parsing;
		m_schemaIndex = 0;
	}

	// Calculate if selected files have different parent
	//
	bool differentParentId = false;
	int firstParentId = -1;

	for (std::shared_ptr<DbFile> dbFile : out)
	{
		if (firstParentId == -1)
		{
			firstParentId = dbFile->parentId();
			continue;
		}

		if (firstParentId != dbFile->parentId())
		{
			differentParentId = true;
			break;
		}
	}

	// Load schemas from files
	//
	for (std::shared_ptr<DbFile> dbFile : out)
	{
		if (m_stop == true)
		{
			break;
		}

		std::shared_ptr<VFrame30::Schema> schema = VFrame30::Schema::Create(dbFile->data());
		if (schema == nullptr)
		{
			throw(tr("Failed to load schema from '%1'!").arg(dbFile->fileName()));
		}

		{
			QMutexLocker l(&m_statisticsMutex);
			m_schemaIndex++;
			m_currentSchemaId = schema->schemaId();
		}

		QString fileName = dbFile->fileName();

		if (differentParentId == true)
		{
			// Include full file path
			//
			int parentId = dbFile->parentId();
			while(true)
			{
				DbFileInfo parentFileInfo;

				bool result = db()->getFileInfo(parentId, &parentFileInfo, nullptr);
				if (result == false || parentFileInfo.parentId() == 0)
				{
					break;
				}

				fileName = parentFileInfo.fileName() + '/' + fileName;
				parentId = parentFileInfo.parentId();
			};
		}

		(*schemas)[fileName] = schema;
	}

	return;
}
