#include "../Builder/SoftwareCfgGenerator.h"
#include "../Builder/ApplicationLogicCompiler.h"
#include "IssueLogger.h"
#include "../lib/DeviceHelper.h"
#include "../lib/ServiceSettings.h"
#include "../VFrame30/LogicSchema.h"


namespace Builder
{
	// ---------------------------------------------------------------------------------
	//
	//	SoftwareCfgGenerator class implementation
	//
	// ---------------------------------------------------------------------------------

	HashedVector<QString, Hardware::DeviceModule*> SoftwareCfgGenerator::m_lmList;
	HashedVector<QString, Hardware::Software*> SoftwareCfgGenerator::m_softwareList;
	QList<SoftwareCfgGenerator::SchemaFile> SoftwareCfgGenerator::m_schemaFileList;

	SoftwareCfgGenerator::SoftwareCfgGenerator(DbController* db,
												Hardware::Software* software,
												SignalSet* signalSet,
												Hardware::EquipmentSet* equipment,
												BuildResultWriter* buildResultWriter) :
		m_dbController(db),
		m_software(software),
		m_signalSet(signalSet),
		m_equipment(equipment),
		m_buildResultWriter(buildResultWriter)
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
					  QString(tr("Can't create 'configuration.xml' file for software %1")).
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


	bool SoftwareCfgGenerator::generalSoftwareCfgGeneration(DbController* db, SignalSet* signalSet, Hardware::EquipmentSet* equipment, BuildResultWriter* buildResultWriter)
	{
		if (buildResultWriter == nullptr)
		{
			assert(false);
			return false;
		}

		IssueLogger* log = buildResultWriter->log();

		if (log == nullptr)
		{
			assert(false);
			return false;
		}

		if (db == nullptr ||
			signalSet == nullptr ||
			equipment == nullptr)
		{
			LOG_INTERNAL_ERROR(log);
			assert(false);
			return false;
		}

		bool result = true;

		// add general software cfg generation here:
		//

		result &= buildLmList(equipment, log);

		result &= buildSoftwareList(equipment, log);

		result &= checkLmToSoftwareLinks(log);

		// Add Schemas to Build result
		//
		result &= writeSchemas(db, buildResultWriter, log);

		return result;
	}

	bool SoftwareCfgGenerator::writeSchemas(DbController* db, BuildResultWriter* buildResultWriter, IssueLogger* log)
	{
		if (db == nullptr ||
			buildResultWriter == nullptr ||
			log == nullptr)
		{
			assert(db);
			assert(buildResultWriter);
			assert(log);
			return false;
		}

		// Get all Application Logic schemas
		//
		m_schemaFileList.clear();	// m_schemaFileList is filled in next two calls of SoftwareCfgGenerator::writeSchemasList

		bool result = true;
		result &= writeSchemasList(db, buildResultWriter, db->alFileId(), QLatin1String(".") + ::AlFileExtension, "LogicSchemas", "LogicSchema", log);

		// Write LogicSchemas/SchemaDetais.pbuf
		//
		result &= writeAppLogicSchemasDetails(m_schemaFileList, buildResultWriter, "LogicSchemas", log);

		// Get all Monitor schemas
		//
		result &= writeSchemasList(db, buildResultWriter, db->mvsFileId(), QLatin1String(".") + ::MvsFileExtension, "MonitorSchemas", "MonitorSchema", log);

		return result;
	}

	bool SoftwareCfgGenerator::writeSchemasList(DbController* db, BuildResultWriter* buildResultWriter, int parentFileId, QString fileExtension, QString subDir, QString group, IssueLogger* log)
	{
		if (db == nullptr ||
			buildResultWriter == nullptr ||
			log == nullptr)
		{
			assert(db);
			assert(buildResultWriter);
			assert(log);
			return false;
		}

		// Get File list
		//
		std::vector<DbFileInfo> fileList;

		bool returnResult = true;
		bool result = false;

		if (buildResultWriter->isRelease() == true)
		{
			// To Do getting files for release
			//
			assert(false);
			returnResult = false;
		}
		else
		{
			 result = db->getFileList(&fileList, parentFileId, fileExtension, true, nullptr);
			 if (result == false)
			 {
				 log->errPDB2001(parentFileId, fileExtension, db->lastError());
				 return false;
			 }
		}

		// Get file instance and parse it
		//
		for (const DbFileInfo& f : fileList)
		{
			if (f.action() == VcsItemAction::Deleted)		// File is deleted, it can be in Debug build
			{
				qDebug() << "Skip file " << f.fileId() << ", " << f.fileName() << ", it was marked as deleted";
				continue;
			}

			std::shared_ptr<DbFile> file;

			result = db->getLatestVersion(f, &file, nullptr);
			if (result == false || file.get() == nullptr)
			{
				log->errPDB2002(f.fileId(), f.fileName(), db->lastError());
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

			// Add file to build result
			//
			result = buildResultWriter->addFile(subDir, schema->schemaId() + fileExtension, schema->schemaId(), group, file->data());
			if (result == false)
			{
				returnResult = false;
				continue;
			}

			SchemaFile schemaFile;

			schemaFile.id = schema->schemaId();
			schemaFile.subDir = subDir;
			schemaFile.fileName = schema->schemaId() + fileExtension;		// File is stored under this name
			schemaFile.group = group;
			schemaFile.details = schema->details();

			m_schemaFileList.push_back(schemaFile);
		}

		return returnResult;
	}

	bool SoftwareCfgGenerator::writeAppLogicSchemasDetails(const QList<SchemaFile>& schemaFiles, BuildResultWriter* buildResultWriter, QString dir, IssueLogger* log)
	{
		if (buildResultWriter == nullptr ||
			log == nullptr)
		{
			assert(false);
			return false;
		}

		bool ok = true;
		bool result = true;

		VFrame30::SchemaDetailsSet sds;
		for (const SchemaFile& sf : schemaFiles)
		{
			std::shared_ptr<VFrame30::SchemaDetails> details = std::make_shared<VFrame30::SchemaDetails>();

			ok = details->parseDetails(sf.details);
			result &= ok;

			if (ok == false)
			{
				log->errINT1001(tr("Parse schema detais error."), sf.id);
			}

			sds.add(details);
		}

		QByteArray fileData;
		sds.Save(fileData);

		buildResultWriter->addFile(dir, "SchemaDetails.pbuf", fileData, false);

		return result;
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
							log->wrnCFG3016(adapterProperties.adapterID,
											DataSource::LmEthernetAdapterProperties::PROP_TUNING_SERVICE_ID);
						}
						else
						{
							if (m_softwareList.contains(adapterProperties.tuningServiceID) == false)
							{
								log->wrnCFG3015(adapterProperties.adapterID,
												DataSource::LmEthernetAdapterProperties::PROP_TUNING_SERVICE_ID,
												adapterProperties.tuningServiceID);
							}
							else
							{
								software = m_softwareList[adapterProperties.tuningServiceID];

								if (software->type() != E::SoftwareType::TuningService)
								{
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
							log->wrnCFG3016(adapterProperties.adapterID,
											DataSource::LmEthernetAdapterProperties::PROP_APP_DATA_SERVICE_ID);
						}
						else
						{
							if (m_softwareList.contains(adapterProperties.appDataServiceID) == false)
							{
								log->wrnCFG3015(adapterProperties.adapterID,
												DataSource::LmEthernetAdapterProperties::PROP_APP_DATA_SERVICE_ID,
												adapterProperties.appDataServiceID);
							}
							else
							{
								software = m_softwareList[adapterProperties.appDataServiceID];

								if (software->type() != E::SoftwareType::AppDataService)
								{
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
							log->wrnCFG3016(adapterProperties.adapterID,
											DataSource::LmEthernetAdapterProperties::PROP_DIAG_DATA_SERVICE_ID);
						}
						else
						{
							if (m_softwareList.contains(adapterProperties.diagDataServiceID) == false)
							{
								log->wrnCFG3015(adapterProperties.adapterID,
												DataSource::LmEthernetAdapterProperties::PROP_DIAG_DATA_SERVICE_ID,
												adapterProperties.diagDataServiceID);
							}
							else
							{
								software = m_softwareList[adapterProperties.diagDataServiceID];

								if (software->type() != E::SoftwareType::DiagDataService)
								{
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

		parameters += " -id=" + m_software->equipmentIdTemplate() + "\n";

		return true;
	}

}


