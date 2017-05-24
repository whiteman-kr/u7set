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

			//
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

			//
			// AppDataServiceID2
			//
			QString appDataServiceId2 = getObjectProperty<QString>(m_software->equipmentIdTemplate(), "AppDataServiceID2", &ok).trimmed();
			if (ok == false)
			{
				return false;
			}

			if (appDataServiceId2.isEmpty() == true)
			{
				QString errorStr = tr("Monitor configuration error %1, property DataAquisitionServiceID2 is invalid")
								   .arg(m_software->equipmentIdTemplate());

				m_log->writeError(errorStr);
				writeErrorSection(xmlWriter, errorStr);
				return false;
			}

			//
			// DataAquisitionServiceStrID1->ClientRequestIP, ClientRequestPort
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
			dasSettings1.readFromDevice(dasObject1, m_log);

			AppDataServiceSettings dasSettings2;
			dasSettings2.readFromDevice(dasObject2, m_log);

			// Get ip addresses and ports, write them to configurations
			//
			{
				xmlWriter.writeStartElement("DataAquisitionService");
				std::shared_ptr<int*> writeEndDataAquisitionService(nullptr, [&xmlWriter](void*)
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
			}	// DataAquisitionService


			//
			// Archive Service Settings
			//

//			// ArchiveServiceStrID1
//			//
//			QString asStrID1 = getObjectProperty<QString>(m_software->strId(), "ArchiveServiceStrID1", &ok).trimmed();
//			if (ok == false)
//			{
//				return false;
//			}

//			if (asStrID1.isEmpty() == true)
//			{
//				QString errorStr = tr("Monitor configuration error %1, property ArchiveServiceStrID1 is invalid")
//								   .arg(m_software->strId());

//				m_log->writeError(errorStr);
//				writeErrorSection(xmlWriter, errorStr);
//				return false;
//			}

//			// ArchiveServiceStrID2
//			//
//			QString asStrID2 = getObjectProperty<QString>(m_software->strId(), "ArchiveServiceStrID2", &ok).trimmed();
//			if (ok == false)
//			{
//				return false;
//			}

//			if (asStrID2.isEmpty() == true)
//			{
//				QString errorStr = tr("Monitor configuration error %1, property ArchiveServiceStrID2 is invalid")
//								   .arg(m_software->strId());

//				m_log->writeError(errorStr);
//				writeErrorSection(xmlWriter, errorStr);
//				return false;
//			}

//			// DataAquisitionServiceStrID1->ClientRequestIP, ClientRequestPort
//			//
//			QString clientRequestIP1 = getObjectProperty<QString>(dacStrID1, "ClientRequestIP", &ok).trimmed();
//			if (ok == false)
//			{
//				return false;
//			}

//			QString clientRequestPort1 = getObjectProperty<QString>(dacStrID1, "ClientRequestPort", &ok).trimmed();
//			if (ok == false)
//			{
//				return false;
//			}

//			// DataAquisitionServiceStrID1->ClientRequestIP, ClientRequestPort
//			//
//			QString clientRequestIP2 = getObjectProperty<QString>(dacStrID2, "ClientRequestIP", &ok).trimmed();
//			if (ok == false)
//			{
//				return false;
//			}

//			QString clientRequestPort2 = getObjectProperty<QString>(dacStrID2, "ClientRequestPort", &ok).trimmed();
//			if (ok == false)
//			{
//				return false;
//			}

//			// Get ip addresses and ports, write them to configurations
//			//
//			{
//				xmlWriter.writeStartElement("DataAquisitionService");
//				std::shared_ptr<int*> writeEndDataAquisitionService(nullptr, [&xmlWriter](void*)
//					{
//						xmlWriter.writeEndElement();
//					});

//				// --
//				//
//				xmlWriter.writeAttribute("StrID1", dacStrID1);
//				xmlWriter.writeAttribute("StrID2", dacStrID2);

//				xmlWriter.writeAttribute("ip1", clientRequestIP1);
//				xmlWriter.writeAttribute("port1", clientRequestPort1);
//				xmlWriter.writeAttribute("ip2", clientRequestIP2);
//				xmlWriter.writeAttribute("port2", clientRequestPort2);
//			}	// DataAquisitionService


		} // Settings


		return true;
	}

	void MonitorCfgGenerator::writeErrorSection(QXmlStreamWriter& xmlWriter, QString error)
	{
		xmlWriter.writeTextElement("Error", error);
	}
}
