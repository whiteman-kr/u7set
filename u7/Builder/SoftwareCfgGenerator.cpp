#include "../Builder/SoftwareCfgGenerator.h"
#include "../Builder/ApplicationLogicCompiler.h"
#include "IssueLogger.h"
#include "../include/DeviceHelper.h"
#include "../VFrame30/LogicSchema.h"


namespace Builder
{
	QList<Hardware::DeviceModule*> SoftwareCfgGenerator::m_lmList;
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

		m_subDir = m_software->strId();

		m_cfgXml = m_buildResultWriter->createConfigurationXmlFile(m_subDir);

		if (m_cfgXml == nullptr)
		{
			LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined,
					  QString(tr("Can't create 'configuration.xml' file for software %1")).
					  arg(m_software->strId()));
			return false;
		}

		LOG_MESSAGE(m_log, QString(tr("Generate configuration for: %1")).
					arg(m_software->strId()));

		m_cfgXml->xmlWriter().writeStartElement("Software");

		m_cfgXml->xmlWriter().writeAttribute("Caption", m_software->caption());
		m_cfgXml->xmlWriter().writeAttribute("ID", m_software->strId());
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

			qDebug() << "Build: schema " << schema->strID() << " is loaded";

			// Add file to build result
			//
			result = buildResultWriter->addFile(subDir, schema->strID() + "." + fileExtension, group, file->data());
			if (result == false)
			{
				returnResult = false;
				continue;
			}

			SchemaFile schemaFile;

			schemaFile.id = schema->strID();
			schemaFile.subDir = subDir;
			schemaFile.fileName = schema->strID() + "." + fileExtension;		// File is stored under this name
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

				m_lmList.append(module);
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
}


