#include "MonitorCfgGenerator.h"
#include "TuningClientCfgGenerator.h"
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

		// Generate tuning signals file
		//
		if (m_tuningEnabled == true)
		{
			result &= writeTuningSignals();
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
			ok = writeTuningServiceSection(xmlWriter);
			if (ok == false)
			{
				return false;
			}

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
		Hardware::Software* appDataService1 = nullptr;
		Hardware::Software* appDataService2 = nullptr;
		ok1 = true;
		ok2 = true;

		if (appDataServiceId1.isEmpty() == false)
		{
			appDataService1 = dynamic_cast<Hardware::Software*>(m_equipment->deviceObject(appDataServiceId1));

			if (appDataService1 == nullptr)
			{
				m_log->errCFG3021(m_software->equipmentId(), "AppDataServiceID1", appDataServiceId1);

				QString errorStr = tr("Object %1 is not found").arg(appDataServiceId1);
				writeErrorSection(m_cfgXml->xmlWriter(), errorStr);

				ok1 = false;
			}
		}

		if (appDataServiceId2.isEmpty() == false)
		{
			appDataService2 = dynamic_cast<Hardware::Software*>(m_equipment->deviceObject(appDataServiceId2));

			if (appDataService2 == nullptr)
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
		AppDataServiceSettings adsSettings1;
		AppDataServiceSettings adsSettings2;
		ok1 = true;
		ok2 = true;

		if (appDataService1 != nullptr)
		{
			ok1 = adsSettings1.readFromDevice(m_equipment, appDataService1, m_log);
		}

		if (appDataService2 != nullptr)
		{
			ok2 = adsSettings2.readFromDevice(m_equipment, appDataService2, m_log);
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

			xmlWriter.writeAttribute("ip1", adsSettings1.clientRequestIP.address().toString());
			xmlWriter.writeAttribute("port1", QString::number(adsSettings1.clientRequestIP.port()));
			xmlWriter.writeAttribute("rtip1", adsSettings1.rtTrendsRequestIP.address().toString());
			xmlWriter.writeAttribute("rtport1", QString::number(adsSettings1.rtTrendsRequestIP.port()));

			xmlWriter.writeAttribute("ip2", adsSettings2.clientRequestIP.address().toString());
			xmlWriter.writeAttribute("port2", QString::number(adsSettings2.clientRequestIP.port()));
			xmlWriter.writeAttribute("rtip2", adsSettings2.rtTrendsRequestIP.address().toString());
			xmlWriter.writeAttribute("rtport2", QString::number(adsSettings2.rtTrendsRequestIP.port()));
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

		// TuningEnable
		//
		m_tuningEnabled = getObjectProperty<bool>(m_software->equipmentIdTemplate(), "TuningEnable", &ok);
		if (ok == false)
		{
			return false;
		}

		QString tuningSources;
		Hardware::Software* tuningServiceObject = nullptr;
		TuningServiceSettings tuningServiceSettings;
		QString tuningServiceId;

		m_tuningSources.clear();

		if (m_tuningEnabled == true)
		{
			// TuningSourceEquipmentID, semicolon or return EquipmentID separated list
			//
			tuningSources = getObjectProperty<QString>(m_software->equipmentIdTemplate(), "TuningSourceEquipmentID", &ok).trimmed();
			if (ok == false)
			{
				return false;
			}

			tuningSources = tuningSources.replace(QChar(QChar::LineFeed), QChar(';'));
			tuningSources = tuningSources.replace(QChar(QChar::CarriageReturn), QChar(';'));
			tuningSources = tuningSources.replace(QChar(QChar::Tabulation), QChar(';'));

			m_tuningSources = tuningSources.split(QChar(';'), QString::SkipEmptyParts);

			if (m_tuningSources.isEmpty() == true)
			{
				m_log->errCFG3022(m_software->equipmentIdTemplate(), "TuningSourceEquipmentID");
				return false;
			}

			// Check for valid EquipmentIds
			//
			for (const QString& tuningEquipmentID : m_tuningSources)
			{
				if (m_equipment->deviceObject(tuningEquipmentID) == nullptr)
				{
					m_log->errEQP6109(tuningEquipmentID, m_software->equipmentIdTemplate());
					return false;
				}
			}

			// TuningServiceID
			//
			tuningServiceId = getObjectProperty<QString>(m_software->equipmentIdTemplate(), "TuningServiceID", &ok).trimmed();

			if (ok == false)
			{
				// getObjectProperty reposrts to log abourt error
				//
				return false;
			}

			// TuningServiceID->ClientRequestIP, ClientRequestPort
			//
			if (tuningServiceId.isEmpty() == true)
			{
				// Property '%1.%2' is empty.
				//
				m_log->errCFG3022(m_software->equipmentIdTemplate(), "TuningServiceID");
				return false;
			}

			tuningServiceObject = dynamic_cast<Hardware::Software*>(m_equipment->deviceObject(tuningServiceId));

			if (tuningServiceObject == nullptr)
			{
				// Property '%1.%2' is linked to undefined software ID '%3'.
				//
				m_log->errCFG3021(m_software->equipmentId(), "TuningServiceID", tuningServiceId);

				QString errorStr = tr("Object %1 is not found").arg(tuningServiceId);
				writeErrorSection(m_cfgXml->xmlWriter(), errorStr);

				return false;
			}

			if (tuningServiceObject->type() != E::SoftwareType::TuningService)
			{
				// Property '%1.%2' is linked to not compatible software '%3'.
				//
				m_log->errCFG3017(m_software->equipmentId(), "TuningServiceID", tuningServiceId);
				return false;
			}

			auto [singleLmControl, hasSingleLmControl] = getObjectProperty<bool>(tuningServiceId, "SingleLmControl");
			if (hasSingleLmControl == false)
			{
				return false;
			}

			if (singleLmControl == true)
			{
				// Mode SingleLmControl is not supported by Monitor. Set TuningServiceID.SingleLmControl to false. Monitor EquipmentID %1, TuningServiceID %2.
				//
				m_log->errCFG3040(m_software->equipmentId(), tuningServiceId);
				return false;
			}

			// Reading TuningService Settings
			//
			ok = tuningServiceSettings.readFromDevice(tuningServiceObject, m_log);	// readFromDevice reports to log about errors

			if (ok == false)
			{
				return false;
			}
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
			xmlWriter.writeAttribute("Enable", m_tuningEnabled ? "true" : "false");

			if (m_tuningEnabled == true)
			{
				xmlWriter.writeAttribute("TuningServiceID", tuningServiceId);

				xmlWriter.writeAttribute("ip", tuningServiceSettings.clientRequestIP.address().toString());
				xmlWriter.writeAttribute("port", QString::number(tuningServiceSettings.clientRequestIP.port()));
			}
		}	// TuningService

		// TuningSources -- EqupmentIDs for LM's to tune
		//
		if (m_tuningEnabled == true)
		{
			xmlWriter.writeTextElement(QLatin1String("TuningSources"), m_tuningSources.join(QLatin1String("; ")));
		}

		return true;
	}

	void MonitorCfgGenerator::writeErrorSection(QXmlStreamWriter& xmlWriter, QString error)
	{
		xmlWriter.writeTextElement("Error", error);
	}

	bool MonitorCfgGenerator::writeTuningSignals()
	{
		if (m_tuningSources.empty() == true)
		{
			assert(m_tuningSources.empty() == false);
			return false;
		}

		::Proto::AppSignalSet tuningSet;

		bool ok = TuningClientCfgGenerator::createTuningSignals(m_tuningSources, m_signalSet, &tuningSet);
		if (ok == false)
		{
			m_log->errINT1000("Generate tuning signal set error: MonitorCfgGenerator::writeTuningSignals, call for TuningClientCfgGenerator::createTuningSignals");
			return false;
		}

		// Write number of signals
		//
		QByteArray data;
		data.resize(tuningSet.ByteSize());

		tuningSet.SerializeToArray(data.data(), tuningSet.ByteSize());

		// Write file
		//
		BuildFile* buildFile = m_buildResultWriter->addFile(m_software->equipmentIdTemplate(), "TuningSignals.dat", CFG_FILE_ID_TUNING_SIGNALS, "", data);

		if (buildFile == nullptr)
		{
			m_log->errCMN0012("TuningSignals.dat");
			return false;
		}

		ok = m_cfgXml->addLinkToFile(buildFile);
		return ok;
	}
}
