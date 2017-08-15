#include "MonitorCfgGenerator.h"
#include "../../lib/ServiceSettings.h"
#include "../../VFrame30/Schema.h"

namespace Builder
{
	MonitorCfgGenerator::MonitorCfgGenerator(DbController* db, Hardware::Software* software, SignalSet* signalSet, Hardware::EquipmentSet* equipment, BuildResultWriter* buildResultWriter) :
		SoftwareCfgGenerator(db, software, signalSet, equipment, buildResultWriter)
	{
	}


	MonitorCfgGenerator::~MonitorCfgGenerator()
	{
	}

	bool MonitorCfgGenerator::generateConfiguration()
	{
		if (m_software == nullptr ||
			m_software->type() != E::SoftwareType::Monitor ||
			m_equipment == nullptr ||
			m_cfgXml == nullptr ||
			m_buildResultWriter == nullptr)
		{
			assert(m_software);
			assert(m_software->type() == E::SoftwareType::Monitor);
			assert(m_equipment);
			assert(m_cfgXml);
			assert(m_buildResultWriter);
			return false;
		}

		// Writing GlobalScript
		//
		bool result = true;

		if (m_software->propertyExists("GlobalScript") == false)
		{
			m_log->errCFG3000("GlobalScript", m_software->equipmentIdTemplate());
			result = false;
		}
		else
		{
			QString globalScript = m_software->propertyValue("GlobalScript").toString();
			BuildFile* globalScriptBuildFile = m_buildResultWriter->addFile(m_software->equipmentIdTemplate(), "GlobalScript.js", globalScript);

			m_cfgXml->addLinkToFile(globalScriptBuildFile);
		}

		// write XML via m_cfgXml->xmlWriter()
		//
		result &= writeMonitorSettings();

		// add link to configuration files (previously written) via m_cfgXml->addLinkToFile(...)
		//
		QString alsExt = QLatin1String(".") + ::AlFileExtension;
		QString mvsExt = QLatin1String(".") + ::MvsFileExtension;

		for (const SchemaFile& schemaFile : SoftwareCfgGenerator::m_schemaFileList)
		{
			// Add Application Logic and Monitor schemas only
			//
			if (schemaFile.fileName.endsWith(alsExt, Qt::CaseInsensitive) == true ||
				schemaFile.fileName.endsWith(mvsExt, Qt::CaseInsensitive) == true)
			{
				m_cfgXml->addLinkToFile(schemaFile.subDir, schemaFile.fileName);
			}
		}

		// Generate description file for all schemas
		//
		VFrame30::SchemaDetailsSet detailsSet;

		for (const SchemaFile& schemaFile : SoftwareCfgGenerator::m_schemaFileList)
		{
			auto details = std::make_shared<VFrame30::SchemaDetails>(schemaFile.details);
			details->m_guids.clear();		// Do we really need SchemaItemGuids in Monitor? If yes, delete this line

			detailsSet.add(details);
		}

		QByteArray schemaSetFileData;

		bool saveOk = detailsSet.Save(schemaSetFileData);
		assert(saveOk);

		if (saveOk == true)
		{
			BuildFile* schemaDetailsBuildFile = m_buildResultWriter->addFile(m_software->equipmentIdTemplate(), "SchemaDetails.pbuf", schemaSetFileData);
			m_cfgXml->addLinkToFile(schemaDetailsBuildFile);
		}

		return result;
	}

	bool MonitorCfgGenerator::writeMonitorSettings()
	{
		// write XML via m_cfgXml->xmlWriter()
		//
		QXmlStreamWriter& xmlWriter = m_cfgXml->xmlWriter();

		{
			xmlWriter.writeStartElement("Settings");
			std::shared_ptr<int*> writeEndSettings(nullptr, [&xmlWriter](void*)
				{
					xmlWriter.writeEndElement();
				});

			// --
			//
			bool ok = true;

			// StartSchemaID
			//
			QString startSchemaId = getObjectProperty<QString>(m_software->equipmentIdTemplate(), "StartSchemaID", &ok).trimmed();
			if (ok == false)
			{
				return false;
			}

			if (startSchemaId.isEmpty() == true)
			{
				QString errorStr = tr("Monitor configuration error %1, property startSchemaId is invalid")
								   .arg(m_software->equipmentIdTemplate());

				m_log->writeError(errorStr);
				writeErrorSection(xmlWriter, errorStr);
				return false;
			}

			xmlWriter.writeTextElement("StartSchemaID", startSchemaId);

			// AppDataService
			//
			ok = writeAppDataServiceSection(xmlWriter);
			if (ok == false)
			{
				return false;
			}

			// ArchiveService
			//
			ok = writeArchiveServiceSection(xmlWriter);
			if (ok == false)
			{
				return false;
			}

		} // Settings


		return true;
	}

	bool MonitorCfgGenerator::writeAppDataServiceSection(QXmlStreamWriter& xmlWriter)
	{
		bool ok = false;

		// AppDataServiceID1(2)
		//
		QString appDataServiceId1 = getObjectProperty<QString>(m_software->equipmentIdTemplate(), "AppDataServiceID1", &ok).trimmed();
		if (ok == false)
		{
			return false;
		}

		if (appDataServiceId1.isEmpty() == true)
		{
			QString errorStr = tr("Monitor configuration error %1, property AppDataServiceID1 is invalid")
							   .arg(m_software->equipmentIdTemplate());

			m_log->writeError(errorStr);
			writeErrorSection(xmlWriter, errorStr);
			return false;
		}

		// AppDataServiceID2
		//
		QString appDataServiceId2 = getObjectProperty<QString>(m_software->equipmentIdTemplate(), "AppDataServiceID2", &ok).trimmed();
		if (ok == false)
		{
			return false;
		}

		if (appDataServiceId2.isEmpty() == true)
		{
			QString errorStr = tr("Monitor configuration error %1, property AppDataServiceID2 is invalid")
							   .arg(m_software->equipmentIdTemplate());

			m_log->writeError(errorStr);
			writeErrorSection(xmlWriter, errorStr);
			return false;
		}

		// AppDataServiceStrID1->ClientRequestIP, ClientRequestPort
		//
		Hardware::Software* dasObject1 = dynamic_cast<Hardware::Software*>(m_equipment->deviceObject(appDataServiceId1));
		Hardware::Software* dasObject2 = dynamic_cast<Hardware::Software*>(m_equipment->deviceObject(appDataServiceId2));

		if (dasObject1 == nullptr)
		{
			QString errorStr = tr("Object %1 is not found").arg(appDataServiceId1);

			m_log->writeError(errorStr);
			writeErrorSection(m_cfgXml->xmlWriter(), errorStr);
			return false;
		}

		if (dasObject2 == nullptr)
		{
			QString errorStr = tr("Object %1 is not found").arg(appDataServiceId2);

			m_log->writeError(errorStr);
			writeErrorSection(m_cfgXml->xmlWriter(), errorStr);
			return false;
		}

		AppDataServiceSettings dasSettings1;

		ok = dasSettings1.readFromDevice(m_equipment, dasObject1, m_log);
		if (ok == false)
		{
			return false;
		}

		AppDataServiceSettings dasSettings2;

		ok = dasSettings2.readFromDevice(m_equipment, dasObject2, m_log);
		if (ok == false)
		{
			return false;
		}

		// AppDataService -- Get ip addresses and ports, write them to configurations
		//
		{
			xmlWriter.writeStartElement("AppDataService");
			std::shared_ptr<int*> writeEndElementService(nullptr, [&xmlWriter](void*)
				{
					xmlWriter.writeEndElement();
				});

			// --
			//
			xmlWriter.writeAttribute("AppDataServiceID1", appDataServiceId1);
			xmlWriter.writeAttribute("AppDataServiceID2", appDataServiceId2);

			xmlWriter.writeAttribute("ip1", dasSettings1.clientRequestIP.address().toString());
			xmlWriter.writeAttribute("port1", QString::number(dasSettings1.clientRequestIP.port()));
			xmlWriter.writeAttribute("ip2", dasSettings2.clientRequestIP.address().toString());
			xmlWriter.writeAttribute("port2", QString::number(dasSettings2.clientRequestIP.port()));
		}	// AppDataService

		return true;
	}

	bool MonitorCfgGenerator::writeArchiveServiceSection(QXmlStreamWriter& xmlWriter)
	{
		bool ok = true;

		// ArchiveServiceID1(2)
		//
		QString archiveServiceId1 = getObjectProperty<QString>(m_software->equipmentIdTemplate(), "ArchiveServiceID1", &ok).trimmed();
		if (ok == false)
		{
			return false;
		}

		if (archiveServiceId1.isEmpty() == true)
		{
			QString errorStr = tr("Monitor configuration error %1, property ArchiveServiceID1 is invalid")
							   .arg(m_software->equipmentIdTemplate());

			m_log->writeError(errorStr);
			writeErrorSection(xmlWriter, errorStr);
			return false;
		}

		// ArchiveServiceID2
		//
		QString archiveServiceId2 = getObjectProperty<QString>(m_software->equipmentIdTemplate(), "ArchiveServiceID2", &ok).trimmed();
		if (ok == false)
		{
			return false;
		}

		if (archiveServiceId2.isEmpty() == true)
		{
			QString errorStr = tr("Monitor configuration error %1, property ArchiveServiceID2 is invalid")
							   .arg(m_software->equipmentIdTemplate());

			m_log->writeError(errorStr);
			writeErrorSection(xmlWriter, errorStr);
			return false;
		}

		// ArchiveServiceID1(2)->ClientRequestIP, ClientRequestPort
		//
		Hardware::Software* archiveServiceObject1 = dynamic_cast<Hardware::Software*>(m_equipment->deviceObject(archiveServiceId1));
		Hardware::Software* archiveServiceObject2 = dynamic_cast<Hardware::Software*>(m_equipment->deviceObject(archiveServiceId2));

		if (archiveServiceObject1 == nullptr)
		{
			QString errorStr = tr("Object %1 is not found").arg(archiveServiceId1);

			m_log->writeError(errorStr);
			writeErrorSection(m_cfgXml->xmlWriter(), errorStr);
			return false;
		}

		if (archiveServiceObject2 == nullptr)
		{
			QString errorStr = tr("Object %1 is not found").arg(archiveServiceId2);

			m_log->writeError(errorStr);
			writeErrorSection(m_cfgXml->xmlWriter(), errorStr);
			return false;
		}

		ArchivingServiceSettings archiveServiceSettings1;
		archiveServiceSettings1.readFromDevice(archiveServiceObject1, m_log);

		ArchivingServiceSettings archiveServiceSettings2;
		archiveServiceSettings2.readFromDevice(archiveServiceObject2, m_log);

		// ArchiveService -- Get ip addresses and ports, write them to configurations
		//
		{
			xmlWriter.writeStartElement("ArchiveService");
			std::shared_ptr<int*> writeEndElement(nullptr, [&xmlWriter](void*)
			{
				xmlWriter.writeEndElement();
			});

			// --
			//
			xmlWriter.writeAttribute("ArchiveServiceID1", archiveServiceId1);
			xmlWriter.writeAttribute("ArchiveServiceID2", archiveServiceId2);

			xmlWriter.writeAttribute("ip1", archiveServiceSettings1.clientRequestIP.address().toString());
			xmlWriter.writeAttribute("port1", QString::number(archiveServiceSettings1.clientRequestIP.port()));
			xmlWriter.writeAttribute("ip2", archiveServiceSettings2.clientRequestIP.address().toString());
			xmlWriter.writeAttribute("port2", QString::number(archiveServiceSettings2.clientRequestIP.port()));
		}	// ArchiveService

		return true;
	}

	void MonitorCfgGenerator::writeErrorSection(QXmlStreamWriter& xmlWriter, QString error)
	{
		xmlWriter.writeTextElement("Error", error);
	}
}
