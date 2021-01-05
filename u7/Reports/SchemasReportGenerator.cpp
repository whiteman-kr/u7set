#include "SchemasReportGenerator.h"
#include "../VFrame30/Schema.h"
#include "../../lib/Ui/DialogProgress.h"

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

void SchemasReportGeneratorThread::run(const std::vector<std::shared_ptr<DbFileInfo>>& files, const QString& filePath, bool singleFile)
{
	// Create View

	ReportSchemaView* schemaView = new ReportSchemaView(m_parent);

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
																 filePath,
																 singleFile);

	// Create Progress Dialog

	DialogProgress* dialogProgress = new DialogProgress(QObject::tr("Exporting Schemas to PDF"), 1, m_parent);

	// Create thread

	QThread* thread = new QThread;

	worker->moveToThread(thread);

	QObject::connect(thread, &QThread::started, worker, &SchemasReportGenerator::process);

	QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);	// Schedule thread deleting

	QObject::connect(dialogProgress, &DialogProgress::getProgress, worker, &SchemasReportGenerator::progressRequested, Qt::DirectConnection);

	QObject::connect(dialogProgress, &DialogProgress::cancelClicked, worker, &SchemasReportGenerator::stop, Qt::DirectConnection);

	QObject::connect(worker, &SchemasReportGenerator::progressChanged, dialogProgress, &DialogProgress::setProgressSingle);

	//  Schedule objects deleting

	QObject::connect(worker, &SchemasReportGenerator::finished, [thread, dialogProgress, worker, schemaView]()
	{
		thread->quit();

		dialogProgress->deleteLater();

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
										 std::vector<std::shared_ptr<DbFileInfo>> files,
										 const QString& filePath,
										 bool singleFile):
	ReportGenerator(schemaView),
	m_serverIp(serverIp),
	m_serverPort(serverPort),
	m_serverUserName(serverUserName),
	m_serverPassword(serverPassword),
	m_projectName(projectName),
	m_userName(userName),
	m_userPassword(userPassword),
	m_filePath(filePath),
	m_singleFile(singleFile)
{
	for (std::shared_ptr<DbFileInfo> file : files)
	{
		m_files.push_back(*file);
	}

	return;
}

SchemasReportGenerator::SchemasReportGenerator(ReportSchemaView* schemaView,
										 std::vector<std::shared_ptr<VFrame30::Schema>> schemas,
										 std::vector<std::shared_ptr<QBuffer>>* outputBuffers,
										 bool singleFile):
	ReportGenerator(schemaView),
	m_outputBuffers(outputBuffers),
	m_singleFile(singleFile)
{
	for (std::shared_ptr<VFrame30::Schema> schema : schemas)
	{
		m_schemas[schema->schemaId() + ".pdf"] = schema;
	}

	return;
}

SchemasReportGenerator::~SchemasReportGenerator()
{
	qDebug() << "SchemasReportWorker deleted";
}

void SchemasReportGenerator::process()
{
	createSchemasReport();

	emit finished();

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
			progressText = tr("Loading schema: %1").arg(currentSchemaId());
			progress = schemaIndex();
			progressMax = schemasCount();
		}
		break;
	case SchemasReportGenerator::WorkerStatus::Parsing:
		{
			progressText = tr("Parsing schema: %1").arg(currentSchemaId());
			progress = schemaIndex();
			progressMax = schemasCount();
		}
		break;
	case SchemasReportGenerator::WorkerStatus::Rendering:
		{
			progressText = tr("Rendering schema: %1").arg(currentSchemaId());
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

void SchemasReportGenerator::createSchemasReport()
{
	m_font = QFont("Times", 24);

	if (m_schemas.empty() == true)
	{
		// Load schemas from files
		{
			QMutexLocker l(&m_statisticsMutex);
			m_currentStatus = WorkerStatus::Loading;

			m_schemasCount = static_cast<int>(m_files.size());
			m_schemaIndex = 0;
		}

		db()->disableProgress();

		db()->setHost(m_serverIp);
		db()->setPort(m_serverPort);
		db()->setServerUsername(m_serverUserName);
		db()->setServerPassword(m_serverPassword);

		bool ok = db()->openProject(m_projectName, m_userName, m_userPassword, nullptr);
		if (ok == false)
		{
			int todo_error_message = 1;
			qDebug() << "Failed to open project!";
			Q_ASSERT(ok);
			return;
		}

		// Get files from the database

		std::vector<std::shared_ptr<DbFile>> out;

		for (const DbFileInfo& fi : m_files)
		{
			std::shared_ptr<DbFile> f;

			ok = db()->getLatestVersion(fi, &f, nullptr);
			if (ok == false)
			{
				int todo_error_message = 1;
				qDebug() << "Failed to load file " << fi.fileName();
				Q_ASSERT(ok);
				continue;
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

		for (std::shared_ptr<DbFile> dbFile : out)
		{
			std::shared_ptr<VFrame30::Schema> schema = VFrame30::Schema::Create(dbFile->data());
			if (schema == nullptr)
			{
				int todo_error_message = 1;
				qDebug() << "Failed to load schema!";
				Q_ASSERT(false);
				return;
			}

			{
				QMutexLocker l(&m_statisticsMutex);
				m_schemaIndex++;
				m_currentSchemaId = schema->schemaId();
			}

			m_schemas[dbFile->fileName()] = schema;
		}

		db()->closeProject(nullptr);
	}
	else
	{
		QMutexLocker l(&m_statisticsMutex);
		m_schemasCount = static_cast<int>(m_schemas.size());
	}

	{
		QMutexLocker l(&m_statisticsMutex);
		m_currentStatus = WorkerStatus::Rendering;

		m_schemaIndex = 0;
	}

	// Save schemas to PDF

	QPdfWriter* pdfWriter = nullptr;

	QPainter* painter = nullptr;

	if (m_singleFile == true)
	{
		// Create single PDF writer

		if (m_outputBuffers != nullptr)
		{
			std::shared_ptr<QBuffer> b = std::make_shared<QBuffer>();

			m_outputBuffers->push_back(b);

			pdfWriter = new QPdfWriter(b.get());
		}
		else
		{
			pdfWriter = new QPdfWriter(filePath());
		}

		pdfWriter->setPageSize(QPageSize(QPageSize::A4));
		pdfWriter->setPageOrientation(QPageLayout::Portrait);
		pdfWriter->setPageMargins(QMargins(20, 15, 10, 15), QPageLayout::Unit::Millimeter);
		pdfWriter->setResolution(resolution());

		painter = new QPainter(pdfWriter);
	}

	bool firstSchema = true;

	for (auto it = m_schemas.begin(); it != m_schemas.end(); it++)
	{
		if (m_stop == true)
		{
			return;
		}

		const std::shared_ptr<VFrame30::Schema> schema = it->second;

		const QString& schemaId = schema->schemaId();

		{
			QMutexLocker l(&m_statisticsMutex);
			m_schemaIndex++;
			m_currentSchemaId = schemaId;
		}

		if (m_singleFile == false)
		{
			// Create separate PDF writer

			if (m_outputBuffers != nullptr)
			{
				std::shared_ptr<QBuffer> b = std::make_shared<QBuffer>();

				m_outputBuffers->push_back(b);

				pdfWriter = new QPdfWriter(b.get());
			}
			else
			{
				int todo_what_if_same_filenames_in_different_folders = 1;

				QString fileName = it->first;

				int pos = fileName.lastIndexOf('.');
				if (pos != -1)
				{
					fileName = fileName.left(pos);
				}
				fileName += tr(".pdf");

				pdfWriter = new QPdfWriter(filePath() + "//" + fileName);
			}


			pdfWriter->setPageSize(QPageSize(QPageSize::A4));
			pdfWriter->setPageOrientation(QPageLayout::Portrait);
			pdfWriter->setPageMargins(QMargins(20, 15, 10, 15), QPageLayout::Unit::Millimeter);
			pdfWriter->setResolution(resolution());

			painter = new QPainter(pdfWriter);
		}

		if (pdfWriter == nullptr || painter == nullptr)
		{
			Q_ASSERT(pdfWriter);
			Q_ASSERT(painter);
			return;
		}

		// Add schema drawing

		std::shared_ptr<ReportSection> section = std::make_shared<ReportSection>(schema->schemaId());

		section->addText(tr("Schema: %1\n").arg(schemaId), currentCharFormat(), currentBlockFormat());
		section->setSchema(schema);

		QRect pageRectPixels = pdfWriter->pageLayout().paintRectPixels(pdfWriter->resolution());

		section->render(QSizeF(pageRectPixels.width(), pageRectPixels.height()));

		// Print Text Document

		if (m_singleFile == true)
		{
			if (firstSchema == true)
			{
				firstSchema = false;
			}
			else
			{
				pdfWriter->newPage();
			}
		}

		printDocument(pdfWriter, section->textDocument(), painter, nullptr, nullptr, 0);

		// Print Schemas

		printSchema(pdfWriter, section->textDocument(), painter, section->schema(), section->compareItemActions());

		if (m_singleFile == false)
		{
			// Delete separate PDF writer

			delete painter;
			delete pdfWriter;
		}
	}

	if (m_singleFile == true)
	{
		// Delete single PDF writer

		delete painter;
		delete pdfWriter;
	}

	return;
}
