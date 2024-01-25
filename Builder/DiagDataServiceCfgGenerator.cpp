#include "DiagDataServiceCfgGenerator.h"
#include "SoftwareSettingsGetter.h"
#include "../OnlineLib/SoftwareSettings.h"
#include "../lib/DataSource.h"
#include "../HardwareLib/DiagSignal.h"

namespace Builder
{
	// -----------------------------------------------------------------------------------------
	//
	// DiagDataServiceCfgGenerator class implementation
	//
	// -----------------------------------------------------------------------------------------

	DiagDataServiceCfgGenerator::DiagDataServiceCfgGenerator(Context* context, Hardware::Software* software) :
		SoftwareCfgGenerator(context, software)
	{
	}

	DiagDataServiceCfgGenerator::~DiagDataServiceCfgGenerator()
	{
	}

	bool DiagDataServiceCfgGenerator::createSettingsProfile(const QString& profile)
	{
		DiagDataServiceSettingsGetter settingsGetter;

		if (settingsGetter.readSoftwareSettings(m_context, m_software) == false)
		{
			return false;
		}

		bool result = m_settingsSet.addProfile<DiagDataServiceSettings>(profile, settingsGetter);

		result &= writeRunScriptFile(profile, settingsGetter, E::OS::Windows);
		result &= writeRunScriptFile(profile, settingsGetter, E::OS::Linux);

		return result;
	}

	bool DiagDataServiceCfgGenerator::generateConfigurationStep1()
	{
		bool result = false;

		do
		{
			if (writeDiagSignalTypesXml() == false) break;
			if (writeDiagDataSourcesXml() == false) break;
			if (writeAcquiredDiagSignalsFile() == false) break;

			result = true;
		}
		while(false);

		return result;
	}

	bool DiagDataServiceCfgGenerator::writeRunScriptFile(const QString& profile,
														 const DiagDataServiceSettings& settings,
														 E::OS os)
	{
		TEST_PTR_RETURN_FALSE(m_software);

		QString content = getBuildInfoComments(os);

		QString cmdLine = getCommonCmdLine(settings.cfgServiceIP1, settings.cfgServiceIP2, os, true);

		if (cmdLine.isEmpty() == true)
		{
			return false;
		}

		content += cmdLine;

		BuildFile* buildFile = m_buildResultWriter->addFile(getRunScriptDirectory(os),
															getRunScriptName(profile, os),
															content);

		TEST_PTR_RETURN_FALSE(buildFile);

		return true;
	}

	bool DiagDataServiceCfgGenerator::writeDiagSignalTypesXml()
	{
		TEST_PTR_RETURN_FALSE(m_context);
		TEST_PTR_RETURN_FALSE(m_context->m_diagSignalTypes);

		Hardware::DiagSignalTypes dsts;

		m_context->m_diagSignalTypes->get(dsts.mutableDiagSignalTypes());

		QByteArray fileData;
		XmlWriteHelper xml(&fileData);

		xml.setAutoFormatting(true);

		dsts.writeToXml(xml);

		BuildFile* buildFile = m_buildResultWriter->addFile(softwareCfgSubdir(), File::DIAG_SIGNAL_TYPES_XML,
															CfgFileId::DIAG_SIGNAL_TYPES, "", fileData);
		if (buildFile == nullptr)
		{
			return false;
		}

		return m_cfgXml->addLinkToFile(buildFile);
	}

	bool DiagDataServiceCfgGenerator::writeDiagDataSourcesXml()
	{
		bool result = true;

		m_lmAcquiredDiagSignals.clear();

		QVector<DataSource> dataSources;

		QStringList profiles = m_settingsSet.getSettingsProfiles();

		for(const QString& profile : profiles)
		{
			std::shared_ptr<const DiagDataServiceSettings> settings =
					m_settingsSet.getSettingsProfile<DiagDataServiceSettings>(profile);

			TEST_PTR_LOG_RETURN_FALSE(settings, m_log);

			quint32 receivingNetmask = settings->diagDataReceivingNetmask.toIPv4Address();

			quint32 receivingSubnet = settings->diagDataReceivingIP.address32() & receivingNetmask;

			for(const Hardware::DeviceModule* lm : m_context->m_lmModules)
			{
				if (lm == nullptr)
				{
					LOG_INTERNAL_ERROR(m_log);
					result = false;
					continue;
				}

				DataSource ds;

				ds.setProfile(profile);

				result &= SoftwareSettingsGetter::getLmPropertiesFromDevice(lm, E::LanControllerType::DiagData,
																			m_context, &ds);

				ds.lanControllersInfo().filterLansByDiagDataServiceID(m_software->equipmentIdTemplate());

				int connectedAdaptersCount = 0;

				for(const LanControllerInfo& lan : ds.lanControllersInfo()())
				{
					if (lan.diagDataEnable == false || lan.diagDataServiceID != m_software->equipmentIdTemplate())
					{
						continue;
					}

					if (connectedAdaptersCount > 0)
					{
						// Several ethernet adapters of LM %1 are connected to DiagDataService %2.
						//
						m_log->errCFG3053(lm->equipmentIdTemplate(), m_software->equipmentIdTemplate());
						result = false;
						continue;
					}

					if ((QHostAddress(lan.diagDataIP).toIPv4Address() & receivingNetmask) != receivingSubnet)
					{
						// Different subnet address in data source IP %1 (%2) and data receiving IP %3 (%4).
						//
						m_log->errCFG3043(lan.diagDataIP,
										  lan.equipmentID,
										  settings->diagDataReceivingIP.addressStr(),
										  equipmentID());
						result = false;
						continue;
					}

					connectedAdaptersCount++;

					result &= findAcquiredDiagSignals(lm);

					appendAquiredDiagSignalsToDataSource(lm, &ds);

					dataSources.append(ds);
				}
			}
		}

		result &= findAcquiredParentObjects();

		RETURN_IF_FALSE(result)

		//

		QByteArray fileData;
		result &= DataSourcesXML<DataSource>::writeToXml(dataSources, &fileData);

		RETURN_IF_FALSE(result)

		//

		BuildFile* buildFile = m_buildResultWriter->addFile(softwareCfgSubdir(),
															File::DIAG_DATA_SOURCES_XML,
															CfgFileId::DIAG_DATA_SOURCES, "", fileData);
		if (buildFile == nullptr)
		{
			return false;
		}

		return m_cfgXml->addLinkToFile(buildFile);
	}

	bool DiagDataServiceCfgGenerator::writeAcquiredDiagSignalsFile()
	{
		QByteArray fileData;

		fileData.resize(m_protoAcquiredDiagSignals.ByteSizeLong());

		m_protoAcquiredDiagSignals.SerializeWithCachedSizesToArray(reinterpret_cast<::google::protobuf::uint8*>(fileData.data()));

		BuildFile* buildFile = m_buildResultWriter->addFile( softwareCfgSubdir(),
												File::ACQUIRED_DIAG_SIGNALS_ASGS,
												CfgFileId::ACQUIRED_DIAG_SIGNALS, "", fileData);
		if (buildFile == nullptr)
		{
			return false;
		}

		return m_cfgXml->addLinkToFile(buildFile);
	}

	bool DiagDataServiceCfgGenerator::appendAquiredDiagSignalsToDataSource(const Hardware::DeviceModule* lm, DataSource* ds)
	{
		TEST_PTR_RETURN_FALSE(lm);
		TEST_PTR_RETURN_FALSE(ds);

		auto it = m_lmAcquiredDiagSignals.find(calcHash(lm->equipmentIdTemplate()));

		if (it == m_lmAcquiredDiagSignals.end())
		{
			Q_ASSERT(false);
			return false;
		}

		const std::vector<DiagSignalConstShared>& acquiredDiagSignals = it->second;

		for(const DiagSignalConstShared& diagSignal : acquiredDiagSignals)
		{
			ds->appendAssociatedSignal(E::LanControllerType::DiagData, diagSignal->equipmentIdTemplate());
		}

		return true;
	}

	bool DiagDataServiceCfgGenerator::findAcquiredDiagSignals(const Hardware::DeviceModule* lm)
	{
		TEST_PTR_RETURN_FALSE(lm);

		Hash lmEquipHash = calcHash(lm->equipmentIdTemplate());

		if (m_lmAcquiredDiagSignals.contains(lmEquipHash) == true)
		{
			return true;			// diagSignals already found
		}

		bool result = true;

		auto [it, b] = m_lmAcquiredDiagSignals.emplace(lmEquipHash, std::vector<DiagSignalConstShared>{});

		std::vector<DiagSignalConstShared>& acquiredDiagSignals = it->second;

		std::shared_ptr<const Hardware::DeviceChassis> chassis = lm->getParentChassisShared();

		if (chassis == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		const std::vector<std::shared_ptr<Hardware::DeviceObject>>& chassisChildren = chassis->children();

		int moduleDiagPacketOffset = 0;		// from beginning of FODIP

		for(const std::shared_ptr<Hardware::DeviceObject>& devObject : chassisChildren)
		{
			std::shared_ptr<Hardware::DeviceModule> module = devObject->toModule();

			TEST_PTR_CONTINUE(module);

			Hash hash = calcHash(module->equipmentIdTemplate());

			Q_ASSERT(m_moduleDiagDataOffset.contains(hash) == false);

			m_moduleDiagDataOffset.emplace(hash, moduleDiagPacketOffset);

			int diagDataPacketSizeW = 0;

			if (module->isLogicModule() == true)
			{
				Q_ASSERT(module->place() == 0);

				std::shared_ptr<LmDescription> lmDescription = m_context->m_lmDescriptions->get(module.get());

				TEST_PTR_CONTINUE(lmDescription);

				diagDataPacketSizeW = static_cast<int>(lmDescription->memory().m_txDiagDataSize);
			}
			else
			{
				result &= DeviceHelper::getIntProperty(module.get(), EquipmentPropNames::TX_DIAG_DATA_SIZE, &diagDataPacketSizeW, m_log);
			}

			DeviceHelper::getChildDiagSignals(chassis, &acquiredDiagSignals);

			moduleDiagPacketOffset += diagDataPacketSizeW;
		}

		return result;
	}

	bool DiagDataServiceCfgGenerator::findAcquiredParentObjects()
	{
		bool result = true;

		std::set<Hash> acquiredSignalsHashes;				// set of calcHash(diagSignal->equipmentID)
		std::set<Hash> acquiredObjectHashes;				// set of calcHash(diagObject->equipmentID)

		for(const auto& [lmEquipmentID, acquiredDiagSignals] : m_lmAcquiredDiagSignals)
		{
			for(const auto& diagSignal : acquiredDiagSignals)
			{
				Hash signalHash = calcHash(diagSignal->equipmentIdTemplate());

				if (acquiredSignalsHashes.contains(signalHash))
				{
					Q_ASSERT(false);
					continue;
				}

				acquiredSignalsHashes.emplace(signalHash);

				AcquiredDiagSignal ads(diagSignal);

				std::shared_ptr<Hardware::DeviceObject> parent = diagSignal->parent();

				while(parent != nullptr)
				{
					if (parent->isRoot())
					{
						break;
					}

					if (parent->isController())
					{
						std::shared_ptr<const Hardware::DeviceController> controller = parent->toController();

						if (controller == nullptr)
						{
							LOG_INTERNAL_ERROR(m_log);
							result = false;
							continue;
						}

						ads.absAddr.addWord(controller->diagDataOffset());
					}

					if (parent->isModule())
					{
						auto it = m_moduleDiagDataOffset.find(calcHash(parent->equipmentIdTemplate()));

						if (it != m_moduleDiagDataOffset.end())
						{
							ads.absAddr.addWord(it->second);
						}
						else
						{
							Q_ASSERT(false);
						}
					}

					Hash parentHash = calcHash(parent->equipmentIdTemplate());

					if (acquiredObjectHashes.contains(parentHash) == false)
					{
						AcquiredDiagObject ado(parent);

						Network::AcquiredDiagObject* protoAdo = m_protoAcquiredDiagSignals.add_diagobjects();
						ado.saveToProto(protoAdo);

						acquiredObjectHashes.emplace(parentHash);

						parent = parent->parent();
					}
					else
					{
						if (parent->isChassis())
						{
							break;			// parent chassis and hence upper objects already added to map
						}
					}
				}

				Network::AcquiredDiagSignal* protoAds = m_protoAcquiredDiagSignals.add_diagsignals();
				ads.saveToProto(protoAds);
			}
		}

		return result;
	}
}
