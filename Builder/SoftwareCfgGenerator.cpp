#include "SoftwareCfgGenerator.h"
#include "ApplicationLogicCompiler.h"
#include "../lib/DeviceHelper.h"
#include "../lib/ServiceSettings.h"


namespace Builder
{
	// ---------------------------------------------------------------------------------
	//
	//	SoftwareCfgGenerator class implementation
	//
	// ---------------------------------------------------------------------------------

	HashedVector<QString, Hardware::DeviceModule*> SoftwareCfgGenerator::m_lmList;
	HashedVector<QString, Hardware::Software*> SoftwareCfgGenerator::m_softwareList;
	std::multimap<QString, std::shared_ptr<SoftwareCfgGenerator::SchemaFile>> SoftwareCfgGenerator::m_schemaTagToFile;


	SoftwareCfgGenerator::SoftwareCfgGenerator(Context* context, Hardware::Software* software) :
		m_context(context),
		m_dbController(&context->m_db),
		m_software(software),
		m_signalSet(context->m_signalSet.get()),
		m_equipment(context->m_equipmentSet.get()),
		m_buildResultWriter(context->m_buildResultWriter.get())
	{
		assert(context);
	}

	SoftwareCfgGenerator::~SoftwareCfgGenerator()
	{
	}

	bool SoftwareCfgGenerator::run()
	{
		if (m_dbController == nullptr ||
			m_software == nullptr ||
			m_signalSet == nullptr ||
			m_equipment == nullptr ||
			m_buildResultWriter == nullptr)
		{
			assert(false);
			return false;
		}

		m_log = m_buildResultWriter->log();

		if (m_log == nullptr)
		{
			assert(false);
			return false;
		}

		m_deviceRoot = m_equipment->root();

		if (m_deviceRoot == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		m_subDir = m_software->equipmentIdTemplate();

		m_cfgXml = m_buildResultWriter->createConfigurationXmlFile(m_subDir);

		if (m_cfgXml == nullptr)
		{
			LOG_ERROR_OBSOLETE(m_log, IssuePrefix::NotDefined,
					  QString(tr("Can't create 'Configuration.xml' file for software %1")).
					  arg(m_software->equipmentIdTemplate()));
			return false;
		}

		LOG_MESSAGE(m_log, QString(tr("Generate configuration for: %1")).
					arg(m_software->equipmentIdTemplate()));

		m_cfgXml->xmlWriter().writeStartElement("Software");

		m_cfgXml->xmlWriter().writeAttribute("Caption", m_software->caption());
		m_cfgXml->xmlWriter().writeAttribute("ID", m_software->equipmentIdTemplate());
		m_cfgXml->xmlWriter().writeAttribute("Type", QString("%1").arg(static_cast<int>(m_software->type())));

		m_cfgXml->xmlWriter().writeEndElement();	// </Software>

		bool result = true;

		result &= generateConfiguration();

		return result;
	}


	bool SoftwareCfgGenerator::generalSoftwareCfgGeneration(Context* context)
	{
		if (context == nullptr)
		{
			assert(false);
			return false;
		}

		if (context->m_buildResultWriter == nullptr)
		{
			assert(false);
			return false;
		}

		IssueLogger* log = context->m_log;

		if (log == nullptr)
		{
			assert(false);
			return false;
		}

		if (context->m_signalSet == nullptr ||
			context->m_equipmentSet == nullptr)
		{
			LOG_INTERNAL_ERROR(log);
			assert(false);
			return false;
		}

		bool result = true;

		// add general software cfg generation here:
		//
		result &= buildLmList(context->m_equipmentSet.get(), log);

		result &= buildSoftwareList(context->m_equipmentSet.get(), log);

		result &= checkLmToSoftwareLinks(log);

		// Add Schemas to Build result
		//
		result &= loadAllSchemas(context);

		return result;
	}

	bool SoftwareCfgGenerator::loadAllSchemas(Context* context)
	{
		if (context == nullptr)
		{
			Q_ASSERT(context);
			return false;
		}

		DbController& db = context->m_db;
		IssueLogger* log = context->m_log;

		if (log == nullptr)
		{
			Q_ASSERT(log);
			return false;
		}

		// Project property Generate Extra debug Info
		//
		bool generateExtraDebugIno = false;
		db.getProjectProperty(Db::ProjectProperty::GenerateExtraDebugInfo, &generateExtraDebugIno, nullptr);

		// --
		//
		DbFileTree filesTree;									// Filed in loadAllSchemas

		if (bool ok = db.getFileListTree(&filesTree, db.schemaFileId(), "%", true, nullptr);
			ok == false)
		{
			log->errPDB2001(db.schemaFileId(), "%", db.lastError());
			return false;
		}

		// Remove all marked as deleted files
		//
		filesTree.removeIf([](const DbFileInfo& f)
			{
				return f.action() == VcsItemAction::Deleted;
			});

		// Remove all unsuported files and marked for deleting
		//
		std::vector<DbFileInfo> files = filesTree.toVectorIf([](const DbFileInfo& f)
						{
							return  (f.action() != VcsItemAction::Deleted) &&
									(f.isFolder() == false) &&
									(f.fileName().endsWith(QLatin1String(".") + Db::File::AlFileExtension, Qt::CaseInsensitive) ||
									 f.fileName().endsWith(QLatin1String(".") + Db::File::MvsFileExtension, Qt::CaseInsensitive) ||
									 f.fileName().endsWith(QLatin1String(".") + Db::File::DvsFileExtension, Qt::CaseInsensitive) ||
									 f.fileName().endsWith(QLatin1String(".") + Db::File::UfbFileExtension, Qt::CaseInsensitive) ||
									 f.fileName().endsWith(QLatin1String(".") + Db::File::TvsFileExtension, Qt::CaseInsensitive));
						});

		// Multithreaded load all schemas
		//
		struct FileSchemaStruct
		{
			std::shared_ptr<DbFile> file;
			std::shared_ptr<VFrame30::Schema> schema;		// This param my be nullptr if schema does not have any SchemaItemFrame
		};

		std::map<QString, FileSchemaStruct> schemaMap;		// Key is SchemaID
		QMutex schemasMutex;	// Used only in loading schemas, when concurency is possible

		// --
		//
		std::atomic_bool returnResult = true;		// returnResult is used in multithreaded schema load, that's why it is atomic
		std::atomic_bool iterruptRequest = false;

		std::vector<QFuture<bool>> loadSchemaTasks;
		loadSchemaTasks.reserve(files.size());

		for (const DbFileInfo& f : files)
		{
			// Check for cancel
			//
			if (QThread::currentThread()->isInterruptionRequested() == true)
			{
				return false;
			}

			// --
			//
			LOG_MESSAGE(log, tr("Loading %1").arg(f.fileName()));

			// --
			//
			std::shared_ptr<DbFile> fileLatestVersion;

			if (bool ok  = db.getLatestVersion(f, &fileLatestVersion, nullptr);
				ok == false || fileLatestVersion.get() == nullptr)
			{
				log->errPDB2002(f.fileId(), f.fileName(), db.lastError());
				returnResult = false;
				continue;
			}

			// Read schema files
			//
			auto task = QtConcurrent::run(
				[fileLatestVersion, log, &returnResult, &iterruptRequest, &schemaMap, &schemasMutex]() -> bool
				{
					if (iterruptRequest == true)
					{
						return false;
					}

					std::shared_ptr<VFrame30::Schema> schema = VFrame30::Schema::Create(fileLatestVersion->data());
					if (schema == nullptr)
					{
						// File loading/parsing error, file is damaged or has incompatible format, file name '%1'.
						//
						log->errCMN0010(fileLatestVersion->fileName());
						returnResult = false;
					}

					// ADD EVEN EXCLUDED FOR BUILD SCHEMAS
					// as it can be pannel schema
					//
					//if (schema->excludeFromBuild() == false)
					{
						QMutexLocker locker(&schemasMutex);	// Mutext used only here, as only here concurent access to schemas is possible
						schemaMap[schema->schemaId()] = FileSchemaStruct{fileLatestVersion, schema};
					}

					return true;
				});

			loadSchemaTasks.push_back(task);
		}

		// Wait for finish and process interrupt request
		//
		do
		{
			bool allFinished = true;
			for (auto& task : loadSchemaTasks)
			{
				QThread::yieldCurrentThread();
				if (task.isRunning() == true)
				{
					allFinished = false;
					break;
				}
			}

			if (allFinished == true)
			{
				break;	// THE EXIT FROM DO/WHILE LOOP!
			}
			else
			{
				// Set iterruptRequest, so work threads can get it and exit
				//
				iterruptRequest = QThread::currentThread()->isInterruptionRequested();
				QThread::yieldCurrentThread();
			}
		} while (true);

		filesTree.clear();		// Files are already loaded and not required anymore
		files.clear();			// Files are already loaded and not required anymore

		// All schemas are parsed and loaded to map schemas
		// iterate them and joint schemas to left/right/top/buttom
		//
		QString group{"Schema"};
		bool schemaItemFrameWasProcessed = false;

		auto findPannelSchemaFunc = [&schemaMap](QString pannelSchemaId)
			{
				if (auto pannelSchemaIt = schemaMap.find(pannelSchemaId);
					pannelSchemaIt != schemaMap.end())
				{
					return pannelSchemaIt->second.schema;
				}

				return std::shared_ptr<VFrame30::Schema>{};
			};

		auto joinSschemasFunc = [context, log, findPannelSchemaFunc, &schemaItemFrameWasProcessed](auto schema, QString pannelSchemaId, Qt::Edge edge)
		{
			if (pannelSchemaId.isEmpty() == false)
			{
				std::shared_ptr<VFrame30::Schema> pannelSchema = findPannelSchemaFunc(pannelSchemaId);

				if (pannelSchema == nullptr)
				{
					log->errALP4080(schema->schemaId(), pannelSchemaId);
					return false;
				}

				// Expand schema, move all items right
				//
				joinSchemas(context, schema.get(), pannelSchema.get(), edge);

				schemaItemFrameWasProcessed = true;
			}

			return true;
		};

		for (auto& [schemaId, fileSchema] : schemaMap)
		{
			std::shared_ptr<DbFile>& file = fileSchema.file;
			std::shared_ptr<VFrame30::Schema>& schema = fileSchema.schema;

			Q_ASSERT(file);
			Q_ASSERT(schema);

			if (schema->schemaId() != schemaId)
			{
				Q_ASSERT(schema->schemaId() == schemaId);
				log->errINT1000(tr("SchemaIDs are not equal: ") + Q_FUNC_INFO);
				return false;
			}

			// Left schemas
			//
			bool joinResult = true;

			if (schema->joinHorzPriority() == true)
			{
				joinResult &= joinSschemasFunc(schema, schema->joinLeftSchemaId().trimmed(), Qt::LeftEdge);
				joinResult &= joinSschemasFunc(schema, schema->joinRightSchemaId().trimmed(), Qt::RightEdge);

				joinResult &= joinSschemasFunc(schema, schema->joinTopSchemaId().trimmed(), Qt::TopEdge);
				joinResult &= joinSschemasFunc(schema, schema->joinBottomSchemaId().trimmed(), Qt::BottomEdge);
			}
			else
			{
				joinResult &= joinSschemasFunc(schema, schema->joinTopSchemaId().trimmed(), Qt::TopEdge);
				joinResult &= joinSschemasFunc(schema, schema->joinBottomSchemaId().trimmed(), Qt::BottomEdge);

				joinResult &= joinSschemasFunc(schema, schema->joinLeftSchemaId().trimmed(), Qt::LeftEdge);
				joinResult &= joinSschemasFunc(schema, schema->joinRightSchemaId().trimmed(), Qt::RightEdge);
			}

			if (joinResult == false)
			{
				returnResult = false;
			}

			//
			// Add file to build result
			//
			QString subDir = "Schemas." + file->extension();
			QStringList schemaTags = schema->tagsAsList();

			if (schemaItemFrameWasProcessed == true)
			{
				QByteArray ba;
				schema->saveToByteArray(&ba);

				file->setData(std::move(ba));
			}

			if (bool ok = context->m_buildResultWriter->addFile(subDir, file->fileName(), schema->schemaId(), schemaTags.join(";"), file->data(), false);
				ok == false)
			{

				continue;
			}

			// Write schema scripts
			//
			if (generateExtraDebugIno == true)
			{
				bool writeScriptOk = writeSchemaScriptProperties(schema.get(), subDir + "/Scripts/" + schema->schemaId(), context->m_buildResultWriter.get());

				if (writeScriptOk == false)
				{
					returnResult = false;
					continue;
				}
			}

			// --
			//
			std::shared_ptr<SchemaFile> schemaFile = std::make_shared<SchemaFile>(schema->schemaId(), file->fileName(), subDir, group, "");

			if (bool parseOk = schemaFile->details.parseDetails(schema->details());
				parseOk == false)
			{
				log->errINT1001(tr("Parse schema detais error."), schema->schemaId());
				returnResult = false;
				continue;
			}

			for (const QString& t : schemaTags)
			{
				m_schemaTagToFile.insert({t.toLower(), schemaFile});
			}
		}

		return returnResult;
	}

	bool SoftwareCfgGenerator::writeSchemaScriptProperties(VFrame30::Schema* schema, QString dir, BuildResultWriter* buildResultWriter)
	{
		Q_ASSERT(schema);
		Q_ASSERT(buildResultWriter);

		bool result = true;

		for (std::shared_ptr<VFrame30::SchemaLayer> layer : schema->Layers)
		{
			for (const SchemaItemPtr& item : layer->Items)
			{
				std::vector<std::shared_ptr<Property>> props = item->properties();

				for (const std::shared_ptr<Property>& property : props)
				{
					if (property->isScript() == true)
					{
						QString script = property->value().toString().trimmed();

						if (script.isEmpty() == false)
						{
							QString fileName;

							if (item->label().isEmpty() == false)
							{
								fileName = item->label() + "." + property->caption() + ".js";
							}
							else
							{
								fileName += QString("%1 - %2")
												.arg(QString(item->metaObject()->className()).remove(QStringLiteral("VFrame30::")))
												.arg(item->guid().toString())
											+ "." + property->caption()
											+ ".js";
							}

							BuildFile* file = buildResultWriter->addFile(dir, fileName, property->value().toString());

							if (file == nullptr)
							{
								result = false;
							}
						}
					}
				}
			}
		}

		return result;
	}

	void SoftwareCfgGenerator::clearStaticData()
	{
		m_lmList.clear();
		m_softwareList.clear();
		m_schemaTagToFile.clear();

		return;
	}

	void SoftwareCfgGenerator::initSubsystemKeyMap(SubsystemKeyMap* subsystemKeyMap, const Hardware::SubsystemStorage* subsystems)
	{
		if (subsystemKeyMap == nullptr || subsystems == nullptr)
		{
			assert(false);
			return;
		}

		subsystemKeyMap->clear();

		int subsystemsCount = subsystems->count();

		for(int i = 0; i < subsystemsCount; i++)
		{
			std::shared_ptr<Hardware::Subsystem> subsystem = subsystems->get(i);

			if (subsystem == nullptr)
			{
				assert(false);
				continue;
			}

			subsystemKeyMap->insert(subsystem->subsystemId(), subsystem->key());
		}
	}

	QString SoftwareCfgGenerator::equipmentID() const
	{
		if (m_software == nullptr)
		{
			assert(false);
			return QString();
		}

		return m_software->equipmentIdTemplate();
	}

	bool SoftwareCfgGenerator::buildLmList(Hardware::EquipmentSet* equipment, IssueLogger* log)
	{
		if (equipment == nullptr)
		{
			assert(false);
			return false;
		}

		bool result = true;

		m_lmList.clear();

		equipmentWalker(equipment->root(), [&result](Hardware::DeviceObject* currentDevice)
			{
				if (currentDevice == nullptr)
				{
					assert(false);
					result = false;
					return;
				}

				if (currentDevice->isModule() == false)
				{
					return;
				}

				Hardware::DeviceModule* module = currentDevice->toModule();

				if (module->isLogicModule() == false)
				{
					return;
				}

				m_lmList.insert(module->equipmentId(), module);
			}
		);

		if (result == true)
		{
			LOG_MESSAGE(log, QString(tr("Logic Modules list building... OK")));
		}
		else
		{
			LOG_ERROR_OBSOLETE(log, IssuePrefix::NotDefined, QString(tr("Can't build Logic Modules list")));
		}

		return result;
	}


	bool SoftwareCfgGenerator::buildSoftwareList(Hardware::EquipmentSet *equipment, IssueLogger* log)
	{
		if (equipment == nullptr)
		{
			assert(false);
			return false;
		}

		bool result = true;

		m_softwareList.clear();

		equipmentWalker(equipment->root(), [&result](Hardware::DeviceObject* currentDevice)
			{
				if (currentDevice == nullptr)
				{
					assert(false);
					result = false;
					return;
				}

				if (currentDevice->isSoftware() == false)
				{
					return;
				}

				Hardware::Software* software = currentDevice->toSoftware();

				if (software == nullptr)
				{
					assert(false);
					result = false;
					return;
				}

				m_softwareList.insert(software->equipmentId(), software);
			}
		);

		if (result == true)
		{
			LOG_MESSAGE(log, QString(tr("Software list building... OK")));
		}
		else
		{
			LOG_ERROR_OBSOLETE(log, IssuePrefix::NotDefined, QString(tr("Can't build software list")));
		}

		return result;
	}


	bool SoftwareCfgGenerator::checkLmToSoftwareLinks(IssueLogger* log)
	{
		bool result = true;

		for(Hardware::DeviceModule* lm : m_lmList)
		{
			for(int adapter = DataSource::LM_ETHERNET_ADAPTER1; adapter <= DataSource::LM_ETHERNET_ADAPTER3; adapter++)
			{
				DataSource::LmEthernetAdapterProperties adapterProperties;
				Hardware::Software* software = nullptr;

				result &= adapterProperties.getLmEthernetAdapterNetworkProperties(lm, adapter, log);

				if (result == false)
				{
					break;
				}

				if (adapter ==  DataSource::LM_ETHERNET_ADAPTER1)
				{
					// tuning adapter
					//
					if (adapterProperties.tuningEnable == true)
					{
						if (adapterProperties.tuningServiceID.isEmpty() == true)
						{
							// Property '%1.%2' is empty.
							//
							log->errCFG3022(adapterProperties.adapterID,
											DataSource::LmEthernetAdapterProperties::PROP_TUNING_SERVICE_ID);
						}
						else
						{
							if (m_softwareList.contains(adapterProperties.tuningServiceID) == false)
							{
								// Property '%1.%2' is linked to undefined software ID '%3'.
								//
								log->errCFG3021(adapterProperties.adapterID,
												DataSource::LmEthernetAdapterProperties::PROP_TUNING_SERVICE_ID,
												adapterProperties.tuningServiceID);
							}
							else
							{
								software = m_softwareList[adapterProperties.tuningServiceID];

								if (software->type() != E::SoftwareType::TuningService)
								{
									// Property '%1.%2' is linked to not compatible software '%3'.
									//
									log->errCFG3017(adapterProperties.adapterID,
													DataSource::LmEthernetAdapterProperties::PROP_TUNING_SERVICE_ID,
													adapterProperties.tuningServiceID);
									result = false;
								}
							}
						}
					}
				}
				else
				{
					// app and diag data adapter

					// test appDataServiceID property
					//
					if (adapterProperties.appDataEnable == true)
					{
						if (adapterProperties.appDataServiceID.isEmpty() == true)
						{
							// Property '%1.%2' is empty.
							//
							log->errCFG3022(adapterProperties.adapterID,
											DataSource::LmEthernetAdapterProperties::PROP_APP_DATA_SERVICE_ID);
						}
						else
						{
							if (m_softwareList.contains(adapterProperties.appDataServiceID) == false)
							{
								// Property '%1.%2' is linked to undefined software ID '%3'.
								//
								log->errCFG3021(adapterProperties.adapterID,
												DataSource::LmEthernetAdapterProperties::PROP_APP_DATA_SERVICE_ID,
												adapterProperties.appDataServiceID);
							}
							else
							{
								software = m_softwareList[adapterProperties.appDataServiceID];

								if (software->type() != E::SoftwareType::AppDataService)
								{
									// Property '%1.%2' is linked to not compatible software '%3'.
									//
									log->errCFG3017(adapterProperties.adapterID,
													DataSource::LmEthernetAdapterProperties::PROP_APP_DATA_SERVICE_ID,
													adapterProperties.appDataServiceID);
									result = false;
								}
							}
						}
					}

					// test diagDataServiceID property
					//
					if (adapterProperties.diagDataEnable == true)
					{
						if (adapterProperties.diagDataServiceID.isEmpty() == true)
						{
							// Property '%1.%2' is empty.
							//
							log->errCFG3022(adapterProperties.adapterID,
											DataSource::LmEthernetAdapterProperties::PROP_DIAG_DATA_SERVICE_ID);
						}
						else
						{
							if (m_softwareList.contains(adapterProperties.diagDataServiceID) == false)
							{
								// Property '%1.%2' is linked to undefined software ID '%3'.
								//
								log->errCFG3021(adapterProperties.adapterID,
												DataSource::LmEthernetAdapterProperties::PROP_DIAG_DATA_SERVICE_ID,
												adapterProperties.diagDataServiceID);
							}
							else
							{
								software = m_softwareList[adapterProperties.diagDataServiceID];

								if (software->type() != E::SoftwareType::DiagDataService)
								{
									// Property '%1.%2' is linked to not compatible software '%3'.
									//
									log->errCFG3017(adapterProperties.adapterID,
													DataSource::LmEthernetAdapterProperties::PROP_DIAG_DATA_SERVICE_ID,
													adapterProperties.diagDataServiceID);
									result = false;
								}
							}
						}
					}
				}
			}
		}

		return result;
	}

	QString SoftwareCfgGenerator::getBuildInfoCommentsForBat()
	{
		BuildInfo&& b = m_buildResultWriter->buildInfo();

		QString comments = "@rem Project: " + b.project + "\n";
		comments += "@rem BuildNo: " + QString::number(b.id) + "\n";
		comments += "@rem Type: " + b.typeStr() + "\n";
		comments += "@rem Date: " + b.dateStr() + "\n";
		comments += "@rem Changeset: " + QString::number(b.changeset) + "\n";
		comments += "@rem User: " + b.user + "\n";
		comments += "@rem Workstation: " + b.workstation + "\n\n";

		return comments;
	}

	QString SoftwareCfgGenerator::getBuildInfoCommentsForSh()
	{
		BuildInfo&& b = m_buildResultWriter->buildInfo();

		QString comments = "#!/bin/bash\n\n";

		comments += "# Project: " + b.project + "\n";
		comments += "# BuildNo: " + QString::number(b.id) + "\n";
		comments += "# Type: " + b.typeStr() + "\n";
		comments += "# Date: " + b.dateStr() + "\n";
		comments += "# Changeset: " + QString::number(b.changeset) + "\n";
		comments += "# User: " + b.user + "\n";
		comments += "# Workstation: " + b.workstation + "\n\n";

		return comments;
	}

	bool SoftwareCfgGenerator::getConfigIp(QString* cfgIP1, QString* cfgIP2)
	{
		TEST_PTR_RETURN_FALSE(m_log);

		TEST_PTR_LOG_RETURN_FALSE(m_equipment, m_log);
		TEST_PTR_LOG_RETURN_FALSE(m_software, m_log);
		TEST_PTR_LOG_RETURN_FALSE(cfgIP1, m_log);
		TEST_PTR_LOG_RETURN_FALSE(cfgIP2, m_log);

		cfgIP1->clear();
		cfgIP2->clear();

		QString cfgServiceID1;
		QString cfgServiceID2;

		HostAddressPort cfgServiceIP1;
		HostAddressPort cfgServiceIP2;

		bool result = true;

		result = ServiceSettings::getCfgServiceConnection(m_equipment, m_software,
														   &cfgServiceID1, &cfgServiceIP1,
														   &cfgServiceID2, &cfgServiceIP2,
														   m_log);
		if (result == false)
		{
			return false;
		}

		if (cfgServiceID1.isEmpty() == false)
		{
			*cfgIP1 = cfgServiceIP1.addressPortStr();
		}

		if (cfgServiceID2.isEmpty() == false)
		{
			*cfgIP2 = cfgServiceIP2.addressPortStr();
		}

		return true;
	}

	bool SoftwareCfgGenerator::getServiceParameters(QString& parameters)
	{
		parameters += " -e";

		QString cfgIP1;
		QString cfgIP2;
		
		if (getConfigIp(&cfgIP1, &cfgIP2) == false)
		{
			return false;
		}

		if (cfgIP1.isEmpty() == true && cfgIP2.isEmpty() == true)
		{
			m_log->errALC5140(m_software->equipmentIdTemplate());
			return false;
		}

		if (cfgIP1.isEmpty() == true)
		{
			cfgIP1 = cfgIP2;
		}

		if (cfgIP2.isEmpty() == true)
		{
			cfgIP2 = cfgIP1;
		}

		parameters += " -cfgip1=" + cfgIP1 + " -cfgip2=" + cfgIP2;

		parameters += " -id=" + m_software->equipmentIdTemplate();

		return true;
	}

	bool SoftwareCfgGenerator::joinSchemas(Context* context, VFrame30::Schema* schema, const VFrame30::Schema* pannel, Qt::Edge edge)
	{
		if (context == nullptr || schema == nullptr || pannel == nullptr)
		{
			Q_ASSERT(context);
			Q_ASSERT(schema);
			Q_ASSERT(pannel);
			return false;
		}

		IssueLogger* log = context->m_log;

		if (schema->unit() != pannel->unit())
		{
			log->errALP4082(schema->schemaId(), pannel->schemaId());
			return false;
		}

		if (schema->schemaId() == pannel->schemaId())
		{
			log->errALP4081(schema->schemaId());
			return false;
		}

		// Expand schema and calc rects
		//
		QRectF schemaRect;		// New rect for existing items
		QRectF pannelRect;		// Rect to move pannel items yo

		switch (edge)
		{
		case Qt::Edge::LeftEdge:
		case Qt::Edge::RightEdge:
			{
				schemaRect = QRectF{0, 0, schema->docWidth(), schema->docHeight()};
				pannelRect = QRectF{0, 0, pannel->docWidth(), pannel->docHeight()};

				schema->setDocWidth(schema->docWidth() + pannel->docWidth());
				schema->setDocHeight(std::max(schema->docHeight(), pannel->docHeight()));

				if (edge == Qt::Edge::LeftEdge)
				{
					schemaRect.moveRight(schema->docWidth());
				}
				else // edge == Qt::Edge::RightEdge)
				{
					pannelRect.moveRight(schema->docWidth());
				}
			}
			break;

		case Qt::Edge::TopEdge:
		case Qt::Edge::BottomEdge:
			{
				schemaRect = QRectF{0, 0, schema->docWidth(), schema->docHeight()};
				pannelRect = QRectF{0, 0, pannel->docWidth(), pannel->docHeight()};

				schema->setDocWidth(std::max(schema->docWidth(), pannel->docWidth()));
				schema->setDocHeight(schema->docHeight() + pannel->docHeight());

				if (edge == Qt::Edge::TopEdge)
				{
					schemaRect.moveBottom(schema->docHeight());
				}
				else // edge == Qt::Edge::BottomEdge)
				{
					pannelRect.moveBottom(schema->docHeight());
				}
			}
			break;

		default:
			Q_ASSERT(false);
			log->errINT1000(tr("Edge param error, edge %1,  function %2").arg(edge).arg(Q_FUNC_INFO));
			return false;
		}

		Q_ASSERT(schemaRect.isNull() == false);
		Q_ASSERT(pannelRect.isNull() == false);

		// Move schema items to new pos in schemaRect
		//
		if (schemaRect.topLeft().isNull() == false)
		{
			for (std::shared_ptr<VFrame30::SchemaLayer> layer :  schema->Layers)
			{
				if (layer == nullptr)
				{
					Q_ASSERT(layer);
					log->errINT1000(QString("Layer is nullptr for schema %1: %2").arg(schema->schemaId()).arg(Q_FUNC_INFO));
					return false;
				}

				for (SchemaItemPtr& item :  layer->Items)
				{
					if (item == nullptr)
					{
						Q_ASSERT(item);
						log->errINT1000(tr("Item is nullptr for schema %1: %2").arg(schema->schemaId()).arg(Q_FUNC_INFO));
						return false;
					}

					item->moveItem(schemaRect.left(), schemaRect.top());
				}
			}
		}

		// Add pannel items to pannelRect
		//
		for (std::shared_ptr<VFrame30::SchemaLayer> pannelLayer : pannel->Layers)
		{
			Q_ASSERT(pannelLayer);

			auto foundDestLayerIt = std::find_if(schema->Layers.begin(), schema->Layers.end(),
												 [pannelLayer](auto l) { return l->name() == pannelLayer->name(); } );

			if (foundDestLayerIt == schema->Layers.end())
			{
				// Source layer is not found in destination, copy to compile layer,
				// if compile layer does not exists either, then copy to the first layer
				//
				foundDestLayerIt = std::find_if(schema->Layers.begin(), schema->Layers.end(),
												[](auto l) { return l->compile(); } );

				if (foundDestLayerIt == schema->Layers.end())
				{
					foundDestLayerIt = schema->Layers.begin();
				}
			}

			Q_ASSERT(foundDestLayerIt != schema->Layers.end());

			// Copy all form sourceLayer to destLayer, keep the order of items and insert all them right at the end of items
			//
			std::shared_ptr<VFrame30::SchemaLayer> destLayer = *foundDestLayerIt;

			if (destLayer == nullptr)
			{
				Q_ASSERT(destLayer);
				return false;
			}

			for (SchemaItemPtr sourceItem : pannelLayer->Items)
			{
				// Make a deep copy of source item, set new guid and label to it
				//
				Proto::Envelope savedItem;

				if (bool saveOk = sourceItem->Save(&savedItem);
					saveOk == false)
				{
					Q_ASSERT(saveOk);
					return false;
				}

				SchemaItemPtr newItem = VFrame30::SchemaItem::Create(savedItem);
				if (newItem == nullptr)
				{
					Q_ASSERT(newItem);
					return false;
				}

				newItem->setNewGuid();		// generate new guids for item and its pins

				// From new label for SchemaItem: SchemaID_FblItemRectLabel
				//
				newItem->setLabel(schema->schemaId() + "_" + newItem->label());

				// Insert newItem to destionation schema layer
				//
				newItem->moveItem(pannelRect.left(), pannelRect.top());

				// --
				//
				destLayer->Items.push_back(newItem);
			}
		}

		return true;
	}

	bool SoftwareCfgGenerator::loadFileFromDatabase(DbController* db, int parentId, const QString& fileName, QString *errorCode, QByteArray* data)
	{
		if (db == nullptr || errorCode == nullptr || data == nullptr)
		{
			assert(errorCode);
			assert(db);
			assert(data);
			return false;
		}

		// Load the file from the database
		//

		std::vector<DbFileInfo> fileList;
		bool ok = db->getFileList(&fileList, parentId, fileName, true, nullptr);
		if (ok == false || fileList.size() != 1)
		{
			*errorCode = QObject::tr("File %1 is not found.").arg(fileName);
			return false;
		}

		std::shared_ptr<DbFile> file;
		ok = db->getLatestVersion(fileList[0], &file, nullptr);
		if (ok == false || file == nullptr)
		{
			*errorCode = QObject::tr("Get latest version of %1 failed.").arg(fileName);
			return false;
		}

		file->swapData(*data);

		return true;
	}
}


