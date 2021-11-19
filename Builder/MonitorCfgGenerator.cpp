#include "MonitorCfgGenerator.h"
#include "TuningClientCfgGenerator.h"
#include "../lib/SoftwareSettings.h"
#include "../lib/SoftwareSettingsGetter.h"
#include "../VFrame30/Schema.h"
#include "Context.h"
#include "../lib/ClientBehavior.h"

namespace Builder
{
	MonitorCfgGenerator::MonitorCfgGenerator(Context* context, Hardware::Software* software) :
		SoftwareCfgGenerator(context, software)
	{
	}

	MonitorCfgGenerator::~MonitorCfgGenerator()
	{
	}

	bool MonitorCfgGenerator::createSettingsProfile(const QString& profile)
	{
		MonitorSettingsGetter settingsGetter;

		if (settingsGetter.readFromDevice(m_context, m_software) == false)
		{
			return false;
		}

		return m_settingsSet.addProfile<MonitorSettings>(profile, settingsGetter);
	}

	bool MonitorCfgGenerator::generateConfigurationStep1()
	{
		if (m_software == nullptr ||
			m_software->softwareType() != E::SoftwareType::Monitor ||
			m_equipment == nullptr ||
			m_cfgXml == nullptr ||
			m_buildResultWriter == nullptr)
		{
			Q_ASSERT(m_software);
			Q_ASSERT(m_software->softwareType() == E::SoftwareType::Monitor);
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

		result &= initSchemaTags();

		result &= initTuningSources();

		// Add links to schema files (previously written) via m_cfgXml->addLinkToFile(...)
		//
		result &= writeSchemasByTags();

		std::shared_ptr<const MonitorSettings> settings = m_settingsSet.getSettingsDefaultProfile<MonitorSettings>();

		TEST_PTR_LOG_RETURN_FALSE(settings, m_log);

		if (settings->tuningEnabled == true)
		{
			// Check tuning users list
			//
			if (settings->tuningLogin == true &&
				settings->tuningUserAccounts.split(';', Qt::SkipEmptyParts).isEmpty() == true)
			{
				m_log->errEQP6202(m_software->equipmentIdTemplate());
				return false;
			}

			// Generate tuning signals file
			//
			result &= writeTuningSignals();
		}

		// Generate behavior
		//
		result &= writeMonitorBehavior();

		// Generate logo
		//
		result &= writeMonitorLogo();

		// Add link to FILE_COMPARATORS_SET (Common/Comparator.set)
		//
		if (BuildFile* compBuildFile = m_buildResultWriter->getBuildFileByID(Directory::COMMON, CfgFileId::COMPARATOR_SET);
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

	bool MonitorCfgGenerator::initSchemaTags()
	{
		std::shared_ptr<const MonitorSettings> settings = m_settingsSet.getSettingsDefaultProfile<MonitorSettings>();

		TEST_PTR_LOG_RETURN_FALSE(settings, m_log);

		m_schemaTagList = settings->getSchemaTags();

		if (settings->tuningEnabled == true &&
			m_context->m_projectProperties.safetyProject() == true)
		{
			// Tuning for Monitor is forbiden for Safety Projects
			// Stupid decision but not mine
			//
			m_log->errEQP6200(m_software->equipmentIdTemplate());
			return false;
		}

		return true;
	}

	bool MonitorCfgGenerator::initTuningSources()
	{
		std::shared_ptr<const MonitorSettings> settings = m_settingsSet.getSettingsDefaultProfile<MonitorSettings>();

		// Fill list with LAN controller IDs which are connected to the TuningService this client is connected to

		QStringList serviceTuningSourcesList;

		std::shared_ptr<Hardware::DeviceObject> tuningServiceObject = m_equipment->deviceObject(settings->tuningServiceID);
		if (tuningServiceObject == nullptr)
		{
			m_log->errCFG3021(m_software->equipmentId(), EquipmentPropNames::TUNING_SERVICE_ID, settings->tuningServiceID);
			return false;
		}

		std::shared_ptr<Hardware::Software> tuningServiceSoftware = tuningServiceObject->toSoftware();
		if (tuningServiceSoftware == nullptr)
		{
			m_log->errCFG3021(m_software->equipmentId(), EquipmentPropNames::TUNING_SERVICE_ID, settings->tuningServiceID);
			return false;
		}

		TuningServiceSettingsGetter tsg;
		if (tsg.readFromDevice(m_context, tuningServiceSoftware.get()) == false)
		{
			return false;
		}

		for (int c = 0; c < tsg.channelCount; c++)
		{
			for (const TuningServiceSettings::TuningClient& tc : tsg.channelSettings[c].clients)
			{
				if (tc.equipmentID == m_software->equipmentId())
				{
					serviceTuningSourcesList = tc.sourcesIDs;
					break;
				}
			}
		}

		// Read TuningSourceEquipmentID property from Monitor. If is is filled, check if all strings are present in tuningSourcesList

		QStringList localTuningSourcesList = settings->getTuningSources();

		if (localTuningSourcesList.isEmpty() == false)
		{
			// Parse localTuningSourcesList
			//
			m_tuningSources = localTuningSourcesList;

			// Check for valid EquipmentIds
			//
			for (const QString& localTuningSourceId : localTuningSourcesList)
			{
				if (m_equipment->deviceObject(localTuningSourceId) == nullptr)
				{
					// This source does not exist
					//
					m_log->errEQP6109(localTuningSourceId, m_software->equipmentIdTemplate());
					return false;
				}

				if (serviceTuningSourcesList.contains(localTuningSourceId) == false)
				{
					// This source is for TuningService which is not connected to this Monitor
					//
					m_log->errEQP6203(localTuningSourceId, m_software->equipmentId(), settings->tuningServiceID);
					return false;
				}
			}
		}
		else
		{
			m_tuningSources = serviceTuningSourcesList;
		}

		if (m_tuningSources.isEmpty() == true)
		{
			m_log->errEQP6205(m_software->equipmentId());
			return false;
		}

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
		VFrame30::SchemaDetailsSet detailsSet;

		for (auto schemaFile : monitorSchemas)
		{
			result &= m_cfgXml->addLinkToFile(schemaFile->subDir, schemaFile->fileName);
			detailsSet.add(schemaFile->details);
		}

		// Save details
		//
		{
			QByteArray fileData;

			if (bool ok = detailsSet.saveToByteArray(&fileData);
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
		data.resize(static_cast<int>(tuningSet.ByteSizeLong()));

		tuningSet.SerializeToArray(data.data(), static_cast<int>(tuningSet.ByteSizeLong()));

		// Write file
		//
		BuildFile* buildFile = m_buildResultWriter->addFile(m_software->equipmentIdTemplate(), "TuningSignals.dat", CfgFileId::TUNING_SIGNALS, "", data);

		if (buildFile == nullptr)
		{
			m_log->errCMN0012("TuningSignals.dat");
			return false;
		}

		ok = m_cfgXml->addLinkToFile(buildFile);
		return ok;
	}

	bool MonitorCfgGenerator::writeMonitorBehavior()
	{
		if (m_dbController == nullptr)
		{
			Q_ASSERT(m_dbController);
			return false;
		}

		bool ok = true;
		QString behaviorId = getObjectProperty<QString>(m_software->equipmentIdTemplate(), "BehaviorID", &ok).trimmed();
		if (ok == false)
		{
			return false;
		}

		if (behaviorId.isEmpty() == true)
		{
			return true;
		}

		// Load all clients behavior
		//
		ClientBehaviorStorage allBehaviorStorage;
		QString errorCode;
		QByteArray dbData;
		int etcFileId = m_dbController->systemFileId(DbDir::EtcDir);

		bool result = loadFileFromDatabase(m_dbController, etcFileId, allBehaviorStorage.dbFileName(), &errorCode, &dbData);
		if (result == false)
		{
			m_log->errPDB2002(-1, allBehaviorStorage.dbFileName(), errorCode);
			return false;
		}

		if (allBehaviorStorage.load(dbData, &errorCode) == false)
		{
			m_log->errCMN0010(allBehaviorStorage.dbFileName());
			return false;
		}

		// Find behavior for current Monitor
		//
		ClientBehaviorStorage monitorBehaviorStorage;

		std::vector<std::shared_ptr<MonitorBehavior>> behaviors = allBehaviorStorage.monitorBehaviors();

		for (auto b : behaviors)
		{
			if (b->behaviorId() == behaviorId)
			{
				monitorBehaviorStorage.add(b);
				break;
			}
		}

		if (monitorBehaviorStorage.count() == 0)
		{
			m_log->errEQP6210(behaviorId, m_software->equipmentIdTemplate());
			return false;
		}

		// Save monitor behavior to XML
		//
		QByteArray data;
		monitorBehaviorStorage.save(&data);

		// Write file
		//
		BuildFile* buildFile = m_buildResultWriter->addFile(m_software->equipmentIdTemplate(), "MonitorBehavior.xml", CfgFileId::CLIENT_BEHAVIOR, "", data);
		if (buildFile == nullptr)
		{
			return false;
		}

		ok = m_cfgXml->addLinkToFile(buildFile);

		return ok;
	}

	bool MonitorCfgGenerator::writeMonitorLogo()
	{
		if (m_dbController == nullptr)
		{
			Q_ASSERT(m_dbController);
			return false;
		}

		bool ok = true;
		QString logoFile = getObjectProperty<QString>(m_software->equipmentIdTemplate(), "Logo", &ok).trimmed();
		if (ok == false)
		{
			return false;
		}

		DbFileInfo fi;
		ok = m_dbController->getFileInfo(logoFile, &fi, nullptr);
		if (ok == false || fi.isNull() == true)
		{
			m_log->errPDB2007(logoFile, m_software->equipmentIdTemplate(), "Logo");
			return false;
		}

		std::shared_ptr<DbFile> file;
		ok = m_dbController->getLatestVersion(fi, &file, nullptr);
		if (ok == false || file == nullptr)
		{
			m_log->errPDB2002(fi.fileId(), fi.fileName(), m_dbController->lastError());
			return false;
		}

		QByteArray data;
		file->swapData(data);

		// Try to parse image
		//
		QImage image = QImage::fromData(data);
		if (image.isNull() == true)
		{
			m_log->errCMN0010(logoFile);
			return false;
		}

		// Write file
		//
		QString buildFileName = tr("Logo.%1").arg(QFileInfo(fi.fileName()).completeSuffix());

		BuildFile* buildFile = m_buildResultWriter->addFile(m_software->equipmentIdTemplate(), buildFileName, CfgFileId::LOGO, "", data);
		if (buildFile == nullptr)
		{
			return false;
		}

		ok = m_cfgXml->addLinkToFile(buildFile);

		return ok;
	}
}
