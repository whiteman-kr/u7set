#include "../Builder/SoftwareCfgGenerator.h"
#include "../Builder/ApplicationLogicCompiler.h"
#include "IssueLogger.h"
#include "../include/DeviceHelper.h"
#include "../VFrame30/LogicSchema.h"


namespace Builder
{
	HashedVector<QString, Hardware::DeviceModule*> SoftwareCfgGenerator::m_lmList;
	HashedVector<QString, Hardware::Software*> SoftwareCfgGenerator::m_softwareList;
	QList<SoftwareCfgGenerator::SchemaFile> SoftwareCfgGenerator::m_schemaFileList;


	SoftwareCfgGenerator::SoftwareCfgGenerator(DbController* db, Hardware::Software* software, SignalSet* signalSet, Hardware::EquipmentSet* equipment, BuildResultWriter* buildResultWriter) :
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
			LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined,
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
		result &= writeSchemasList(db, buildResultWriter, db->alFileId(), AlFileExtension, "LogicSchemas", "LogicSchema", log);

		// Get all Monitor schemas
		//
		result &= writeSchemasList(db, buildResultWriter, db->mvsFileId(), MvsFileExtension, "MonitorSchemas", "MonitorSchema", log);

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
			VFrame30::Schema* schemaRawPtr = VFrame30::Schema::Create(file->data());
			std::shared_ptr<VFrame30::Schema> schema(schemaRawPtr);

			if (schemaRawPtr == false)
			{
				log->errCMN0010(f.fileName());
				returnResult = false;
				continue;
			}

			qDebug() << "Build: schema " << schema->schemaID() << " is loaded";

			// Add file to build result
			//
			result = buildResultWriter->addFile(subDir, schema->schemaID() + "." + fileExtension, group, file->data());
			if (result == false)
			{
				returnResult = false;
				continue;
			}

			SchemaFile schemaFile;

			schemaFile.id = schema->schemaID();
			schemaFile.subDir = subDir;
			schemaFile.fileName = schema->schemaID() + "." + fileExtension;		// File is stored under this name
			schemaFile.group = group;

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

				if (module->isLM() == false)
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
			LOG_ERROR_OBSOLETE(log, IssuePrexif::NotDefined, QString(tr("Can't build Logic Modules list")));
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
			LOG_MESSAGE(log, QString(tr("Logic Modules list building... OK")));
		}
		else
		{
			LOG_ERROR_OBSOLETE(log, IssuePrexif::NotDefined, QString(tr("Can't build Logic Modules list")));
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
					if (adapterProperties.tuningServiceID.isEmpty() == true)
					{
						LOG_WARNING_OBSOLETE(log, IssueType::NotDefined,
											 QString(tr("Adapter property '%1.TuningServiceID' is empty")).
											 arg(adapterProperties.adapterID));
						continue;
					}

					if (m_softwareList.contains(adapterProperties.tuningServiceID) == false)
					{
						LOG_WARNING_OBSOLETE(log, IssueType::NotDefined,
											 QString(tr("Adapter property '%1.TuningServiceID' is linked to undefined softwareID '%2'")).
											 arg(adapterProperties.adapterID).arg(adapterProperties.tuningServiceID));
						continue;
					}

					software = m_softwareList[adapterProperties.tuningServiceID];

					if (software->type() != E::SoftwareType::TuningService)
					{
						LOG_ERROR_OBSOLETE(log, IssueType::NotDefined,
										 QString(tr("Adapter property '%1.TuningServiceID' linked to not suitable software '%2'")).
										 arg(adapterProperties.adapterID).arg(adapterProperties.tuningServiceID));
						result = false;
						continue;
					}
				}
				else
				{
					// app and diag data adapter

					// test appDataServiceID property
					//
					if (adapterProperties.appDataServiceID.isEmpty() == true)
					{
						LOG_WARNING_OBSOLETE(log, IssueType::NotDefined,
											 QString(tr("Adapter property '%1.AppDataServiceID' is empty")).
											 arg(adapterProperties.adapterID));
						continue;
					}

					if (m_softwareList.contains(adapterProperties.appDataServiceID) == false)
					{
						LOG_WARNING_OBSOLETE(log, IssueType::NotDefined,
											 QString(tr("Adapter property '%1.AppDataServiceID' is linked to undefined softwareID '%2'")).
											 arg(adapterProperties.adapterID).arg(adapterProperties.appDataServiceID));
						continue;
					}

					software = m_softwareList[adapterProperties.appDataServiceID];

					if (software->type() != E::SoftwareType::DataAcquisitionService)
					{
						LOG_ERROR_OBSOLETE(log, IssueType::NotDefined,
										 QString(tr("Adapter property '%1.AppDataServiceID' linked to not suitable software '%2'")).
										 arg(adapterProperties.adapterID).arg(adapterProperties.appDataServiceID));
						result = false;
						continue;
					}

					// test diagDataServiceID property
					//
					if (adapterProperties.diagDataServiceID.isEmpty() == true)
					{
						LOG_WARNING_OBSOLETE(log, IssueType::NotDefined,
											 QString(tr("Adapter property '%1.DiagDataServiceID' is empty")).
											 arg(adapterProperties.adapterID));
						continue;
					}

					if (m_softwareList.contains(adapterProperties.diagDataServiceID) == false)
					{
						LOG_WARNING_OBSOLETE(log, IssueType::NotDefined,
											 QString(tr("Adapter property '%1.DiagDataServiceID' is linked to undefined softwareID '%2'")).
											 arg(adapterProperties.adapterID).arg(adapterProperties.diagDataServiceID));
						continue;
					}

					software = m_softwareList[adapterProperties.diagDataServiceID];

					if (software->type() != E::SoftwareType::DataAcquisitionService)
					{
						LOG_ERROR_OBSOLETE(log, IssueType::NotDefined,
										 QString(tr("Adapter property '%1.DiagDataServiceID' linked to not suitable software '%2'")).
										 arg(adapterProperties.adapterID).arg(adapterProperties.diagDataServiceID));
						result = false;
						continue;
					}
				}
			}
		}

		return result;
	}


	bool SoftwareCfgGenerator::LmEthernetAdapterNetworkProperties::getLmEthernetAdapterNetworkProperties(Hardware::DeviceModule* lm, int adptrNo, IssueLogger* log)
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

		QString suffix = QString("_ETHERNET0%1").arg(adapterNo);

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
			result &= DeviceHelper::getBoolProperty(adapter, "TuningEnable", &tuningEnable, log);
			result &= DeviceHelper::getStrProperty(adapter, "TuningIP", &tuningIP, log);
			result &= DeviceHelper::getIntProperty(adapter, "TuningPort", &tuningPort, log);
			result &= DeviceHelper::getStrProperty(adapter, "TuningServiceID", &tuningServiceID, log);
			return result;
		}

		if (adptrNo == LM_ETHERNET_ADAPTER2 ||
			adptrNo == LM_ETHERNET_ADAPTER3)
		{
			// application and diagnostics data adapter
			//
			result &= DeviceHelper::getBoolProperty(adapter, "AppDataEnable", &appDataEnable, log);
			result &= DeviceHelper::getStrProperty(adapter, "AppDataIP", &appDataIP, log);
			result &= DeviceHelper::getIntProperty(adapter, "AppDataPort", &appDataPort, log);
			result &= DeviceHelper::getStrProperty(adapter, "AppDataServiceID", &appDataServiceID, log);

			result &= DeviceHelper::getBoolProperty(adapter, "DiagDataEnable", &diagDataEnable, log);
			result &= DeviceHelper::getStrProperty(adapter, "DiagDataIP", &diagDataIP, log);
			result &= DeviceHelper::getIntProperty(adapter, "DiagDataPort", &diagDataPort, log);
			result &= DeviceHelper::getStrProperty(adapter, "DiagDataServiceID", &diagDataServiceID, log);

			return result;
		}

		assert(false);
		return false;
	}



}


