#include "../OnlineLib/SoftwareSettings.h"
#include "../VFrame30/Schema.h"

#include "Context.h"
#include "DiagnosticsCfgGenerator.h"
#include "ScriptChecker.h"
#include "SoftwareSettingsGetter.h"


namespace Builder
{
	DiagnosticsCfgGenerator::DiagnosticsCfgGenerator(Context* context, Hardware::Software* software) :
		SoftwareCfgGenerator{context, software}
	{
	}

	bool DiagnosticsCfgGenerator::createSettingsProfile(const QString& profile)
	{
		DiagnosticsSettingsGetter settingsGetter;

		if (settingsGetter.readSoftwareSettings(m_context, m_software) == false)
		{
			return false;
		}

		return m_settingsSet.addProfile<DiagnosticsSettings>(profile, settingsGetter);
	}

	bool DiagnosticsCfgGenerator::generateConfigurationStep1()
	{
		if (m_software == nullptr ||
			m_software->softwareType() != E::SoftwareType::Diagnostics ||
			m_equipment == nullptr ||
			m_cfgXml == nullptr ||
			m_buildResultWriter == nullptr)
		{
			Q_ASSERT(m_software && m_software->softwareType() == E::SoftwareType::Diagnostics);
			Q_ASSERT(m_equipment);
			Q_ASSERT(m_cfgXml);
			Q_ASSERT(m_buildResultWriter);
			return false;
		}

		bool result = true;

		result &= saveScriptProperties("GlobalScript", File::GLOBAL_SCRIPT);
		result &= initSchemaTags();

		// Add links to schema files (previously written) via m_cfgXml->addLinkToFile(...)
		//
		result &= writeSchemasByTags();

		std::shared_ptr<const DiagnosticsSettings> settings = m_settingsSet.getSettingsDefaultProfile<DiagnosticsSettings>();
		TEST_PTR_LOG_RETURN_FALSE(settings, m_log);

		result &= writeAppSignals();
		result &= writeBehaviorFile();
		result &= writeDiagnosticsLogo();

		return result;
	}

	bool DiagnosticsCfgGenerator::generateConfigurationStep2()
	{
		if (bool ok = SoftwareCfgGenerator::generateConfigurationStep2();
			ok == false)
		{
			return false;
		}

		// Check that schema from property startSchemaId is exist. Check for all existing profiles.
		// m_detailsSet must be filled in.
		//
		bool result = true;
		for (QStringList profiles = m_settingsSet.getSettingsProfiles();
			 const QString & profile : profiles)
		{
			std::shared_ptr<const DiagnosticsSettings> profileSettings = m_settingsSet.getSettingsProfile<DiagnosticsSettings>(profile);
			TEST_PTR_LOG_RETURN_FALSE(profileSettings, m_log);

			QString startSchemaId = profileSettings->startSchemaId;
			if (startSchemaId.isEmpty() == false)
			{
				if (m_detailsSet.schemaDetails(startSchemaId) == nullptr)
				{
					m_log->errEQP6211(m_software->equipmentId(), startSchemaId, profile);
					result = false;
				}
			}
		}

		return result;
	}

	bool DiagnosticsCfgGenerator::initSchemaTags()
	{
		std::shared_ptr<const DiagnosticsSettings> settings = m_settingsSet.getSettingsDefaultProfile<DiagnosticsSettings>();

		TEST_PTR_LOG_RETURN_FALSE(settings, m_log);

		m_schemaTagList = settings->getSchemaTags();

		return true;
	}

	bool DiagnosticsCfgGenerator::writeSchemasByTags()
	{
		// class SoftwareCfgGenerator
		//		static std::multimap<QString, std::shared_ptr<SchemaFile>> m_schemaTagToFile;
		//

		bool result = true;
		std::set<std::shared_ptr<SchemaFile>> diagSchemas;

		// If tag list is empty, then link all Diagnostics schemas
		//
		if (m_schemaTagList.isEmpty() == true)
		{
			for (auto& [tag, schemaFile] : SoftwareCfgGenerator::m_schemaTagToFile)
			{
				Q_UNUSED(tag);
				if (schemaFile->fileName.endsWith(QStringLiteral(".") + Db::File::DvsFileExtension, Qt::CaseInsensitive) == true)
				{
					diagSchemas.insert(schemaFile);
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

					diagSchemas.insert(schemaFile);
				}
			}
		}

		// --
		//
		m_detailsSet.clear();

		for (const auto& schemaFile : diagSchemas)
		{
			result &= m_cfgXml->addLinkToFile(schemaFile->subDir, schemaFile->fileName);
			m_detailsSet.add(schemaFile->details);
		}

		// Save details
		//
		{
			QByteArray fileData;

			if (bool ok = m_detailsSet.saveToByteArray(&fileData);
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

	bool DiagnosticsCfgGenerator::writeAppSignals()
	{
		// TODO: Write some basic info about AppSignals.
		//
		return true;

	//	::Proto::AppSignalSet tuningSet;

	//	bool ok = TuningClientCfgGenerator::createTuningSignals(m_tuningSources, m_signalSet, &tuningSet);
	//	if (ok == false)
	//	{
	//		m_log->errINT1000("Generate tuning signal set error: MonitorCfgGenerator::writeTuningSignals, call for TuningClientCfgGenerator::createTuningSignals");
	//		return false;
	//	}

	//	// Write number of signals
	//	//
	//	QByteArray data;
	//	data.resize(static_cast<int>(tuningSet.ByteSizeLong()));

	//	tuningSet.SerializeToArray(data.data(), static_cast<int>(tuningSet.ByteSizeLong()));

	//	// Write file
	//	//
	//	BuildFile* buildFile = m_buildResultWriter->addFile(m_software->equipmentIdTemplate(), "TuningSignals.dat", CfgFileId::TUNING_SIGNALS, "", data);

	//	if (buildFile == nullptr)
	//	{
	//		m_log->errCMN0012("TuningSignals.dat");
	//		return false;
	//	}

	//	ok = m_cfgXml->addLinkToFile(buildFile);
	//	return ok;
	//}

	//bool MonitorCfgGenerator::writeMonitorBehavior()
	//{
	//	if (m_dbController == nullptr)
	//	{
	//		Q_ASSERT(m_dbController);
	//		return false;
	//	}

	//	bool ok = true;
	//	QString behaviorId = getObjectProperty<QString>(m_software->equipmentIdTemplate(), "BehaviorID", &ok).trimmed();
	//	if (ok == false)
	//	{
	//		return false;
	//	}

	//	if (behaviorId.isEmpty() == true)
	//	{
	//		return true;
	//	}

	//	// Load all clients behavior
	//	//
	//	ClientBehaviorStorage allBehaviorStorage;
	//	QString errorCode;
	//	QByteArray dbData;
	//	int etcFileId = m_dbController->systemFileId(DbDir::EtcDir);

	//	bool result = loadFileFromDatabase(m_dbController, etcFileId, allBehaviorStorage.dbFileName(), &errorCode, &dbData);
	//	if (result == false)
	//	{
	//		m_log->errPDB2002(-1, allBehaviorStorage.dbFileName(), errorCode);
	//		return false;
	//	}

	//	if (allBehaviorStorage.load(dbData, &errorCode) == false)
	//	{
	//		m_log->errCMN0010(allBehaviorStorage.dbFileName());
	//		return false;
	//	}

	//	// Find behavior for current Monitor
	//	//
	//	ClientBehaviorStorage monitorBehaviorStorage;

	//	std::vector<std::shared_ptr<MonitorBehavior>> behaviors = allBehaviorStorage.monitorBehaviors();

	//	for (auto b : behaviors)
	//	{
	//		if (b->behaviorId() == behaviorId)
	//		{
	//			monitorBehaviorStorage.add(b);
	//			break;
	//		}
	//	}

	//	if (monitorBehaviorStorage.count() == 0)
	//	{
	//		m_log->errEQP6210(behaviorId, m_software->equipmentIdTemplate());
	//		return false;
	//	}

	//	// Save monitor behavior to XML
	//	//
	//	QByteArray data;
	//	monitorBehaviorStorage.save(&data);

	//	// Write file
	//	//
	//	BuildFile* buildFile = m_buildResultWriter->addFile(m_software->equipmentIdTemplate(), "MonitorBehavior.xml", CfgFileId::CLIENT_BEHAVIOR, "", data);
	//	if (buildFile == nullptr)
	//	{
	//		return false;
	//	}

	//	ok = m_cfgXml->addLinkToFile(buildFile);

	//	return ok;
	}

	bool DiagnosticsCfgGenerator::writeBehaviorFile()
	{
		// TODO: Write behavior file.
		return true;
	}

	bool DiagnosticsCfgGenerator::writeDiagnosticsLogo()
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
} // namespace Builder
