#include "MonitorCfgGenerator.h"
#include "TuningClientCfgGenerator.h"
#include "../lib/SoftwareSettings.h"
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
		if (m_settings.tuningEnabled == true)
		{
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

	bool MonitorCfgGenerator::getSettingsXml(QXmlStreamWriter& xmlWriter)
	{
		XmlWriteHelper xml(xmlWriter);

		return m_settings.writeToXml(xml);

		/*
		xmlWriter.writeStartElement("Settings");

		{
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

				m_schemaTagList = schemaTags.split(QRegExp("\\W+"), Qt::SkipEmptyParts);

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
		}

		return true;*/
	}

	bool MonitorCfgGenerator::writeMonitorSettings()
	{
		bool result = m_settings.readFromDevice(m_equipment, m_software, m_log);

		if (result == false)
		{
			return false;
		}

		m_schemaTagList = m_settings.getSchemaTags();

		if (m_settings.tuningEnabled == true &&
			m_context->m_projectProperties.safetyProject() == true)
		{
			// Tuning for Monitor is forbiden for Safety Projects
			// Stupid decision but not mine
			//
			m_log->errEQP6200(m_software->equipmentIdTemplate());
			return false;
		}

		m_tuningSources = m_settings.getTuningSources();

		return getSettingsXml(m_cfgXml->xmlWriter());
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

		bool result = loadFileFromDatabase(m_dbController, m_dbController->etcFileId(), allBehaviorStorage.dbFileName(), &errorCode, &dbData);
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
