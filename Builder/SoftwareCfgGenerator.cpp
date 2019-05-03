#include "SoftwareCfgGenerator.h"
#include "ApplicationLogicCompiler.h"
#include "IssueLogger.h"
#include "../lib/DeviceHelper.h"
#include "../lib/ServiceSettings.h"
#include "../VFrame30/Schema.h"


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
		DbController& db = context->m_db;
		IssueLogger* log = context->m_log;

		DbFileTree filesTree;									// Filed in loadAllSchemas

		if (bool ok = db.getFileListTree(&filesTree, db.schemaFileId(), "%", true, nullptr);
			ok == false)
		{
			log->errPDB2001(db.schemaFileId(), "%", db.lastError());
			return false;
		}

		filesTree.removeIf([](const DbFileInfo& f)
			{
				return f.action() == VcsItemAction::Deleted;
			});

		// Remove all unsuported fileas and marked for deleting
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

		bool returnResult = true;

		QString group = "Schema";

		for (const DbFileInfo& f : files)
		{
			QString subDir = "Schemas." + f.extension();

			std::shared_ptr<DbFile> file;

			if (bool ok  = db.getLatestVersion(f, &file, nullptr);
				ok == false || file.get() == nullptr)
			{
				log->errPDB2002(f.fileId(), f.fileName(), db.lastError());
				returnResult = false;
				continue;
			}

			// Parse file
			//
			std::shared_ptr<VFrame30::Schema> schema = VFrame30::Schema::Create(file->data());

			if (schema == nullptr)
			{
				log->errCMN0010(f.fileName());
				returnResult = false;
				continue;
			}

			//qDebug() << "Build: schema " << schema->schemaId() << " is loaded";

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

			if (schema->excludeFromBuild() == true)
			{
				continue;
			}

			// Add file to build result
			//
			if (bool ok = context->m_buildResultWriter->addFile(subDir, file->fileName(), schema->schemaId(), schema->tagsAsList().join(";"), file->data(), false);
				ok == false)
			{
				returnResult = false;
				continue;
			}

			// --
			//
			QStringList schemaTags = schema->tagsAsList();
			for (const QString& t : schemaTags)
			{
				m_schemaTagToFile.insert({t.toLower(), schemaFile});
			}
		}

		return returnResult;
	}


//	bool SoftwareCfgGenerator::writeAppLogicSchemasDetails(const QList<SchemaFile>& schemaFiles, BuildResultWriter* buildResultWriter, QString dir, IssueLogger* log)
//	{
//		if (buildResultWriter == nullptr ||
//			log == nullptr)
//		{
//			assert(false);
//			return false;
//		}

//		bool result = true;

//		VFrame30::SchemaDetailsSet sds;
//		for (const SchemaFile& sf : schemaFiles)
//		{
//			std::shared_ptr<VFrame30::SchemaDetails> details = std::make_shared<VFrame30::SchemaDetails>(sf.details);
//			sds.add(details);
//		}

//		QByteArray fileData;
//		sds.saveToByteArray(&fileData);

//		buildResultWriter->addFile(dir, "SchemaDetails.pbuf", fileData, false);

//		return result;
//	}

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

}


