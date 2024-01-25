#include "SoftwareCfgGenerator.h"
#include "AppLogicCompiler.h"
#include "DeviceHelper.h"
#include "LanControllerInfoHelper.h"
#include "ScriptChecker.h"

#include "../OnlineLib/SoftwareSettings.h"
#include "../VFrame30/SchemaLayer.h"
#include "../VFrame30/PropertyNames.h"
#include "../VduSchemaGenerator/VduSchemaGenerator.h"


namespace Builder
{
	// ---------------------------------------------------------------------------------
	//
	//	SoftwareCfgGenerator class implementation
	//
	// ---------------------------------------------------------------------------------

	std::multimap<QString, std::shared_ptr<SoftwareCfgGenerator::SchemaFile>> SoftwareCfgGenerator::m_schemaTagToFile;
	std::map<QString, QString> SoftwareCfgGenerator::m_tuningSimIpPorts;

	SoftwareCfgGenerator::SoftwareCfgGenerator(Context* context, Hardware::Software* software) :
		m_context(context),
		m_software(software),
		m_dbController(&context->m_db),
		m_signalSet(context->m_signalSet.get()),
		m_equipment(context->m_equipmentSet.get()),
		m_buildResultWriter(context->m_buildResultWriter.get()),
		m_log(context->m_log),
		m_settingsSet(software->softwareType())
	{
		assert(context);

		m_softwareControllersIDs = DeviceHelper::getSoftwareControllersIDs(software);
	}

	SoftwareCfgGenerator::~SoftwareCfgGenerator()
	{
	}

	bool SoftwareCfgGenerator::createSettingsProfile(const QString& profile)
	{
		TEST_PTR_RETURN_FALSE(m_log);
		TEST_PTR_LOG_RETURN_FALSE(m_software, m_log);

		// Method should be override in classes derived from SoftwareCfgGenerator like this:
		//
		//	Specific_SettingsGetter settingsGetter;
		//
		//	if (settingsGetter.readFromDevice(m_context, m_software) == false)
		//	{
		//		return false;
		//	}
		//
		//	return m_settingsSet.addProfile<Specific_Settings>(profile, &settingsGetter);
		//
		//

		Q_UNUSED(profile);

		LOG_INTERNAL_ERROR_MSG(m_log, QString("Method createSettingsProfile(...) is not implemented for software %1 (type %2)").
									arg(equipmentID()).arg(E::valueToString<E::SoftwareType>(m_software->softwareType())));

		return false;
	}

	bool SoftwareCfgGenerator::generateConfigurationStep2()
	{
		return true;
	}

	bool SoftwareCfgGenerator::getSettingsXml(QXmlStreamWriter& xmlWriter)
	{
		XmlWriteHelper xml(xmlWriter);

		return m_settingsSet.writeToXml(xml);
	}

	bool SoftwareCfgGenerator::createConfigurationXml()
	{
		if (m_log == nullptr ||
			m_dbController == nullptr ||
			m_software == nullptr ||
			m_signalSet == nullptr ||
			m_equipment == nullptr ||
			m_buildResultWriter == nullptr)
		{
			assert(false);
			return false;
		}

		m_cfgXml = m_buildResultWriter->createConfigurationXmlFile(softwareCfgSubdir());

		if (m_cfgXml == nullptr)
		{
			LOG_ERROR_OBSOLETE(m_log, IssuePrefix::NotDefined,
					  QString(tr("Can't create 'Configuration.xml' file for software %1")).
					  arg(m_software->equipmentIdTemplate()));
			return false;
		}

		writeSoftwareSection(m_cfgXml->xmlWriter(), true);

		bool result = getSettingsXml(m_cfgXml->xmlWriter());

		return result;
	}

	bool SoftwareCfgGenerator::generalSoftwareCfgGeneration(Context* context)
	{
		if (context == nullptr)
		{
			assert(context);
			return false;
		}

		if (context->m_buildResultWriter == nullptr)
		{
			assert(context->m_buildResultWriter);
			return false;
		}

		IssueLogger* log = context->m_log;

		if (log == nullptr)
		{
			assert(log);
			return false;
		}

		if (context->m_signalSet == nullptr ||
			context->m_equipmentSet == nullptr)
		{
			LOG_INTERNAL_ERROR(log);
			assert(context->m_signalSet);
			assert(context->m_equipmentSet);
			return false;
		}

		bool result = true;

		// add general software cfg generation here:
		//
		result &= checkLmToSoftwareLinks(context);

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
		int schemaFileId = db.systemFileId(DbDir::SchemasDir);

		if (bool ok = db.getFileListTree(&filesTree, schemaFileId, "%", true, nullptr);
			ok == false)
		{
			log->errPDB2001(schemaFileId, "%", db.lastError());
			return false;
		}

		// Remove all marked as deleted files
		//
		filesTree.removeIf([](const DbFileInfo& f)
			{
				return f.action() == E::VcsItemAction::Deleted;
			});

		// Remove all unsupported files and marked for deleting
		//
		std::vector<DbFileInfo> files = filesTree.toVectorIf([](const DbFileInfo& f)
						{
							return  (f.action() != E::VcsItemAction::Deleted) &&
									(f.isFolder() == false) &&
									(f.fileName().endsWith(QLatin1String(".") + Db::File::AlFileExtension, Qt::CaseInsensitive) ||
									 f.fileName().endsWith(QLatin1String(".") + Db::File::MvsFileExtension, Qt::CaseInsensitive) ||
									 f.fileName().endsWith(QLatin1String(".") + Db::File::DvsFileExtension, Qt::CaseInsensitive) ||
									 f.fileName().endsWith(QLatin1String(".") + Db::File::UfbFileExtension, Qt::CaseInsensitive) ||
									 f.fileName().endsWith(QLatin1String(".") + Db::File::TvsFileExtension, Qt::CaseInsensitive) ||
									 f.fileName().endsWith(QLatin1String(".") + Db::File::VduFileExtension, Qt::CaseInsensitive));
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
		std::atomic_bool interruptRequest = false;

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
				[fileLatestVersion, log, &returnResult, &interruptRequest, &schemaMap, &schemasMutex]() -> bool
				{
					if (interruptRequest == true)
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
						return false;
					}

					if (schema->excludeFromBuild() == false)
					{
						QMutexLocker locker(&schemasMutex);	// Mutex used only here, as only here concurrent access to schemas is possible
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
				// Set interruptRequest, so work threads can get it and exit
				//
				interruptRequest = QThread::currentThread()->isInterruptionRequested();
				QThread::yieldCurrentThread();
			}
		} while (true);

		// Add AppSignalIDs to packed logic output items.
		//  
		for (auto& [schemaId, fileSchema] : schemaMap)
		{
			std::shared_ptr<DbFile>& file = fileSchema.file;
			std::shared_ptr<VFrame30::Schema>& schema = fileSchema.schema;

			Q_ASSERT(file);
			Q_ASSERT(schema);

			bool schemaModified = false;

			for (auto& layer : schema->layers())
			{
				for (auto& schemaItem : layer->items())
				{
					auto schemaItemAfb = schemaItem->toSchemaItemAfb();
					if (schemaItemAfb == nullptr || schemaItemAfb->isPackedLogic() == false)
					{
						continue;
					}

					// This is a packed logic item, it is an output counterpart if it does not have any inputs 
					// (on schema, in the parser and compiler they are added, but on schema not).
					//
					if (schemaItemAfb->inputsCount() != 0)
					{
						continue;
					}

					// This is a packed logic output item, add property AppSignalIDs to it.
					//
					auto packedLogicSourcesIt = context->m_packedLogicSources.find(schemaItemAfb->label());
					if (packedLogicSourcesIt == context->m_packedLogicSources.end())
					{
						continue;
					}

					const std::list<LmPackedLogicSources>& packedLogicSources = packedLogicSourcesIt->second;

					std::set<QString> appSignalIds;
					for (const auto& src : packedLogicSources)
					{
						for (const auto& srcItem : src.sources)
						{
							QString someId = srcItem.appSignalID.isEmpty() == false ?
												 srcItem.appSignalID :
												 srcItem.sourceItemLabelOut;

							appSignalIds.insert(someId);
						}
					}

					// Join set to a string.
					// 
					QStringList appSignalIdsList;
					for (const QString& appSignalId : appSignalIds)
					{
						appSignalIdsList << appSignalId;
					}

					if (appSignalIdsList.isEmpty() == false)
					{
						schemaModified = true;
						schemaItemAfb->setPackedLogicInputSignalIds(appSignalIdsList);
					}
				}
			}

			if (schemaModified == true)
			{
				QByteArray ba;
				schema->saveToByteArray(&ba);

				file->setData(std::move(ba));
			}
		}

		// All schemas are parsed and loaded to map schemas
		// iterate them and join schemas to left/right/top/bottom
		//
		QString group{"Schema"};
		bool schemaItemFrameWasProcessed = false;

		auto findPanelSchemaFunc = [&schemaMap](QString panelSchemaId)
			{
				if (auto panelSchemaIt = schemaMap.find(panelSchemaId);
					panelSchemaIt != schemaMap.end())
				{
					return panelSchemaIt->second.schema;
				}

				return std::shared_ptr<VFrame30::Schema>{};
			};

		auto joinSchemasFunc = [context, log, findPanelSchemaFunc, &schemaItemFrameWasProcessed](auto schema, QString panelSchemaId, Qt::Edge edge)
		{
			if (panelSchemaId.isEmpty() == false)
			{
				std::shared_ptr<VFrame30::Schema> panelSchema = findPanelSchemaFunc(panelSchemaId);

				if (panelSchema == nullptr)
				{
					log->errALP4080(schema->schemaId(), panelSchemaId);
					return false;
				}

				// Expand schema, move all items right
				//
				joinSchemas(context, schema.get(), panelSchema.get(), edge);

				schemaItemFrameWasProcessed = true;
			}

			return true;
		};

		std::map<QString, VFrame30::SchemaDetailsSet> schemaDetails;	//	Key is subDir for schema

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
				joinResult &= joinSchemasFunc(schema, schema->joinLeftSchemaId().trimmed(), Qt::LeftEdge);
				joinResult &= joinSchemasFunc(schema, schema->joinRightSchemaId().trimmed(), Qt::RightEdge);

				joinResult &= joinSchemasFunc(schema, schema->joinTopSchemaId().trimmed(), Qt::TopEdge);
				joinResult &= joinSchemasFunc(schema, schema->joinBottomSchemaId().trimmed(), Qt::BottomEdge);
			}
			else
			{
				joinResult &= joinSchemasFunc(schema, schema->joinTopSchemaId().trimmed(), Qt::TopEdge);
				joinResult &= joinSchemasFunc(schema, schema->joinBottomSchemaId().trimmed(), Qt::BottomEdge);

				joinResult &= joinSchemasFunc(schema, schema->joinLeftSchemaId().trimmed(), Qt::LeftEdge);
				joinResult &= joinSchemasFunc(schema, schema->joinRightSchemaId().trimmed(), Qt::RightEdge);
			}

			if (joinResult == false)
			{
				returnResult = false;
			}

			//
			// Check all script on schemas
			//
			if (bool checkScriptResult = ScriptChecker::checkSchema(schema.get(), *log);
				checkScriptResult == false)
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

			// Write schema details for SchemaDetails by folder (Schemas.als/mvs/ufb...)
			//
			VFrame30::SchemaDetailsSet& sds = schemaDetails[subDir];
			sds.add(schema->details(filesTree.filePath(file->fileId())));

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

			if (bool parseOk = schemaFile->details.parseDetails(schema->details(filesTree.filePath(file->fileId())));
				parseOk == false)
			{
				log->errINT1001(tr("Parse schema details error."), schema->schemaId());
				returnResult = false;
				continue;
			}

			for (const QString& t : schemaTags)
			{
				m_schemaTagToFile.insert({t.toLower(), schemaFile});
			}
		}

		// Save
		//		Schemas.als/SchemaDetails.pbuf
		//		Schemas.ufb/SchemaDetails.pbuf
		//		...
		//
		for (const auto&[subDir, sds] : schemaDetails)
		{
			QByteArray fileData;

			if (bool ok = sds.saveToByteArray(&fileData);
				ok == false)
			{
				continue;
			}

			context->m_buildResultWriter->addFile(subDir, "SchemaDetails.pbuf", fileData);
		}

		// Generate VDU schemas in vdu-native format, save them to build result /VDU/Schemas
		//
		for (auto& [schemaId, fileSchema] : schemaMap)
		{
			std::shared_ptr<VFrame30::Schema>& schema = fileSchema.schema;
			Q_ASSERT(schema);

			if (schema->isVduSchema() == false)
			{
				continue;
			}

			bool convertOk = generateVduSchemas(static_cast<const VFrame30::VduSchema&>(*schema), *context);
			if (convertOk == false)
			{
				returnResult = false;
			}
		}

		return returnResult;
	}

	bool SoftwareCfgGenerator::generateVduSchemas(const VFrame30::VduSchema& schema, Context& context)
	{
		IssueLogger* log = context.m_log;
		Q_ASSERT(log);

		bool result = true;

		LOG_MESSAGE(log, tr("Converting schema %1 to VDU format.").arg(schema.schemaId())); 

		QByteArray data;
		QStringList errorMessages;
		result = vdu::VduSchemaGenerator::generateVduSchema(schema, data, errorMessages);

		QString fileName = QString("%1.%2")
							   .arg(schema.schemaId())
							   .arg(Db::File::VduNativeFileExtension);

		context.m_buildResultWriter->addFile(Directory::VDU + "/Schemas", fileName, data);

		return result;
	}

	bool SoftwareCfgGenerator::writeSchemaScriptProperties(VFrame30::Schema* schema, QString dir, BuildResultWriter* buildResultWriter)
	{
		Q_ASSERT(schema);
		Q_ASSERT(buildResultWriter);

		bool result = true;

		for (const auto& layer : schema->layers())
		{
			for (const SchemaItemPtr& item : layer->items())
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
		m_schemaTagToFile.clear();
		m_tuningSimIpPorts.clear();

		return;
	}

	void SoftwareCfgGenerator::writeSoftwareSection(QXmlStreamWriter& xmlWriter, bool finalizeSection)
	{
		TEST_PTR_RETURN(m_log);
		TEST_PTR_LOG_RETURN(m_software, m_log);
		TEST_PTR_LOG_RETURN(m_software->parent(), m_log);

		xmlWriter.writeStartElement(XmlElement::SOFTWARE);

		xmlWriter.writeAttribute(XmlAttribute::CAPTION, m_software->caption());
		xmlWriter.writeAttribute(XmlAttribute::EQUIPMENT_ID, m_software->equipmentIdTemplate());
		xmlWriter.writeAttribute(XmlAttribute::TYPE, QString("%1").arg(static_cast<int>(m_software->softwareType())));

		std::shared_ptr<Hardware::Workstation> ws = m_software->parent()->toWorkstation();

		if (ws == nullptr)
		{
			LOG_INTERNAL_ERROR_MSG(m_log, QString("Software %1 isn't placed in workstation.").arg(m_software->equipmentIdTemplate()));
			return;
		}

		xmlWriter.writeAttribute(XmlAttribute::WORKSTATION_EQUIPMENT_ID, ws->equipmentIdTemplate());

		xmlWriter.writeAttribute(XmlAttribute::SOFTWARE_CONTROLLERS,
								 m_softwareControllersIDs.join(Separator::COMMA));

		if (finalizeSection == true)
		{
			xmlWriter.writeEndElement();	// </Software>
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

	bool SoftwareCfgGenerator::checkLmToSoftwareLinks(Context* context)
	{
		TEST_PTR_RETURN_FALSE(context);

		IssueLogger* log = context->m_log;

		TEST_PTR_RETURN_FALSE(log);

		bool result = true;

		for(Hardware::DeviceModule* lm : context->m_lmModules)
		{
			std::shared_ptr<LmDescription> lmDescription = context->m_lmDescriptions->get(lm);

			if (lmDescription == nullptr)
			{
				LOG_INTERNAL_ERROR_MSG(log, QString("LmDescription is not found for module %1").arg(lm->equipmentIdTemplate()));
				result = false;
				continue;
			}

			const LmDescription::Lan& lan = lmDescription->lan();

			for(const LmDescription::LanController& lanController : lan.m_lanControllers)
			{
				LanControllerInfo lanControllerInfo;
				const Hardware::Software* software = nullptr;

				result &= LanControllerInfoHelper::getInfo(*lm, lanController.m_type, lanController.m_place,
														   *context, false, &lanControllerInfo, log);

				if (result == false)
				{
					continue;
				}

				if (lanControllerInfo.isTuningEnabled() == true)
				{
					if (lanControllerInfo.tuningServiceID.isEmpty() == true)
					{
						// Property '%1.%2' is empty.
						//
						log->errCFG3022(lanControllerInfo.equipmentID,
										EquipmentPropNames::TUNING_SERVICE_ID);
						result = false;
					}
					else
					{
						software = getConnectedSoftware(context,
														lanControllerInfo.tuningServiceID,
														true);
						if (software == nullptr)
						{
							// Property '%1.%2' is linked to undefined software ID '%3'.
							//
							log->errCFG3021(lanControllerInfo.equipmentID,
											EquipmentPropNames::TUNING_SERVICE_ID,
											lanControllerInfo.tuningServiceID);
							result = false;
						}
						else
						{
							if (software->softwareType() != E::SoftwareType::TuningService)
							{
								// Property '%1.%2' is linked to not compatible software '%3'.
								//
								log->errCFG3017(lanControllerInfo.equipmentID,
												EquipmentPropNames::TUNING_SERVICE_ID,
												lanControllerInfo.tuningServiceID);
								result = false;
							}
						}
					}
				}

				if (lanControllerInfo.isAppDataEnabled() == true)
				{
					if (lanControllerInfo.appDataServiceID.isEmpty() == true)
					{
						// Property '%1.%2' is empty.
						//
						log->errCFG3022(lanControllerInfo.equipmentID,
										EquipmentPropNames::APP_DATA_SERVICE_ID);
						result = false;
					}
					else
					{
						software = getConnectedSoftware(context,
														lanControllerInfo.appDataServiceID,
														false);

						if (software == nullptr)
						{
							// Property '%1.%2' is linked to undefined software ID '%3'.
							//
							log->errCFG3021(lanControllerInfo.equipmentID,
											EquipmentPropNames::APP_DATA_SERVICE_ID,
											lanControllerInfo.appDataServiceID);
							result = false;
						}
						else
						{
							if (software->softwareType() != E::SoftwareType::AppDataService)
							{
								// Property '%1.%2' is linked to not compatible software '%3'.
								//
								log->errCFG3017(lanControllerInfo.equipmentID,
												EquipmentPropNames::APP_DATA_SERVICE_ID,
												lanControllerInfo.appDataServiceID);
								result = false;
							}
						}
					}
				}

				if (lanControllerInfo.isDiagDataEnabled() == true)
				{
					if (lanControllerInfo.diagDataServiceID.isEmpty() == true)
					{
						// Property '%1.%2' is empty.
						//
						log->errCFG3022(lanControllerInfo.equipmentID,
										EquipmentPropNames::DIAG_DATA_SERVICE_ID);
						result = false;
					}
					else
					{
						software = getConnectedSoftware(context,
														lanControllerInfo.diagDataServiceID,
														false);

						if (software == nullptr)
						{
							// Property '%1.%2' is linked to undefined software ID '%3'.
							//
							log->errCFG3021(lanControllerInfo.equipmentID,
											EquipmentPropNames::DIAG_DATA_SERVICE_ID,
											lanControllerInfo.diagDataServiceID);
							result = false;
						}
						else
						{
							if (software->softwareType() != E::SoftwareType::DiagDataService)
							{
								// Property '%1.%2' is linked to not compatible software '%3'.
								//
								log->errCFG3017(lanControllerInfo.equipmentID,
												EquipmentPropNames::DIAG_DATA_SERVICE_ID,
												lanControllerInfo.diagDataServiceID);
								result = false;
							}
						}
					}
				}
			}
		}

		return result;
	}

	const Hardware::Software* SoftwareCfgGenerator::getConnectedSoftware(const Context* context,
																   const QString& equipmentID,
																   bool checkConnectionToControllers)
	{
		auto it = context->m_software.find(equipmentID);

		if (it != context->m_software.end())
		{
			return it->second;
		}

		if (checkConnectionToControllers == false)
		{
			return nullptr;
		}

		auto device = context->m_equipmentSet->deviceObject(equipmentID);

		if (device == nullptr)
		{
			return nullptr;
		}

		if (device->isController() == false)
		{
			return nullptr;
		}

		return device->getParentSoftware();
	}

	bool SoftwareCfgGenerator::joinSchemas(Context* context, VFrame30::Schema* schema, const VFrame30::Schema* panel, Qt::Edge edge)
	{
		if (context == nullptr || schema == nullptr || panel == nullptr)
		{
			Q_ASSERT(context);
			Q_ASSERT(schema);
			Q_ASSERT(panel);
			return false;
		}

		IssueLogger* log = context->m_log;

		if (schema->unit() != panel->unit())
		{
			log->errALP4082(schema->schemaId(), panel->schemaId());
			return false;
		}

		if (schema->schemaId() == panel->schemaId())
		{
			log->errALP4081(schema->schemaId());
			return false;
		}

		// Expand schema and calc rectangles
		//
		QRectF schemaRect;		// New rect for existing items
		QRectF panelRect;		// Rect to move panel items yo

		switch (edge)
		{
		case Qt::Edge::LeftEdge:
		case Qt::Edge::RightEdge:
			{
				schemaRect = QRectF{0, 0, schema->docWidth(), schema->docHeight()};
				panelRect = QRectF{0, 0, panel->docWidth(), panel->docHeight()};

				schema->setDocWidth(schema->docWidth() + panel->docWidth());
				schema->setDocHeight(std::max(schema->docHeight(), panel->docHeight()));

				if (edge == Qt::Edge::LeftEdge)
				{
					schemaRect.moveRight(schema->docWidth());
				}
				else // edge == Qt::Edge::RightEdge)
				{
					panelRect.moveRight(schema->docWidth());
				}
			}
			break;

		case Qt::Edge::TopEdge:
		case Qt::Edge::BottomEdge:
			{
				schemaRect = QRectF{0, 0, schema->docWidth(), schema->docHeight()};
				panelRect = QRectF{0, 0, panel->docWidth(), panel->docHeight()};

				schema->setDocWidth(std::max(schema->docWidth(), panel->docWidth()));
				schema->setDocHeight(schema->docHeight() + panel->docHeight());

				if (edge == Qt::Edge::TopEdge)
				{
					schemaRect.moveBottom(schema->docHeight());
				}
				else // edge == Qt::Edge::BottomEdge)
				{
					panelRect.moveBottom(schema->docHeight());
				}
			}
			break;

		default:
			Q_ASSERT(false);
			log->errINT1000(tr("Edge param error, edge %1,  function %2").arg(edge).arg(Q_FUNC_INFO));
			return false;
		}

		Q_ASSERT(schemaRect.isNull() == false);
		Q_ASSERT(panelRect.isNull() == false);

		// Move schema items to new pos in schemaRect
		//
		if (schemaRect.topLeft().isNull() == false)
		{
			for (const auto& layer :  schema->layers())
			{
				if (layer == nullptr)
				{
					Q_ASSERT(layer);
					log->errINT1000(QString("Layer is nullptr for schema %1: %2").arg(schema->schemaId()).arg(Q_FUNC_INFO));
					return false;
				}

				for (const SchemaItemPtr& item :  layer->items())
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

		// Add panel items to panelRect
		//
		for (const auto& panelLayer : panel->layers())
		{
			Q_ASSERT(panelLayer);

			auto foundDestLayerIt = std::find_if(schema->layers().begin(), schema->layers().end(),
												 [panelLayer](auto l) { return l->name() == panelLayer->name(); } );

			if (foundDestLayerIt == schema->layers().end())
			{
				// Source layer is not found in destination, copy to compile layer,
				// if compile layer does not exists either, then copy to the first layer
				//
				foundDestLayerIt = std::find_if(schema->layers().begin(), schema->layers().end(),
												[](auto l) { return l->compile(); } );

				if (foundDestLayerIt == schema->layers().end())
				{
					foundDestLayerIt = schema->layers().begin();
				}
			}

			Q_ASSERT(foundDestLayerIt != schema->layers().end());

			// Copy all form sourceLayer to destLayer, keep the order of items and insert all them right at the end of items
			//
			std::shared_ptr<VFrame30::SchemaLayer> destLayer = *foundDestLayerIt;

			if (destLayer == nullptr)
			{
				Q_ASSERT(destLayer);
				return false;
			}

			for (const SchemaItemPtr& sourceItem : panelLayer->items())
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

				// Insert newItem to destination schema layer
				//
				newItem->moveItem(panelRect.left(), panelRect.top());

				// --
				//
				destLayer->pushBackItem(newItem);
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

	QString SoftwareCfgGenerator::getBuildInfoComments(E::OS os) const
	{
		QString commStart = getCommentStart(os);

		OnlineLib::BuildInfo b = m_buildResultWriter->buildInfo();

		QString comments;

		comments += commStart + " Project: " + b.project + "\n";
		comments += commStart + " BuildNo: " + QString::number(b.id) + "\n";
		comments += commStart + " Date: " + b.dateStr() + "\n";
		comments += commStart + " Changeset: " + QString::number(b.changeset) + "\n";
		comments += commStart + " User: " + b.user + "\n";
		comments += commStart + " Workstation: " + b.workstation + "\n\n";

		return comments;
	}

	QString SoftwareCfgGenerator::getCommonCmdLine(const HostAddressPort& cfgSrvIp1,
													const HostAddressPort& cfgSrvIp2,
													E::OS os,
													bool runAsConsoleApp)
	{
		static const std::map<E::SoftwareType, QString> servicesBins =
		{
			{ E::SoftwareType::AppDataService, QString("AppDataSrv") },
			{ E::SoftwareType::ArchiveService, QString("ArchSrv") },
			{ E::SoftwareType::DiagDataService, QString("DiagDataSrv") },
			{ E::SoftwareType::TuningService, QString("TuningSrv") },
			{ E::SoftwareType::TestClient, QString("TestClient") },
			{ E::SoftwareType::TuningClient, QString("TuningClient") },
			{ E::SoftwareType::Monitor, QString("Monitor") },
			{ E::SoftwareType::Metrology, QString("Metrology") },
			{ E::SoftwareType::GatewayService, QString("GatewaySrv") },
		};

		auto it = servicesBins.find(m_software->softwareType());

		if (it == servicesBins.end())
		{
			LOG_INTERNAL_ERROR_MSG(m_log, "Exe-file name not found for software type " + E::valueToString<E::SoftwareType>(m_software->softwareType()));
			return QString();
		}

		QString cmdLine;

		switch(os)
		{
		case E::OS::Windows:
			cmdLine = it->second + ".exe";
			break;

		case E::OS::Linux:
			cmdLine = "./" + it->second;
			break;

		default:
			Q_ASSERT(false);
			return QString();
		}

		if (runAsConsoleApp == true)
		{
			cmdLine += " -e";
		}

		if (cfgSrvIp1.isEmpty() == true && cfgSrvIp2.isEmpty() == true)
		{
			m_log->errALC5140(m_software->equipmentIdTemplate());
			return QString();
		}

		cmdLine += " -cfgip1=" + (cfgSrvIp1.isEmpty() == false ? cfgSrvIp1.addressPortStr() :  cfgSrvIp2.addressPortStr());
		cmdLine += " -cfgip2=" + (cfgSrvIp2.isEmpty() == false ? cfgSrvIp2.addressPortStr() :  cfgSrvIp1.addressPortStr());

		cmdLine += " -id=" + m_software->equipmentIdTemplate();

		return cmdLine;
	}

	QString SoftwareCfgGenerator::getCommentStart(E::OS os) const
	{
		switch(os)
		{
		case E::OS::Windows:
			return "@rem";

		case E::OS::Linux:
			return "#";

		default:
			Q_ASSERT(false);
		}

		return QString();
	}

	QString SoftwareCfgGenerator::getRunScriptDirectory(E::OS os) const
	{
		switch(os)
		{
		case E::OS::Windows:
			return Directory::RUN_SERVICE_SCRIPTS_WINDOWS;

		case E::OS::Linux:
			return Directory::RUN_SERVICE_SCRIPTS_LINUX;

		default:
			Q_ASSERT(false);
		}

		return QString();
	}

	QString SoftwareCfgGenerator::getRunScriptName(const QString& profile, E::OS os) const
	{
		QString extention;

		switch(os)
		{
		case E::OS::Windows:
			extention = "bat";
			break;

		case E::OS::Linux:
			extention = "sh";
			break;

		default:
			Q_ASSERT(false);
			return QString();
		}

		return profile + "_" + m_software->equipmentIdTemplate().toLower() + "." + extention;
	}

	bool SoftwareCfgGenerator::writeMatsUsers(const QString& propertyName, const QStringList& tuningUserAccounts)
	{
		Builder::DbMatsUserStorage storage;

		QString errorCode;
		if (storage.load(m_dbController, errorCode) == false)
		{
			m_log->errCMN0010(File::MATSUSERS_XML);
			return false;
		}

		// Check if TuningUserAccounts exist in MATS users storage
		//
		for (const QString& user : tuningUserAccounts)
		{
			if (std::find_if(storage.users().begin(), storage.users().end(), [&user](const OnlineLib::MatsUser& matsUser)
							 {
								 return matsUser.login() == user;
							 }) == storage.users().end())
			{
				m_log->errEQP6212(propertyName, user, m_software->equipmentIdTemplate());
				return false;
			}
		}

		// Save MATS users to XML
		//
		QByteArray data;
		storage.saveToByteArray(data);

		// Write file
		//
		BuildFile* buildFile = m_buildResultWriter->addFile(m_software->equipmentIdTemplate(), File::MATSUSERS_XML, CfgFileId::MATSUSERS, "", data);
		if (buildFile == nullptr)
		{
			m_log->errCMN0012(File::MATSUSERS_XML);
			return false;
		}

		bool ok = m_cfgXml->addLinkToFile(buildFile);
		return ok;
	}
}


