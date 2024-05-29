#include "TuningClientCfgGenerator.h"
#include "ScriptChecker.h"
#include "SoftwareSettingsGetter.h"

#include "../OnlineLib/SoftwareSettings.h"

#include <VFrame30/LogicSchema.h>
#include <Behavior/ClientBehaviorStorage.h>
#include <Behavior/TuningClientBehavior.h>
#include "AppSignalListStorage.h"

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

		if (settingsGetter.readSoftwareSettings(m_context, m_software) == false)
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

		std::shared_ptr<const TuningClientSettings> settings = m_settingsSet.getSettingsDefaultProfile<TuningClientSettings>();

		bool result = true;

		// Check tuning users list
		//
		if (settings->tuningLogin == true &&
				settings->tuningUserAccounts.split(Separator::SEMICOLON, Qt::SkipEmptyParts).isEmpty() == true)
		{
			m_log->errEQP6202(EquipmentPropNames::TUNING_USER_ACCOUNTS, EquipmentPropNames::TUNING_LOGIN, m_software->equipmentIdTemplate());
			return false;
		}

		QStringList tuningSources;
		result &= createTuningEquipmentList(&tuningSources);

		// Generate tuning signals
		//
		std::vector<AppSignal*> tuningSignals;
		createTuningSignals(tuningSources, m_signalSet, tuningSignals);

		// Write tuning signals
		//
		result &= writeTuningSignals(tuningSignals);

		// Write Tuning Signal Lists
		//
		{
			ILogFileStub logFileStub;
			Builder::AppSignalListsProvider tuningSignalProvider(tuningSignals);

			// --
			//
			result &= createObjectFilters(tuningSignalProvider, tuningSources);
			result &= writeObjectFilters();

			// Write AppSignalLists
			//
			result &= writeAppSignalLists(tuningSignalProvider, settings->appSignalListIds, settings->appSignalListMasks, settings->appSignalListTags);
		}

		result &= writeTuningSchemas();

		result &= writeGlobalScript();

		result &= writeTuningClientBehavior();

		if (settings->tuningLogin == true)
		{
			result &= writeMatsUsers(EquipmentPropNames::TUNING_USER_ACCOUNTS,
									 settings->tuningUserAccounts.split(Separator::SEMICOLON, Qt::SkipEmptyParts));
		}

		return result;
	}

	bool TuningClientCfgGenerator::createTuningEquipmentList(QStringList* equipmentList)
	{
		if (equipmentList == nullptr)
		{
			assert(equipmentList);
			return false;
		}

		bool result = true;

		equipmentList->clear();

		std::shared_ptr<const TuningClientSettings> settings = m_settingsSet.getSettingsDefaultProfile<TuningClientSettings>();

		for(const SoftwareEndpoint::TuningService& tsc : settings->tuningServices)
		{
			std::shared_ptr<Hardware::DeviceObject> tuningServiceObject = m_equipment->deviceObject(tsc.equipmentId);
			if (tuningServiceObject == nullptr)
			{
				m_log->errCFG3021(m_software->equipmentId(), EquipmentPropNames::TUNING_SERVICE_ID, tsc.equipmentId);
				result = false;
				continue;
			}

			std::shared_ptr<Hardware::Software> tuningServiceSoftware = tuningServiceObject->toSoftware();
			if (tuningServiceSoftware == nullptr)
			{
				m_log->errCFG3021(m_software->equipmentId(), EquipmentPropNames::TUNING_SERVICE_ID, tsc.equipmentId);
				result = false;
				continue;
			}

			TuningServiceSettingsGetter tsg;

			if (tsg.readSoftwareSettings(m_context, tuningServiceSoftware.get()) == false)
			{
				result = false;
				continue;
			}

			TuningServiceSettingsGetter::TuningClient tunClient = tsg.getTuningClient(equipmentID());

			if (tunClient.isValid() == true)
			{
				QStringList clientEquipmentList = tunClient.uniqueSourcesIDs();

				for (const QString& ce : clientEquipmentList )
				{
					if (equipmentList->contains(ce) == false)
					{
						equipmentList->append(ce);
					}
				}
			}
			else
			{
				LOG_INTERNAL_ERROR_MSG(m_log, QString("TuningClient %1 isn't found in clients list of TuningService %2").
									   arg(equipmentID()).arg(tsc.equipmentId));
				result = false;
			}
		}

		return result;
	}

	bool TuningClientCfgGenerator::createObjectFilters(const ISignalManager& tuningSignalManager,
													   const QStringList& equipmentList)
	{
		bool ok = true;

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
				m_log->errEQP6220(p.first, p.second);
			}

			return false;
		}

		// Create schemas and equipment filters
		//
		ok = createEquipmentAndSchemaFilters(equipmentList, tuningSignalManager);
		if (ok == false)
		{
			assert(false);
			return false;
		}

		// Create counter filters for schemas and equipment from templates
		//
		createCounterFiltersFromTemplates();


		// Count all hashes contained in filters and save them
		//
		m_tuningFilterStorage.createSignalsAndEqipmentHashes(tuningSignalManager,
															 tuningSignalManager.signalHashes(),
															 m_tuningFilterStorage.root().get(),
															 TuningFilter::Source::All);

		return true;
	}


	bool TuningClientCfgGenerator::createEquipmentAndSchemaFilters(const QStringList& equipmentList,
														  const ISignalManager& tuningSignalManager)
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
				
				for (const auto schemaSignals = schema->getSignalList();
					 const QString& schemaSignal : schemaSignals)
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

	void TuningClientCfgGenerator::createCounterFiltersFromTemplates()
	{
		std::vector<std::shared_ptr<TuningFilter>> templateFilters;

		// Find counter templates

		const std::shared_ptr<TuningFilter>& root = m_tuningFilterStorage.root();

		int count = root->childFiltersCount();
		for (int i = count - 1; i >= 0; i--)
		{
			std::shared_ptr<TuningFilter> f = root->childFilter(i);

			if (f->isCounter() == true && f->counterType() == TuningFilter::CounterType::FilterTree)
			{
				templateFilters.insert(templateFilters.begin(), f);
			}
		}

		// Add counter filters to every schema and equipment filter

		count = root->childFiltersCount();

		for (int i = 0; i < count; i++)
		{
			std::shared_ptr<TuningFilter> f = root->childFilter(i);

			if (f->isSourceSchema() == true || f->isSourceEquipment() == true) // This is parent schemas or equipment filter
			{
				Q_ASSERT(f->hasDiscreteCounter() == false);

				int schemaCount = f->childFiltersCount();

				for (int s = 0; s < schemaCount; s++)
				{
					std::shared_ptr<TuningFilter> sf = f->childFilter(s);

					Q_ASSERT(sf->hasDiscreteCounter() == false);

					Q_ASSERT(sf->isSourceSchema() == true || sf->isSourceEquipment() == true);

					for (auto& tf: templateFilters)
					{
						std::shared_ptr<TuningFilter> cf = std::make_shared<TuningFilter>(*tf);
						sf->addChild(cf);
					}
				}
			}
		}
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
		VFrame30::SchemaDetailsSet detailsSet;

		for (auto schemaFile : tuningSchemas)
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

			// Check script correctness
			//
			result &= ScriptChecker::checkEquipmentProperty(globalScript, equipmentID(), "GlobalScript", *m_log);

			// Write file.
			//
			BuildFile* globalScriptBuildFile = m_buildResultWriter->addFile(m_software->equipmentIdTemplate(), File::GLOBAL_SCRIPT, CfgFileId::TUNING_GLOBALSCRIPT, "", globalScript);

			m_cfgXml->addLinkToFile(globalScriptBuildFile);
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
		Behavior::ClientBehaviorStorage allBehaviorStorage;
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
		Behavior::ClientBehaviorStorage tcBehaviorStorage;

		for (auto behaviors = allBehaviorStorage.tuningClientBehaviors();
			 auto b : behaviors)
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
}
