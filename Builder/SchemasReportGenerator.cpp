#include "SchemasReportGenerator.h"
#include "../ReportLib/ReportPrinter.h"

namespace Builder
{
	using namespace ReportLib;

	//
	// SchemasReportGenerator
	//

	SchemasReportGenerator::SchemasReportGenerator(std::shared_ptr<ReportSchemaView> schemaView,
													 const AppSignalSet *signalSet,
													 const QString& serverIp,
													 int serverPort,
													 const QString& serverUserName,
													 const QString& serverPassword,
													 const QString& projectName,
													 const QString& userName,
													 const QString& userPassword,
													 std::vector<DbFileInfo> files,
													 const QString& filePath):
		m_schemaView(schemaView),
		m_appSignalProvider(signalSet),
		m_appSignalController(&m_appSignalProvider, nullptr),
		m_inputFiles(files),
		m_filePath(filePath),
		m_serverIp(serverIp),
		m_serverPort(serverPort),
		m_serverUserName(serverUserName),
		m_serverPassword(serverPassword),
		m_projectName(projectName),
		m_userName(userName),
		m_userPassword(userPassword),
		m_marginFont{"Arial", 8, QFont::Normal}
	{
		return;
	}

	SchemasReportGenerator::~SchemasReportGenerator()
	{
		qDebug() << "SchemasReportWorker deleted";
	}

	void SchemasReportGenerator::setPageLayout(const QPageLayout& pageLayout)
	{
		m_pageLayout = pageLayout;
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

		result.push_back({db->systemFileId(DbDir::MonitorSchemasDir),
						  QObject::tr("Monitor Schemas"),
						  true,
						  QPageLayout(QPageSize(QPageSize::A3),
						  QPageLayout::Orientation::Landscape,
						  QMarginsF(30, 20, 15, 20),
						  QPageLayout::Unit::Millimeter)});

		result.push_back({db->systemFileId(DbDir::TuningSchemasDir),
						  QObject::tr("Tuning Schemas"),
						  true, QPageLayout(QPageSize(QPageSize::A3),
						  QPageLayout::Orientation::Landscape,
						  QMarginsF(30, 20, 15, 20),
						  QPageLayout::Unit::Millimeter)});

		result.push_back({db->systemFileId(DbDir::DiagnosticsSchemasDir),
						  QObject::tr("Diagnostics Schemas"),
						  true, QPageLayout(QPageSize(QPageSize::A3),
						  QPageLayout::Orientation::Landscape,
						  QMarginsF(30, 20, 15, 20),
						  QPageLayout::Unit::Millimeter)});

		result.push_back({db->systemFileId(DbDir::AppLogicDir),
						  QObject::tr("Application Logic"),
						  true,
						  QPageLayout(QPageSize(QPageSize::A3),
						  QPageLayout::Orientation::Landscape,
						  QMarginsF(30, 20, 15, 20),
						  QPageLayout::Unit::Millimeter)});

		result.push_back({db->systemFileId(DbDir::UfblDir),
						  QObject::tr("UFBL Descriptions"),
						  true,
						  QPageLayout(QPageSize(QPageSize::A3),
						  QPageLayout::Orientation::Landscape,
						  QMarginsF(30, 20, 15, 20),
						  QPageLayout::Unit::Millimeter)});

		return result;
	}

	void SchemasReportGenerator::exportFilesToPdf()
	{
		std::map<QString, std::shared_ptr<VFrame30::Schema>> schemas;	// Key is full path to schema file

		try
		{
			openProject();

			loadSchemas(m_inputFiles, schemas);

			closeProject();
		}
		catch (QString errorMessage)
		{
			closeProject();

			emit finished(errorMessage);
		}

		{
			QMutexLocker l(&m_statisticsMutex);
			m_statistics.m_currentStatus = WorkerStatus::Rendering;
			m_statistics.m_schemaIndex = 0;
		}

		// Save schemas to PDF
		//
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
				m_statistics.m_schemaIndex++;
				m_statistics.m_currentSchemaId = schemaId;
			}

			QString fileName = it->first;

			fileName.replace('/', '_');

			qsizetype pos = fileName.lastIndexOf('.');
			if (pos != -1)
			{
				fileName = fileName.left(pos);
			}
			fileName += tr(".pdf");

			std::shared_ptr<Report> report = std::make_shared<Report>(m_projectName, fileName);
			report->setPageLayout(m_pageLayout);

			{
				auto reportSchema = ReportSchema::create(tr("Schema: %1").arg(schema->schemaId()), {}, schema, {});

				auto schemaDrawingSection = report->addSection(ReportSection::create(tr("Schema: %1").arg(schema->schemaId())));
				schemaDrawingSection->addSchema(reportSchema);
			}

			ReportPrinter printer(m_schemaView);

			if (filePath().isEmpty() == false)
			{
				// Print to file
				//
				printer.print(*report, filePath() + fileName, m_stop);
			}
			else
			{
				// Print to buffer
				//
				QBuffer buffer(&m_outputData[fileName]);
				printer.print(*report, buffer, m_stop);
			}

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

			loadSchemas(m_inputFiles, schemas);

			closeProject();
		}

		catch (QString errorMessage)
		{
			closeProject();

			emit finished(errorMessage);
		}

		{
			QMutexLocker l(&m_statisticsMutex);
			m_statistics.m_currentStatus = WorkerStatus::Rendering;
			m_statistics.m_schemaIndex = 0;
		}

		std::shared_ptr<Report> report = std::make_shared<Report>(m_projectName, filePath());
		report->setPageLayout(m_pageLayout);

		// Init margins
		//
		report->addMarginItem({tr("Project: %1").arg(m_projectName), -1, -1, {m_marginFont, Qt::AlignLeft | Qt::AlignTop}});

		report->addMarginItem({tr("%OBJECT%"), -1, -1, {m_marginFont, Qt::AlignRight | Qt::AlignTop}});

		report->addMarginItem({tr("%PAGE%"), -1, -1, {m_marginFont, Qt::AlignRight | Qt::AlignBottom}});

		{
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
					m_statistics.m_schemaIndex++;
					m_statistics.m_currentSchemaId = schemaId;
				}

				auto reportSchema = ReportSchema::create(tr("Schema: %1").arg(schemaId), {}, schema, {});

				auto schemaDrawingSection = report->addSection(ReportSection::create(tr("Schema: %1").arg(schemaId)));
				schemaDrawingSection->addSchema(reportSchema);
			}
		}

		ReportPrinter printer(m_schemaView);

		if (filePath().isEmpty() == false)
		{
			// Print to file
			//
			printer.print(*report, filePath(), m_stop);
		}
		else
		{
			// Print to buffer
			//
			QBuffer buffer(&m_outputData[filePath()]);
			printer.print(*report, buffer, m_stop);
		}

		emit finished(QString());

		return;
	}

	void SchemasReportGenerator::exportAllSchemasToAlbums()
	{
		std::vector<SchemaFilesGroup> schemaFilesGroups;

		try
		{
			openProject();

			schemaFilesGroups.push_back({db()->systemFileId(DbDir::AppLogicDir), tr("ApplicationLogic")});
			schemaFilesGroups.push_back({db()->systemFileId(DbDir::MonitorSchemasDir), tr("MonitorSchemas")});
			schemaFilesGroups.push_back({db()->systemFileId(DbDir::TuningSchemasDir), tr("TuningSchemas")});
			schemaFilesGroups.push_back({db()->systemFileId(DbDir::DiagnosticsSchemasDir), tr("DiagnosticsSchemas")});
			schemaFilesGroups.push_back({db()->systemFileId(DbDir::UfblDir), tr("UFBSchemas")});

			for (SchemaFilesGroup& sfg : schemaFilesGroups)
			{
				if (m_stop == true)
				{
					break;
				}

				// Fill schemas files
				//
				DbFileTree fileTree;

				{
					QMutexLocker l(&m_statisticsMutex);
					m_statistics.m_currentSchemaType = sfg.caption;
				}

				bool ok = db()->getFileListTree(&fileTree, sfg.fileId, true/*removeDeleted*/, nullptr);
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
							fi->fileName().endsWith("." + QString(Db::File::TvsFileExtension)) == false &&
							fi->fileName().endsWith("." + QString(Db::File::DvsFileExtension)) == false)
					{
						continue;
					}

					sfg.schemasFiles.push_back(*fi);
				}

				// Load and parse schemas
				//
				loadSchemas(sfg.schemasFiles, sfg.schemas);

				if (m_stop == true)
				{
					break;
				}

				if (sfg.schemas.empty() == true)
				{
					continue;
				}

				// Render schemas
				//
				renderSchemas(sfg);


				// Clear loaded schemas after parsing
				//
				clearSchemas(sfg);
			}

			closeProject();
		}

		catch (QString errorMessage)
		{
			closeProject();

			emit finished(errorMessage);
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
		int progressMin = 0;
		int progressMax = 0;

		getProgress(&progress, &progressMin, &progressMax, &progressText);

		emit progressChanged(progress, 0, progressMax, progressText);

		return;
	}

	void SchemasReportGenerator::getProgress(int* progress, int* progressMin, int* progressMax, QString* progressText)
	{
		if (progress == nullptr || progressMin == nullptr || progressMax == nullptr || progressText == nullptr)
		{
			Q_ASSERT(progress);
			Q_ASSERT(progressMin);
			Q_ASSERT(progressMax);
			Q_ASSERT(progressText);
			return;
		}

		SchemasReportGenerator::Statistics stats = statistics();

		*progressMin = 0;

		switch (stats.m_currentStatus)
		{
		case SchemasReportGenerator::WorkerStatus::Idle:
			{
				*progressText = tr("Idle");
			}
			break;
		case SchemasReportGenerator::WorkerStatus::Loading:
			{
				if (stats.m_currentSchemaType.isEmpty() == false)
				{
					*progressText = tr("Loading schema: %1/%2")
							.arg(stats.m_currentSchemaType)
							.arg(stats.m_currentSchemaId);
				}
				else
				{
					*progressText = tr("Loading schema: %1").arg(stats.m_currentSchemaId);
				}
				*progress = stats.m_schemaIndex;
				*progressMax = stats.m_schemasCount;
			}
			break;
		case SchemasReportGenerator::WorkerStatus::Parsing:
			{
				if (stats.m_currentSchemaType.isEmpty() == false)
				{
					*progressText = tr("Parsing schema: %1/%2")
							.arg(stats.m_currentSchemaType)
							.arg(stats.m_currentSchemaId);
				}
				else
				{
					*progressText = tr("Parsing schema: %1")
							.arg(stats.m_currentSchemaId);
				}
				*progress = stats.m_schemaIndex;
				*progressMax = stats.m_schemasCount;
			}
			break;
		case SchemasReportGenerator::WorkerStatus::Rendering:
			{
				if (stats.m_currentSchemaType.isEmpty() == false)
				{
					*progressText = tr("Rendering schema: %1/%2")
							.arg(stats.m_currentSchemaType)
							.arg(stats.m_currentSchemaId);
				}
				else
				{
					*progressText = tr("Rendering schema: %1").arg(stats.m_currentSchemaId);
				}
				*progress = stats.m_schemaIndex;
				*progressMax = stats.m_schemasCount;
			}
			break;
		}
	}

	QStringList SchemasReportGenerator::outputFilesList() const
	{
		QStringList result;
		for (const auto& it : m_outputData)
		{
			result.push_back(it.first);
		}
		return result;
	}

	const QByteArray& SchemasReportGenerator::outputData(const QString& fileName)
	{
		auto it = m_outputData.find(fileName);
		if (it == m_outputData.end())
		{
			Q_ASSERT(false);
			static QByteArray e;
			return e;
		}

		return it->second;
	}

	SchemasReportGenerator::Statistics SchemasReportGenerator::statistics() const
	{
		QMutexLocker l(&m_statisticsMutex);
		return m_statistics;
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


	void SchemasReportGenerator::loadSchemas(const std::vector<DbFileInfo>& files, std::map<QString, std::shared_ptr<VFrame30::Schema> >& schemas)
	{
		schemas.clear();

		// Load schemas from files
		{
			QMutexLocker l(&m_statisticsMutex);
			m_statistics.m_currentStatus = WorkerStatus::Loading;

			m_statistics.m_schemasCount = static_cast<int>(files.size());
			m_statistics.m_schemaIndex = 0;
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
				m_statistics.m_schemaIndex++;
				m_statistics.m_currentSchemaId = f->fileName();
			}

			out.push_back(f);
		}

		// Parse schemas

		{
			QMutexLocker l(&m_statisticsMutex);
			m_statistics.m_currentStatus = WorkerStatus::Parsing;
			m_statistics.m_schemaIndex = 0;
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
		auto context = VFrame30::Context::create(&m_appSignalController, nullptr, nullptr, nullptr);

		for (const std::shared_ptr<DbFile>& dbFile : out)
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

			schema->setContext(context);

			{
				QMutexLocker l(&m_statisticsMutex);
				m_statistics.m_schemaIndex++;
				m_statistics.m_currentSchemaId = schema->schemaId();
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

			schemas[fileName] = schema;
		}

		return;
	}

	void SchemasReportGenerator::renderSchemas(const SchemaFilesGroup& sfg)
	{
		// Render schemas
		//
		{
			QMutexLocker l(&m_statisticsMutex);
			m_statistics.m_currentStatus = WorkerStatus::Rendering;
			m_statistics.m_currentSchemaType = sfg.caption;
			m_statistics.m_schemaIndex = 0;
			m_statistics.m_schemasCount = static_cast<int>(sfg.schemas.size());
		}

		std::shared_ptr<Report> report = std::make_shared<Report>(m_projectName, filePath());
		report->setPageLayout(m_pageLayout);

		// Init margins
		report->addMarginItem({tr("Project: %1").arg(m_projectName), -1, -1, {m_marginFont, Qt::AlignLeft | Qt::AlignTop}});

		report->addMarginItem({tr("%OBJECT%"), -1, -1, {m_marginFont, Qt::AlignRight | Qt::AlignTop}});

		{
			// Find page layout

			QPageLayout pl = report->pageLayout();

			bool plFound = false;

			for (const ReportFileTypeParams& rp : m_reportFileTypeParams)
			{
				if (rp.fileId() == sfg.fileId)
				{
					pl = rp.pageLayout();

					plFound = true;
					break;
				}
			}

			if (plFound == false)
			{
				// File type was not found
				Q_ASSERT(false);
			}

			// Render schemas

			for (auto it = sfg.schemas.begin(); it != sfg.schemas.end(); it++)
			{
				if (m_stop == true)
				{
					break;
				}

				const std::shared_ptr<VFrame30::Schema> schema = it->second;
				const QString& schemaId = schema->schemaId();

				{
					QMutexLocker l(&m_statisticsMutex);
					m_statistics.m_schemaIndex++;
					m_statistics.m_currentSchemaId = schemaId;
				}

				auto reportSchema = ReportSchema::create(tr("Schema: %1").arg(schemaId), {}, schema, {});

				auto schemaDrawingSection = report->addSection(ReportSection::create(tr("Schema: %1").arg(schemaId)));
				schemaDrawingSection->addSchema(reportSchema);
			}
		}

		ReportPrinter printer(m_schemaView);

		if (filePath().isEmpty() == false)
		{
			QString fileName = tr("%1/%2_%3.pdf").arg(filePath()).arg(m_projectName).arg(sfg.caption);
			// Print to file
			//
			printer.print(*report, fileName, m_stop);
		}
		else
		{
			// Print to buffer
			//
			QBuffer buffer(&m_outputData[sfg.caption + ".pdf"]);
			printer.print(*report, buffer, m_stop);

		}
	}

	void SchemasReportGenerator::clearSchemas(SchemaFilesGroup& sfg)
	{
		sfg.schemas.clear();

	}
}
