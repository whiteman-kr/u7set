#include "../Builder/SoftwareCfgGenerator.h"
#include "../Builder/ApplicationLogicCompiler.h"
#include "IssueLogger.h"
#include "../lib/DeviceHelper.h"
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
		m_schemaFileList.clear();

		bool result = true;
		result &= writeSchemasList(db, buildResultWriter, db->alFileId(), QLatin1String(".") + ::AlFileExtension, "LogicSchemas", "LogicSchema", log);

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

			qDebug() << "Build: schema " << schema->schemaId() << " is loaded";

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
			for(int adapter = LM_ETHERNET_ADAPTER1; adapter <= LM_ETHERNET_ADAPTER3; adapter++)
			{
				LmEthernetAdapterNetworkProperties adapterProperties;
				Hardware::Software* software = nullptr;

				result &= adapterProperties.getLmEthernetAdapterNetworkProperties(lm, adapter, log);

				if (result == false)
				{
					break;
				}

				if (adapter ==  LM_ETHERNET_ADAPTER1)
				{
					// tuning adapter
					//
					if (adapterProperties.tuningEnable == true)
					{
						if (adapterProperties.tuningServiceID.isEmpty() == true)
						{
							log->wrnCFG3016(adapterProperties.adapterID,
											LmEthernetAdapterNetworkProperties::PROP_TUNING_SERVICE_ID);
						}
						else
						{
							if (m_softwareList.contains(adapterProperties.tuningServiceID) == false)
							{
								log->wrnCFG3015(adapterProperties.adapterID,
												LmEthernetAdapterNetworkProperties::PROP_TUNING_SERVICE_ID,
												adapterProperties.tuningServiceID);
							}
							else
							{
								software = m_softwareList[adapterProperties.tuningServiceID];

								if (software->type() != E::SoftwareType::TuningService)
								{
									log->errCFG3017(adapterProperties.adapterID,
													LmEthernetAdapterNetworkProperties::PROP_TUNING_SERVICE_ID,
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
											LmEthernetAdapterNetworkProperties::PROP_APP_DATA_SERVICE_ID);
						}
						else
						{
							if (m_softwareList.contains(adapterProperties.appDataServiceID) == false)
							{
								log->wrnCFG3015(adapterProperties.adapterID,
												LmEthernetAdapterNetworkProperties::PROP_APP_DATA_SERVICE_ID,
												adapterProperties.appDataServiceID);
							}
							else
							{
								software = m_softwareList[adapterProperties.appDataServiceID];

								if (software->type() != E::SoftwareType::AppDataService)
								{
									log->errCFG3017(adapterProperties.adapterID,
													LmEthernetAdapterNetworkProperties::PROP_APP_DATA_SERVICE_ID,
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
											LmEthernetAdapterNetworkProperties::PROP_DIAG_DATA_SERVICE_ID);
						}
						else
						{
							if (m_softwareList.contains(adapterProperties.diagDataServiceID) == false)
							{
								log->wrnCFG3015(adapterProperties.adapterID,
												LmEthernetAdapterNetworkProperties::PROP_DIAG_DATA_SERVICE_ID,
												adapterProperties.diagDataServiceID);
							}
							else
							{
								software = m_softwareList[adapterProperties.diagDataServiceID];

								if (software->type() != E::SoftwareType::DiagDataService)
								{
									log->errCFG3017(adapterProperties.adapterID,
													LmEthernetAdapterNetworkProperties::PROP_DIAG_DATA_SERVICE_ID,
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

	// ---------------------------------------------------------------------------------
	//
	//	SoftwareCfgGenerator::LmEthernetAdapterNetworkProperties class implementation
	//
	// ---------------------------------------------------------------------------------

	const char* SoftwareCfgGenerator::LmEthernetAdapterNetworkProperties::PROP_TUNING_ENABLE = "TuningEnable";
	const char* SoftwareCfgGenerator::LmEthernetAdapterNetworkProperties::PROP_TUNING_IP = "TuningIP";
	const char* SoftwareCfgGenerator::LmEthernetAdapterNetworkProperties::PROP_TUNING_PORT = "TuningPort";
	const char* SoftwareCfgGenerator::LmEthernetAdapterNetworkProperties::PROP_TUNING_SERVICE_ID = "TuningServiceID";

	const char* SoftwareCfgGenerator::LmEthernetAdapterNetworkProperties::PROP_APP_DATA_ENABLE = "AppDataEnable";
	const char* SoftwareCfgGenerator::LmEthernetAdapterNetworkProperties::PROP_APP_DATA_IP = "AppDataIP";
	const char* SoftwareCfgGenerator::LmEthernetAdapterNetworkProperties::PROP_APP_DATA_PORT = "AppDataPort";
	const char* SoftwareCfgGenerator::LmEthernetAdapterNetworkProperties::PROP_APP_DATA_SERVICE_ID = "AppDataServiceID";

	const char* SoftwareCfgGenerator::LmEthernetAdapterNetworkProperties::PROP_DIAG_DATA_ENABLE = "DiagDataEnable";
	const char* SoftwareCfgGenerator::LmEthernetAdapterNetworkProperties::PROP_DIAG_DATA_IP = "DiagDataIP";
	const char* SoftwareCfgGenerator::LmEthernetAdapterNetworkProperties::PROP_DIAG_DATA_PORT = "DiagDataPort";
	const char* SoftwareCfgGenerator::LmEthernetAdapterNetworkProperties::PROP_DIAG_DATA_SERVICE_ID = "DiagDataServiceID";

	const char* SoftwareCfgGenerator::LmEthernetAdapterNetworkProperties::LM_ETHERNET_CONROLLER_SUFFIX_FORMAT_STR = "_ETHERNET0%1";

	bool SoftwareCfgGenerator::LmEthernetAdapterNetworkProperties::getLmEthernetAdapterNetworkProperties(const Hardware::DeviceModule* lm, int adptrNo, IssueLogger* log)
	{
		if (log == nullptr)
		{
			assert(false);
			return false;
		}

		if (lm == nullptr)
		{
			LOG_INTERNAL_ERROR(log);
			assert(false);
			return false;
		}

		if (adptrNo < LM_ETHERNET_ADAPTER1 ||
			adptrNo > LM_ETHERNET_ADAPTER3)
		{
			LOG_INTERNAL_ERROR(log);
			assert(false);
			return false;
		}

		adapterNo = adptrNo;

		QString suffix = QString(LM_ETHERNET_CONROLLER_SUFFIX_FORMAT_STR).arg(adapterNo);

		Hardware::DeviceController* adapter = DeviceHelper::getChildControllerBySuffix(lm, suffix, log);

		if (adapter == nullptr)
		{
			return false;
		}

		adapterID = adapter->equipmentIdTemplate();

		bool result = true;

		if (adptrNo == LM_ETHERNET_ADAPTER1)
		{
			// tunig adapter
			//
			result &= DeviceHelper::getBoolProperty(adapter, PROP_TUNING_ENABLE, &tuningEnable, log);
			result &= DeviceHelper::getStrProperty(adapter, PROP_TUNING_IP, &tuningIP, log);
			result &= DeviceHelper::getIntProperty(adapter, PROP_TUNING_PORT, &tuningPort, log);
			result &= DeviceHelper::getStrProperty(adapter, PROP_TUNING_SERVICE_ID, &tuningServiceID, log);
			return result;
		}

		if (adptrNo == LM_ETHERNET_ADAPTER2 ||
			adptrNo == LM_ETHERNET_ADAPTER3)
		{
			// application and diagnostics data adapter
			//
			result &= DeviceHelper::getBoolProperty(adapter, PROP_APP_DATA_ENABLE, &appDataEnable, log);
			result &= DeviceHelper::getStrProperty(adapter, PROP_APP_DATA_IP, &appDataIP, log);
			result &= DeviceHelper::getIntProperty(adapter, PROP_APP_DATA_PORT, &appDataPort, log);
			result &= DeviceHelper::getStrProperty(adapter, PROP_APP_DATA_SERVICE_ID, &appDataServiceID, log);

			result &= DeviceHelper::getBoolProperty(adapter, PROP_DIAG_DATA_ENABLE, &diagDataEnable, log);
			result &= DeviceHelper::getStrProperty(adapter, PROP_DIAG_DATA_IP, &diagDataIP, log);
			result &= DeviceHelper::getIntProperty(adapter, PROP_DIAG_DATA_PORT, &diagDataPort, log);
			result &= DeviceHelper::getStrProperty(adapter, PROP_DIAG_DATA_SERVICE_ID, &diagDataServiceID, log);

			return result;
		}

		assert(false);
		return false;
	}



}


