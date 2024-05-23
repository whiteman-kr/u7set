#include "AppDataServiceCfgGenerator.h"
#include "SoftwareSettingsGetter.h"

#include "../UtilsLib/XmlHelper.h"
#include "../UtilsLib/WUtils.h"
#include "../OnlineLib/DataSource.h"
#include "../OnlineLib/SoftwareSettings.h"

#include <HardwareLib/DeviceModule.h>


namespace Builder
{
	AppDataServiceCfgGenerator::AppDataServiceCfgGenerator(Context* context,
														   Hardware::Software* software) :
		SoftwareCfgGenerator(context, software)
	{
		Q_ASSERT(context != nullptr);
	}

	AppDataServiceCfgGenerator::~AppDataServiceCfgGenerator()
	{
	}

	bool AppDataServiceCfgGenerator::createSettingsProfile(const QString& profile)
	{
		AppDataServiceSettingsGetter settingsGetter;

		if (settingsGetter.readSoftwareSettings(m_context, m_software) == false)
		{
			return false;
		}

		bool result =  m_settingsSet.addProfile<AppDataServiceSettings>(profile, settingsGetter);

		result &= writeRunScriptFile(profile, settingsGetter, E::OS::Windows);
		result &= writeRunScriptFile(profile, settingsGetter, E::OS::Linux);

		return result;
	}

	bool AppDataServiceCfgGenerator::generateConfigurationStep1()
	{
		bool result = false;

		do
		{
			if (writeAppDataSourcesXml() == false) break;
			if (writeAppSignalsXml() == false) break;				// AppSignals.xml for AZPZ
			if (writeAppSignalsExtXml() == false) break;			// AppSignalsExt.xml as extra debug info
			if (writeAcquiredAppSignalsFile() == false) break;

			result = true;
		}
		while(false);

		return result;
	}

	bool AppDataServiceCfgGenerator::writeAppSignalsExtXml(	const Context* context,
															const AppSignalSet* signalSet,
															const std::set<Hash>* limitedSet,
															const QString& subDir)
	{
		TEST_PTR_RETURN_FALSE(context);
		TEST_PTR_RETURN_FALSE(context->m_log);

		IssueLogger* log = context->m_log;

		TEST_PTR_LOG_RETURN_FALSE(signalSet, log);

		// limitedSet may be nullptr

		BuildResultWriter* resultWriter = context->m_buildResultWriter.get();

		TEST_PTR_LOG_RETURN_FALSE(resultWriter, log);

		bool result = true;

		QByteArray extData;
		XmlWriteHelper extXml(&extData);

		extXml.setAutoFormatting(true);
		extXml.writeStartDocument();
		extXml.writeStartElement(XmlElement::APP_SIGNALS);
		extXml.writeIntAttribute(XmlAttribute::BUILD_ID, resultWriter->buildInfo().id);
		extXml.writeStartElement(XmlElement::SIGNALS);

		int signalCount = 0;

		if (limitedSet != nullptr)
		{
			signalCount = static_cast<int>(limitedSet->size());
		}
		else
		{
			signalCount = static_cast<int>(signalSet->size());
		}

		extXml.writeIntAttribute(XmlAttribute::COUNT, signalCount);

		int writtenSignalsCount = 0;

		for(const AppSignal* signal : *signalSet)
		{
			if (limitedSet != nullptr &&
				limitedSet->contains(calcHash(signal->appSignalID())) == false)
			{
				continue;
			}

			signal->writeToXml(extXml);

			writtenSignalsCount++;
		}

		if (writtenSignalsCount != signalCount)
		{
			Q_ASSERT(false);
			LOG_INTERNAL_ERROR(log);
			result = false;
		}

		extXml.writeEndElement();	// </Signals>

		extXml.writeStartElement(XmlElement::ACTUATORS);
		extXml.writeIntAttribute(XmlAttribute::COUNT, TO_INT(context->m_actuators.size()));

		for(const auto& [actuatorID, devModule] : context->m_actuators)
		{
			if (devModule == nullptr)
			{
				LOG_INTERNAL_ERROR(log);
				result = false;
				continue;
			}

			extXml.writeStartElement(XmlElement::ACTUATOR);
			extXml.writeStringAttribute(XmlAttribute::ID, actuatorID);

			QString actuatorDesc;

			if (DeviceHelper::isPropertyExists(devModule, EquipmentPropNames::ACTUATOR_DESCRIPTION) == true)
			{
				bool res = DeviceHelper::getStrProperty(devModule, EquipmentPropNames::ACTUATOR_DESCRIPTION, &actuatorDesc, log);

				//result &= res;
			}

			extXml.writeStringAttribute(XmlAttribute::DESCRIPTION, actuatorDesc);
			extXml.writeStringAttribute(XmlAttribute::EQUIPMENT_ID, devModule->equipmentIdTemplate());
			extXml.writeStringAttribute(XmlAttribute::MODULE_CAPTION, devModule->caption());
			extXml.writeEndElement();	// </Actuator>
		}

		extXml.writeEndElement();	// </Actuators>

		extXml.writeEndElement();	// </AppSignals>
		extXml.writeEndDocument();

		BuildFile* buildFile = resultWriter->addFile(subDir,
													File::APP_SIGNALS_EXT_XML,
													CfgFileId::APP_SIGNALS_EXT, "", extData);
		if (buildFile == nullptr)
		{
			return false;
		}

		return result;
	}

	bool AppDataServiceCfgGenerator::writeAppDataSourcesXml()
	{
		bool result = true;

		m_acquiredAppSignals.clear();

		QVector<OnlineLib::DataSource> dataSources;

		QStringList profiles = m_settingsSet.getSettingsProfiles();

		for(const QString& profile : profiles)
		{
			std::shared_ptr<const AppDataServiceSettings> settings =
					m_settingsSet.getSettingsProfile<AppDataServiceSettings>(profile);

			TEST_PTR_LOG_RETURN_FALSE(settings, m_log);

			quint32 receivingNetmask = settings->appDataReceivingNetmask.toIPv4Address();

			quint32 receivingSubnet = settings->appDataReceivingIP.address32() & receivingNetmask;

			for(Hardware::DeviceModule* lm : m_context->m_fscModules)
			{
				if (lm == nullptr)
				{
					LOG_INTERNAL_ERROR(m_log);
					result = false;
					continue;
				}

				OnlineLib::DataSource ds;

				ds.setProfile(profile);

				result &= SoftwareSettingsGetter::getLmPropertiesFromDevice(lm, E::LanControllerType::AppData,
																			m_context, &ds);

				ds.lanControllersInfo().filterLansByAppDataServiceID(m_software->equipmentIdTemplate());

				int connectedAdaptersCount = 0;

				for(const LanControllerInfo& lan : ds.lanControllersInfo()())
				{
					if (lan.appDataEnable == false || lan.appDataServiceID != m_software->equipmentIdTemplate())
					{
						continue;
					}

					if (connectedAdaptersCount > 0)
					{
						// Several ethernet adapters of LM %1 are connected to same AppDataService %2.
						//
						m_log->errCFG3030(lm->equipmentIdTemplate(), m_software->equipmentIdTemplate());
						result = false;
						continue;
					}

					if ((QHostAddress(lan.appDataIP).toIPv4Address() & receivingNetmask) != receivingSubnet)
					{
						// Different subnet address in data source IP %1 (%2) and data receiving IP %3 (%4).
						//
						m_log->errCFG3043(lan.appDataIP,
										  lan.equipmentID,
										  settings->appDataReceivingIP.addressStr(),
										  equipmentID());
						result = false;
						continue;
					}

					connectedAdaptersCount++;

					result &= findAppDataSourceAcquiredSignals(ds);	// inside fills m_acquiredAppSignals also

					dataSources.append(ds);
				}
			}
		}

		RETURN_IF_FALSE(result)

		//

		QByteArray fileData;
		result &= OnlineLib::DataSourcesXML<OnlineLib::DataSource>::writeToXml(dataSources, &fileData);

		RETURN_IF_FALSE(result)

		//

		BuildFile* buildFile = m_buildResultWriter->addFile(softwareCfgSubdir(),
															File::APP_DATA_SOURCES_XML,
															CfgFileId::APP_DATA_SOURCES, "", fileData);

		if (buildFile == nullptr)
		{
			return false;
		}

		m_cfgXml->addLinkToFile(buildFile);

		return result;
	}

	bool AppDataServiceCfgGenerator::writeAppSignalsXml()
	{
		if (m_context->generateAppSignalsXml() == false)
		{
			return true;
		}

		QByteArray azpzData;
		XmlWriteHelper azpzXml(&azpzData);

		azpzXml.setAutoFormatting(true);
		azpzXml.writeStartDocument();
		azpzXml.writeStartElement(XmlElement::APP_SIGNALS);
		azpzXml.writeIntAttribute(XmlAttribute::BUILD_ID, m_buildResultWriter->buildInfo().id);
		azpzXml.writeStartElement(XmlElement::SIGNALS);
		azpzXml.writeIntAttribute(XmlAttribute::COUNT, TO_INT(m_acquiredAppSignals.size()));

		int writtenSignalsCount = 0;

		for(const AppSignal* signal : *m_signalSet)
		{
			if (m_acquiredAppSignals.contains(calcHash(signal->appSignalID())) == false)
			{
				continue;
			}

			signal->writeToAzpzXml(azpzXml);

			writtenSignalsCount++;
		}

		if (writtenSignalsCount != m_acquiredAppSignals.size())
		{
			Q_ASSERT(false);
			LOG_INTERNAL_ERROR(m_log);
		}

		azpzXml.writeEndElement();	// </Signals>
		azpzXml.writeEndElement();	// </AppSignals>
		azpzXml.writeEndDocument();

		BuildFile* buildFile = m_buildResultWriter->addFile(softwareCfgSubdir(),
															File::APP_SIGNALS_XML,
															CfgFileId::APP_SIGNALS, "", azpzData);
		if (buildFile == nullptr)
		{
			return false;
		}

		return true;
	}

	bool AppDataServiceCfgGenerator::writeAppSignalsExtXml()
	{
		if (m_context->generateAppSignalsExtXml() == false)
		{
			return true;
		}

		return writeAppSignalsExtXml(m_context,
									dynamic_cast<AppSignalSet*>(m_signalSet),
									&m_acquiredAppSignals,
									softwareCfgSubdir());
	}

	bool AppDataServiceCfgGenerator::writeAcquiredAppSignalsFile()
	{
		TEST_PTR_RETURN_FALSE(m_signalSet);
		TEST_PTR_RETURN_FALSE(m_buildResultWriter);

		::Proto::AppSignalSet protoAppSignalSet;

		for(const Hash hash : m_acquiredAppSignals)
		{
			AppSignal* appSignal = m_signalSet->getSignalByHash(hash);

			TEST_PTR_CONTINUE(appSignal);

			::Proto::AppSignal* protoAppSignal = protoAppSignalSet.add_appsignal();

			appSignal->saveToProto(protoAppSignal);
		}

		int dataSize = static_cast<int>(protoAppSignalSet.ByteSizeLong());

		QByteArray data;

		data.resize(dataSize);

		protoAppSignalSet.SerializeWithCachedSizesToArray(reinterpret_cast<::google::protobuf::uint8*>(data.data()));

		BuildFile* acquiredAppSignalsFile = m_buildResultWriter->addFile(softwareCfgSubdir(),
																		 File::ACQUIRED_APP_SIGNALS_ASGS,
																		 CfgFileId::ACQUIRED_APP_SIGNALS, "",
																		 data, true);
		TEST_PTR_RETURN_FALSE(acquiredAppSignalsFile);

		m_cfgXml->addLinkToFile(acquiredAppSignalsFile);

		return true;
	}

	bool AppDataServiceCfgGenerator::writeRunScriptFile(const QString& profile,
														const AppDataServiceSettings& settings,
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

	bool AppDataServiceCfgGenerator::findAppDataSourceAcquiredSignals(OnlineLib::DataSource& appDataSource)
	{
		Hardware::DeviceObject* lm = m_equipment->deviceObject(appDataSource.moduleEquipmentID()).get();

		if (lm == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		const Hardware::DeviceChassis* dataSourceChassis = lm->getParentChassis();

		for(const AppSignal* appSignal : *m_signalSet)
		{
			CONTINUE_IF_FALSE(appSignal->isAcquired());

			QString appSignalEquipmentID = appSignal->equipmentID();

			if (appSignalEquipmentID.isEmpty())
			{
				continue;
			}

			Hardware::DeviceObject* device = m_equipment->deviceObject(appSignalEquipmentID).get();

			if (device == nullptr)
			{
				LOG_WARNING_OBSOLETE(m_log, IssuePrefix::NotDefined, QString("Signal '%1' bound with an unknown device '%2'").
					arg(appSignal->appSignalID()).arg(appSignalEquipmentID));
				continue;
			}

			const Hardware::DeviceChassis* chassis = device->getParentChassis();

			if (chassis == dataSourceChassis)
			{
				appDataSource.appendAssociatedSignal(E::LanControllerType::AppData, appSignal->appSignalID());

				m_acquiredAppSignals.insert(calcHash(appSignal->appSignalID()));
			}
		}

		return true;
	}
}
