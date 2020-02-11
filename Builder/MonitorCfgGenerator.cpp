#include "MonitorCfgGenerator.h"
#include "TuningClientCfgGenerator.h"
#include "../lib/ServiceSettings.h"
#include "../VFrame30/Schema.h"
#include "Context.h"
#include "../lib/ClientBehaviour.h"

namespace Builder
{
	MonitorCfgGenerator::MonitorCfgGenerator(Context* context, Hardware::Software* software) :
		SoftwareCfgGenerator(context, software)
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
			Q_ASSERT(m_software);
			Q_ASSERT(m_software->type() == E::SoftwareType::Monitor);
			Q_ASSERT(m_equipment);
			Q_ASSERT(m_cfgXml);
			Q_ASSERT(m_buildResultWriter);
			return false;
		}

		// Writing GlobalScript
		//
		bool result = true;

		result &= saveScriptProperties("GlobalScript", "GlobalScript.js");

		// Writing event scripts: OnConfigurationArrived
		//
		result &= saveScriptProperties("OnConfigurationArrived", "OnConfigurationArrived.js");

		// write XML via m_cfgXml->xmlWriter()
		//
		result &= writeMonitorSettings();

		// Add links to schema files (previously written) via m_cfgXml->addLinkToFile(...)
		//
		result &= writeSchemasByTags();

		// Generate tuning signals file
		//
		if (m_tuningEnabled == true)
		{
			result &= writeTuningSignals();
		}

		// Generate behaviour
		//
		result &= writeMonitorBehaviour();

		// Add link to FILE_COMPARATORS_SET (Common/Comparator.set)
		//
		if (BuildFile* compBuildFile = m_buildResultWriter->getBuildFileByID(DIR_COMMON, CFG_FILE_ID_COMPARATOR_SET);
			compBuildFile != nullptr)
		{
			m_cfgXml->addLinkToFile(compBuildFile);
		}
		else
		{
			result = false;
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

			// StartSchemaID
			//
			{
				bool ok = true;
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
			}

			// SchemaTags
			//
			{
				bool ok = true;
				QString schemaTags = getObjectProperty<QString>(m_software->equipmentIdTemplate(), "SchemaTags", &ok);
				if (ok == false)
				{
					return false;
				}

				m_schemaTagList = schemaTags.split(QRegExp("\\W+"), QString::SkipEmptyParts);

				for (QString& tag : m_schemaTagList)
				{
					tag = tag.toLower();
				}

				schemaTags = m_schemaTagList.join("; ");

				xmlWriter.writeTextElement("SchemaTags", schemaTags);
			}

			// AppDataService
			//
			if (bool ok = writeAppDataServiceSection(xmlWriter);
				ok == false)
			{
				return false;
			}

			// ArchiveService
			//
			if (bool ok = writeArchiveServiceSection(xmlWriter);
				ok == false)
			{
				return false;
			}

			// TuningService
			//
			if (bool ok = writeTuningServiceSection(xmlWriter);
				ok == false)
			{
				return false;
			}

		} // Settings

		return true;
	}

	bool MonitorCfgGenerator::saveScriptProperties(QString scriptProperty, QString fileName)
	{
		bool result = true;

		if (m_software->propertyExists(scriptProperty) == false)
		{
			m_log->errCFG3000(scriptProperty, m_software->equipmentIdTemplate());
			result = false;
		}
		else
		{
			QString script = m_software->propertyValue(scriptProperty).toString();
			BuildFile* scriptBuildFile = m_buildResultWriter->addFile(m_software->equipmentIdTemplate(), fileName, script);

			if (scriptBuildFile != nullptr)
			{
				m_cfgXml->addLinkToFile(scriptBuildFile);
			}
			else
			{
				result = false;
			}
		}

		return result;
	}

	bool MonitorCfgGenerator::writeSchemasByTags()
	{
		// class SoftwareCfgGenerator
		//		static std::multimap<QString, std::shared_ptr<SchemaFile>> m_schemaTagToFile;
		//

		bool result = true;
		std::set<std::shared_ptr<SchemaFile>> monitorSchemas;

		// If tag list is empty, then link all Monitor and ApplicationLogic schemas
		//
		if (m_schemaTagList.isEmpty() == true)
		{
			for (auto&[tag, schemaFile] : SoftwareCfgGenerator::m_schemaTagToFile)
			{
				Q_UNUSED(tag);
				if (schemaFile->fileName.endsWith(QStringLiteral(".") + Db::File::AlFileExtension, Qt::CaseInsensitive) == true ||
					schemaFile->fileName.endsWith(QStringLiteral(".") + Db::File::MvsFileExtension, Qt::CaseInsensitive) == true)
				{
					monitorSchemas.insert(schemaFile);
				}
			}
		}
		else
		{
			for (QString tag : m_schemaTagList)
			{
				tag = tag.toLower();

				auto tagRange = m_schemaTagToFile.equal_range(tag);

				for (auto it = tagRange.first; it != tagRange.second; ++it)
				{
					const QString& mapTag = it->first;
					std::shared_ptr<SchemaFile> schemaFile = it->second;

					if (mapTag != tag ||
						schemaFile == nullptr)
					{
						Q_ASSERT(mapTag == tag);
						Q_ASSERT(schemaFile);
						continue;
					}

					monitorSchemas.insert(schemaFile);
				}
			}
		}

		// --
		//
		VFrame30::SchemaDetailsSet detaisSet;

		for (auto schemaFile : monitorSchemas)
		{
			result &= m_cfgXml->addLinkToFile(schemaFile->subDir, schemaFile->fileName);
			detaisSet.add(schemaFile->details);
		}

		// Save details
		//
		{
			QByteArray fileData;

			if (bool ok = detaisSet.saveToByteArray(&fileData);
				ok == true)
			{
				BuildFile* schemaDetailsBuildFile = m_buildResultWriter->addFile(m_software->equipmentIdTemplate(), "SchemaDetails.pbuf", fileData);

				if (schemaDetailsBuildFile != nullptr)
				{
					result &= m_cfgXml->addLinkToFile(schemaDetailsBuildFile);
				}
				else
				{
					result = false;
				}
			}
			else
			{
				return false;
			}
		}

		return result;
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
			else
			{
				if (appDataService1->type() != E::SoftwareType::AppDataService)
				{
					m_log->errCFG3017(m_software->equipmentId(), "AppDataServiceID1", appDataServiceId1);

					QString errorStr = tr("Property %1.%2 is linked to not compatible software %3.")
											.arg(m_software->equipmentId())
											.arg("AppDataServiceID1")
											.arg(appDataServiceId1);

					writeErrorSection(m_cfgXml->xmlWriter(), errorStr);

					ok1 = false;
				}
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
			else
			{
				if (appDataService2->type() != E::SoftwareType::AppDataService)
				{
					m_log->errCFG3017(m_software->equipmentId(), "AppDataServiceID2", appDataServiceId2);

					QString errorStr = tr("Property %1.%2 is linked to not compatible software %3.")
											.arg(m_software->equipmentId())
											.arg("AppDataServiceID2")
											.arg(appDataServiceId2);

					writeErrorSection(m_cfgXml->xmlWriter(), errorStr);

					ok1 = false;
				}
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
		QString archiveServiceId1;
		QString archiveServiceId2;

		// Get ArchServiceID from AppDataService
		//
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

		// Get ArchServiceID form AppDataService
		//
		ok1 = true;
		ok2 = true;

		if (appDataServiceId1.isEmpty() == false)
		{
			auto[archSrv1, ok] = getObjectProperty<QString>(appDataServiceId1, "ArchiveServiceID");

			if (ok == false)
			{
				ok1 = false;
			}

			archiveServiceId1 = archSrv1;
		}

		if (appDataServiceId2.isEmpty() == false)
		{
			auto[archSrv2, ok] = getObjectProperty<QString>(appDataServiceId2, "ArchiveServiceID");

			if (ok == false)
			{
				ok2 = false;
			}

			archiveServiceId2 = archSrv2;
		}

		if (ok1 == false || ok2 == false)
		{
			return false;
		}

		// --
		//
		ok1 = true;
		ok2 = true;

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

		if (m_tuningEnabled == true &&
			m_dbController->currentProject().safetyProject() == true)
		{
			// Tuning for Monitor is forbiden for Safety Projects
			// Stupid decision but not mine
			//
			m_log->errEQP6200(m_software->equipmentIdTemplate());
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
			//Q_ASSERT(m_tuningSources.empty() == false);
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

	bool MonitorCfgGenerator::writeMonitorBehaviour()
	{
		if (m_dbController == nullptr)
		{
			Q_ASSERT(m_dbController);
			return false;
		}

		bool ok = true;
		QString behaviourId = getObjectProperty<QString>(m_software->equipmentIdTemplate(), "BehaviourID", &ok).trimmed();
		if (ok == false)
		{
			return false;
		}

		if (behaviourId.isEmpty() == true)
		{
			return true;
		}

		// Load all clients behaviour
		//
		ClientBehaviourStorage allBehaviourStorage;

		QString errorCode;

		QByteArray dbData;

		bool result = loadFileFromDatabase(m_dbController, allBehaviourStorage.dbFileName(), &errorCode, &dbData);
		if (result == false)
		{
			m_log->errCMN0010(allBehaviourStorage.dbFileName());
			return false;
		}

		if (allBehaviourStorage.load(dbData, &errorCode) == false)
		{
			m_log->errCMN0010(allBehaviourStorage.dbFileName());
			return false;
		}

		// Find behaviour for current monitor
		//
		ClientBehaviourStorage monitorBehaviourStorage;

		std::vector<std::shared_ptr<MonitorBehaviour>> behaviours = allBehaviourStorage.monitorBehavoiurs();

		for (auto b : behaviours)
		{
			if (b->id() == behaviourId)
			{
				monitorBehaviourStorage.add(b);
				break;
			}
		}

		if (monitorBehaviourStorage.count() == 0)
		{
			m_log->errEQP6210(m_software->equipmentIdTemplate(), behaviourId);
			return false;
		}

		// Save monitor behaviour to XML
		//
		QByteArray data;
		monitorBehaviourStorage.save(data);

		// Write file
		//
		BuildFile* buildFile = m_buildResultWriter->addFile(m_software->equipmentIdTemplate(), "MonitorBehaviour.xml", CFG_FILE_ID_BEHAVIOUR, "", data);

		if (buildFile == nullptr)
		{
			m_log->errCMN0012("MonitorBehaviour.xml");
			return false;
		}

		ok = m_cfgXml->addLinkToFile(buildFile);

		return ok;
	}
}
