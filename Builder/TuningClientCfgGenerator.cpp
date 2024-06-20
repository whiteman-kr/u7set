#include "TuningClientCfgGenerator.h"
#include "ScriptChecker.h"
#include "SoftwareSettingsGetter.h"

#include "../OnlineLib/SoftwareSettings.h"

#include "AppSignalListStorage.h"
#include <Behavior/ClientBehaviorStorage.h>
#include <Behavior/TuningClientBehavior.h>
#include <VFrame30/LogicSchema.h>

namespace Builder
{

	TuningClientCfgGenerator::TuningClientCfgGenerator(Context* context, Hardware::Software* software) :
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
		if (m_software == nullptr || m_software->softwareType() != E::SoftwareType::TuningClient || m_equipment == nullptr ||
			m_cfgXml == nullptr || m_buildResultWriter == nullptr || m_subsystems == nullptr)
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
		if (settings->tuningLogin == true && settings->tuningUserAccounts.split(Separator::SEMICOLON, Qt::SkipEmptyParts).isEmpty() == true)
		{
			m_log->errEQP6202(EquipmentPropNames::TUNING_USER_ACCOUNTS,
							  EquipmentPropNames::TUNING_LOGIN,
							  m_software->equipmentIdTemplate());
			return false;
		}

		QStringList tuningSources;
		result &= createTuningEquipmentList(&tuningSources);

		// Generate tuning signals
		//
		std::vector<AppSignal*> tuningSignals = createTuningSignals(tuningSources, *m_signalSet);

		// Write tuning signals
		//
		result &= writeTuningSignals(tuningSignals);

		// Write Tuning Ui
		//
		result &= writeTuningUi(settings->appSignalListIds, settings->appSignalListMasks, settings->appSignalListTags);

		// Write Tuning Signal Lists
		//
		{
			Builder::AppSignalListsProvider tuningSignalProvider(tuningSignals);

			// Create Equipment lists
			//
			if (settings->filterByEquipment == true)
			{
				result &= createEquipmentLists(tuningSources, tuningSignalProvider);
			}

			// Create Schemas lists
			//
			QStringList appSignalListIds = settings->appSignalListIds;
			if (settings->filterBySchema == true)
			{
				result &= createSchemasLists(tuningSignalProvider);
			}

			// Write AppSignalLists
			//
			result &=
				writeAppSignalLists(tuningSignalProvider, settings->appSignalListIds, settings->appSignalListMasks, settings->appSignalListTags);
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

		for (const SoftwareEndpoint::TuningService& tsc : settings->tuningServices)
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

				for (const QString& ce : clientEquipmentList)
				{
					if (equipmentList->contains(ce) == false)
					{
						equipmentList->append(ce);
					}
				}
			}
			else
			{
				LOG_INTERNAL_ERROR_MSG(
					m_log,
					QString("TuningClient %1 isn't found in clients list of TuningService %2").arg(equipmentID()).arg(tsc.equipmentId));
				result = false;
			}
		}

		return result;
	}

	bool TuningClientCfgGenerator::createEquipmentLists(const QStringList& equipmentList, const ISignalManager& tuningSignalManager)
	{
		// Filter for EquipmentId
		//
		for (const QString& equipmentId : equipmentList)
		{
			std::shared_ptr<AppSignalLists::AppSignalList> list = std::make_shared<AppSignalLists::AppSignalList>();
			list->setId(equipmentId);
			list->setCaption(equipmentId);
			list->setEquipmentIDMask(equipmentId);

			// Add filtered signals to the list
			//
			auto& mutableAppCache = list->mutableAppListHashesCache();
			auto& mutableTuningCache = list->mutableTuningListHashesCache();
			auto tuningSignals = tuningSignalManager.signalList();
			for (const auto& asp : tuningSignals)
			{
				if (list->appSignalMatch(asp) == true)
				{
					Hash hash = ::calcHash(asp.appSignalId());
					mutableAppCache.insert(hash);
					mutableTuningCache.insert(hash);
				}
			}

			// Add tag "created by ide" to the list
			//
			list->systemTagsList().push_back(AppSignalLists::AppSignalList::tagEquipment);
			list->systemTagsList().push_back(AppSignalLists::AppSignalList::tagIde);


			// Save list to the data buffer
			//
			Proto::Envelope envelope;
			list->SaveData(&envelope);

			QByteArray data;
			data.resize(static_cast<int>(envelope.ByteSizeLong()));

			bool result = envelope.SerializeToArray(data.data(), static_cast<int>(envelope.ByteSizeLong()));
			if (result == false)
			{
				Q_ASSERT(result);
				return false;
			}

			// Write file
			//
			QString fileName = tr("%1.%2").arg(list->id()).arg(Db::File::AppSignalListFileExtension);
			BuildFile* listsFile = m_context->m_buildResultWriter->addFile(m_software->equipmentIdTemplate(),
																		   fileName,
																		   fileName,
																		   {CfgFileTag::APPSIGNALLISTS},
																		   data);
			if (listsFile == nullptr)
			{
				Q_ASSERT(listsFile);
				return false;
			}
			m_cfgXml->addLinkToFile(listsFile);
		}


		return true;
	}

	bool TuningClientCfgGenerator::createSchemasLists(const ISignalManager& tuningSignalManager) 
	{
		// Filter for EquipmentId
		//
		for (const std::shared_ptr<VFrame30::LogicSchema>& schema : m_context->m_appLogicSchemas)
		{
			std::shared_ptr<AppSignalLists::AppSignalList> list = std::make_shared<AppSignalLists::AppSignalList>();
			list->setId(schema->schemaId());
			list->setCaption(schema->caption());

			auto& mutableAppCache = list->mutableAppListHashesCache();
			auto& mutableTuningCache = list->mutableTuningListHashesCache();

			for (const auto schemaSignals = schema->getSignalList(); const QString& schemaSignal : schemaSignals)
			{
				Hash hash = ::calcHash(schemaSignal);

				// find if this signal is a tuning signal
				//
				if (tuningSignalManager.signalExists(hash) == false)
				{
					continue;
				}

				AppSignalLists::AppSignalListItem li(schemaSignal);
				list->add(li);

				mutableAppCache.insert(hash);
				mutableTuningCache.insert(hash);
			}

			if (list->count() == 0)
			{
				// Do not add empty filters
				//
				continue;
			}

			// Add tag "created by ide" to the list
			//
			list->systemTagsList().push_back(AppSignalLists::AppSignalList::tagSchema);
			list->systemTagsList().push_back(AppSignalLists::AppSignalList::tagIde);

			// Save list to the data buffer
			//
			Proto::Envelope envelope;
			list->SaveData(&envelope);

			QByteArray data;
			data.resize(static_cast<int>(envelope.ByteSizeLong()));

			bool result = envelope.SerializeToArray(data.data(), static_cast<int>(envelope.ByteSizeLong()));
			if (result == false)
			{
				Q_ASSERT(result);
				return false;
			}

			// Write file
			//
			QString fileName = tr("%1.%2").arg(list->id()).arg(Db::File::AppSignalListFileExtension);
			BuildFile* listsFile =
				m_buildResultWriter->addFile(m_software->equipmentIdTemplate(), fileName, fileName, {CfgFileTag::APPSIGNALLISTS}, data);
			if (listsFile == nullptr)
			{
				Q_ASSERT(listsFile);
				return false;
			}
			m_cfgXml->addLinkToFile(listsFile);
		}

		return true;
	}

	bool TuningClientCfgGenerator::writeTuningUi(const QStringList& appSignalListIds,
												 const QStringList& appSignalListMasks,
												 const QStringList& appSignalListTags)
	{
		TuningLib::TuningUiStorage tuningUiStorage;

		bool ok = true;

		QString uiConfiguration = getObjectProperty<QString>(m_software->equipmentIdTemplate(), "UiConfiguration", &ok).trimmed();
		if (ok == false)
		{
			return false;
		}

		if (uiConfiguration.isEmpty() == true)
		{
			m_log->errCFG3022(m_software->equipmentId(), "UiConfiguration");
			return false;
		}

		// Load project ui
		//
		QString errorCode;
		ok = tuningUiStorage.load(uiConfiguration.toUtf8(), &errorCode);
		if (ok == false)
		{
			m_log->errEQP6107("UiConfiguration", m_software->equipmentId());
			return false;
		}

		// Check if all signal lists specified in Filters property exist
		//
		QStringList appSignalLists;
		for (const auto& [id, list] : m_appSignalsListIdToList)
		{
			// Check if this list is for this software
			//
			if (list->listMatch(appSignalListIds, appSignalListMasks, appSignalListTags) == false)
			{
				continue;
			}

			appSignalLists.push_back(id);
		}
		std::vector<std::pair<QString, QString>> notFoundFilters = tuningUiStorage.checkFilters(appSignalLists);
		for (const auto& [filterId, uiCaption] : notFoundFilters)
		{
			m_log->errEQP6251(filterId, uiCaption, m_software->equipmentId());
		}

		// Save Ui
		//
		QByteArray data;
		ok = tuningUiStorage.save(data);
		if (ok == false)
		{
			assert(false);
			return false;
		}

		BuildFile* buildFile =
			m_buildResultWriter->addFile(m_software->equipmentIdTemplate(), "TuningUi.xml", CfgFileId::TUNING_UI, "", data);
		if (buildFile == nullptr)
		{
			m_log->errCMN0012("TuningUi.xml");
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
			for (auto& [tag, schemaFile] : SoftwareCfgGenerator::m_schemaTagToFile)
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

					if (mapTag != tag || schemaFile == nullptr)
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

			if (bool ok = detailsSet.saveToByteArray(&fileData); ok == true)
			{
				BuildFile* schemaDetailsBuildFile =
					m_buildResultWriter->addFile(m_software->equipmentIdTemplate(), "SchemaDetails.pbuf", fileData);

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
			BuildFile* globalScriptBuildFile = m_buildResultWriter->addFile(m_software->equipmentIdTemplate(),
																			File::GLOBAL_SCRIPT,
																			CfgFileId::TUNING_GLOBALSCRIPT,
																			"",
																			globalScript);

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

		for (auto behaviors = allBehaviorStorage.tuningClientBehaviors(); auto b : behaviors)
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
		BuildFile* buildFile = m_buildResultWriter->addFile(m_software->equipmentIdTemplate(),
															"TuningClientBehavior.xml",
															CfgFileId::CLIENT_BEHAVIOR,
															"",
															data);

		if (buildFile == nullptr)
		{
			m_log->errCMN0012("TuningClientBehavior.xml");
			return false;
		}

		ok = m_cfgXml->addLinkToFile(buildFile);

		return ok;
	}
} // namespace Builder
