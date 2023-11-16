#include "SchemasReportGenerator.h"
#include "../VFrame30/LogicSchema.h"

namespace Builder
{
	using namespace ReportLib;

	//
	// SchemasReportOptions
	//
	bool SchemasReportOptions::load(DbController* db)
	{
		if (db == nullptr)
		{
			Q_ASSERT(db);
			return false;
		}

		QString value;
		db->getUserProperty("SchemasReportOptions.footers", &value, "false", nullptr);
		footers = (value == "true") ? true : false;

		db->getUserProperty("SchemasReportOptions.itemsLabels", &value, "false", nullptr);
		itemsLabels = (value == "true") ? true : false;

		db->getUserProperty("SchemasReportOptions.signalsDetails", &value, "false", nullptr);
		signalsDetails = (value == "true") ? true : false;

		db->getUserProperty("SchemasReportOptions.schemaTags", &value, "false", nullptr);
		QStringList tagsStrings = value.split(";", Qt::SkipEmptyParts);
		schemaTags.clear();
		for (const QString& tagsString : tagsStrings)
		{
			QStringList ts = tagsString.split("=");
			if (ts.size() != 2)
			{
				Q_ASSERT(false);
				continue;
			}
			schemaTags[ts[0]] = ts[1] == "1" ? true : false;
		}


		return true;
	}

	bool SchemasReportOptions::save(DbController* db)
	{
		if (db == nullptr)
		{
			Q_ASSERT(db);
			return false;
		}

		db->setUserProperty("SchemasReportOptions.footers", footers ? "true" : "false", nullptr);
		db->setUserProperty("SchemasReportOptions.itemsLabels", itemsLabels ? "true" : "false", nullptr);
		db->setUserProperty("SchemasReportOptions.signalsDetails", signalsDetails ? "true" : "false", nullptr);

		QString tagsString;
		for (const auto& it : schemaTags)
		{
			tagsString.append(it.first + "=");
			tagsString.append(it.second == true ? "1;" : "0;");
		}
		db->setUserProperty("SchemasReportOptions.schemaTags", tagsString, nullptr);

		return true;
	}

	void SchemasReportOptions::setTags(const std::set<QString>& tagsSet)
	{
		for (const QString& tag : tagsSet)
		{
			if (schemaTags.find(tag) == schemaTags.end())
			{
				schemaTags[tag] = true;
			}
		}
	}

	//
	// SchemasReportFileTypeParams
	//
	SchemaTypesParams::SchemaTypesParams(int fileId, const QString& caption, bool selected, QPageLayout pageLayout):
		m_fileId(fileId),
		m_caption(caption),
		m_selected(selected),
		m_pageLayout(pageLayout)
	{
	}

	int SchemaTypesParams::fileId() const
	{
		return m_fileId;
	}

	const QString& SchemaTypesParams::caption() const
	{
		return m_caption;
	}

	bool SchemaTypesParams::selected() const
	{
		return m_selected;
	}

	void SchemaTypesParams::setSelected(bool value)
	{
		m_selected = value;
	}

	const QPageLayout& SchemaTypesParams::pageLayout() const
	{
		return m_pageLayout;
	}

	void SchemaTypesParams::setPageLayout(const QPageLayout& layout)
	{
		m_pageLayout = layout;
	}

	//
	// SchemaSignalInfo
	//

	SchemaSignalInfo::SchemaSignalInfo(const VFrame30::FblItemRect* item, const QString& appSignalId, const QStringList& otherSchemasIds, IAppSignalManager& appSignals)
	{
		input = item->isInputSignalElement();
		received = item->isReceiverElement();
		commented = item->commented();
		x = item->left();
		y = item->top();
		signalId = appSignalId;

		bool found = false;
		AppSignalParam asp = appSignals.signalParam(signalId, &found);

		if (found == true)
		{
			caption = asp.caption();
		}
		else
		{
			caption = QObject::tr("<font color=\"red\">%1</font>").arg(signalId);
		}

		if (item->isSignalElement() == true)
		{
			impact = item->toSignalElement()->impactAppSignalIdList().contains(signalId);
		}

		if (otherSchemasIds.empty() == false)
		{
			schemasList = otherSchemasIds.join("<br>");
		}
		else
		{
			if (received == true)
			{
				schemasList = QObject::tr("This Schema Only");
			}
			else
			{
				schemasList = input ? QObject::tr("Start Point") : QObject::tr("End Point");
			}
		}

		if (item->textColor() != Qt::black)
		{
			color = item->textColor().name();
		}
	}

	QStringList SchemaSignalInfo::toStringList() const
	{
		QStringList l;

		if (color.isEmpty() == false)
		{
			l.push_back(QObject::tr("<font color=\"%1\">%2</font>").arg(color).arg(signalId));
		}
		else
		{
			l.push_back(signalId);
		}

		l.push_back(caption);

		QString typeStr;
		if (received == true)
		{
			typeStr = QObject::tr("Received");
		}
		else
		{
			typeStr = input ? QObject::tr("Input") : QObject::tr("Output");
		}
		if (impact == true)
		{
			typeStr += QObject::tr(", Impact");
		}
		if (commented == true)
		{
			typeStr += QObject::tr(", Commented");
		}
		l.push_back(typeStr);

		l.push_back(schemasList);

		return l;
	}

	bool SchemaSignalInfo::less(const SchemaSignalInfo& a, const SchemaSignalInfo& b)
	{
		if (a.input != b.input)
		{
			return a.input > b.input;
		}
		if (a.x != b.x)
		{
			return a.x < b.x;
		}
		if (a.y != b.y)
		{
			return a.y < b.y;
		}
		return a.signalId < b.signalId;
	}

	//
	// SchemaLoopbackInfo
	//
	SchemaLoopbackInfo::SchemaLoopbackInfo(const VFrame30::SchemaItemLoopback* loopbackItem, const QStringList& otherSchemasIds)
	{
		source = loopbackItem->isLoopbackSourceElement();
		commented = loopbackItem->commented();
		x = loopbackItem->left();
		y = loopbackItem->top();

		loopbackId = loopbackItem->loopbackId();

		if (otherSchemasIds.empty() == false)
		{
			schemasList = otherSchemasIds.join("<br>");
		}
		else
		{
			schemasList = QObject::tr("This Schema Only");
		}

		if (loopbackItem->textColor() != Qt::black)
		{
			color = loopbackItem->textColor().name();
		}
	}

	QStringList SchemaLoopbackInfo::toStringList() const
	{
		QStringList l;

		if (color.isEmpty() == false)
		{
			l.push_back(QObject::tr("<font color=\"%1\">%2</font>").arg(color).arg(loopbackId));
		}
		else
		{
			l.push_back(loopbackId);
		}

		QString typeStr = source ? QObject::tr("Source") : QObject::tr("Target");
		if (commented == true)
		{
			typeStr += QObject::tr(", Commented");
		}
		l.push_back(typeStr);

		l.push_back(schemasList);
		return l;
	}

	bool SchemaLoopbackInfo::less(const SchemaLoopbackInfo& a, const SchemaLoopbackInfo& b)
	{
		if (a.source != b.source)
		{
			return a.source > b.source;
		}
		if (a.x != b.x)
		{
			return a.x < b.x;
		}
		if (a.y != b.y)
		{
			return a.y < b.y;
		}
		return a.loopbackId < b.loopbackId;
	}

	//
	// SchemaConnectionInfo
	//
	SchemaConnectionInfo::SchemaConnectionInfo(const VFrame30::SchemaItemConnection* connectionItem, const QString& connectionId, const QStringList& otherSchemasIds)
	{
		transmitter = connectionItem->isTransmitterElement();
		commented = connectionItem->commented();
		x = connectionItem->left();
		y = connectionItem->top();

		this->connectionId = connectionId;

		if (otherSchemasIds.empty() == false)
		{
			schemasList = otherSchemasIds.join("<br>");
		}
		else
		{
			schemasList = QObject::tr("This Schema Only");
		}

		if (connectionItem->textColor() != Qt::black)
		{
			color = connectionItem->textColor().name();
		}
	}

	QStringList SchemaConnectionInfo::toStringList() const
	{
		QStringList l;

		if (color.isEmpty() == false)
		{
			l.push_back(QObject::tr("<font color=\"%1\">%2</font>").arg(color).arg(connectionId));
		}
		else
		{
			l.push_back(connectionId);
		}

		QString typeStr = transmitter ? QObject::tr("Transmitter") : QObject::tr("Receiver");
		if (commented == true)
		{
			typeStr += QObject::tr(", Commented");
		}
		l.push_back(typeStr);

		l.push_back(schemasList);
		return l;
	}

	bool SchemaConnectionInfo::less(const SchemaConnectionInfo& a, const SchemaConnectionInfo& b)
	{
		if (a.transmitter != b.transmitter)
		{
			return a.transmitter > b.transmitter;
		}
		if (a.x != b.x)
		{
			return a.x < b.x;
		}
		if (a.y != b.y)
		{
			return a.y < b.y;
		}
		return a.connectionId < b.connectionId;
	}

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
												   const QString& filePath,
												   const SchemasReportOptions& options,
												   const std::vector<SchemaTypesParams>& schemaTypesParams):
		m_schemaView(schemaView),
		m_printer(schemaView),
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
		m_normalFont{"Arial", 9, QFont::Normal},
		m_marginFont{"Arial", 8, QFont::Normal},
		m_tableFont{"Arial", 9, QFont::Normal},
		m_options(options),
		m_schemaTypesParams(schemaTypesParams)
	{
		return;
	}

	SchemasReportGenerator::~SchemasReportGenerator()
	{
		qDebug() << "SchemasReportWorker deleted";
	}

	std::vector<SchemaTypesParams> SchemasReportGenerator::defaultFileTypesParams(DbController* db)
	{
		std::vector<SchemaTypesParams> result;

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

	void SchemasReportGenerator::exportFilesToMultiplePdf()
	{
		if (filePath().isEmpty() == true)
		{
			Q_ASSERT(false);
			return;
		}

		VFrame30::SchemaDetailsSet detailsSet;
		std::map<QString, std::shared_ptr<VFrame30::Schema>> schemas;		// Key is schema ID - schemas to be exported

		try
		{
			openProject();

			loadSchemas(m_inputFiles, schemas, detailsSet);

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

			std::shared_ptr<Report> report = std::make_shared<Report>(m_projectName,
																	  filePath() + QDir::separator() + schemaId + ".pdf");

			if (m_options.footers == true)
			{
				report->addMarginItem({tr("Project: %1").arg(m_projectName), -1, -1, {m_marginFont, Qt::AlignLeft | Qt::AlignTop}});
				report->addMarginItem({tr("%PAGE%"), -1, -1, {m_marginFont, Qt::AlignRight | Qt::AlignBottom}});
				report->addMarginItem({"%TAG%", -1, -1, {m_marginFont, Qt::AlignRight | Qt::AlignTop}});
			}

			{
				auto reportSchema = ReportSchema::create(tr("Schema: %1").arg(schema->schemaId()), {}, schema, {});
				auto pageLayout = getSchemaPageLayout(schema);
				auto schemaDrawingSection = report->addSection(ReportSection::create(schema->schemaId(), pageLayout));
				schemaDrawingSection->addSchema(reportSchema);
			}

			{
				QMutexLocker l(&m_statisticsMutex);
				m_statistics.m_currentStatus = WorkerStatus::Printing;
			}

			// Print to file
			//
			m_printer.print(*report, report->path(), m_stop);
		}

		emit finished(QString());

		return;
	}

	void SchemasReportGenerator::exportFilesToSinglePdf()
	{
		if (filePath().isEmpty() == true)
		{
			Q_ASSERT(false);
			return;
		}

		VFrame30::SchemaDetailsSet detailsSet;
		std::map<QString, std::shared_ptr<VFrame30::Schema>> schemas;		// Key is schema ID - schemas to be exported

		try
		{
			openProject();

			loadSchemas(m_inputFiles, schemas, detailsSet);

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

		// Init margins
		//
		if (m_options.footers == true)
		{
			report->addMarginItem({tr("Project: %1").arg(m_projectName), -1, -1, {m_marginFont, Qt::AlignLeft | Qt::AlignTop}});
			report->addMarginItem({tr("%PAGE%"), -1, -1, {m_marginFont, Qt::AlignRight | Qt::AlignBottom}});
			report->addMarginItem({"%TAG%", -1, -1, {m_marginFont, Qt::AlignRight | Qt::AlignTop}});
		}

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
				auto pageLayout = getSchemaPageLayout(schema);
				auto schemaDrawingSection = report->addSection(ReportSection::create(schemaId, pageLayout));
				schemaDrawingSection->setTag(tr("%1 - %2").arg(schema->schemaId()).arg(schema->caption()));
				schemaDrawingSection->addSchema(reportSchema);
			}
		}

		{
			QMutexLocker l(&m_statisticsMutex);
			m_statistics.m_currentStatus = WorkerStatus::Printing;
		}

		// Print to file
		//
		m_printer.print(*report, report->path(), m_stop);

		emit finished(QString());

		return;
	}

	void SchemasReportGenerator::exportAllSchemasToAlbums()
	{
		try
		{
			openProject();

			for (auto& stp : m_schemaTypesParams)
			{
				std::vector<DbFileInfo> schemasFiles;

				if (m_stop == true)
				{
					break;
				}

				if (stp.selected() == false)
				{
					continue;
				}

				// Fill schemas files
				//
				DbFileTree fileTree;

				{
					QMutexLocker l(&m_statisticsMutex);
					m_statistics.m_currentSchemaType = stp.caption();
				}

				bool ok = db()->getFileListTree(&fileTree, stp.fileId(), true/*removeDeleted*/, nullptr);
				if (ok == false)
				{
					throw(tr("DbController::getFileListTree failed on fileId = %1").arg(db()->systemFileId(DbDir::SchemasDir)));
				}

				const std::map<int, std::shared_ptr<DbFileInfo>>& files = fileTree.files();

				for (auto it = files.begin(); it != files.end(); it++)
				{
					const std::shared_ptr<DbFileInfo>& fi = it->second;

					// Filter files by extension
					//
					if (fi->fileName().endsWith("." + QString(Db::File::AlFileExtension)) == false &&
							fi->fileName().endsWith("." + QString(Db::File::UfbFileExtension)) == false &&
							fi->fileName().endsWith("." + QString(Db::File::MvsFileExtension)) == false &&
							fi->fileName().endsWith("." + QString(Db::File::TvsFileExtension)) == false &&
							fi->fileName().endsWith("." + QString(Db::File::DvsFileExtension)) == false)
					{
						continue;
					}

					// Filter files by schema tags
					//
					VFrame30::SchemaDetails details;
					bool ok = details.parseDetails(fi->details());
					if (ok == true)
					{
						bool schemaTagFound = false;
						for (const auto& [tag, tagEnabled] : m_options.schemaTags)
						{
							if (tagEnabled == true && details.schemaTags().contains(tag) == true)
							{
								schemaTagFound = true;
								break;
							}
						}
						if (schemaTagFound == false)
						{
							continue;
						}
					}

					// Add file to list
					//
					schemasFiles.push_back(*fi);
				}

				// Load and parse schemas
				//
				std::map<QString, std::shared_ptr<VFrame30::Schema>> schemas;		// Key is schema ID
				VFrame30::SchemaDetailsSet detailsSet;

				loadSchemas(schemasFiles, schemas, detailsSet);

				if (schemas.empty() == true)
				{
					continue;
				}

				// Render schemas
				//
				if (m_stop == true)
				{
					break;
				}

				renderSchemas(schemas, detailsSet, stp.caption(), stp.pageLayout());
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
		case SchemasReportGenerator::WorkerStatus::Printing:
			{
				*progressText = tr("Printing to PDF Document...");

				const ReportPrinter::Statistics ps = m_printer.statistics();

				switch(ps.status)
				{
				case ReportPrinter::Statistics::Preview:
					*progressText = tr("Generating preview, section: %1/%2")
							.arg(ps.sectionIndex)
							.arg(ps.sectionCount);
					*progress = ps.sectionIndex;
					*progressMax = ps.sectionCount;
				break;
				case ReportPrinter::Statistics::Rendering:
					*progressText = tr("Rendering report, section: %1/%2")
							.arg(ps.sectionIndex)
							.arg(ps.sectionCount);
					*progress = ps.sectionIndex;
					*progressMax = ps.sectionCount;
				break;
				case ReportPrinter::Statistics::Printing:
					*progressText = tr("Printing report, page: %1/%2")
							.arg(ps.pageIndex)
							.arg(ps.pagesCount);
					*progress = ps.pageIndex;
					*progressMax = ps.pagesCount;
				break;
				}
			}
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


	void SchemasReportGenerator::loadSchemas(const std::vector<DbFileInfo>& files,
											 std::map<QString, std::shared_ptr<VFrame30::Schema>>& schemas,
											 VFrame30::SchemaDetailsSet& detailsSet)
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
		int firstParentId = -1;

		for (const std::shared_ptr<DbFile>& dbFile : out)
		{
			if (firstParentId == -1)
			{
				firstParentId = dbFile->parentId();
				continue;
			}

			if (firstParentId != dbFile->parentId())
			{
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

			schemas[schema->schemaId()] = schema;

			if (m_options.signalsDetails == true)
			{
				detailsSet.add(schema->details("."));
			}
		}

		return;
	}

	void SchemasReportGenerator::renderSchemas(const std::map<QString, std::shared_ptr<VFrame30::Schema>> schemas,
											   const VFrame30::SchemaDetailsSet& detailsSet,
											   const QString& groupName,
											   const QPageLayout pageLayout)
	{
		// Render schemas
		//
		{
			QMutexLocker l(&m_statisticsMutex);
			m_statistics.m_currentStatus = WorkerStatus::Rendering;
			m_statistics.m_currentSchemaType = groupName;
			m_statistics.m_schemaIndex = 0;
			m_statistics.m_schemasCount = static_cast<int>(schemas.size());
		}

		std::shared_ptr<Report> report = std::make_shared<Report>(m_projectName,
																  tr("%1/%2_%3.pdf").arg(filePath()).arg(m_projectName).arg(groupName));

		// Init margins
		//
		if (m_options.footers == true)
		{
			report->addMarginItem({tr("Project: %1").arg(m_projectName), -1, -1, {m_marginFont, Qt::AlignLeft | Qt::AlignTop}});
			report->addMarginItem({tr("%PAGE%"), -1, -1, {m_marginFont, Qt::AlignRight | Qt::AlignBottom}});
			report->addMarginItem({"%TAG%", -1, -1, {m_marginFont, Qt::AlignRight | Qt::AlignTop}});
		}

		// Create table of contents
		//
		{
			auto contentsSection = report->addSection(ReportSection::create("Table of Contents", pageLayout));
			contentsSection->setTag(groupName);

			contentsSection->addText(tr("Table of Contents"), {m_normalFont, Qt::AlignHCenter});

			auto contentsTable = ReportTable::create({m_tableFont,
													  {tr("Schema ID"), tr("Caption"), tr("Page")},
													  {30, 50, 20},
													  Qt::AlignLeft});

			contentsSection->addTable(contentsTable);


			for (const auto &[schemaId, schema] : schemas)
			{
				if (m_stop == true)
				{
					break;
				}

				QStringList l;
				l.push_back(schemaId);
				l.push_back(schema->caption());
				l.push_back(tr("%1(%2)")
							.arg(ReportTagStorage::tagSectionStartPage)
							.arg(schemaId));

				contentsTable->insertRow(l);

			}
		}

		// Render schemas
		//
		{
			for (const auto &[schemaId, schema] : schemas)
			{
				if (m_stop == true)
				{
					break;
				}

				{
					QMutexLocker l(&m_statisticsMutex);
					m_statistics.m_schemaIndex++;
					m_statistics.m_currentSchemaId = schemaId;
				}

				auto reportSchema = ReportSchema::create(tr("Schema: %1").arg(schemaId), {}, schema, {});

				auto schemaDrawingSection = report->addSection(ReportSection::create(schemaId, pageLayout));
				schemaDrawingSection->setTag(tr("%1 - %2").arg(schema->schemaId()).arg(schema->caption()));
				schemaDrawingSection->addSchema(reportSchema);

				if (m_options.signalsDetails == true && schema->isLogicSchema() == true)
				{
					createLogicSchemaSignalsDetails(report, pageLayout, schema, schemas, detailsSet);
				}
			}
		}

		{
			QMutexLocker l(&m_statisticsMutex);
			m_statistics.m_currentStatus = WorkerStatus::Printing;
		}

		// Preview the report to calculate page numbers for every section
		{
			std::vector<ReportLib::RenderedSection> renderedSections;
			if (m_printer.preview(*report, renderedSections, m_stop) == false)
			{
				return;
			}

			int page = 1;
			for (const ReportLib::RenderedSection& rs : renderedSections)
			{
				rs.section()->setStartPage(page);
				page += rs.pagesCount();
			}
		}

		// Print report to PDF

		if (report->path().isEmpty() == false)
		{
			// Print to file
			//
			m_printer.print(*report, report->path(), m_stop);
		}
		else
		{
			// Print to buffer
			//
			QBuffer buffer(&m_outputData[groupName + ".pdf"]);
			m_printer.print(*report, buffer, m_stop);

		}
	}

	QPageLayout SchemasReportGenerator::getSchemaPageLayout(const std::shared_ptr<VFrame30::Schema>& schema) const
	{
		qreal marginSizeMM = m_options.footers ? 15 : 0;

		// Initialize PDF page size
		//
		QPageLayout::Orientation orientation = (schema->docWidth() < schema->docHeight()) ?
					QPageLayout::Portrait : QPageLayout::Landscape;

		switch(schema->unit())
		{
		case SchemaUnit::Inch:
			return QPageLayout(QPageSize(QSizeF(schema->docWidth(), schema->docHeight()), QPageSize::Inch),
							   QPageLayout::Portrait,
							   QMarginsF(marginSizeMM / 25.4, marginSizeMM / 25.4, marginSizeMM / 25.4, marginSizeMM / 25.4),
							   QPageLayout::Inch);

		case SchemaUnit::Millimeter:
			return QPageLayout(QPageSize(QSizeF(schema->docWidth(), schema->docHeight()), QPageSize::Millimeter),
							   QPageLayout::Portrait,
							   QMarginsF(marginSizeMM, marginSizeMM, marginSizeMM, marginSizeMM),
							   QPageLayout::Millimeter);

		default:
			// If schema size specified in pixels, use A3 format
			//
			Q_ASSERT(schema->unit() == SchemaUnit::Display);
			return QPageLayout(QPageSize(QPageSize::A3), orientation, QMarginsF(marginSizeMM, marginSizeMM, marginSizeMM, marginSizeMM), QPageLayout::Millimeter);

		}
	}

	void SchemasReportGenerator::createLogicSchemaSignalsDetails(const std::shared_ptr<Report> report,
														  const QPageLayout& pageLayout,
														  const std::shared_ptr<VFrame30::Schema>& schema,
														  const std::map<QString, std::shared_ptr<VFrame30::Schema>>& allSchemas,
														  const VFrame30::SchemaDetailsSet& detailsSet)
	{
		if (schema->isLogicSchema() == false)
		{
			Q_ASSERT(false);
			return;
		}
		VFrame30::LogicSchema* logicSchema = schema->toLogicSchema();
		if (logicSchema == nullptr)
		{
			Q_ASSERT(false);
			return;
		}

		auto schemaDetailsSection = ReportSection::create(tr("Schema Details: %1").arg(schema->schemaId()), pageLayout);
		schemaDetailsSection->setTag(tr("%1 - %2 [Details]").arg(schema->schemaId()).arg(schema->caption()));

		createLogicSchemaIOSignalsDetails(schemaDetailsSection, logicSchema, allSchemas, detailsSet);

		createLogicSchemaLoopbacksDetails(schemaDetailsSection, logicSchema, allSchemas, detailsSet);

		createLogicSchemaConnectionsDetails(schemaDetailsSection, logicSchema, allSchemas, detailsSet);

		if (schemaDetailsSection->objectCount() > 0)
		{
			report->addSection(schemaDetailsSection);
		}

		return;
	}

	void SchemasReportGenerator::createLogicSchemaIOSignalsDetails(const std::shared_ptr<ReportLib::ReportSection> section,
										   const VFrame30::LogicSchema* logicSchema,
										   const std::map<QString, std::shared_ptr<VFrame30::Schema>>& allSchemas,
										   const VFrame30::SchemaDetailsSet& detailsSet)
	{
		auto table = ReportTable::create({m_tableFont,
										  {tr("Signal ID"), tr("Caption"), tr("Type"), tr("Schemas")},
										  {20, 30, 20, 30},
										  Qt::AlignLeft});
		table->setHtmlEscaped(false);

		// Get list of signals for current schema
		//

		std::vector<SchemaSignalInfo> tableContents;

		auto f = [this, &tableContents, &detailsSet, &logicSchema, &allSchemas](const QString& signalId, const VFrame30::FblItemRect* item){

			// Get list of schemas which contain this signal (other schemas)
			//
			QStringList otherSchemasIds;
			QStringList otherSchemasList = detailsSet.schemasByAppSignalId(signalId);
			for (const QString& otherSchemaId : otherSchemasList)
			{
				if (otherSchemaId == logicSchema->schemaId())
				{
					continue;	// Skip current schema
				}

				// Get other schema
				//
				const auto& otherSchemaIt = allSchemas.find(otherSchemaId);
				if (otherSchemaIt == allSchemas.end())
				{
					Q_ASSERT(false);
					continue;
				}

				const std::shared_ptr<VFrame30::Schema>& otherSchema = otherSchemaIt->second;
				if (otherSchema == nullptr)
				{
					Q_ASSERT(otherSchema);
					continue;
				}
				VFrame30::LogicSchema* otherLogicSchema = otherSchema->toLogicSchema();
				if (otherLogicSchema == nullptr)
				{
					Q_ASSERT(false);
					return;
				}

				// Item is SchemaItemSignal* element
				//
				if (const VFrame30::SchemaItemSignal* signalElement = item->toSignalElement();
						signalElement != nullptr)
				{
					auto otherItemSignalsMap = otherLogicSchema->getSignalItemsMap();
					auto r = std::find_if(otherItemSignalsMap.begin(), otherItemSignalsMap.end(), [signalElement, signalId](const auto& it)
					{
						// Find a signal item on other schema that has opposite type (input vs output)
						//
						const QString& otherSignalId = it.first;
						const VFrame30::SchemaItemSignal* otherItem = it.second;
						return otherItem->isInputSignalElement() != signalElement->isInputSignalElement() &&
								otherSignalId == signalId;
					});
					if (r != otherItemSignalsMap.end())
					{
						otherSchemasIds.push_back(tr("%1 [p.%2(%1)]%3")
												  .arg(otherSchemaId)
												  .arg(ReportTagStorage::tagSectionStartPage)
												  .arg(otherLogicSchema->excludeFromBuild() ? " (excluded)" : ""));
					}
				}

				// Item is SchemaItemReceiver* element
				//
				if (const VFrame30::SchemaItemReceiver* receiverElement = item->toReceiverElement();
						receiverElement != nullptr)
				{
					auto otherItemSignalSet = otherLogicSchema->getSignalSet();
					auto r = std::find_if(otherItemSignalSet.begin(), otherItemSignalSet.end(), [signalId](const auto& it)
					{
						// Find an item on other schema with this signal
						//
						const QString& otherSignalId = it;
						return otherSignalId == signalId;
					});
					if (r != otherItemSignalSet.end())
					{
						otherSchemasIds.push_back(tr("%1 [p.%2(%1)]%3")
												  .arg(otherSchemaId)
												  .arg(ReportTagStorage::tagSectionStartPage)
												  .arg(otherLogicSchema->excludeFromBuild() ? " (excluded)" : ""));
					}
				}
			}

			// Fill signal info
			//

			SchemaSignalInfo ssi(item, signalId, otherSchemasIds, m_appSignalProvider);
			tableContents.push_back(ssi);
		};

		// Get SchemaItemSignal* objects
		//
		std::map<QString, VFrame30::SchemaItemSignal*> itemSignalsMap = logicSchema->getSignalItemsMap();

		for (const auto&[signalId, item] : itemSignalsMap)
		{
			f(signalId, item);
		}

		// Get SchemaItemReceiver objects
		//
		std::map<QString, VFrame30::SchemaItemReceiver*> itemReceiversMap = logicSchema->getSignalReceiversMap();

		for (const auto&[signalId, item] : itemReceiversMap)
		{
			f(signalId, item);
		}

		// Sort signals: input first, then sort by x coordinate, then sort by y coordinate
		//
		std::sort(tableContents.begin(), tableContents.end(), SchemaSignalInfo::less);

		// Output data to a table
		//
		for (const SchemaSignalInfo& ssi : tableContents)
		{
			table->insertRow(ssi.toStringList());
		}

		if (table->rowCount() > 0)
		{
			section->addText(tr("Schema '%1 - %2' Signals").arg(logicSchema->schemaId()).arg(logicSchema->caption()),
										  {m_normalFont, Qt::AlignHCenter});
			section->addTable(table);
		}
	}

	void SchemasReportGenerator::createLogicSchemaLoopbacksDetails(const std::shared_ptr<ReportLib::ReportSection> section,
										   const VFrame30::LogicSchema* logicSchema,
										   const std::map<QString, std::shared_ptr<VFrame30::Schema>>& allSchemas,
										   const VFrame30::SchemaDetailsSet& detailsSet)
	{

		auto table = ReportTable::create({m_tableFont,
										  {tr("Loopback ID"), tr("Type"), tr("Schemas")},
										  {30, 20, 50},
										  Qt::AlignLeft});
		table->setHtmlEscaped(false);

		// Get list of loopbacks for current schema
		//

		std::map<QString, VFrame30::SchemaItemLoopback*> loopbacksMap = logicSchema->getLoopbacksMap();

		std::vector<SchemaLoopbackInfo> tableContents;
		tableContents.reserve(loopbacksMap.size());

		for (const auto&[loopbackId, loopbackItem]: loopbacksMap)
		{
			// Get list of schemas which contain this loopbackId (other schemas)
			//
			QStringList otherSchemasIds;
			QStringList otherSchemasList = detailsSet.schemasByLoopbackId(loopbackItem->loopbackId());

			for (const QString& otherSchemaId : otherSchemasList)
			{
				if (otherSchemaId == logicSchema->schemaId())
				{
					continue;	// Skip current schema
				}

				// Get other schema
				//
				const auto& otherSchemaIt = allSchemas.find(otherSchemaId);
				if (otherSchemaIt == allSchemas.end())
				{
					Q_ASSERT(false);
					continue;
				}
				const std::shared_ptr<VFrame30::Schema>& otherSchema = otherSchemaIt->second;
				if (otherSchema == nullptr)
				{
					Q_ASSERT(otherSchema);
					continue;
				}
				VFrame30::LogicSchema* otherLogicSchema = otherSchema->toLogicSchema();
				if (otherLogicSchema == nullptr)
				{
					Q_ASSERT(false);
					return;
				}

				auto otherLoopbackSet = otherLogicSchema->getLoopbacksMap();

				auto r = std::find_if(otherLoopbackSet.begin(), otherLoopbackSet.end(), [loopbackItem](const auto& it)
				{
					// Find a loopback on other schema that has opposite type
					//
					const VFrame30::SchemaItemLoopback* item = it.second;
					return loopbackItem->isLoopbackSourceElement() != item->isLoopbackSourceElement() &&
							item->loopbackId() == loopbackItem->loopbackId();
				});
				if (r != otherLoopbackSet.end())
				{
					otherSchemasIds.push_back(tr("%1 [p.%2(%1)]%3")
											  .arg(otherSchemaId)
											  .arg(ReportTagStorage::tagSectionStartPage)
											  .arg(otherLogicSchema->excludeFromBuild() ? " (excluded)" : ""));
				}
			}

			// Fill loopback info
			//

			SchemaLoopbackInfo ssi(loopbackItem, otherSchemasIds);
			tableContents.push_back(ssi);
		}

		// Sort loopbacks:  by x coordinate, then sort by y coordinate
		//
		std::sort(tableContents.begin(), tableContents.end(), SchemaLoopbackInfo::less);

		// Output data to a table
		//
		for (const SchemaLoopbackInfo& ssi : tableContents)
		{
			table->insertRow(ssi.toStringList());
		}

		if (table->rowCount() > 0)
		{
			section->addText(tr("\n\nSchema '%1 - %2' Loopbacks").arg(logicSchema->schemaId()).arg(logicSchema->caption()),
										  {m_normalFont, Qt::AlignHCenter});
			section->addTable(table);
		}
	}

	void SchemasReportGenerator::createLogicSchemaConnectionsDetails(const std::shared_ptr<ReportLib::ReportSection> section,
											 const VFrame30::LogicSchema* logicSchema,
											 const std::map<QString, std::shared_ptr<VFrame30::Schema>>& allSchemas,
											 const VFrame30::SchemaDetailsSet& detailsSet)
	{
		auto table = ReportTable::create({m_tableFont,
										  {tr("Connection ID"), tr("Type"), tr("Schemas")},
										  {30, 20, 50},
										  Qt::AlignLeft});
		table->setHtmlEscaped(false);

		// Get list of signals for current schema
		//

		std::map<QString, VFrame30::SchemaItemTransmitter*> transmitersMap = logicSchema->getTransmittersMap();
		std::map<QString, VFrame30::SchemaItemReceiver*> receiversMap = logicSchema->getReceiversMap();

		std::vector<SchemaConnectionInfo> tableContents;
		tableContents.reserve(transmitersMap.size() + receiversMap.size());

		for (const auto&[connectionId, transmitterItem] : transmitersMap)
		{
			// Get list of connections which contain this connectionId (other schemas)
			//
			QStringList otherSchemasIds;
			QStringList otherSchemasList = detailsSet.schemasByConnectionId(connectionId);

			for (const QString& otherSchemaId : otherSchemasList)
			{
				if (otherSchemaId == logicSchema->schemaId())
				{
					continue;	// Skip current schema
				}

				// Get other schema
				//
				const auto& otherSchemaIt = allSchemas.find(otherSchemaId);
				if (otherSchemaIt == allSchemas.end())
				{
					Q_ASSERT(false);
					continue;
				}
				const std::shared_ptr<VFrame30::Schema>& otherSchema = otherSchemaIt->second;
				if (otherSchema == nullptr)
				{
					Q_ASSERT(otherSchema);
					continue;
				}
				VFrame30::LogicSchema* otherLogicSchema = otherSchema->toLogicSchema();
				if (otherLogicSchema == nullptr)
				{
					Q_ASSERT(false);
					return;
				}

				auto otherReceiversMap = otherLogicSchema->getReceiversMap();

				auto r = std::find_if(otherReceiversMap.begin(), otherReceiversMap.end(), [connectionId](const auto& it)
				{
					// Find a loopback on other schema that has opposite type
					//
					const VFrame30::SchemaItemReceiver* item = it.second;
					return item->connectionIdsAsList().contains(connectionId);
				});
				if (r != otherReceiversMap.end())
				{
					otherSchemasIds.push_back(tr("%1 [p.%2(%1)]%3")
											  .arg(otherSchemaId)
											  .arg(ReportTagStorage::tagSectionStartPage)
											  .arg(otherLogicSchema->excludeFromBuild() ? " (excluded)" : ""));
				}
			}

			// Fill transmitter info
			//

			SchemaConnectionInfo ssi(transmitterItem, connectionId, otherSchemasIds);
			tableContents.push_back(ssi);
		}

		for (const auto&[connectionId, receiverItem] : receiversMap)
		{
			// Get list of connections which contain this connectionId (other schemas)
			//
			QStringList otherSchemasIds;
			QStringList otherSchemasList = detailsSet.schemasByConnectionId(connectionId);

			for (const QString& otherSchemaId : otherSchemasList)
			{
				if (otherSchemaId == logicSchema->schemaId())
				{
					continue;	// Skip current schema
				}

				// Get other schema
				//
				const auto& otherSchemaIt = allSchemas.find(otherSchemaId);
				if (otherSchemaIt == allSchemas.end())
				{
					Q_ASSERT(false);
					continue;
				}

				const std::shared_ptr<VFrame30::Schema>& otherSchema = otherSchemaIt->second;
				if (otherSchema == nullptr)
				{
					Q_ASSERT(otherSchema);
					continue;
				}
				VFrame30::LogicSchema* otherLogicSchema = otherSchema->toLogicSchema();
				if (otherLogicSchema == nullptr)
				{
					Q_ASSERT(false);
					return;
				}

				auto otherTransmittersMap = otherLogicSchema->getTransmittersMap();

				auto r = std::find_if(otherTransmittersMap.begin(), otherTransmittersMap.end(), [connectionId](const auto& it)
				{
					// Find a loopback on other schema that has opposite type
					//
					const VFrame30::SchemaItemTransmitter* item = it.second;
					return item->connectionIdsAsList().contains(connectionId);
				});
				if (r != otherTransmittersMap.end())
				{
					otherSchemasIds.push_back(tr("%1 [p.%2(%1)]%3")
											  .arg(otherSchemaId)
											  .arg(ReportTagStorage::tagSectionStartPage)
											  .arg(otherLogicSchema->excludeFromBuild() ? " (excluded)" : ""));
				}
			}

			// Fill receiver info
			//

			SchemaConnectionInfo ssi(receiverItem, connectionId, otherSchemasIds);
			tableContents.push_back(ssi);
		}

		// Sort connections: transmitters first, then sort by x coordinate, then sort by y coordinate
		//
		std::sort(tableContents.begin(), tableContents.end(), SchemaConnectionInfo::less);

		// Output data to a table
		//
		for (const SchemaConnectionInfo& ssi : tableContents)
		{
			table->insertRow(ssi.toStringList());
		}

		if (table->rowCount() > 0)
		{
			section->addText(tr("\n\nSchema '%1 - %2' Connections").arg(logicSchema->schemaId()).arg(logicSchema->caption()),
										  {m_normalFont, Qt::AlignHCenter});
			section->addTable(table);
		}
	}
}
