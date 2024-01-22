#include "DiagDataServiceCfgGenerator.h"
#include "SoftwareSettingsGetter.h"
#include "../OnlineLib/SoftwareSettings.h"
#include "../lib/DataSource.h"
#include "../HardwareLib/DiagSignal.h"

namespace Builder
{
	AcquiredDiagObject::AcquiredDiagObject(std::shared_ptr<const Hardware::DeviceObject> dev,
										   Hardware::DeviceType devType,
											const QString& equipID) :
		device(dev),
		deviceType(devType),
		equipmentID(equipID)
	{
	}

	AcquiredDiagSignal::AcquiredDiagSignal(std::shared_ptr<const Hardware::DiagSignal> ds) :
		AcquiredDiagObject(ds, ds->deviceType(), ds->equipmentIdTemplate())
	{
		TEST_PTR_RETURN(ds);

		diagLevel = ds->level();
		diagSignalTypeID = ds->signalTypeId();
		isReflection = ds->isReflection();
		reflectedSignalID = ds->reflectedSignalId();
		validitySignalID = ds->validitySignalId();
		valueSizeBit = ds->valueBitSize();
		discreteContainerSize = ds->discreteContainerSize();
		logChanges = ds->logChanges();
		archive = ds->archive();
		reserved = ds->reserved();
		apertureType = ds->apertureType();
		coarseAperture = ds->coarseAperture();
		fineAperture = ds->fineAperture();

		// signal data address from beginning of module diag data offset in RUP diag packet
		// includes DiagDataOffset or parent controllers
		//
		absAddr = Address16(ds->valueOffset(), ds->valueBit());
	}

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
//			if (writeAppSignalsXml() == false) break;				// AppSignals.xml for AZPZ
//			if (writeAppSignalsExtXml() == false) break;			// AppSignalsExt.xml as extra debug info
//			if (writeAcquiredAppSignalsFile() == false) break;

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

		m_acquiredDiagSignals.clear();

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

					dataSources.append(ds);
				}

				result &= findAcquiredDiagSignals(lm);
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

		m_cfgXml->addLinkToFile(buildFile);

		return result;
	}

	bool DiagDataServiceCfgGenerator::findAcquiredDiagSignals(const Hardware::DeviceModule* lm)
	{
		bool result = true;

		std::shared_ptr<const Hardware::DeviceChassis> chassis = lm->getParentChassisShared();

		if (chassis == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		DeviceHelper::getChildDiagSignals(chassis, &m_acquiredDiagSignals);

		return result;
	}

	bool DiagDataServiceCfgGenerator::findAcquiredParentObjects()
	{
		bool result = true;

		for(const auto& diagSignal : m_acquiredDiagSignals)
		{
			std::shared_ptr<Hardware::DeviceObject> parent = diagSignal->parent();

			while(parent != nullptr)
			{
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

				Hash parentHash = calcHash(parent->equipmentIdTemplate());

				prevAquiredObject->parentHash = parentHash;

				const auto [it, b] = m_acquiredDiagObjects.emplace(parentHash,
																   AcquiredDiagObject(parent, parent->deviceType(),
																					  parent->equipmentIdTemplate()));

				if (b == false)
				{
					break;			// parent and uppper objects already in m_acquiredDiagObjects
				}

				prevAquiredObject = &it->second;

				parent = parent->parent();
			}
		}

		return result;
	}
}
