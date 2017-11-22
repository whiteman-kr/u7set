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

			// TuningService
			//
//			ok = writeTuningServiceSection(xmlWriter);
//			if (ok == false)
//			{
//				return false;
//			}

		} // Settings


		return true;
	}

	bool MonitorCfgGenerator::writeAppDataServiceSection(QXmlStreamWriter& xmlWriter)
	{
		bool ok1 = false;
		bool ok2 = false;

		// AppDataServiceID
		//
		QString appDataServiceId1 = getObjectProperty<QString>(m_software->equipmentIdTemplate(), "AppDataServiceID1", &ok1).trimmed();
		QString appDataServiceId2 = getObjectProperty<QString>(m_software->equipmentIdTemplate(), "AppDataServiceID2", &ok2).trimmed();

		if (ok1 == false || ok2 == false)
		{
			return false;
		}

		// AppDataServiceStrID1->ClientRequestIP, ClientRequestPort
		//
		Hardware::Software* dasObject1 = nullptr;
		Hardware::Software* dasObject2 = nullptr;
		ok1 = true;
		ok2 = true;

		if (appDataServiceId1.isEmpty() == false)
		{
			dasObject1 = dynamic_cast<Hardware::Software*>(m_equipment->deviceObject(appDataServiceId1));

			if (dasObject1 == nullptr)
			{
				m_log->errCFG3021(m_software->equipmentId(), "AppDataServiceID1", appDataServiceId1);

				QString errorStr = tr("Object %1 is not found").arg(appDataServiceId1);
				writeErrorSection(m_cfgXml->xmlWriter(), errorStr);

				ok1 = false;
			}
		}

		if (appDataServiceId2.isEmpty() == false)
		{
			dasObject2 = dynamic_cast<Hardware::Software*>(m_equipment->deviceObject(appDataServiceId2));

			if (dasObject2 == nullptr)
			{
				m_log->errCFG3021(m_software->equipmentId(), "AppDataServiceID2", appDataServiceId2);

				QString errorStr = tr("Object %1 is not found").arg(appDataServiceId2);
				writeErrorSection(m_cfgXml->xmlWriter(), errorStr);

				ok2 = false;
			}
		}

		if (ok1 == false || ok2 == false)
		{
			return false;
		}

		// Reading AppDataService Settings
		//
		AppDataServiceSettings dasSettings1;
		AppDataServiceSettings dasSettings2;
		ok1 = true;
		ok2 = true;

		if (dasObject1 != nullptr)
		{
			ok1 = dasSettings1.readFromDevice(m_equipment, dasObject1, m_log);
		}

		if (dasObject2 != nullptr)
		{
			ok2 = dasSettings2.readFromDevice(m_equipment, dasObject2, m_log);
		}

		if (ok1 == false || ok2 == false)
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
		bool ok1 = true;
		bool ok2 = true;

		// ArchiveServiceID1
		//
		QString archiveServiceId1 = getObjectProperty<QString>(m_software->equipmentIdTemplate(), "ArchiveServiceID1", &ok1).trimmed();
		QString archiveServiceId2 = getObjectProperty<QString>(m_software->equipmentIdTemplate(), "ArchiveServiceID2", &ok2).trimmed();

		if (ok1 == false || ok2 == false)
		{
			return false;
		}

		// ArchiveServiceID1(2)->ClientRequestIP, ClientRequestPort
		//
		Hardware::Software* archiveServiceObject1 = nullptr;
		Hardware::Software* archiveServiceObject2 = nullptr;
		ok1 = true;
		ok2 = true;

		if (archiveServiceId1.isEmpty() == false)
		{
			archiveServiceObject1 = dynamic_cast<Hardware::Software*>(m_equipment->deviceObject(archiveServiceId1));

			if (archiveServiceObject1 == nullptr)
			{
				m_log->errCFG3021(m_software->equipmentId(), "ArchiveServiceID1", archiveServiceId1);

				QString errorStr = tr("Object %1 is not found").arg(archiveServiceId1);
				writeErrorSection(m_cfgXml->xmlWriter(), errorStr);

				ok1 = false;
			}
		}

		if (archiveServiceId2.isEmpty() == false)
		{
			archiveServiceObject2 = dynamic_cast<Hardware::Software*>(m_equipment->deviceObject(archiveServiceId2));

			if (archiveServiceObject2 == nullptr)
			{
				m_log->errCFG3021(m_software->equipmentId(), "AppDataServiceID2", archiveServiceId2);

				QString errorStr = tr("Object %1 is not found").arg(archiveServiceId2);
				writeErrorSection(m_cfgXml->xmlWriter(), errorStr);

				ok2 = false;
			}
		}

		if (ok1 == false || ok2 == false)
		{
			return false;
		}

		// Reading ArchiveService Settings
		//
		ArchivingServiceSettings archiveServiceSettings1;
		ArchivingServiceSettings archiveServiceSettings2;
		ok1 = true;
		ok2 = true;

		if (archiveServiceObject1 != nullptr)
		{
			ok1 = archiveServiceSettings1.readFromDevice(archiveServiceObject1, m_log);
		}

		if (archiveServiceObject2 != nullptr)
		{
			ok2 = archiveServiceSettings2.readFromDevice(archiveServiceObject2, m_log);
		}

		if (ok1 == false || ok2 == false)
		{
			return false;
		}

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

	bool MonitorCfgGenerator::writeTuningServiceSection(QXmlStreamWriter& xmlWriter)
	{
		bool ok = true;
		bool ok1 = true;
		bool ok2 = true;

		// TuningEnable
		//
		bool tuningEnable = getObjectProperty<bool>(m_software->equipmentIdTemplate(), "TuningEnable", &ok);
		if (ok == false)
		{
			return false;
		}

		// TuningSourceEquipmentID, semicolon or return EquipmentID separated list
		//
		QString tuningSources = getObjectProperty<QString>(m_software->equipmentIdTemplate(), "TuningSourceEquipmentID", &ok).trimmed();
		if (ok == false)
		{
			return false;
		}

		tuningSources = tuningSources.replace(QChar(QChar::LineFeed), QChar(';'));
		tuningSources = tuningSources.replace(QChar(QChar::CarriageReturn), QChar(';'));
		tuningSources = tuningSources.replace(QChar(QChar::Tabulation), QChar(';'));

		QStringList tuningSourceList = tuningSources.split(QChar(';'), QString::SkipEmptyParts);

		if (tuningEnable == true &&
			tuningSourceList.isEmpty() == true)
		{
			// Warning, tuning is enabled but no equipment to tune set
			//
			m_log->wrnCFG3016(m_software->equipmentIdTemplate(), "TuningSourceEquipmentID");
		}

		// TuningServiceID
		//
		QString tuningServiceId1 = getObjectProperty<QString>(m_software->equipmentIdTemplate(), "TuningServiceID1", &ok1).trimmed();
		QString tuningServiceId2 = getObjectProperty<QString>(m_software->equipmentIdTemplate(), "TuningServiceID2", &ok2).trimmed();

		if (ok1 == false || ok2 == false)
		{
			return false;
		}

		// TuningServiceID1(2)->ClientRequestIP, ClientRequestPort
		//
		Hardware::Software* tuningServiceObject1 = nullptr;
		Hardware::Software* tuningServiceObject2 = nullptr;
		ok1 = true;
		ok2 = true;

		if (tuningServiceId1.isEmpty() == false)
		{
			tuningServiceObject1 = dynamic_cast<Hardware::Software*>(m_equipment->deviceObject(tuningServiceId1));

			if (tuningServiceObject1 == nullptr)
			{
				m_log->errCFG3021(m_software->equipmentId(), "TuningServiceID1", tuningServiceId1);

				QString errorStr = tr("Object %1 is not found").arg(tuningServiceId1);
				writeErrorSection(m_cfgXml->xmlWriter(), errorStr);

				ok1 = false;
			}
		}


		if (tuningServiceId2.isEmpty() == false)
		{
			tuningServiceObject2 = dynamic_cast<Hardware::Software*>(m_equipment->deviceObject(tuningServiceId2));

			if (tuningServiceObject2 == nullptr)
			{
				m_log->errCFG3021(m_software->equipmentId(), "TuningServiceID2", tuningServiceId2);

				QString errorStr = tr("Object %1 is not found").arg(tuningServiceId2);
				writeErrorSection(m_cfgXml->xmlWriter(), errorStr);

				ok2 = false;
			}
		}

		if (ok1 == false || ok2 == false)
		{
			return false;
		}

		// Reading TuningService Settings
		//
		TuningServiceSettings tuningServiceSettings1;
		TuningServiceSettings tuningServiceSettings2;
		ok1 = true;
		ok2 = true;

		if (tuningServiceObject1 != nullptr)
		{
			ok1 = tuningServiceSettings1.readFromDevice(tuningServiceObject1, m_log);
		}

		if (tuningServiceObject2 != nullptr)
		{
			ok2 = tuningServiceSettings2.readFromDevice(tuningServiceObject2, m_log);
		}

		if (ok1 == false || ok2 == false)
		{
			return false;
		}

		// TuningService -- Get ip addresses and ports, write them to configurations
		//
		{
			xmlWriter.writeStartElement("TuningService");
			std::shared_ptr<int*> writeEndElement(nullptr, [&xmlWriter](void*)
			{
				xmlWriter.writeEndElement();
			});

			// --
			//
			xmlWriter.writeAttribute("Enable", tuningEnable ? "true" : "false");

			xmlWriter.writeAttribute("TuningServiceID1", tuningServiceId1);
			xmlWriter.writeAttribute("TuningServiceID2", tuningServiceId2);

			xmlWriter.writeAttribute("ip1", tuningServiceSettings1.clientRequestIP.address().toString());
			xmlWriter.writeAttribute("port1", QString::number(tuningServiceSettings1.clientRequestIP.port()));
			xmlWriter.writeAttribute("ip2", tuningServiceSettings2.clientRequestIP.address().toString());
			xmlWriter.writeAttribute("port2", QString::number(tuningServiceSettings2.clientRequestIP.port()));
		}	// TuningService

		// TuningSources -- EqupmentIDs for LM's to tune
		//
		{
			xmlWriter.writeTextElement(QLatin1String("TuningSources"), tuningSourceList.join(QLatin1String("; ")));
		}

		return true;
	}

	void MonitorCfgGenerator::writeErrorSection(QXmlStreamWriter& xmlWriter, QString error)
	{
		xmlWriter.writeTextElement("Error", error);
	}
}
