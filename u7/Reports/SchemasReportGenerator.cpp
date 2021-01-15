#include "SchemasReportGenerator.h"
#include "../VFrame30/Schema.h"
#include "../../lib/Ui/DialogProgress.h"
#include <QPageSetupDialog>
#include <QPrinter>

QPageSize SchemasReportGeneratorThread::m_albumPageSize = QPageSize(QPageSize::A3);
QPageLayout::Orientation SchemasReportGeneratorThread::m_albumOrientation = QPageLayout::Orientation::Landscape;
QMarginsF SchemasReportGeneratorThread::m_albumMargins = QMarginsF(0, 0, 0, 0);

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

void SchemasReportGeneratorThread::run(const std::vector<DbFileInfo>& files, bool album)
{
	// Ask for target file/folder

	QString filePath;

	if (album == true)
	{
		// Ask for page format

		QPrinter printer(QPrinter::HighResolution);

		QPageSize::PageSizeId id = QPageSize::id(m_albumPageSize.sizePoints(), QPageSize::FuzzyOrientationMatch);
		if (id == QPageSize::Custom)
		{
			id = QPageSize::A3;
		}

		printer.setFullPage(true);
		printer.setPageSize(QPageSize(id));
		printer.setPageOrientation(m_albumOrientation);
		printer.setPageMargins(m_albumMargins, QPageLayout::Unit::Millimeter);

		QPageSetupDialog d(&printer, m_parent);
		if (d.exec() != QDialog::Accepted)
		{
			return;
		}

		m_albumPageSize = QPageSize(d.printer()->pageLayout().pageSize());
		m_albumOrientation = d.printer()->pageLayout().orientation();
		m_albumMargins = d.printer()->pageLayout().margins();

		// Ask for file name

		filePath = QFileDialog::getSaveFileName(m_parent, qAppName(), "./",QObject::tr("PDF documents (*.pdf)"));
	}
	else
	{
		// Ask for file name

		filePath = QFileDialog::getExistingDirectory(m_parent, QObject::tr("Select Directory"), QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	}

	if (filePath.isNull() == true || filePath.isEmpty() == true)
	{
		return;
	}

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

	if (album == true)
	{
		worker->setPageSize(m_albumPageSize);
		worker->setPageMargins(m_albumMargins);
	}

	// Create Progress Dialog

	DialogProgress* dialogProgress = new DialogProgress(QObject::tr("Exporting Schemas to PDF"), 1, m_parent);

	// Create thread

	QThread* thread = new QThread;

	worker->moveToThread(thread);

	if (album == true)
	{
		QObject::connect(thread, &QThread::started, worker, &SchemasReportGenerator::createAlbum);
	}
	else
	{
		QObject::connect(thread, &QThread::started, worker, &SchemasReportGenerator::createPDFs);
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
	m_files(files)
{
	m_font = QFont("Times", 24);
	m_marginFont = QFont("Times", static_cast<int>(24 * (96.0 / resolution())));

	setPageMargins(QMarginsF(0, 0, 0, 0));

	return;
}

/*
SchemasReportGenerator::SchemasReportGenerator(ReportSchemaView* schemaView,
										 std::vector<std::shared_ptr<VFrame30::Schema>> schemas,
										 std::vector<std::shared_ptr<QBuffer>>* outputBuffers,
										 bool singleFile):
	ReportGenerator(schemaView),
	m_outputBuffers(outputBuffers),
	m_singleFile(singleFile)
{
	m_font = QFont("Times", 24);
	m_marginFont = QFont("Times", static_cast<int>(24 * (96.0 / resolution())));

	for (std::shared_ptr<VFrame30::Schema> schema : schemas)
	{
		m_schemas[schema->schemaId() + ".pdf"] = schema;
	}

	return;
}*/


SchemasReportGenerator::~SchemasReportGenerator()
{
	qDebug() << "SchemasReportWorker deleted";
}

void SchemasReportGenerator::createPDFs()
{
	if (m_schemas.empty() == true)
	{
		try
		{
			loadSchemas();
		}

		catch (QString errorMessage)
		{
			emit finished(errorMessage);
		}
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

		std::shared_ptr<QPdfWriter> pdfWriter;

		// Create separate PDF writer

		if (m_outputBuffers != nullptr)
		{
			std::shared_ptr<QBuffer> b = std::make_shared<QBuffer>();
			m_outputBuffers->push_back(b);

			pdfWriter = std::make_shared<QPdfWriter>(b.get());
		}
		else
		{
			QString fileName = it->first;

			fileName.replace('/', '_');

			int pos = fileName.lastIndexOf('.');
			if (pos != -1)
			{
				fileName = fileName.left(pos);
			}
			fileName += tr(".pdf");

			pdfWriter = std::make_shared<QPdfWriter>(filePath() + "//" + fileName);
		}

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

void SchemasReportGenerator::createAlbum()
{
	if (m_schemas.empty() == true)
	{
		try
		{
			loadSchemas();
		}

		catch (QString errorMessage)
		{
			emit finished(errorMessage);
		}
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

	std::shared_ptr<QPdfWriter> pdfWriter;

	// Create single PDF writer

	if (m_outputBuffers != nullptr)
	{
		std::shared_ptr<QBuffer> b = std::make_shared<QBuffer>();
		m_outputBuffers->push_back(b);

		pdfWriter = std::make_shared<QPdfWriter>(b.get());
	}
	else
	{
		pdfWriter = std::make_shared<QPdfWriter>(filePath());
	}

	pdfWriter->setTitle(m_projectName);
	pdfWriter->setPageSize(pageSize());
	pdfWriter->setPageMargins(pageMargins(), QPageLayout::Unit::Millimeter);
	pdfWriter->setResolution(resolution());

	bool firstSchema = true;

	QPainter painter(pdfWriter.get());

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

		if (firstSchema == true)
		{
			firstSchema = false;
		}
		else
		{
			pdfWriter->newPage();
		}

		printSchema(pdfWriter.get(), &painter, schema, {}, {});
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

void SchemasReportGenerator::loadSchemas()
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
		throw(tr("Failed to open project!"));
	}

	// Get files from the database

	std::vector<std::shared_ptr<DbFile>> out;

	for (const DbFileInfo& fi : m_files)
	{
		std::shared_ptr<DbFile> f;

		ok = db()->getLatestVersion(fi, &f, nullptr);
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

		m_schemas[fileName] = schema;
	}

	db()->closeProject(nullptr);

	return;
}
