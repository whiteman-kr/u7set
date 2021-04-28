#include "TuningClientCfgGenerator.h"
#include "../lib/SoftwareSettings.h"
#include "../VFrame30/Schema.h"
#include "../lib/ClientBehavior.h"

namespace Builder
{

	TuningClientCfgGenerator::TuningClientCfgGenerator(Context* context, Hardware::Software* software)	:
		SoftwareCfgGenerator(context, software),
		m_subsystems(context->m_subsystems.get())
	{
		assert(context);
	}

	bool TuningClientCfgGenerator::createSettingsProfile(const QString& profile)
	{
		TuningClientSettingsGetter settingsGetter;

		if (settingsGetter.readFromDevice(m_context, m_software) == false)
		{
			return false;
		}

		return m_settingsSet.addProfile<TuningClientSettings>(profile, settingsGetter);
	}

	bool TuningClientCfgGenerator::generateConfigurationStep1()
	{
		if (m_software == nullptr ||
				m_software->softwareType() != E::SoftwareType::TuningClient ||
				m_equipment == nullptr ||
				m_cfgXml == nullptr ||
				m_buildResultWriter == nullptr ||
				m_subsystems == nullptr)
		{
			assert(m_software);
			assert(m_software->softwareType() == E::SoftwareType::Monitor);
			assert(m_equipment);
			assert(m_cfgXml);
			assert(m_buildResultWriter);
			assert(m_subsystems);
			return false;
		}

		IssueLogger* log = m_buildResultWriter->log();
		if (log == nullptr)
		{
			assert(log);
			return false;
		}

		bool result = true;

		QStringList equipmentList;

		result &= createEquipmentList(&equipmentList);
		if (result == false)
		{
			return result;
		}

		// Generate tuning signals
		//
		if (equipmentList.empty() == true)
		{
			log->errCFG3022(m_software->equipmentIdTemplate(), "TuningSourceEquipmentID");
			return false;
		}

		result &= createTuningSignals(equipmentList, m_signalSet, &m_tuningSet);
		if (result == false)
		{
			return result;
		}

		result &= writeTuningSchemas();
		if (result == false)
		{
			return result;
		}

		// --
		//
		result &= createObjectFilters(equipmentList);
		if (result == false)
		{
			return result;
		}

		result &= writeTuningSignals();
		if (result == false)
		{
			return result;
		}

		result &= writeObjectFilters();
		if (result == false)
		{
			return result;
		}

		result &= writeGlobalScript();

		result &= writeTuningClientBehavior();

		return result;
	}

	bool TuningClientCfgGenerator::createTuningSignals(const QStringList& equipmentList, const SignalSet* signalSet, Proto::AppSignalSet* tuningSet)
	{
		if (tuningSet == nullptr ||
			signalSet == nullptr ||
			equipmentList.empty() == true)
		{
			assert(tuningSet);
			assert(signalSet);
			assert(equipmentList.empty() == false);
			return false;
		}

		// Create signals
		//
		tuningSet->Clear();

		int signalsCount = signalSet->count();

		for (int i = 0; i < signalsCount; i++)
		{
			const AppSignal& s = (*signalSet)[i];

			if (s.enableTuning() == false)
			{
				continue;
			}

			// Check EquipmentIdMasks
			//
			bool result = false;

			for (QString m : equipmentList)
			{
				m = m.trimmed();

				if (m.isEmpty() == true)
				{
					continue;
				}

				QRegExp rx(m);
				rx.setPatternSyntax(QRegExp::Wildcard);
				if (rx.exactMatch(s.lmEquipmentID()))
				{
					result = true;
					break;
				}
			}

			if (result == false)
			{
				continue;
			}

			::Proto::AppSignal* aspMessage = tuningSet->add_appsignal();
			s.serializeTo(aspMessage);
		}

		return true;
	}

	bool TuningClientCfgGenerator::createEquipmentList(QStringList* equipmentList)
	{
		QString equipmentString;

		if (equipmentList == nullptr)
		{
			assert(equipmentList);
			return false;
		}

		//
		// equipmentList
		//
		bool ok = false;

		equipmentString = getObjectProperty<QString>(m_software->equipmentIdTemplate(), "TuningSourceEquipmentID", &ok).trimmed();
		if (ok == false)
		{
			return false;
		}

		// Parse equipmentList
		//
		if (equipmentString.isEmpty() == false)
		{
			equipmentString.replace(' ', ';');
			equipmentString.replace('\n', ';');
			equipmentString.remove('\r');
			*equipmentList = equipmentString.split(';');
		}

		// Check for valid EquipmentIds
		//
		for (const QString& tuningEquipmentID : *equipmentList)
		{
			if (m_equipment->deviceObject(tuningEquipmentID) == nullptr)
			{
				m_log->errEQP6109(tuningEquipmentID, m_software->equipmentIdTemplate());
				return false;
			}
		}

		return true;
	}

	bool TuningClientCfgGenerator::createObjectFilters(const QStringList& equipmentList)
	{
		bool ok = true;

		TuningSignalManager tuningSignalManager;
		tuningSignalManager.load(m_tuningSet);

		//
		// Filters
		//
		QString filters = getObjectProperty<QString>(m_software->equipmentIdTemplate(), "Filters", &ok).trimmed();
		if (ok == false)
		{
			return false;
		}

		if (filters.isEmpty() == true)
		{
			m_log->errCFG3022(m_software->equipmentId(), "Filters");
			return false;
		}

		// Load project filters

		QString errorCode;

		ok = m_tuningFilterStorage.load(filters.toUtf8(), &errorCode);
		if (ok == false)
		{
			m_log->errEQP6107("Filters", m_software->equipmentId());
			return false;
		}

		// Check all filters for non-existing signals

		std::vector<std::pair<QString, QString>> notFoundSignalsAndFilters;

		m_tuningFilterStorage.checkFilterSignals(tuningSignalManager.signalHashes(), notFoundSignalsAndFilters);

		if (notFoundSignalsAndFilters.empty() == false)
		{
			for (const std::pair<QString, QString>& p: notFoundSignalsAndFilters)
			{
				m_log->errEQP6108(p.first, p.second, m_software->equipmentId());
			}

			return false;
		}

		// Create schemas and equipment filters

		ok = createAutomaticFilters(equipmentList, tuningSignalManager);
		if (ok == false)
		{
			assert(false);
			return false;
		}

		return true;
	}


	bool TuningClientCfgGenerator::writeTuningSignals()
	{
		// Write number of signals
		//
		QByteArray data;
		data.resize(static_cast<int>(m_tuningSet.ByteSizeLong()));

		m_tuningSet.SerializeToArray(data.data(), static_cast<int>(m_tuningSet.ByteSizeLong()));

		// Write file
		//
		BuildFile* buildFile = m_buildResultWriter->addFile(m_software->equipmentIdTemplate(), "TuningSignals.dat", CfgFileId::TUNING_SIGNALS, "", data);

		if (buildFile == nullptr)
		{
			m_log->errCMN0012("TuningSignals.dat");
			return false;
		}

		m_cfgXml->addLinkToFile(buildFile);

		return true;
	}

	bool TuningClientCfgGenerator::writeObjectFilters()
	{
		// Save filters to file

		QByteArray data;

		bool ok = m_tuningFilterStorage.save(data);
		if (ok == false)
		{
			assert(false);
			return false;
		}

		BuildFile* buildFile = m_buildResultWriter->addFile(m_software->equipmentIdTemplate(), "ObjectFilters.xml", CfgFileId::TUNING_FILTERS, "",  data);

		if (buildFile == nullptr)
		{
			m_log->errCMN0012("ObjectFilters.xml");
			return false;
		}

		m_cfgXml->addLinkToFile(buildFile);

		return true;

	}

	bool TuningClientCfgGenerator::writeTuningSchemas()
	{
		bool result = true;

		std::shared_ptr<const TuningClientSettings> settings = m_settingsSet.getSettingsDefaultProfile<TuningClientSettings>();

		TEST_PTR_LOG_RETURN_FALSE(settings, m_log);

		QStringList schemaTagList = settings->getSchemaTags();

		std::set<std::shared_ptr<SchemaFile>> tuningSchemas;

		// If tag list is empty, then link all Tuning schemas
		//
		if (schemaTagList.isEmpty() == true)
		{
			for (auto&[tag, schemaFile] : SoftwareCfgGenerator::m_schemaTagToFile)
			{
				Q_UNUSED(tag);
				if (schemaFile->fileName.endsWith(QStringLiteral(".") + Db::File::TvsFileExtension, Qt::CaseInsensitive) == true)
				{
					tuningSchemas.insert(schemaFile);
				}
			}
		}
		else
		{
			for (QString tag : schemaTagList)
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
						assert(mapTag == tag);
						assert(schemaFile);
						continue;
					}

					tuningSchemas.insert(schemaFile);
				}
			}
		}

		// --
		//
		VFrame30::SchemaDetailsSet detaisSet;

		for (auto schemaFile : tuningSchemas)
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

	bool TuningClientCfgGenerator::writeGlobalScript()
	{
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
			BuildFile* globalScriptBuildFile = m_buildResultWriter->addFile(m_software->equipmentIdTemplate(), "GlobalScript.js", CfgFileId::TUNING_GLOBALSCRIPT, "", globalScript);

			m_cfgXml->addLinkToFile(globalScriptBuildFile);
		}

		// Writing OnConfigurationArrived
		//
		result = true;

		if (m_software->propertyExists("OnConfigurationArrived") == false)
		{
			m_log->errCFG3000("OnConfigurationArrived", m_software->equipmentIdTemplate());
			result = false;
		}
		else
		{
			QString arrivedScript = m_software->propertyValue("OnConfigurationArrived").toString();
			BuildFile* arrivedScriptBuildFile = m_buildResultWriter->addFile(m_software->equipmentIdTemplate(), "OnConfigurationArrivedScript.js", CfgFileId::TUNING_CONFIGARRIVEDSCRIPT, "", arrivedScript);

			m_cfgXml->addLinkToFile(arrivedScriptBuildFile);
		}

		return result;
	}

	bool TuningClientCfgGenerator::writeTuningClientBehavior()
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
			m_log->errCMN0010(allBehaviorStorage.dbFileName());
			return false;
		}

		if (allBehaviorStorage.load(dbData, &errorCode) == false)
		{
			m_log->errCMN0010(allBehaviorStorage.dbFileName());
			return false;
		}

		// Find behavior for current tuning client
		//
		ClientBehaviorStorage tcBehaviorStorage;

		std::vector<std::shared_ptr<TuningClientBehavior>> behaviors = allBehaviorStorage.tuningClientBehaviors();

		for (auto b : behaviors)
		{
			if (b->behaviorId() == behaviorId)
			{
				tcBehaviorStorage.add(b);
				break;
			}
		}

		if (tcBehaviorStorage.count() == 0)
		{
			m_log->errEQP6210(behaviorId, m_software->equipmentIdTemplate());
			return false;
		}

		// Save monitor behavior to XML
		//
		QByteArray data;
		tcBehaviorStorage.save(&data);

		// Write file
		//
		BuildFile* buildFile = m_buildResultWriter->addFile(m_software->equipmentIdTemplate(), "TuningClientBehavior.xml", CfgFileId::CLIENT_BEHAVIOR, "", data);

		if (buildFile == nullptr)
		{
			m_log->errCMN0012("TuningClientBehavior.xml");
			return false;
		}

		ok = m_cfgXml->addLinkToFile(buildFile);

		return ok;
	}

	void TuningClientCfgGenerator::writeErrorSection(QXmlStreamWriter& xmlWriter, QString error)
	{
		xmlWriter.writeTextElement("Error", error);
	}

	bool TuningClientCfgGenerator::createAutomaticFilters(const QStringList& equipmentList,
														  const TuningSignalManager& tuningSignalManager)
	{
		std::shared_ptr<const TuningClientSettings> settings = m_settingsSet.getSettingsDefaultProfile<TuningClientSettings>();

		TEST_PTR_LOG_RETURN_FALSE(settings, m_log);

		if (settings->filterBySchema == true)
		{
			// Filter for Schema
			//
			std::shared_ptr<TuningFilter> ofSchema = std::make_shared<TuningFilter>(TuningFilter::InterfaceType::Tree);
			ofSchema->setID("%AUTOFILTER%_SCHEMA");
			ofSchema->setCaption(QObject::tr("Schemas"));
			ofSchema->setSource(TuningFilter::Source::Schema);

			for (const std::shared_ptr<VFrame30::LogicSchema>& schema : m_context->m_appLogicSchemas)
			{
				std::shared_ptr<TuningFilter> ofTs = std::make_shared<TuningFilter>(TuningFilter::InterfaceType::Tree);

				const std::set<QString> schemaSignals = schema->getSignalMap();
				for (const QString& schemaSignal : schemaSignals)
				{
					Hash hash = ::calcHash(schemaSignal);

					// find if this signal is a tuning signal
					//
					if (tuningSignalManager.signalExists(hash) == false)
					{
						continue;
					}

					TuningFilterSignal ofv;
					ofv.setAppSignalId(schemaSignal);
					ofTs->addFilterSignal(ofv);
				}

				if (ofTs->filterSignalsCount() == 0)
				{
					// Do not add empty filters
					//
					continue;
				}

				ofTs->setID("%AUFOFILTER%_SCHEMA_" + schema->schemaId());

				//QString s = QString("%1 - %2").arg(schemasDetails.m_Id).arg(schemasDetails.m_caption);
				ofTs->setCaption(schema->caption());
				ofTs->setSource(TuningFilter::Source::Schema);

				ofSchema->addChild(ofTs);
			}

			m_tuningFilterStorage.add(ofSchema, true);
		}	 // filterBySchema

		if (settings->filterByEquipment == true)
		{
			// Filter for EquipmentId
			//
			std::shared_ptr<TuningFilter> ofEquipment = std::make_shared<TuningFilter>(TuningFilter::InterfaceType::Tree);
			ofEquipment->setID("%AUTOFILTER%_EQUIPMENT");
			ofEquipment->setCaption(QObject::tr("Equipment"));
			ofEquipment->setSource(TuningFilter::Source::Equipment);

			for (const QString& ts : equipmentList)
			{
				std::shared_ptr<TuningFilter> ofTs = std::make_shared<TuningFilter>(TuningFilter::InterfaceType::Tree);
				ofTs->setEquipmentIDMask(ts);
				ofTs->setID("%AUFOFILTER%_EQUIPMENT_" + ts);
				ofTs->setCaption(ts);
				ofTs->setSource(TuningFilter::Source::Equipment);

				ofEquipment->addChild(ofTs);
			}

			m_tuningFilterStorage.add(ofEquipment, true);
		} // filterByEquipment

		return true;
	}

}
