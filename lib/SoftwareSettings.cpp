#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QObject>

#include "LanControllerInfo.h"
#include "LanControllerInfoHelper.h"
#include "SoftwareSettings.h"
#include "WUtils.h"

// -------------------------------------------------------------------------------------
//
// SoftwareSettings class implementation
//
// -------------------------------------------------------------------------------------

SoftwareSettings::SoftwareSettings(const SoftwareSettings& copy) :
	profile(copy.profile)
{
}

SoftwareSettings::SoftwareSettings(const QString& profile) :
	profile(profile)
{
}

SoftwareSettings::~SoftwareSettings()
{
}

const SoftwareSettings& SoftwareSettings::operator = (const SoftwareSettings& copy)
{
	profile = copy.profile;

	return *this;
}

void SoftwareSettings::writeStartSettings(XmlWriteHelper& xml) const
{
	xml.writeStartElement(XmlElement::SETTINGS);	//	<Settings>

	xml.writeStringAttribute(XmlAttribute::PROFILE, profile);
}

void SoftwareSettings::writeEndSettings(XmlWriteHelper &xml) const
{
	xml.writeEndElement();							//	</Settings>
}

bool SoftwareSettings::readStartSettings(XmlReadHelper& xml)
{
	bool result = xml.findElement(XmlElement::SETTINGS);

	RETURN_IF_FALSE(result);

	result = xml.readStringAttribute(XmlAttribute::PROFILE, &profile);

	return result;
}

// -------------------------------------------------------------------------------------
//
// SoftwareSettingsSet class implementation
//
// -------------------------------------------------------------------------------------

SoftwareSettingsSet::SoftwareSettingsSet(E::SoftwareType softwareType) :
	m_softwareType(softwareType)
{
}

bool SoftwareSettingsSet::addSettingsProfile(const QString& profile, std::shared_ptr<SoftwareSettings> settings)
{
	bool result = false;

	switch(m_softwareType)
	{
	case E::SoftwareType::Monitor:
		result = dynamic_cast<MonitorSettings*>(settings.get()) != nullptr;
		break;

	case E::SoftwareType::ConfigurationService:
		result = dynamic_cast<CfgServiceSettings*>(settings.get()) != nullptr;
		break;

	case E::SoftwareType::AppDataService:
		result = dynamic_cast<AppDataServiceSettings*>(settings.get()) != nullptr;
		break;

	case E::SoftwareType::ArchiveService:
		result = dynamic_cast<ArchivingServiceSettings*>(settings.get()) != nullptr;
		break;

	case E::SoftwareType::TuningService:
		result = dynamic_cast<TuningServiceSettings*>(settings.get()) != nullptr;
		break;

	case E::SoftwareType::DiagDataService:
		result = dynamic_cast<DiagDataServiceSettings*>(settings.get()) != nullptr;
		break;

	case E::SoftwareType::TuningClient:
		result = dynamic_cast<TuningClientSettings*>(settings.get()) != nullptr;
		break;

	case E::SoftwareType::Metrology:
		result = dynamic_cast<MetrologySettings*>(settings.get()) != nullptr;
		break;

	case E::SoftwareType::TestClient:
		result = dynamic_cast<TestClientSettings*>(settings.get()) != nullptr;
		break;

	case E::SoftwareType::ServiceControlManager:
	case E::SoftwareType::Unknown:
	case E::SoftwareType::BaseService:
	default:
		Q_ASSERT(false);
		break;
	}

	if (result == false)
	{
		Q_ASSERT(false);
		return false;
	}

	if (m_settings.find(profile) == m_settings.end())
	{
		m_settings.insert({profile, settings});
		return true;
	}

	Q_ASSERT(false);
	return false;
}

std::shared_ptr<const SoftwareSettings> SoftwareSettingsSet::getSettingsProfile(const QString& profile) const
{
	auto it = m_settings.find(profile);

	if (it != m_settings.end())
	{
		return it->second;
	}

	return nullptr;
}

std::shared_ptr<const SoftwareSettings> SoftwareSettingsSet::getSettingsDefaultProfile() const
{
	return getSettingsProfile(SettingsProfile::DEFAULT);
}

bool SoftwareSettingsSet::writeToXml(XmlWriteHelper& xml)
{
	bool result = true;

	xml.writeStartElement(XmlElement::SETTINGS_SET);
	xml.writeIntAttribute(XmlAttribute::COUNT, static_cast<int>(m_settings.size()));

	for(auto p : m_settings)
	{
		std::shared_ptr<SoftwareSettings> swSettings = p.second;

		if (swSettings == nullptr)
		{
			Q_ASSERT(false);
			continue;
		}

		result &= swSettings->writeToXml(xml);
	}

	xml.writeEndElement();		// </SettingsSet>

	return result;
}

bool SoftwareSettingsSet::readFromXml(XmlReadHelper& xml)
{
	m_settings.clear();

	bool result = true;
	int profilesCount = 0;

	result = xml.findElement(XmlElement::SETTINGS_SET);
	result &= xml.readIntAttribute(XmlAttribute::COUNT, &profilesCount);

	RETURN_IF_FALSE(result);

	for(int i = 0; i < profilesCount; i++)
	{
		std::shared_ptr<SoftwareSettings> settings = createAppropriateSettings();

		if (settings == nullptr)
		{
			return false;
		}

		if (settings->readFromXml(xml) == true)
		{
			result &= addSettingsProfile(settings->profile, settings);
		}
		else
		{
			result = false;
		}
	}

	return result;
}

std::shared_ptr<SoftwareSettings> SoftwareSettingsSet::createAppropriateSettings()
{
	switch(m_softwareType)
	{
	case E::SoftwareType::Monitor:
		return std::make_shared<MonitorSettings>();

	case E::SoftwareType::ConfigurationService:
		return std::make_shared<CfgServiceSettings>();

	case E::SoftwareType::AppDataService:
		return std::make_shared<AppDataServiceSettings>();

	case E::SoftwareType::ArchiveService:
		return std::make_shared<ArchivingServiceSettings>();

	case E::SoftwareType::TuningService:
		return std::make_shared<TuningServiceSettings>();

	case E::SoftwareType::DiagDataService:
		return std::make_shared<DiagDataServiceSettings>();

	case E::SoftwareType::TuningClient:
		return std::make_shared<TuningClientSettings>();

	case E::SoftwareType::Metrology:
		return std::make_shared<MetrologySettings>();

	case E::SoftwareType::TestClient:
		return std::make_shared<TestClientSettings>();

	case E::SoftwareType::ServiceControlManager:
	case E::SoftwareType::Unknown:
	case E::SoftwareType::BaseService:
	default:
		Q_ASSERT(false);
		break;
	}

	return nullptr;
}



#ifdef IS_BUILDER

	// -------------------------------------------------------------------------------------
	//
	// ServiceSettingsGetter class implementation
	//
	// -------------------------------------------------------------------------------------

	SoftwareSettingsGetter::~SoftwareSettingsGetter()
	{
	}

	bool SoftwareSettingsGetter::getSoftwareConnection(const Hardware::EquipmentSet* equipment,
												const Hardware::Software* thisSoftware,
												const QString& propConnectedSoftwareID,
												const QString& propConnectedSoftwareIP,
												const QString& propConnectedSoftwarePort,
												QString* connectedSoftwareID,
												HostAddressPort* connectedSoftwareIP,
												bool emptyAllowed,
												const QString& defaultIP,
												int defaultPort,
												E::SoftwareType requiredSoftwareType,
												Builder::IssueLogger* log)
	{
		TEST_PTR_RETURN_FALSE(log);

		TEST_PTR_LOG_RETURN_FALSE(equipment, log);
		TEST_PTR_LOG_RETURN_FALSE(thisSoftware, log);
		TEST_PTR_LOG_RETURN_FALSE(connectedSoftwareID, log);
		TEST_PTR_LOG_RETURN_FALSE(connectedSoftwareIP, log);

		if (emptyAllowed == true)
		{
			QHostAddress addr;

			if (addr.setAddress(defaultIP) == false)
			{
				LOG_INTERNAL_ERROR(log);
				return false;
			}

			if (defaultPort < Socket::PORT_LOWEST || defaultPort > Socket::PORT_HIGHEST)
			{
				LOG_INTERNAL_ERROR(log);
				return false;
			}
		}

		bool result = true;

		result = DeviceHelper::getStrProperty(thisSoftware, propConnectedSoftwareID, connectedSoftwareID, log);

		if (result == false)
		{
			return false;
		}

		*connectedSoftwareID = connectedSoftwareID->trimmed();

		if (connectedSoftwareID->isEmpty() == true)
		{
			if (emptyAllowed == true)
			{
				//  Property '%1.%2' is empty.
				//
				log->wrnCFG3016(thisSoftware->equipmentIdTemplate(), propConnectedSoftwareID);

				connectedSoftwareIP->setAddressPort(defaultIP, defaultPort);

				return true;
			}

			//  Property '%1.%2' is empty.
			//
			log->errCFG3022(thisSoftware->equipmentIdTemplate(), propConnectedSoftwareID);

			return false;
		}

		const Hardware::Software* connectedSoftware = DeviceHelper::getSoftware(equipment, *connectedSoftwareID);

		if (connectedSoftware == nullptr)
		{
			// Property '%1.%2' is linked to undefined software ID '%3'.
			//
			log->errCFG3021(thisSoftware->equipmentIdTemplate(), propConnectedSoftwareID, *connectedSoftwareID);
			return false;
		}

		if (requiredSoftwareType != E::SoftwareType::Unknown)
		{
			if (connectedSoftware->type() != requiredSoftwareType)
			{
				// Property %1.%2 is linked to not compatible software ID %3.
				//
				log->errCFG3017(thisSoftware->equipmentIdTemplate(), propConnectedSoftwareID, connectedSoftware->equipmentIdTemplate());
				return false;
			}
		}

		result = DeviceHelper::getIpPortProperty(	connectedSoftware,
													propConnectedSoftwareIP,
													propConnectedSoftwarePort,
													connectedSoftwareIP,
													emptyAllowed, defaultIP, defaultPort, log);
		return result;
	}

	bool SoftwareSettingsGetter::getCfgServiceConnection(	const Hardware::EquipmentSet *equipment,
													const Hardware::Software* software,
													QString* cfgServiceID1, HostAddressPort* cfgServiceAddrPort1,
													QString* cfgServiceID2, HostAddressPort* cfgServiceAddrPort2,
													Builder::IssueLogger* log)
	{
		TEST_PTR_RETURN_FALSE(log);

		TEST_PTR_LOG_RETURN_FALSE(equipment, log);
		TEST_PTR_LOG_RETURN_FALSE(software, log);
		TEST_PTR_LOG_RETURN_FALSE(cfgServiceID1, log);
		TEST_PTR_LOG_RETURN_FALSE(cfgServiceAddrPort1, log);
		TEST_PTR_LOG_RETURN_FALSE(cfgServiceID2, log);
		TEST_PTR_LOG_RETURN_FALSE(cfgServiceAddrPort2, log);

		bool result = true;

		result &= getSoftwareConnection(equipment,
										software,
										EquipmentPropNames::CFG_SERVICE_ID1,
										EquipmentPropNames::CLIENT_REQUEST_IP,
										EquipmentPropNames::CLIENT_REQUEST_PORT,
										cfgServiceID1,
										cfgServiceAddrPort1,
										true, Socket::IP_NULL,
										PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST,
										E::SoftwareType::ConfigurationService, log);

		result &= getSoftwareConnection(equipment,
										software,
										EquipmentPropNames::CFG_SERVICE_ID2,
										EquipmentPropNames::CLIENT_REQUEST_IP,
										EquipmentPropNames::CLIENT_REQUEST_PORT,
										cfgServiceID2,
										cfgServiceAddrPort2,
										true, Socket::IP_NULL,
										PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST,
										E::SoftwareType::ConfigurationService, log);
		if (result == false)
		{
			return false;
		}

		if (cfgServiceID1->isEmpty() == true && cfgServiceID2->isEmpty() == true)
		{
			// Software %1 is not linked to ConfigurationService.
			//
			log->errCFG3029(software->equipmentIdTemplate());
			return false;
		}

		return result;
	}

	bool SoftwareSettingsGetter::getLmPropertiesFromDevice(	const Hardware::DeviceModule* lm,
															DataSource::DataType dataType,
															int adapterNo,
															E::LanControllerType lanControllerType,
															const Builder::Context* context,
															DataSource* ds)
	{
		TEST_PTR_RETURN_FALSE(context);

		Builder::IssueLogger* log = context->m_log;

		TEST_PTR_RETURN_FALSE(log);
		TEST_PTR_LOG_RETURN_FALSE(lm, log);
		TEST_PTR_LOG_RETURN_FALSE(ds, log);

		ds->setLmDataType(dataType);
		ds->setLmEquipmentID(lm->equipmentIdTemplate());
		ds->setLmPresetName(lm->presetName());
		ds->setLmModuleType(lm->moduleType());
		ds->setLmCaption(lm->caption());

		bool result = true;

		int lmNumber = 0;
		QString subsystemChannel;
		QString subsystemID;

		result &= DeviceHelper::getIntProperty(lm, EquipmentPropNames::LM_NUMBER, &lmNumber, log);
		result &= DeviceHelper::getStrProperty(lm, EquipmentPropNames::SUBSYSTEM_CHANNEL, &subsystemChannel, log);
		result &= DeviceHelper::getStrProperty(lm, EquipmentPropNames::SUBSYSTEM_ID, &subsystemID, log);

		ds->setLmNumber(lmNumber);
		ds->setLmSubsystemChannel(subsystemChannel);
		ds->setLmSubsystemID(subsystemID);

		int subsystemKey = context->m_subsystems->subsystemKey(subsystemID);

		if (subsystemKey == -1)
		{
			// Subsystem '%1' is not found in subsystem set (Logic Module '%2')
			//
			log->errCFG3001(subsystemID, lm->equipmentIdTemplate());
			return false;
		}

		ds->setLmSubsystemKey(subsystemKey);

		auto pos = context->m_lmsUniqueIDs.find(lm->equipmentIdTemplate());

		if (pos != context->m_lmsUniqueIDs.end())
		{
			ds->setLmUniqueID(pos->second);
		}
		else
		{
			Q_ASSERT(false);		// LM uniqueID isn't found
			ds->setLmUniqueID(0);
		}

		LanControllerInfo lanControllerInfo;

		result &= LanControllerInfoHelper::getInfo(*lm, adapterNo, lanControllerType,
												   &lanControllerInfo, *context->m_equipmentSet.get(), log);

		ds->setLmAdapterID(lanControllerInfo.equipmentID);

		switch(dataType)
		{
		case DataSource::DataType::App:

			assert(lanControllerType == E::LanControllerType::AppData || lanControllerType == E::LanControllerType::AppAndDiagData);

			ds->setLmDataEnable(lanControllerInfo.appDataEnable);
			ds->setLmAddressPort(HostAddressPort(lanControllerInfo.appDataIP, lanControllerInfo.appDataPort));
			ds->setLmDataID(lanControllerInfo.appDataUID);
			ds->setLmDataSize(lanControllerInfo.appDataSizeBytes);
			ds->setLmRupFramesQuantity(lanControllerInfo.appDataFramesQuantity);
			ds->setServiceID(lanControllerInfo.appDataServiceID);
			break;

		case DataSource::DataType::Diag:

			assert(lanControllerType == E::LanControllerType::DiagData || lanControllerType == E::LanControllerType::AppAndDiagData);

			ds->setLmDataEnable(lanControllerInfo.diagDataEnable);
			ds->setLmAddressPort(HostAddressPort(lanControllerInfo.diagDataIP, lanControllerInfo.diagDataPort));
			ds->setLmDataID(lanControllerInfo.diagDataUID);
			ds->setLmDataSize(lanControllerInfo.diagDataSizeBytes);
			ds->setLmRupFramesQuantity(lanControllerInfo.diagDataFramesQuantity);
			ds->setServiceID(lanControllerInfo.diagDataServiceID);
			break;

		case DataSource::DataType::Tuning:

			assert(lanControllerType == E::LanControllerType::Tuning);

			ds->setLmDataEnable(lanControllerInfo.tuningEnable);
			ds->setLmAddressPort(HostAddressPort(lanControllerInfo.tuningIP, lanControllerInfo.tuningPort));
			ds->setLmDataID(0);
			ds->setLmDataSize(0);
			ds->setLmRupFramesQuantity(0);
			ds->setServiceID(lanControllerInfo.tuningServiceID);
			break;

		default:
			assert(false);
		}

		return result;
	}


#endif


// -------------------------------------------------------------------------------------
//
// CfgServiceSettings class implementation
//
// -------------------------------------------------------------------------------------

bool CfgServiceSettings::writeToXml(XmlWriteHelper& xml)
{
	writeStartSettings(xml);

	xml.writeHostAddressPort(EquipmentPropNames::CLIENT_REQUEST_IP,
							 EquipmentPropNames::CLIENT_REQUEST_PORT, clientRequestIP);

	xml.writeHostAddress(EquipmentPropNames::CLIENT_REQUEST_NETMASK, clientRequestNetmask);

	xml.writeStartElement(XmlElement::CLIENTS);
	xml.writeIntAttribute(XmlAttribute::COUNT, clients.count());

	for(const QPair<QString, E::SoftwareType>& pair : clients)
	{
		xml.writeStartElement(XmlElement::CLIENT);

		xml.writeStringAttribute(EquipmentPropNames::EQUIPMENT_ID, pair.first);
		xml.writeStringAttribute(EquipmentPropNames::SOFTWARE_TYPE, E::valueToString(pair.second));

		xml.writeEndElement();	// </Client>
	}

	xml.writeEndElement();	// </Clients>

	writeEndSettings(xml);	// </Settings>

	return true;
}

bool CfgServiceSettings::readFromXml(XmlReadHelper& xml)
{
	clients.clear();

	bool result = false;

	result = readStartSettings(xml);

	RETURN_IF_FALSE(result);

	result &= xml.readHostAddressPort(EquipmentPropNames::CLIENT_REQUEST_IP,
									  EquipmentPropNames::CLIENT_REQUEST_PORT, &clientRequestIP);

	result &= xml.readHostAddress(EquipmentPropNames::CLIENT_REQUEST_NETMASK, &clientRequestNetmask);

	result = xml.findElement(XmlElement::CLIENTS);

	if (result == false)
	{
		return false;
	}

	int clientsCount = 0;

	result &= xml.readIntAttribute(XmlAttribute::COUNT, &clientsCount);

	for(int i = 0; i < clientsCount; i++)
	{
		result &= xml.findElement(XmlElement::CLIENT);

		QString equipmentID;
		QString softwareTypeStr;

		result &= xml.readStringAttribute(EquipmentPropNames::EQUIPMENT_ID, &equipmentID);
		result &= xml.readStringAttribute(EquipmentPropNames::SOFTWARE_TYPE, &softwareTypeStr);

		QPair<QString, E::SoftwareType> pair;

		pair.first = equipmentID;

		bool ok = false;

		pair.second = E::stringToValue<E::SoftwareType>(softwareTypeStr, &ok);

		result &= ok;

		if (result == true)
		{
			clients.append(pair);
		}
	}

	return result;
}

QStringList CfgServiceSettings::knownClients()
{
	QStringList knownClients;

	for(const QPair<QString, E::SoftwareType>& client : clients)
	{
		knownClients.append(client.first.trimmed());
	}

	return knownClients;
}

#ifdef IS_BUILDER

	// -------------------------------------------------------------------------------------
	//
	// CfgServiceSettingsGetter class implementation
	//
	// -------------------------------------------------------------------------------------

	bool CfgServiceSettingsGetter::readFromDevice(	const Builder::Context* context,
													const Hardware::Software* software)
	{
		TEST_PTR_RETURN_FALSE(context);

		Builder::IssueLogger* log = context->m_log;

		TEST_PTR_RETURN_FALSE(log);
		TEST_PTR_LOG_RETURN_FALSE(software, log);

		bool result = true;

		result &= DeviceHelper::getIpPortProperty(software, EquipmentPropNames::CLIENT_REQUEST_IP,
												  EquipmentPropNames::CLIENT_REQUEST_PORT, &clientRequestIP, false, "", 0, log);
		result &= DeviceHelper::getIPv4Property(software, EquipmentPropNames::CLIENT_REQUEST_NETMASK, &clientRequestNetmask, false, "", log);

		return result;
	}

#endif

// -------------------------------------------------------------------------------------
//
// AppDataServiceSettings class implementation
//
// -------------------------------------------------------------------------------------

bool AppDataServiceSettings::writeToXml(XmlWriteHelper& xml)
{
	writeStartSettings(xml);

	xml.writeStringElement(EquipmentPropNames::CFG_SERVICE_ID1, cfgServiceID1);
	xml.writeHostAddressPort(EquipmentPropNames::CFG_SERVICE_IP1,
							 EquipmentPropNames::CFG_SERVICE_PORT1, cfgServiceIP1);

	xml.writeStringElement(EquipmentPropNames::CFG_SERVICE_ID2, cfgServiceID2);
	xml.writeHostAddressPort(EquipmentPropNames::CFG_SERVICE_IP2,
							 EquipmentPropNames::CFG_SERVICE_PORT2, cfgServiceIP2);

	xml.writeHostAddressPort(EquipmentPropNames::APP_DATA_RECEIVING_IP,
							 EquipmentPropNames::APP_DATA_RECEIVING_PORT, appDataReceivingIP);
	xml.writeHostAddress(EquipmentPropNames::APP_DATA_RECEIVING_NETMASK, appDataReceivingNetmask);

	xml.writeIntElement(EquipmentPropNames::AUTO_ARCHIVE_INTERVAL, autoArchiveInterval);

	xml.writeStringElement(EquipmentPropNames::ARCH_SERVICE_ID, archServiceID);
	xml.writeHostAddressPort(EquipmentPropNames::ARCH_SERVICE_IP,
							 EquipmentPropNames::ARCH_SERVICE_PORT, archServiceIP);

	xml.writeHostAddressPort(EquipmentPropNames::CLIENT_REQUEST_IP,
							 EquipmentPropNames::CLIENT_REQUEST_PORT, clientRequestIP);
	xml.writeHostAddress(EquipmentPropNames::CLIENT_REQUEST_NETMASK, clientRequestNetmask);

	xml.writeHostAddressPort(EquipmentPropNames::RT_TRENDS_REQUEST_IP,
							 EquipmentPropNames::RT_TRENDS_REQUEST_PORT, rtTrendsRequestIP);

	writeEndSettings(xml);	// </Settings>

	return true;
}

bool AppDataServiceSettings::readFromXml(XmlReadHelper& xml)
{
	bool result = false;

	result = readStartSettings(xml);

	RETURN_IF_FALSE(result);

	result &= xml.readStringElement(EquipmentPropNames::CFG_SERVICE_ID1, &cfgServiceID1, true);
	result &= xml.readHostAddressPort(EquipmentPropNames::CFG_SERVICE_IP1,
									  EquipmentPropNames::CFG_SERVICE_PORT1, &cfgServiceIP1);

	result &= xml.readStringElement(EquipmentPropNames::CFG_SERVICE_ID2, &cfgServiceID2, true);
	result &= xml.readHostAddressPort(EquipmentPropNames::CFG_SERVICE_IP2,
									  EquipmentPropNames::CFG_SERVICE_PORT2, &cfgServiceIP2);

	result &= xml.readHostAddressPort(EquipmentPropNames::APP_DATA_RECEIVING_IP,
									  EquipmentPropNames::APP_DATA_RECEIVING_PORT, &appDataReceivingIP);
	result &= xml.readHostAddress(EquipmentPropNames::APP_DATA_RECEIVING_NETMASK, &appDataReceivingNetmask);

	result &= xml.readIntElement(EquipmentPropNames::AUTO_ARCHIVE_INTERVAL, &autoArchiveInterval, true);

	result &= xml.readStringElement(EquipmentPropNames::ARCH_SERVICE_ID, &archServiceID, true);
	result &= xml.readHostAddressPort(EquipmentPropNames::ARCH_SERVICE_IP,
									  EquipmentPropNames::ARCH_SERVICE_PORT, &archServiceIP);

	result &= xml.readHostAddressPort(EquipmentPropNames::CLIENT_REQUEST_IP,
									  EquipmentPropNames::CLIENT_REQUEST_PORT, &clientRequestIP);
	result &= xml.readHostAddress(EquipmentPropNames::CLIENT_REQUEST_NETMASK, &clientRequestNetmask);

	result &= xml.readHostAddressPort(EquipmentPropNames::RT_TRENDS_REQUEST_IP,
									  EquipmentPropNames::RT_TRENDS_REQUEST_PORT, &rtTrendsRequestIP);
	return result;
}

#ifdef IS_BUILDER

	// -------------------------------------------------------------------------------------
	//
	// AppDataServiceSettingsGetter class implementation
	//
	// -------------------------------------------------------------------------------------

	bool AppDataServiceSettingsGetter::readFromDevice(const Builder::Context* context,
													  const Hardware::Software* software)
	{
		TEST_PTR_RETURN_FALSE(context);

		Builder::IssueLogger* log = context->m_log;

		TEST_PTR_RETURN_FALSE(log);
		TEST_PTR_LOG_RETURN_FALSE(software, log);

		const Hardware::EquipmentSet* equipment = context->m_equipmentSet.get();

		TEST_PTR_LOG_RETURN_FALSE(equipment, log);

		bool result = true;

		result &= DeviceHelper::getIpPortProperty(software,
												  EquipmentPropNames::APP_DATA_RECEIVING_IP,
												  EquipmentPropNames::APP_DATA_RECEIVING_PORT,
												  &appDataReceivingIP,
												  false, "", 0, log);

		result &= DeviceHelper::getIPv4Property(software, EquipmentPropNames::APP_DATA_RECEIVING_NETMASK,
												&appDataReceivingNetmask, false, "", log);

		result &= DeviceHelper::getIpPortProperty(software,
												  EquipmentPropNames::CLIENT_REQUEST_IP,
												  EquipmentPropNames::CLIENT_REQUEST_PORT,
												  &clientRequestIP,
												  false, "", 0, log);

		int rtTrendsRequestPort = 0;

		result &= DeviceHelper::getPortProperty(software, EquipmentPropNames::RT_TRENDS_REQUEST_PORT,
												&rtTrendsRequestPort, true, PORT_APP_DATA_SERVICE_RT_TRENDS_REQUEST, log);

		rtTrendsRequestIP.setAddressPort(clientRequestIP.addressStr(), rtTrendsRequestPort);

		result &= DeviceHelper::getIPv4Property(software, EquipmentPropNames::CLIENT_REQUEST_NETMASK,
												&clientRequestNetmask, false, "", log);

		result &= getSoftwareConnection(equipment, software,
										EquipmentPropNames::ARCH_SERVICE_ID,
										EquipmentPropNames::APP_DATA_RECEIVING_IP,
										EquipmentPropNames::APP_DATA_RECEIVING_PORT,
										&archServiceID,	&archServiceIP,
										true, Socket::IP_NULL,
										PORT_ARCHIVING_SERVICE_APP_DATA,
										E::SoftwareType::ArchiveService, log);

		result &= getCfgServiceConnection(equipment, software, &cfgServiceID1, &cfgServiceIP1,
										  &cfgServiceID2, &cfgServiceIP2, log);

		result &= DeviceHelper::getIntProperty(software, EquipmentPropNames::AUTO_ARCHIVE_INTERVAL,
											   &autoArchiveInterval, log);
		return result;
	}

#endif

// -------------------------------------------------------------------------------------
//
// DiagDataServiceSettings class implementation
//
// -------------------------------------------------------------------------------------

bool DiagDataServiceSettings::writeToXml(XmlWriteHelper& xml)
{
	writeStartSettings(xml);

	xml.writeStringElement(EquipmentPropNames::CFG_SERVICE_ID1, cfgServiceID1);
	xml.writeHostAddressPort(EquipmentPropNames::CFG_SERVICE_IP1,
							 EquipmentPropNames::CFG_SERVICE_PORT1, cfgServiceIP1);

	xml.writeStringElement(EquipmentPropNames::CFG_SERVICE_ID2, cfgServiceID2);
	xml.writeHostAddressPort(EquipmentPropNames::CFG_SERVICE_IP2,
							 EquipmentPropNames::CFG_SERVICE_PORT2, cfgServiceIP2);

	xml.writeHostAddressPort(EquipmentPropNames::DIAG_DATA_RECEIVING_IP,
							 EquipmentPropNames::DIAG_DATA_RECEIVING_PORT, diagDataReceivingIP);
	xml.writeHostAddress(EquipmentPropNames::DIAG_DATA_RECEIVING_NETMASK, diagDataReceivingNetmask);

	xml.writeStringElement(EquipmentPropNames::ARCH_SERVICE_ID, archServiceID);
	xml.writeHostAddressPort(EquipmentPropNames::ARCH_SERVICE_IP,
							 EquipmentPropNames::ARCH_SERVICE_PORT, archServiceIP);

	xml.writeHostAddressPort(EquipmentPropNames::CLIENT_REQUEST_IP,
							 EquipmentPropNames::CLIENT_REQUEST_PORT, clientRequestIP);
	xml.writeHostAddress(EquipmentPropNames::CLIENT_REQUEST_NETMASK, clientRequestNetmask);

	writeEndSettings(xml);	// </Settings>

	return true;
}

bool DiagDataServiceSettings::readFromXml(XmlReadHelper& xml)
{
	bool result = false;

	result = readStartSettings(xml);

	RETURN_IF_FALSE(result);

	result &= xml.readStringElement(EquipmentPropNames::CFG_SERVICE_ID1, &cfgServiceID1, true);
	result &= xml.readHostAddressPort(EquipmentPropNames::CFG_SERVICE_IP1,
									  EquipmentPropNames::CFG_SERVICE_PORT1, &cfgServiceIP1);

	result &= xml.readStringElement(EquipmentPropNames::CFG_SERVICE_ID2, &cfgServiceID2, true);
	result &= xml.readHostAddressPort(EquipmentPropNames::CFG_SERVICE_IP2,
									  EquipmentPropNames::CFG_SERVICE_PORT2, &cfgServiceIP2);

	result &= xml.readHostAddressPort(EquipmentPropNames::DIAG_DATA_RECEIVING_IP,
									  EquipmentPropNames::DIAG_DATA_RECEIVING_PORT, &diagDataReceivingIP);
	result &= xml.readHostAddress(EquipmentPropNames::DIAG_DATA_RECEIVING_NETMASK, &diagDataReceivingNetmask);

	result &= xml.readStringElement(EquipmentPropNames::ARCH_SERVICE_ID, &archServiceID, true);
	result &= xml.readHostAddressPort(EquipmentPropNames::ARCH_SERVICE_IP,
									  EquipmentPropNames::ARCH_SERVICE_PORT, &archServiceIP);

	result &= xml.readHostAddressPort(EquipmentPropNames::CLIENT_REQUEST_IP,
									  EquipmentPropNames::CLIENT_REQUEST_PORT, &clientRequestIP);
	result &= xml.readHostAddress(EquipmentPropNames::CLIENT_REQUEST_NETMASK, &clientRequestNetmask);

	return result;
}


#ifdef IS_BUILDER

	// -------------------------------------------------------------------------------------
	//
	// DiagDataServiceSettingsGettter class implementation
	//
	// -------------------------------------------------------------------------------------

	bool DiagDataServiceSettingsGetter::readFromDevice(const Builder::Context* context,
													   const Hardware::Software* software)
	{
		TEST_PTR_RETURN_FALSE(context);

		Builder::IssueLogger* log = context->m_log;

		TEST_PTR_RETURN_FALSE(log);
		TEST_PTR_LOG_RETURN_FALSE(software, log);

		const Hardware::EquipmentSet* equipment = context->m_equipmentSet.get();

		TEST_PTR_LOG_RETURN_FALSE(equipment, log);

		bool result = true;

		result &= DeviceHelper::getIpPortProperty(software,
												  EquipmentPropNames::DIAG_DATA_RECEIVING_IP,
												  EquipmentPropNames::DIAG_DATA_RECEIVING_PORT,
												  &diagDataReceivingIP,
												  false, "", 0, log);

		result &= DeviceHelper::getIPv4Property(software, EquipmentPropNames::DIAG_DATA_RECEIVING_NETMASK,
												&diagDataReceivingNetmask, false, "", log);

		result &= DeviceHelper::getIpPortProperty(software,
												  EquipmentPropNames::CLIENT_REQUEST_IP,
												  EquipmentPropNames::CLIENT_REQUEST_PORT,
												  &clientRequestIP,
												  false, "", 0, log);

		result &= DeviceHelper::getIPv4Property(software, EquipmentPropNames::CLIENT_REQUEST_NETMASK,
												&clientRequestNetmask, false, "", log);

		result &= getSoftwareConnection(equipment, software,
										EquipmentPropNames::ARCH_SERVICE_ID,
										EquipmentPropNames::DIAG_DATA_RECEIVING_IP,
										EquipmentPropNames::DIAG_DATA_RECEIVING_PORT,
										&archServiceID,	&archServiceIP,
										true, Socket::IP_NULL,
										PORT_ARCHIVING_SERVICE_DIAG_DATA,
										E::SoftwareType::ArchiveService, log);

		result &= getCfgServiceConnection(equipment, software, &cfgServiceID1, &cfgServiceIP1,
										  &cfgServiceID2, &cfgServiceIP2, log);
		return result;
	}

#endif

// -------------------------------------------------------------------------------------
//
// TuningServiceSettings class implementation
//
// -------------------------------------------------------------------------------------

bool TuningServiceSettings::writeToXml(XmlWriteHelper& xml)
{
	writeStartSettings(xml);

	xml.writeStringElement(EquipmentPropNames::EQUIPMENT_ID, equipmentID);

	xml.writeHostAddressPort(EquipmentPropNames::CLIENT_REQUEST_IP,
							 EquipmentPropNames::CLIENT_REQUEST_PORT, clientRequestIP);
	xml.writeHostAddress(EquipmentPropNames::CLIENT_REQUEST_NETMASK, clientRequestNetmask);
	xml.writeHostAddressPort(EquipmentPropNames::TUNING_DATA_IP,
							 EquipmentPropNames::TUNING_DATA_PORT, tuningDataIP);
	xml.writeHostAddress(EquipmentPropNames::TUNING_DATA_NETMASK, tuningDataNetmask);
	xml.writeBoolElement(EquipmentPropNames::SINGLE_LM_CONTROL, singleLmControl);
	xml.writeBoolElement(EquipmentPropNames::DISABLE_MODULES_TYPE_CHECKING, disableModulesTypeChecking);
	xml.writeHostAddressPort(EquipmentPropNames::TUNING_SIM_IP,
							 EquipmentPropNames::TUNING_SIM_PORT,
							 tuningSimIP);

	// write tuning sources info
	//
	xml.writeStartElement(XmlElement::TUNING_SOURCES);
	xml.writeIntAttribute(XmlAttribute::COUNT, static_cast<int>(sources.size()));

	for(uint i = 0; i < sources.size(); i++)
	{
		TuningSource& ts = sources[i];

		xml.writeStartElement(XmlElement::TUNING_SOURCE);

		xml.writeStringAttribute(EquipmentPropNames::LM_EQUIPMENT_ID, ts.lmEquipmentID);
		xml.writeStringAttribute(EquipmentPropNames::PORT_EQUIPMENT_ID, ts.portEquipmentID);
		xml.writeStringAttribute(EquipmentPropNames::TUNING_DATA_IP, ts.tuningDataIP.addressPortStr());

		xml.writeEndElement();		// TUNING_SOURCE
	}

	xml.writeEndElement();			// TUNING_SOURCES

	// write tuning clients info
	//
	xml.writeStartElement(XmlElement::TUNING_CLIENTS);
	xml.writeIntAttribute(XmlAttribute::COUNT, static_cast<int>(clients.size()));

	for(uint i = 0; i < clients.size(); i++)
	{
		TuningClient& tc = clients[i];

		xml.writeStartElement(XmlElement::TUNING_CLIENT);
		xml.writeStringAttribute(EquipmentPropNames::EQUIPMENT_ID, tc.equipmentID);

		xml.writeStartElement(XmlElement::TUNING_SOURCES);
		xml.writeIntAttribute(XmlAttribute::COUNT, tc.sourcesIDs.count());
		xml.writeString(tc.sourcesIDs.join(Separator::SEMICOLON));
		xml.writeEndElement();		// TUNING_SOURCES

		xml.writeEndElement();		// TUNING_CLIENT
	}

	xml.writeEndElement();			// TUNING_CLIENTS

	writeEndSettings(xml);			// </Settings>

	return true;
}

bool TuningServiceSettings::readFromXml(XmlReadHelper& xml)
{
	bool result = false;

	result = readStartSettings(xml);

	RETURN_IF_FALSE(result);

	result &= xml.readStringElement(EquipmentPropNames::EQUIPMENT_ID, &equipmentID, true);

	result &= xml.readHostAddressPort(EquipmentPropNames::CLIENT_REQUEST_IP,
									  EquipmentPropNames::CLIENT_REQUEST_PORT, &clientRequestIP);
	result &= xml.readHostAddress(EquipmentPropNames::CLIENT_REQUEST_NETMASK, &clientRequestNetmask);
	result &= xml.readHostAddressPort(EquipmentPropNames::TUNING_DATA_IP,
									  EquipmentPropNames::TUNING_DATA_PORT, &tuningDataIP);
	result &= xml.readHostAddress(EquipmentPropNames::TUNING_DATA_NETMASK, &tuningDataNetmask);

	result &= xml.readBoolElement(EquipmentPropNames::SINGLE_LM_CONTROL, &singleLmControl, true);
	result &= xml.readBoolElement(EquipmentPropNames::DISABLE_MODULES_TYPE_CHECKING, &disableModulesTypeChecking, true);

	result &= xml.readHostAddressPort(EquipmentPropNames::TUNING_SIM_IP,
									  EquipmentPropNames::TUNING_SIM_PORT, &tuningSimIP);

	// read tuning sources info
	//
	result &= xml.findElement(XmlElement::TUNING_SOURCES);

	int sourcesCount = 0;

	result &= xml.readIntAttribute(XmlAttribute::COUNT, &sourcesCount);

	RETURN_IF_FALSE(result);

	for(int i = 0; i < sourcesCount; i++)
	{
		TuningSource ts;

		result &= xml.findElement(XmlElement::TUNING_SOURCE);
		result &= xml.readStringAttribute(EquipmentPropNames::LM_EQUIPMENT_ID, &ts.lmEquipmentID);
		result &= xml.readStringAttribute(EquipmentPropNames::PORT_EQUIPMENT_ID, &ts.portEquipmentID);

		QString addressPortStr;

		result &= xml.readStringAttribute(EquipmentPropNames::TUNING_DATA_IP, &addressPortStr);

		BREAK_IF_FALSE(result);

		ts.tuningDataIP.setAddressPortStr(addressPortStr, PORT_LM_TUNING);

		sources.push_back(ts);
	}

	RETURN_IF_FALSE(result);

	// read tuning clients info
	//
	clients.clear();

	result = xml.findElement(XmlElement::TUNING_CLIENTS);

	RETURN_IF_FALSE(result);

	int clientsCount = 0;

	result = xml.readIntAttribute(XmlAttribute::COUNT, &clientsCount);

	RETURN_IF_FALSE(result);

	for(int i = 0; i < clientsCount; i++)
	{
		TuningClient tc;

		result &= xml.findElement(XmlElement::TUNING_CLIENT);
		result &= xml.readStringAttribute(EquipmentPropNames::EQUIPMENT_ID, &tc.equipmentID);
		result &= xml.findElement(XmlElement::TUNING_SOURCES);

		int srcsCount = 0;

		result &= xml.readIntAttribute(XmlAttribute::COUNT, &srcsCount);

		QString sourcesIDs;

		result &= xml.readStringElement(XmlElement::TUNING_SOURCES, &sourcesIDs);

		BREAK_IF_FALSE(result);

		tc.sourcesIDs = sourcesIDs.split(Separator::SEMICOLON, Qt::SkipEmptyParts);

		Q_ASSERT(tc.sourcesIDs.count() == srcsCount);

		clients.push_back(tc);
	}

	return result;
}

#ifdef IS_BUILDER

	// -------------------------------------------------------------------------------------
	//
	// TuningServiceSettingsGetter class implementation
	//
	// -------------------------------------------------------------------------------------

	bool TuningServiceSettingsGetter::readFromDevice(const Builder::Context* context,
													 const Hardware::Software* software)
	{
		TEST_PTR_RETURN_FALSE(context);

		Builder::IssueLogger* log = context->m_log;

		TEST_PTR_RETURN_FALSE(log);
		TEST_PTR_LOG_RETURN_FALSE(software, log);

		bool result = true;

		equipmentID = software->equipmentIdTemplate();

		result &= DeviceHelper::getIpPortProperty(software,
												  EquipmentPropNames::CLIENT_REQUEST_IP,
												  EquipmentPropNames::CLIENT_REQUEST_PORT,
												  &clientRequestIP, false, "", 0, log);

		result &= DeviceHelper::getIPv4Property(software,
												EquipmentPropNames::CLIENT_REQUEST_NETMASK,
												&clientRequestNetmask, false, "", log);

		result &= DeviceHelper::getIpPortProperty(software,
												  EquipmentPropNames::TUNING_DATA_IP,
												  EquipmentPropNames::TUNING_DATA_PORT,
												  &tuningDataIP, false, "", 0, log);

		result &= DeviceHelper::getIPv4Property(software,
												EquipmentPropNames::TUNING_DATA_NETMASK,
												&tuningDataNetmask, false, "", log);

		result &= DeviceHelper::getBoolProperty(software, EquipmentPropNames::SINGLE_LM_CONTROL, &singleLmControl, log);
		result &= DeviceHelper::getBoolProperty(software, EquipmentPropNames::DISABLE_MODULES_TYPE_CHECKING, &disableModulesTypeChecking, log);

		// for a now tuningSimIP isn't read from equipment

		result &= fillTuningSourcesInfo(context, software);

		result &= fillTuningClientsInfo(context, software, singleLmControl);

		return result;
	}

	bool TuningServiceSettingsGetter::fillTuningSourcesInfo(const Builder::Context* context,
															const Hardware::Software* software)
	{
		tuningSources.clear();
		sources.clear();

		Builder::IssueLogger* log = context->m_log;

		bool result = true;

		quint32 receivingNetmask = tuningDataNetmask.toIPv4Address();

		quint32 receivingSubnet = tuningDataIP.address32() & receivingNetmask;

		for(Hardware::DeviceModule* lm : context->m_lmModules)
		{
			if (lm == nullptr)
			{
				LOG_NULLPTR_ERROR(log);
				result = false;
				continue;
			}

			std::shared_ptr<LmDescription> lmDescription = context->m_lmDescriptions->get(lm);

			if (lmDescription == nullptr)
			{
				LOG_INTERNAL_ERROR_MSG(log, QString("LmDescription is not found for module %1").arg(lm->equipmentIdTemplate()));
				result = false;
				continue;
			}

			const LmDescription::Lan& lan = lmDescription->lan();

			for(const LmDescription::LanController& lanController : lan.m_lanControllers)
			{
				if (lanController.isProvideTuning() == false)
				{
					continue;
				}

				Tuning::TuningSource ts;

				result &= getLmPropertiesFromDevice(lm, DataSource::DataType::Tuning,
													   lanController.m_place,
													   lanController.m_type,
													   context,
													   &ts);
				if (result == false)
				{
					continue;
				}

				if (ts.lmDataEnable() == false || ts.serviceID() != software->equipmentIdTemplate())
				{
					continue;
				}

				if ((ts.lmAddress().toIPv4Address() & receivingNetmask) != receivingSubnet)
				{
					// Different subnet address in data source IP %1 (%2) and data receiving IP %3 (%4).
					//
					log->errCFG3043(ts.lmAddress().toString(),
									  ts.lmAdapterID(),
									  tuningDataIP.addressStr(),
									  software->equipmentIdTemplate());
					result = false;
					continue;
				}

				Tuning::TuningData* tuningData = context->m_tuningDataStorage->value(lm->equipmentId(), nullptr);

				if(tuningData != nullptr)
				{
					ts.setTuningData(tuningData);
				}
				else
				{
					LOG_ERROR_OBSOLETE(log, Builder::IssueType::NotDefined,
									   QString(tr("Tuning data for LM '%1' is not found")).arg(lm->equipmentIdTemplate()));
					result = false;
				}

				tuningSources.append(ts);
			}
		}

		for(const Tuning::TuningSource& ts : tuningSources)
		{
			TuningServiceSettings::TuningSource tunSrc;

			tunSrc.lmEquipmentID = ts.lmEquipmentID();
			tunSrc.portEquipmentID = ts.lmAdapterID();
			tunSrc.tuningDataIP = ts.lmAddressPort();

			sources.push_back(tunSrc);
		}

		return result;
	}

	bool TuningServiceSettingsGetter::fillTuningClientsInfo(const Builder::Context* context,
															const Hardware::Software* software,
															bool singleLmControlEnabled)
	{
		clients.clear();

		Builder::IssueLogger* log = context->m_log;

		bool result = true;

		Hardware::DeviceRoot* root = const_cast<Hardware::DeviceRoot*>(software->getParentRoot());

		if (root == nullptr)
		{
			assert(false);
			return false;
		}

		Hardware::equipmentWalker(root,
			[this, &software, &result, &singleLmControlEnabled, &log](Hardware::DeviceObject* currentDevice)
			{
				if (currentDevice->isSoftware() == false)
				{
					return;
				}

				Hardware::Software* tuningClient = dynamic_cast<Hardware::Software*>(currentDevice);

				if (tuningClient == nullptr)
				{
					assert(false);
					result = false;
					return;
				}

				if (tuningClient->type() != E::SoftwareType::TuningClient &&
					tuningClient->type() != E::SoftwareType::Metrology &&
					tuningClient->type() != E::SoftwareType::Monitor &&
					tuningClient->type() != E::SoftwareType::TestClient)
				{
					return;
				}

				// sw is TuningClient or Metrology
				//
				QString tuningServiceID;

				result &= DeviceHelper::getStrProperty(tuningClient, EquipmentPropNames::TUNING_SERVICE_ID, &tuningServiceID, log);

				if (result == false)
				{
					return;
				}

				if (tuningServiceID != software->equipmentIdTemplate())
				{
					return;
				}

				bool tuningEnable = true;			// by default tuning is enabled for known clients without property "TuningEnable"

				if (DeviceHelper::isPropertyExists(tuningClient, EquipmentPropNames::TUNING_ENABLE) == true)
				{
					result &= DeviceHelper::getBoolProperty(tuningClient, EquipmentPropNames::TUNING_ENABLE, &tuningEnable, log);

					if (result == false)
					{
						return;
					}

					if (tuningEnable == false)
					{
						return;
					}

					if (tuningClient->type() == E::SoftwareType::Monitor && singleLmControlEnabled == true)
					{
						// Monitor %1 cannot be connected to TuningService %2 with enabled SingleLmControl mode.
						//
						log->errALC5150(tuningClient->equipmentIdTemplate(), software->equipmentIdTemplate());
						result = false;
					}
				}

				// TuningClient is linked to this TuningService

				TuningClient tc;

				tc.equipmentID = tuningClient->equipmentIdTemplate();

				result &= DeviceHelper::getStrListProperty(tuningClient, EquipmentPropNames::TUNING_SOURCE_EQUIPMENT_ID,
														   &tc.sourcesIDs, log);

				this->clients.push_back(tc);
			}
		);

		return result;
	}

#endif

// -------------------------------------------------------------------------------------
//
// ArchivingServiceSettings class implementation
//
// -------------------------------------------------------------------------------------

bool ArchivingServiceSettings::writeToXml(XmlWriteHelper& xml)
{
	writeStartSettings(xml);

	xml.writeHostAddressPort(EquipmentPropNames::CLIENT_REQUEST_IP,
							 EquipmentPropNames::CLIENT_REQUEST_PORT, clientRequestIP);
	xml.writeHostAddress(EquipmentPropNames::CLIENT_REQUEST_NETMASK, clientRequestNetmask);

	xml.writeHostAddressPort(EquipmentPropNames::APP_DATA_RECEIVING_IP,
							 EquipmentPropNames::APP_DATA_RECEIVING_PORT, appDataReceivingIP);
	xml.writeHostAddress(EquipmentPropNames::APP_DATA_RECEIVING_NETMASK, appDataReceivingNetmask);

	xml.writeHostAddressPort(EquipmentPropNames::DIAG_DATA_RECEIVING_IP,
							 EquipmentPropNames::DIAG_DATA_RECEIVING_PORT, diagDataReceivingIP);
	xml.writeHostAddress(EquipmentPropNames::DIAG_DATA_RECEIVING_NETMASK, diagDataReceivingNetmask);

	xml.writeIntElement(EquipmentPropNames::ARCHIVE_SHORT_TERM_PERIOD, shortTermArchivePeriod);
	xml.writeIntElement(EquipmentPropNames::ARCHIVE_LONG_TERM_PERIOD, longTermArchivePeriod);
	xml.writeStringElement(EquipmentPropNames::ARCHIVE_LOCATION, archiveLocation);

	writeEndSettings(xml);		// </Settings>

	return true;
}

bool ArchivingServiceSettings::readFromXml(XmlReadHelper& xml)
{
	bool result = false;

	result = readStartSettings(xml);

	RETURN_IF_FALSE(result);

	result &= xml.readHostAddressPort(EquipmentPropNames::CLIENT_REQUEST_IP,
									  EquipmentPropNames::CLIENT_REQUEST_PORT, &clientRequestIP);
	result &= xml.readHostAddress(EquipmentPropNames::CLIENT_REQUEST_NETMASK, &clientRequestNetmask);

	result &= xml.readHostAddressPort(EquipmentPropNames::APP_DATA_RECEIVING_IP,
									  EquipmentPropNames::APP_DATA_RECEIVING_PORT, &appDataReceivingIP);
	result &= xml.readHostAddress(EquipmentPropNames::APP_DATA_RECEIVING_NETMASK, &appDataReceivingNetmask);

	result &= xml.readHostAddressPort(EquipmentPropNames::DIAG_DATA_RECEIVING_IP,
									  EquipmentPropNames::DIAG_DATA_RECEIVING_PORT, &diagDataReceivingIP);
	result &= xml.readHostAddress(EquipmentPropNames::DIAG_DATA_RECEIVING_NETMASK, &diagDataReceivingNetmask);

	result &= xml.readIntElement(EquipmentPropNames::ARCHIVE_SHORT_TERM_PERIOD, &shortTermArchivePeriod, true);
	result &= xml.readIntElement(EquipmentPropNames::ARCHIVE_LONG_TERM_PERIOD, &longTermArchivePeriod, true);
	result &= xml.readStringElement(EquipmentPropNames::ARCHIVE_LOCATION, &archiveLocation, true);

	return result;
}

#ifdef IS_BUILDER

	// -------------------------------------------------------------------------------------
	//
	// ArchivingServiceSettingsGetter class implementation
	//
	// -------------------------------------------------------------------------------------

	bool ArchivingServiceSettingsGetter::readFromDevice(const Builder::Context* context,
														const Hardware::Software* software)
	{
		TEST_PTR_RETURN_FALSE(context);

		Builder::IssueLogger* log = context->m_log;

		TEST_PTR_RETURN_FALSE(log);
		TEST_PTR_LOG_RETURN_FALSE(software, log);

		bool result = true;

		result &= DeviceHelper::getIpPortProperty(software,
												  EquipmentPropNames::CLIENT_REQUEST_IP,
												  EquipmentPropNames::CLIENT_REQUEST_PORT,
												  &clientRequestIP, false, "", 0, log);

		result &= DeviceHelper::getIPv4Property(software,
												EquipmentPropNames::CLIENT_REQUEST_NETMASK,
												&clientRequestNetmask,
												false, "", log);
		//

		result &= DeviceHelper::getIpPortProperty(software,
												  EquipmentPropNames::APP_DATA_RECEIVING_IP,
												  EquipmentPropNames::APP_DATA_RECEIVING_PORT,
												  &appDataReceivingIP, false, "", 0, log);

		result &= DeviceHelper::getIPv4Property(software,
												EquipmentPropNames::APP_DATA_RECEIVING_NETMASK,
												&appDataReceivingNetmask,
												false, "", log);
		//

		result &= DeviceHelper::getIpPortProperty(software,
												  EquipmentPropNames::DIAG_DATA_RECEIVING_IP,
												  EquipmentPropNames::DIAG_DATA_RECEIVING_PORT,
												  &diagDataReceivingIP, false, "", 0, log);

		result &= DeviceHelper::getIPv4Property(software,
												EquipmentPropNames::DIAG_DATA_RECEIVING_NETMASK,
												&diagDataReceivingNetmask,
												false, "", log);
		//

		result &= DeviceHelper::getIntProperty(software, EquipmentPropNames::ARCHIVE_SHORT_TERM_PERIOD, &shortTermArchivePeriod, log);
		result &= DeviceHelper::getIntProperty(software, EquipmentPropNames::ARCHIVE_LONG_TERM_PERIOD, &longTermArchivePeriod, log);
		result &= DeviceHelper::getStrProperty(software, EquipmentPropNames::ARCHIVE_LOCATION, &archiveLocation, log);

		return result;
	}

	bool ArchivingServiceSettingsGetter::checkSettings(const Hardware::Software *software, Builder::IssueLogger* log)
	{
		TEST_PTR_RETURN_FALSE(log);
		TEST_PTR_LOG_RETURN_FALSE(software, log);

		bool result = true;

		if (archiveLocation.isEmpty() == true)
		{
			log->wrnCFG3031(software->equipmentIdTemplate(), EquipmentPropNames::ARCHIVE_LOCATION);
		}

		return result;
	}

#endif

// -------------------------------------------------------------------------------------
//
// TestClientSettings class implementation
//
// -------------------------------------------------------------------------------------

bool TestClientSettings::writeToXml(XmlWriteHelper& xml)
{
	writeStartSettings(xml);

	//

	xml.writeStartElement(XmlElement::CFG_SERVICE1);
	xml.writeStringAttribute(EquipmentPropNames::EQUIPMENT_ID, cfgService1_equipmentID);
	xml.writeHostAddressPort(EquipmentPropNames::CLIENT_REQUEST_IP,
							 EquipmentPropNames::CLIENT_REQUEST_PORT, cfgService1_clientRequestIP);
	xml.writeEndElement();	// </CgService1>

	//

	xml.writeStartElement(XmlElement::CFG_SERVICE2);
	xml.writeStringAttribute(EquipmentPropNames::EQUIPMENT_ID, cfgService2_equipmentID);
	xml.writeHostAddressPort(EquipmentPropNames::CLIENT_REQUEST_IP,
							 EquipmentPropNames::CLIENT_REQUEST_PORT, cfgService2_clientRequestIP);
	xml.writeEndElement();	// </CgService2>

	//

	xml.writeStartElement(XmlElement::APP_DATA_SERVICE);
	xml.writeStringAttribute(EquipmentPropNames::EQUIPMENT_ID, appDataService_equipmentID);
	xml.writeHostAddressPort(EquipmentPropNames::APP_DATA_RECEIVING_IP,
							 EquipmentPropNames::APP_DATA_RECEIVING_PORT, appDataService_appDataReceivingIP);
	xml.writeHostAddressPort(EquipmentPropNames::CLIENT_REQUEST_IP,
							 EquipmentPropNames::CLIENT_REQUEST_PORT, appDataService_clientRequestIP);
	xml.writeEndElement();	// </AppDataService>

	//

	xml.writeStartElement(XmlElement::DIAG_DATA_SERVICE);
	xml.writeStringAttribute(EquipmentPropNames::EQUIPMENT_ID, diagDataService_equipmentID);
	xml.writeHostAddressPort(EquipmentPropNames::DIAG_DATA_RECEIVING_IP,
							 EquipmentPropNames::DIAG_DATA_RECEIVING_PORT, diagDataService_diagDataReceivingIP);
	xml.writeHostAddressPort(EquipmentPropNames::CLIENT_REQUEST_IP,
							 EquipmentPropNames::CLIENT_REQUEST_PORT, diagDataService_clientRequestIP);
	xml.writeEndElement();	// </DiagDataService>

	//

	xml.writeStartElement(XmlElement::ARCHIVE_SERVICE);
	xml.writeStringAttribute(EquipmentPropNames::EQUIPMENT_ID, archService_equipmentID);
	xml.writeHostAddressPort(EquipmentPropNames::APP_DATA_RECEIVING_IP,
							 EquipmentPropNames::APP_DATA_RECEIVING_PORT, archService_appDataReceivingIP);
	xml.writeHostAddressPort(EquipmentPropNames::DIAG_DATA_RECEIVING_IP,
							 EquipmentPropNames::DIAG_DATA_RECEIVING_PORT, archService_diagDataReceivingIP);
	xml.writeHostAddressPort(EquipmentPropNames::CLIENT_REQUEST_IP,
							 EquipmentPropNames::CLIENT_REQUEST_PORT, archService_clientRequestIP);
	xml.writeEndElement();	// </ArchService>

	//

	xml.writeStartElement(XmlElement::TUNING_SERVICE);
	xml.writeStringAttribute(EquipmentPropNames::EQUIPMENT_ID, tuningService_equipmentID);
	xml.writeHostAddressPort(EquipmentPropNames::TUNING_DATA_IP,
							 EquipmentPropNames::TUNING_DATA_PORT, tuningService_tuningDataIP);
	xml.writeHostAddressPort(EquipmentPropNames::CLIENT_REQUEST_IP,
							 EquipmentPropNames::CLIENT_REQUEST_PORT, tuningService_clientRequestIP);

	QString tuningSources = tuningService_tuningSources.join(";");
	xml.writeStringElement(EquipmentPropNames::TUNING_SOURCE_EQUIPMENT_ID, tuningSources);

	xml.writeEndElement();	// </TuingService>

	//

	writeEndSettings(xml);	// </Settings>

	return true;
}

bool TestClientSettings::readFromXml(XmlReadHelper& xml)
{
	bool result = true;

	result = readStartSettings(xml);

	RETURN_IF_FALSE(result);

	result &= xml.findElement(XmlElement::CFG_SERVICE1);
	result &= xml.readStringAttribute(EquipmentPropNames::EQUIPMENT_ID, &cfgService1_equipmentID);
	result &= xml.readHostAddressPort(EquipmentPropNames::CLIENT_REQUEST_IP,
									  EquipmentPropNames::CLIENT_REQUEST_PORT, &cfgService1_clientRequestIP);

	result &= xml.findElement(XmlElement::CFG_SERVICE2);
	result &= xml.readStringAttribute(EquipmentPropNames::EQUIPMENT_ID, &cfgService2_equipmentID);
	result &= xml.readHostAddressPort(EquipmentPropNames::CLIENT_REQUEST_IP,
									  EquipmentPropNames::CLIENT_REQUEST_PORT, &cfgService2_clientRequestIP);

	result &= xml.findElement(XmlElement::APP_DATA_SERVICE);
	result &= xml.readStringAttribute(EquipmentPropNames::EQUIPMENT_ID, &appDataService_equipmentID);
	result &= xml.readHostAddressPort(EquipmentPropNames::APP_DATA_RECEIVING_IP,
									  EquipmentPropNames::APP_DATA_RECEIVING_PORT, &appDataService_appDataReceivingIP);
	result &= xml.readHostAddressPort(EquipmentPropNames::CLIENT_REQUEST_IP,
									  EquipmentPropNames::CLIENT_REQUEST_PORT, &appDataService_clientRequestIP);

	result &= xml.findElement(XmlElement::DIAG_DATA_SERVICE);
	result &= xml.readStringAttribute(EquipmentPropNames::EQUIPMENT_ID, &diagDataService_equipmentID);
	result &= xml.readHostAddressPort(EquipmentPropNames::DIAG_DATA_RECEIVING_IP,
									  EquipmentPropNames::DIAG_DATA_RECEIVING_PORT, &diagDataService_diagDataReceivingIP);
	result &= xml.readHostAddressPort(EquipmentPropNames::CLIENT_REQUEST_IP,
									  EquipmentPropNames::CLIENT_REQUEST_PORT, &diagDataService_clientRequestIP);

	result &= xml.findElement(XmlElement::ARCHIVE_SERVICE);
	result &= xml.readStringAttribute(EquipmentPropNames::EQUIPMENT_ID, &archService_equipmentID);
	result &= xml.readHostAddressPort(EquipmentPropNames::APP_DATA_RECEIVING_IP,
									  EquipmentPropNames::APP_DATA_RECEIVING_PORT, &archService_appDataReceivingIP);
	result &= xml.readHostAddressPort(EquipmentPropNames::DIAG_DATA_RECEIVING_IP,
									  EquipmentPropNames::DIAG_DATA_RECEIVING_PORT, &archService_diagDataReceivingIP);
	result &= xml.readHostAddressPort(EquipmentPropNames::CLIENT_REQUEST_IP,
									  EquipmentPropNames::CLIENT_REQUEST_PORT, &archService_clientRequestIP);

	result &= xml.findElement(XmlElement::TUNING_SERVICE);
	result &= xml.readStringAttribute(EquipmentPropNames::EQUIPMENT_ID, &tuningService_equipmentID);
	result &= xml.readHostAddressPort(EquipmentPropNames::TUNING_DATA_IP,
									  EquipmentPropNames::TUNING_DATA_PORT, &tuningService_tuningDataIP);
	result &= xml.readHostAddressPort(EquipmentPropNames::CLIENT_REQUEST_IP,
									  EquipmentPropNames::CLIENT_REQUEST_PORT, &tuningService_clientRequestIP);
	return result;
}

#ifdef IS_BUILDER

	// -------------------------------------------------------------------------------------
	//
	// TestClientSettingsGetter class implementation
	//
	// -------------------------------------------------------------------------------------

	bool TestClientSettingsGetter::readFromDevice(const Builder::Context* context,
												  const Hardware::Software* software)
	{
		TEST_PTR_RETURN_FALSE(context);

		Builder::IssueLogger* log = context->m_log;

		TEST_PTR_RETURN_FALSE(log);
		TEST_PTR_LOG_RETURN_FALSE(software, log);

		const Hardware::EquipmentSet* equipment = context->m_equipmentSet.get();

		bool result = true;

		// Get CfgService connection

		result &= getSoftwareConnection(equipment,
										software,
										EquipmentPropNames::CFG_SERVICE_ID1,
										EquipmentPropNames::CLIENT_REQUEST_IP,
										EquipmentPropNames::CLIENT_REQUEST_PORT,
										&cfgService1_equipmentID,
										&cfgService1_clientRequestIP,
										true, Socket::IP_NULL,
										PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST,
										E::SoftwareType::ConfigurationService, log);
		if (result == false)
		{
			return false;
		}

		result &= getSoftwareConnection(equipment,
										software,
										EquipmentPropNames::CFG_SERVICE_ID2,
										EquipmentPropNames::CLIENT_REQUEST_IP,
										EquipmentPropNames::CLIENT_REQUEST_PORT,
										&cfgService2_equipmentID,
										&cfgService2_clientRequestIP,
										true, Socket::IP_NULL,
										PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST,
										E::SoftwareType::ConfigurationService, log);
		if (result == false)
		{
			return false;
		}

		if (cfgService1_equipmentID.isEmpty() == true && cfgService2_equipmentID.isEmpty() == true)
		{
			// Software %1 is not linked to ConfigurationService.
			//
			log->errCFG3029(software->equipmentIdTemplate());
			return false;
		}

		// Get AppDataService connection

		result &= getSoftwareConnection(equipment,
										software,
										EquipmentPropNames::APP_DATA_SERVICE_ID,
										EquipmentPropNames::APP_DATA_RECEIVING_IP,
										EquipmentPropNames::APP_DATA_RECEIVING_PORT,
										&appDataService_equipmentID,
										&appDataService_appDataReceivingIP,
										false, Socket::IP_NULL,
										PORT_APP_DATA_SERVICE_DATA,
										E::SoftwareType::AppDataService, log);
		if (result == false)
		{
			return false;
		}

		result &= getSoftwareConnection(equipment,
										software,
										EquipmentPropNames::APP_DATA_SERVICE_ID,
										EquipmentPropNames::CLIENT_REQUEST_IP,
										EquipmentPropNames::CLIENT_REQUEST_PORT,
										&appDataService_equipmentID,
										&appDataService_clientRequestIP,
										false, Socket::IP_NULL,
										PORT_APP_DATA_SERVICE_CLIENT_REQUEST,
										E::SoftwareType::AppDataService, log);
		if (result == false)
		{
			return false;
		}

		const Hardware::Software* appDataService = DeviceHelper::getSoftware(equipment, appDataService_equipmentID);

		if (appDataService == nullptr)
		{
			LOG_INTERNAL_ERROR(log);
			return false;
		}

		// Get ArchiveService connection

		result &= getSoftwareConnection(equipment,
										appDataService,
										EquipmentPropNames::ARCH_SERVICE_ID,
										EquipmentPropNames::APP_DATA_RECEIVING_IP,
										EquipmentPropNames::APP_DATA_RECEIVING_PORT,
										&archService_equipmentID,
										&archService_appDataReceivingIP,
										false, Socket::IP_NULL,
										PORT_ARCHIVING_SERVICE_APP_DATA,
										E::SoftwareType::ArchiveService, log);
		if (result == false)
		{
			return false;
		}

		result &= getSoftwareConnection(equipment,
										appDataService,
										EquipmentPropNames::ARCH_SERVICE_ID,
										EquipmentPropNames::DIAG_DATA_RECEIVING_IP,
										EquipmentPropNames::DIAG_DATA_RECEIVING_PORT,
										&archService_equipmentID,
										&archService_diagDataReceivingIP,
										false, Socket::IP_NULL,
										PORT_ARCHIVING_SERVICE_DIAG_DATA,
										E::SoftwareType::ArchiveService, log);
		if (result == false)
		{
			return false;
		}

		result &= getSoftwareConnection(equipment,
										appDataService,
										EquipmentPropNames::ARCH_SERVICE_ID,
										EquipmentPropNames::CLIENT_REQUEST_IP,
										EquipmentPropNames::CLIENT_REQUEST_PORT,
										&archService_equipmentID,
										&archService_clientRequestIP,
										false, Socket::IP_NULL,
										PORT_ARCHIVING_SERVICE_CLIENT_REQUEST,
										E::SoftwareType::ArchiveService, log);
		if (result == false)
		{
			return false;
		}

		// Get TuningService connection

		result &= getSoftwareConnection(equipment,
										software,
										EquipmentPropNames::TUNING_SERVICE_ID,
										EquipmentPropNames::TUNING_DATA_IP,
										EquipmentPropNames::TUNING_DATA_PORT,
										&tuningService_equipmentID,
										&tuningService_tuningDataIP,
										false, Socket::IP_NULL,
										PORT_TUNING_SERVICE_DATA,
										E::SoftwareType::TuningService, log);
		if (result == false)
		{
			return false;
		}

		result &= getSoftwareConnection(equipment,
										software,
										EquipmentPropNames::TUNING_SERVICE_ID,
										EquipmentPropNames::CLIENT_REQUEST_IP,
										EquipmentPropNames::CLIENT_REQUEST_PORT,
										&tuningService_equipmentID,
										&tuningService_clientRequestIP,
										false, Socket::IP_NULL,
										PORT_TUNING_SERVICE_CLIENT_REQUEST,
										E::SoftwareType::TuningService, log);

		result &= DeviceHelper::getStrListProperty(software, EquipmentPropNames::TUNING_SOURCE_EQUIPMENT_ID,
												   &tuningService_tuningSources, log);

		if (result == false)
		{
			return false;
		}

		// Get DiagDataService connection

		result &= getSoftwareConnection(equipment,
										software,
										EquipmentPropNames::DIAG_DATA_SERVICE_ID,
										EquipmentPropNames::DIAG_DATA_RECEIVING_IP,
										EquipmentPropNames::DIAG_DATA_RECEIVING_PORT,
										&diagDataService_equipmentID,
										&diagDataService_diagDataReceivingIP,
										true, Socket::IP_NULL,
										PORT_DIAG_DATA_SERVICE_DATA,
										E::SoftwareType::DiagDataService, log);
		if (result == false)
		{
			return false;
		}

		result &= getSoftwareConnection(equipment,
										software,
										EquipmentPropNames::DIAG_DATA_SERVICE_ID,
										EquipmentPropNames::CLIENT_REQUEST_IP,
										EquipmentPropNames::CLIENT_REQUEST_PORT,
										&diagDataService_equipmentID,
										&diagDataService_clientRequestIP,
										true, Socket::IP_NULL,
										PORT_DIAG_DATA_SERVICE_CLIENT_REQUEST,
										E::SoftwareType::DiagDataService, log);
		return result;
	}

#endif

// -------------------------------------------------------------------------------------
//
// MetrologySettings class implementation
//
// -------------------------------------------------------------------------------------

bool MetrologySettings::writeToXml(XmlWriteHelper& xml)
{
	writeStartSettings(xml);

	xml.writeStartElement(XmlElement::APP_DATA_SERVICE);

	xml.writeBoolAttribute(XmlAttribute::APP_DATA_SERVICE_PROPERTY_IS_VALID1, appDataServicePropertyIsValid1);
	xml.writeStringAttribute(EquipmentPropNames::APP_DATA_SERVICE_ID1, appDataServiceID1);
	xml.writeStringAttribute(XmlAttribute::APP_DATA_SERVICE_IP1, appDataServiceIP1);
	xml.writeIntAttribute(XmlAttribute::APP_DATA_SERVICE_PORT1, appDataServicePort1);

	xml.writeBoolAttribute(XmlAttribute::APP_DATA_SERVICE_PROPERTY_IS_VALID2, appDataServicePropertyIsValid2);
	xml.writeStringAttribute(EquipmentPropNames::APP_DATA_SERVICE_ID2, appDataServiceID2);
	xml.writeStringAttribute(XmlAttribute::APP_DATA_SERVICE_IP2, appDataServiceIP2);
	xml.writeIntAttribute(XmlAttribute::APP_DATA_SERVICE_PORT2, appDataServicePort2);

	xml.writeEndElement();		// </AppDataService>

	xml.writeStartElement(XmlElement::TUNING_SERVICE);

	xml.writeBoolAttribute(XmlAttribute::TUNING_SERVICE_PROPERTY_IS_VALID, tuningServicePropertyIsValid);
	xml.writeStringAttribute(XmlAttribute::SOFTWARE_METROLOGY_ID, softwareMetrologyID);
	xml.writeStringAttribute(XmlAttribute::TUNING_SERVICE_IP, tuningServiceIP);
	xml.writeIntAttribute(XmlAttribute::TUNING_SERVICE_PORT, tuningServicePort);

	xml.writeEndElement();		// </TuningService>

	writeEndSettings(xml);		// </Settings>

	return true;
}

bool MetrologySettings::readFromXml(XmlReadHelper& xml)
{
	bool result = true;

	result = readStartSettings(xml);

	RETURN_IF_FALSE(result);

	// AppDataService
	//
	result &= xml.findElement(XmlElement::APP_DATA_SERVICE);

	// primary
	//
	result &= xml.readBoolAttribute(XmlAttribute::APP_DATA_SERVICE_PROPERTY_IS_VALID1, &appDataServicePropertyIsValid1);
	result &= xml.readStringAttribute(EquipmentPropNames::APP_DATA_SERVICE_ID1, &appDataServiceID1);

	result &= xml.readStringAttribute(XmlAttribute::APP_DATA_SERVICE_IP1, &appDataServiceIP1);
	result &= xml.readIntAttribute(XmlAttribute::APP_DATA_SERVICE_PORT1, &appDataServicePort1);

	// reserve
	//
	result &= xml.readBoolAttribute(XmlAttribute::APP_DATA_SERVICE_PROPERTY_IS_VALID2, &appDataServicePropertyIsValid2);
	result &= xml.readStringAttribute(EquipmentPropNames::APP_DATA_SERVICE_ID2, &appDataServiceID2);

	result &= xml.readStringAttribute(XmlAttribute::APP_DATA_SERVICE_IP2, &appDataServiceIP2);
	result &= xml.readIntAttribute(XmlAttribute::APP_DATA_SERVICE_PORT2, &appDataServicePort2);

	// TuningService
	//
	result &= xml.findElement(XmlElement::TUNING_SERVICE);

	result &= xml.readBoolAttribute(XmlAttribute::TUNING_SERVICE_PROPERTY_IS_VALID, &tuningServicePropertyIsValid);
	result &= xml.readStringAttribute(XmlAttribute::SOFTWARE_METROLOGY_ID, &softwareMetrologyID);
	result &= xml.readStringAttribute(XmlAttribute::TUNING_SERVICE_IP, &tuningServiceIP);
	result &= xml.readIntAttribute(XmlAttribute::TUNING_SERVICE_PORT, &tuningServicePort);

	return result;
}

#ifdef IS_BUILDER

	// -------------------------------------------------------------------------------------
	//
	// MetrologySettings class implementation
	//
	// -------------------------------------------------------------------------------------


	bool MetrologySettingsGetter::readFromDevice(const Builder::Context* context,
												 const Hardware::Software* software)
	{
		TEST_PTR_RETURN_FALSE(context);

		Builder::IssueLogger* log = context->m_log;

		TEST_PTR_RETURN_FALSE(log);
		TEST_PTR_LOG_RETURN_FALSE(software, log);

		const Hardware::EquipmentSet* equipment = context->m_equipmentSet.get();

		appDataServicePropertyIsValid1 = false;
		appDataServicePropertyIsValid2 = false;
		tuningServicePropertyIsValid = false;

		bool result = true;

		result &= DeviceHelper::getStrProperty(software, EquipmentPropNames::APP_DATA_SERVICE_ID1, &appDataServiceID1, log);
		result &= DeviceHelper::getStrProperty(software, EquipmentPropNames::APP_DATA_SERVICE_ID2, &appDataServiceID2, log);

		RETURN_IF_FALSE(result);

		if (appDataServiceID1.isEmpty() == true &&
			appDataServiceID2.isEmpty() == true)
		{
			// Property '%1.%2' is empty.
			//
			log->errCFG3022(software->equipmentId(), EquipmentPropNames::APP_DATA_SERVICE_ID1);
			log->errCFG3022(software->equipmentId(), EquipmentPropNames::APP_DATA_SERVICE_ID2);

			return false;
		}

		if (appDataServiceID1.isEmpty() == false)
		{
			HostAddressPort appDataServiceClientRequestIP1;

			result = getSoftwareConnection(equipment,
											software,
											EquipmentPropNames::APP_DATA_SERVICE_ID1,
											EquipmentPropNames::CLIENT_REQUEST_IP,
											EquipmentPropNames::CLIENT_REQUEST_PORT,
											&appDataServiceID1,
											&appDataServiceClientRequestIP1,
											true,
											Socket::IP_NULL,
											PORT_APP_DATA_SERVICE_CLIENT_REQUEST,
											E::SoftwareType::AppDataService,
											log);
			RETURN_IF_FALSE(result);

			appDataServiceIP1 = appDataServiceClientRequestIP1.addressStr();
			appDataServicePort1 = appDataServiceClientRequestIP1.port();

			appDataServicePropertyIsValid1 = true;
		}

		if (appDataServiceID2.isEmpty() == false)
		{
			HostAddressPort appDataServiceClientRequestIP2;

			result = getSoftwareConnection(equipment,
											software,
											EquipmentPropNames::APP_DATA_SERVICE_ID2,
											EquipmentPropNames::CLIENT_REQUEST_IP,
											EquipmentPropNames::CLIENT_REQUEST_PORT,
											&appDataServiceID2,
											&appDataServiceClientRequestIP2,
											true,
											Socket::IP_NULL,
											PORT_APP_DATA_SERVICE_CLIENT_REQUEST,
											E::SoftwareType::AppDataService,
											log);
			RETURN_IF_FALSE(result);

			appDataServiceIP2 = appDataServiceClientRequestIP2.addressStr();
			appDataServicePort2 = appDataServiceClientRequestIP2.port();

			appDataServicePropertyIsValid2 = true;
		}

		// TuningService
		//
		HostAddressPort tuningServiceClientRequestIP;

		result = getSoftwareConnection(equipment,
										software,
										EquipmentPropNames::TUNING_SERVICE_ID,
										EquipmentPropNames::CLIENT_REQUEST_IP,
										EquipmentPropNames::CLIENT_REQUEST_PORT,
										&tuningServiceID,
										&tuningServiceClientRequestIP,
										false,
										Socket::IP_NULL,
										PORT_TUNING_SERVICE_CLIENT_REQUEST,
										E::SoftwareType::TuningService,
										log);
		RETURN_IF_FALSE(result);

		softwareMetrologyID = software->equipmentIdTemplate();

		tuningServiceIP = tuningServiceClientRequestIP.addressStr();
		tuningServicePort = tuningServiceClientRequestIP.port();

		tuningServicePropertyIsValid = true;

		return	true;
	}

#endif

// -------------------------------------------------------------------------------------
//
// MonitorSettings class implementation
//
// -------------------------------------------------------------------------------------

bool MonitorSettings::writeToXml(XmlWriteHelper& xml)
{
	writeStartSettings(xml);

	//

	xml.writeStringElement(EquipmentPropNames::START_SCHEMA_ID, startSchemaId);
	xml.writeStringElement(EquipmentPropNames::SCHEMA_TAGS, schemaTags);

	//

	xml.writeStartElement(XmlElement::APP_DATA_SERVICE1);

	xml.writeStringAttribute(EquipmentPropNames::EQUIPMENT_ID, appDataServiceID1);
	xml.writeStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, appDataServiceIP1);
	xml.writeIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, appDataServicePort1);
	xml.writeStringAttribute(EquipmentPropNames::RT_TRENDS_REQUEST_IP, realtimeDataIP1);
	xml.writeIntAttribute(EquipmentPropNames::RT_TRENDS_REQUEST_PORT, realtimeDataPort1);

	xml.writeEndElement();			// </AppDataService1>

	//

	xml.writeStartElement(XmlElement::APP_DATA_SERVICE2);

	xml.writeStringAttribute(EquipmentPropNames::EQUIPMENT_ID, appDataServiceID2);
	xml.writeStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, appDataServiceIP2);
	xml.writeIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, appDataServicePort2);
	xml.writeStringAttribute(EquipmentPropNames::RT_TRENDS_REQUEST_IP, realtimeDataIP2);
	xml.writeIntAttribute(EquipmentPropNames::RT_TRENDS_REQUEST_PORT, realtimeDataPort2);

	xml.writeEndElement();			// </AppDataService2>

	//

	xml.writeStartElement(XmlElement::ARCHIVE_SERVICE1);

	xml.writeStringAttribute(EquipmentPropNames::EQUIPMENT_ID, archiveServiceID1);
	xml.writeStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, archiveServiceIP1);
	xml.writeIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, archiveServicePort1);

	xml.writeEndElement();			// </ArchiveService1>

	//

	xml.writeStartElement(XmlElement::ARCHIVE_SERVICE2);

	xml.writeStringAttribute(EquipmentPropNames::EQUIPMENT_ID, archiveServiceID2);
	xml.writeStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, archiveServiceIP2);
	xml.writeIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, archiveServicePort2);

	xml.writeEndElement();			// </ArchiveService2>

	//

	xml.writeStartElement(XmlElement::TUNING_SERVICE);

	xml.writeBoolAttribute(EquipmentPropNames::TUNING_ENABLE, tuningEnabled);
	xml.writeStringAttribute(EquipmentPropNames::EQUIPMENT_ID, tuningServiceID);
	xml.writeStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, tuningServiceIP);
	xml.writeIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, tuningServicePort);

	xml.writeStringElement(EquipmentPropNames::TUNING_SOURCE_EQUIPMENT_ID, tuningSources);

	xml.writeEndElement();			// </TuningService>

	//

	writeEndSettings(xml);;			// </Settings>

	return true;
}

bool MonitorSettings::readFromXml(XmlReadHelper& xml)
{
	clear();

	bool result = true;

	result = readStartSettings(xml);

	RETURN_IF_FALSE(result);

	//

	result &= xml.findElement(EquipmentPropNames::START_SCHEMA_ID);
	result &= xml.readStringElement(EquipmentPropNames::START_SCHEMA_ID, &startSchemaId);

	result &= xml.findElement(EquipmentPropNames::SCHEMA_TAGS);
	result &= xml.readStringElement(EquipmentPropNames::SCHEMA_TAGS, &schemaTags);

	//

	result &= xml.findElement(XmlElement::APP_DATA_SERVICE1);
	result &= xml.readStringAttribute(EquipmentPropNames::EQUIPMENT_ID, &appDataServiceID1);
	result &= xml.readStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, &appDataServiceIP1);
	result &= xml.readIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, &appDataServicePort1);
	result &= xml.readStringAttribute(EquipmentPropNames::RT_TRENDS_REQUEST_IP, &realtimeDataIP1);
	result &= xml.readIntAttribute(EquipmentPropNames::RT_TRENDS_REQUEST_PORT, &realtimeDataPort1);

	//

	result &= xml.findElement(XmlElement::APP_DATA_SERVICE2);
	result &= xml.readStringAttribute(EquipmentPropNames::EQUIPMENT_ID, &appDataServiceID2);
	result &= xml.readStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, &appDataServiceIP2);
	result &= xml.readIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, &appDataServicePort2);
	result &= xml.readStringAttribute(EquipmentPropNames::RT_TRENDS_REQUEST_IP, &realtimeDataIP2);
	result &= xml.readIntAttribute(EquipmentPropNames::RT_TRENDS_REQUEST_PORT, &realtimeDataPort2);

	//

	result &= xml.findElement(XmlElement::ARCHIVE_SERVICE1);
	result &= xml.readStringAttribute(EquipmentPropNames::EQUIPMENT_ID, &archiveServiceID1);
	result &= xml.readStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, &archiveServiceIP1);
	result &= xml.readIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, &archiveServicePort1);

	//

	result &=  xml.findElement(XmlElement::ARCHIVE_SERVICE2);
	result &= xml.readStringAttribute(EquipmentPropNames::EQUIPMENT_ID, &archiveServiceID2);
	result &= xml.readStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, &archiveServiceIP2);
	result &= xml.readIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, &archiveServicePort2);

	//

	result &= xml.findElement(XmlElement::TUNING_SERVICE);
	result &= xml.readBoolAttribute(EquipmentPropNames::TUNING_ENABLE, &tuningEnabled);
	result &= xml.readStringAttribute(EquipmentPropNames::EQUIPMENT_ID, &tuningServiceID);
	result &= xml.readStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, &tuningServiceIP);
	result &= xml.readIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, &tuningServicePort);

	result &= xml.findElement(EquipmentPropNames::TUNING_SOURCE_EQUIPMENT_ID);
	result &= xml.readStringElement(EquipmentPropNames::TUNING_SOURCE_EQUIPMENT_ID, &tuningSources);

	return result;
}

QStringList MonitorSettings::getSchemaTags() const
{
	return  schemaTags.split(Separator::SEMICOLON, Qt::SkipEmptyParts);
}

QStringList MonitorSettings::getTuningSources() const
{
	return  tuningSources.split(Separator::SEMICOLON, Qt::SkipEmptyParts);
}

void MonitorSettings::clear()
{
	startSchemaId.clear();
	schemaTags.clear();

	appDataServiceID1.clear();
	appDataServiceIP1.clear();
	appDataServicePort1 = 0;
	realtimeDataIP1.clear();
	realtimeDataPort1 = 0;

	appDataServiceID2.clear();
	appDataServiceIP2.clear();
	appDataServicePort2 = 0;
	realtimeDataIP2.clear();
	realtimeDataPort2 = 0;

	archiveServiceID1.clear();
	archiveServiceIP1.clear();
	archiveServicePort1 = 0;

	archiveServiceID2.clear();
	archiveServiceIP2.clear();
	archiveServicePort2 = 0;

	tuningEnabled = false;
	tuningServiceID.clear();
	tuningServiceIP.clear();
	tuningServicePort = 0;
	tuningSources.clear();
}


#ifdef IS_BUILDER

	// -------------------------------------------------------------------------------------
	//
	// MonitorSettingsGetter class implementation
	//
	// -------------------------------------------------------------------------------------

	bool MonitorSettingsGetter::readFromDevice(const Builder::Context* context,
											   const Hardware::Software* software)
	{
		clear();

		TEST_PTR_RETURN_FALSE(context);

		Builder::IssueLogger* log = context->m_log;

		TEST_PTR_RETURN_FALSE(log);
		TEST_PTR_LOG_RETURN_FALSE(software, log);

		bool result = true;

		// StartSchemaID
		//
		result = DeviceHelper::getStrProperty(software, EquipmentPropNames::START_SCHEMA_ID, &startSchemaId, log);

		RETURN_IF_FALSE(result);

		startSchemaId = startSchemaId.trimmed();

		if (startSchemaId.isEmpty() == true)
		{
			QString errorStr = tr("Monitor configuration error %1, property startSchemaId is invalid").
									arg(software->equipmentIdTemplate());

			log->writeError(errorStr);
			return false;
		}

		// SchemaTags
		//
		result = DeviceHelper::getStrProperty(software, EquipmentPropNames::SCHEMA_TAGS, &schemaTags, log);

		RETURN_IF_FALSE(result);

		QStringList schemaTagList = schemaTags.split(QRegExp("\\W+"), Qt::SkipEmptyParts);

		for (QString& tag : schemaTagList)
		{
			tag = tag.toLower();
		}

		schemaTags = schemaTagList.join(Separator::SEMICOLON);

		result = readAppDataServiceAndArchiveSettings(context, software);

		RETURN_IF_FALSE(result);

		result = readTuningSettings(context, software);

		return result;
	}

	bool MonitorSettingsGetter::readAppDataServiceAndArchiveSettings(const Builder::Context* context,
																	 const Hardware::Software* software)
	{
		Builder::IssueLogger* log = context->m_log;
		const Hardware::EquipmentSet* equipment = context->m_equipmentSet.get();

		bool result = true;

		// AppDataService settings reading
		//
		result &= DeviceHelper::getStrProperty(software, EquipmentPropNames::APP_DATA_SERVICE_ID1, &appDataServiceID1, log);
		result &= DeviceHelper::getStrProperty(software, EquipmentPropNames::APP_DATA_SERVICE_ID2, &appDataServiceID2, log);

		appDataServiceID1 = appDataServiceID1.trimmed();
		appDataServiceID2 = appDataServiceID2.trimmed();

		if (appDataServiceID1.isEmpty() == true &&
			appDataServiceID2.isEmpty() == true)
		{
			// at least one of this properties shouldn't be empty
			//

			// Property %1.%2 is empty.
			//
			log->errCFG3022(software->equipmentIdTemplate(), EquipmentPropNames::APP_DATA_SERVICE_ID1);
			log->errCFG3022(software->equipmentIdTemplate(), EquipmentPropNames::APP_DATA_SERVICE_ID2);

			return false;
		}

		// AppDataServiceStrID1->ClientRequestIP, ClientRequestPort
		//
		const Hardware::Software* appDataService1 = nullptr;

		if (appDataServiceID1.isEmpty() == false)
		{
			appDataService1 = dynamic_cast<const Hardware::Software*>(equipment->deviceObject(appDataServiceID1));

			if (appDataService1 == nullptr)
			{
				log->errCFG3021(software->equipmentIdTemplate(), EquipmentPropNames::APP_DATA_SERVICE_ID1, appDataServiceID1);

				result = false;
			}
			else
			{
				if (appDataService1->type() != E::SoftwareType::AppDataService)
				{
					log->errCFG3017(software->equipmentIdTemplate(), EquipmentPropNames::APP_DATA_SERVICE_ID1, appDataServiceID1);

					result = false;
				}
			}
		}

		const Hardware::Software* appDataService2 = nullptr;

		if (appDataServiceID2.isEmpty() == false)
		{
			appDataService2 = dynamic_cast<const Hardware::Software*>(equipment->deviceObject(appDataServiceID2));

			if (appDataService2 == nullptr)
			{
				log->errCFG3021(software->equipmentIdTemplate(), EquipmentPropNames::APP_DATA_SERVICE_ID2, appDataServiceID2);

				result = false;
			}
			else
			{
				if (appDataService2->type() != E::SoftwareType::AppDataService)
				{
					log->errCFG3017(software->equipmentIdTemplate(), EquipmentPropNames::APP_DATA_SERVICE_ID2, appDataServiceID2);

					result = false;
				}
			}
		}

		RETURN_IF_FALSE(result);

		// Reading AppDataService Settings
		//
		if (appDataService1 != nullptr)
		{
			AppDataServiceSettingsGetter adsSettings1;

			result &= adsSettings1.readFromDevice(context, appDataService1);

			RETURN_IF_FALSE(result);

			appDataServiceIP1 = adsSettings1.clientRequestIP.addressStr();
			appDataServicePort1 = adsSettings1.clientRequestIP.port();
			realtimeDataIP1 = adsSettings1.rtTrendsRequestIP.addressStr();
			realtimeDataPort1 = adsSettings1.rtTrendsRequestIP.port();

			//

			HostAddressPort archClientRequestIP1;

			result &= getSoftwareConnection(equipment,
											appDataService1,
											EquipmentPropNames::ARCH_SERVICE_ID,
											EquipmentPropNames::CLIENT_REQUEST_IP,
											EquipmentPropNames::CLIENT_REQUEST_PORT,
											&archiveServiceID1,
											&archClientRequestIP1,
											true,
											Socket::IP_NULL,
											PORT_ARCHIVING_SERVICE_CLIENT_REQUEST,
											E::SoftwareType::ArchiveService,
											log);
			RETURN_IF_FALSE(result);

			archiveServiceIP1 = archClientRequestIP1.addressStr();
			archiveServicePort1 = archClientRequestIP1.port();
		}

		if (appDataService2 != nullptr)
		{
			AppDataServiceSettingsGetter adsSettings2;

			result &= adsSettings2.readFromDevice(context, appDataService2);

			RETURN_IF_FALSE(result);

			appDataServiceIP2 = adsSettings2.clientRequestIP.addressStr();
			appDataServicePort2 = adsSettings2.clientRequestIP.port();
			realtimeDataIP2 = adsSettings2.rtTrendsRequestIP.addressStr();
			realtimeDataPort2 = adsSettings2.rtTrendsRequestIP.port();

			//

			HostAddressPort archClientRequestIP2;

			result &= getSoftwareConnection(equipment,
											appDataService2,
											EquipmentPropNames::ARCH_SERVICE_ID,
											EquipmentPropNames::CLIENT_REQUEST_IP,
											EquipmentPropNames::CLIENT_REQUEST_PORT,
											&archiveServiceID2,
											&archClientRequestIP2,
											true,
											Socket::IP_NULL,
											PORT_ARCHIVING_SERVICE_CLIENT_REQUEST,
											E::SoftwareType::ArchiveService,
											log);
			RETURN_IF_FALSE(result);

			archiveServiceIP2 = archClientRequestIP2.addressStr();
			archiveServicePort2 = archClientRequestIP2.port();
		}

		return result;
	}

	bool MonitorSettingsGetter::readTuningSettings(const Builder::Context* context,
												   const Hardware::Software* software)
	{
		Builder::IssueLogger* log = context->m_log;
		const Hardware::EquipmentSet* equipment = context->m_equipmentSet.get();

		bool result = true;

		result = DeviceHelper::getBoolProperty(software, EquipmentPropNames::TUNING_ENABLE, &tuningEnabled, log);

		RETURN_IF_FALSE(result);

		if (tuningEnabled == false)
		{
			return true;
		}

		HostAddressPort tuningClientRequestIP;

		result = getSoftwareConnection(equipment,
										software,
										EquipmentPropNames::TUNING_SERVICE_ID,
										EquipmentPropNames::CLIENT_REQUEST_IP,
										EquipmentPropNames::CLIENT_REQUEST_PORT,
										&tuningServiceID,
										&tuningClientRequestIP,
										false,
										Socket::IP_NULL,
										PORT_ARCHIVING_SERVICE_CLIENT_REQUEST,
										E::SoftwareType::TuningService,
										log);
		RETURN_IF_FALSE(result);

		const Hardware::Software* tuningServiceObject = dynamic_cast<const Hardware::Software*>(equipment->deviceObject(tuningServiceID));

		if (tuningServiceObject == nullptr)			// WTF?
		{
			LOG_INTERNAL_ERROR(log);
			return false;
		}

		bool singleLmControl = false;

		result = DeviceHelper::getBoolProperty(tuningServiceObject, EquipmentPropNames::SINGLE_LM_CONTROL, &singleLmControl, log);

		RETURN_IF_FALSE(result);

		if (singleLmControl == true)
		{
			// Mode SingleLmControl is not supported by Monitor. Set TuningServiceID.SingleLmControl to false. Monitor EquipmentID %1, TuningServiceID %2.
			//
			log->errCFG3040(software->equipmentIdTemplate(), tuningServiceID);
			return false;
		}

		tuningServiceIP = tuningClientRequestIP.addressStr();
		tuningServicePort = tuningClientRequestIP.port();

		//

		result = DeviceHelper::getStrProperty(software, EquipmentPropNames::TUNING_SOURCE_EQUIPMENT_ID, &tuningSources, log);

		RETURN_IF_FALSE(result);

		tuningSources = tuningSources.trimmed();
		tuningSources = tuningSources.replace(QChar(QChar::LineFeed), QChar(';'));
		tuningSources = tuningSources.replace(QChar(QChar::CarriageReturn), QChar(';'));
		tuningSources = tuningSources.replace(QChar(QChar::Tabulation), QChar(';'));

		QStringList tuningSourcesList = tuningSources.split(QChar(';'), Qt::SkipEmptyParts);

		if (tuningSourcesList.isEmpty() == true)
		{
			log->errCFG3022(software->equipmentIdTemplate(), EquipmentPropNames::TUNING_SOURCE_EQUIPMENT_ID);
			return false;
		}

		// Check for valid EquipmentIds
		//
		for (const QString& tuningEquipmentID : tuningSourcesList)
		{
			if (equipment->deviceObject(tuningEquipmentID) == nullptr)
			{
				log->errEQP6109(tuningEquipmentID, software->equipmentIdTemplate());
				return false;
			}
		}

		tuningSources = tuningSourcesList.join(Separator::SEMICOLON);

		return true;
	}

#endif

// -------------------------------------------------------------------------------------
//
// TuningClientSettings class implementation
//
// -------------------------------------------------------------------------------------

bool TuningClientSettings::writeToXml(XmlWriteHelper& xml)
{
	writeStartSettings(xml);

	xml.writeStartElement(XmlElement::TUNING_SERVICE);

	xml.writeStringAttribute(EquipmentPropNames::EQUIPMENT_ID, tuningServiceID);
	xml.writeStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, tuningServiceIP);
	xml.writeIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, tuningServicePort);

	xml.writeEndElement();		// </TuningService>

	xml.writeStartElement(XmlElement::APPEARANCE);

	xml.writeBoolAttribute(EquipmentPropNames::AUTO_APPLAY, autoApply);
	xml.writeBoolAttribute(EquipmentPropNames::SHOW_SIGNALS, showSignals);
	xml.writeBoolAttribute(EquipmentPropNames::SHOW_SCHEMAS, showSchemas);
	xml.writeBoolAttribute(EquipmentPropNames::SHOW_SCHEMAS_LIST, showSchemasList);
	xml.writeBoolAttribute(EquipmentPropNames::SHOW_SCHEMAS_TABS, showSchemasTabs);
	xml.writeIntAttribute(EquipmentPropNames::STATUS_FLAG_FUNCTION, statusFlagFunction);
	xml.writeBoolAttribute(EquipmentPropNames::LOGIN_PER_OPERATION, loginPerOperation);
	xml.writeStringAttribute(EquipmentPropNames::USER_ACCOUNTS, usersAccounts);
	xml.writeIntAttribute(EquipmentPropNames::LOGIN_SESSION_LENGTH, loginSessionLength);

	xml.writeBoolAttribute(EquipmentPropNames::FILTER_BY_EQUIPMENT, filterByEquipment);
	xml.writeBoolAttribute(EquipmentPropNames::FILTER_BY_SCHEMA, filterBySchema);

	xml.writeStringAttribute(EquipmentPropNames::START_SCHEMA_ID, startSchemaID);

	xml.writeEndElement();		// </Appearance>

	xml.writeStringElement(EquipmentPropNames::SCHEMA_TAGS, schemaTags);

	writeEndSettings(xml);		// </Settings>

	return true;
}

bool TuningClientSettings::readFromXml(XmlReadHelper& xml)
{
	bool result = true;

	result = readStartSettings(xml);

	RETURN_IF_FALSE(result);

	result &= xml.findElement(XmlElement::TUNING_SERVICE);

	result &= xml.readStringAttribute(EquipmentPropNames::EQUIPMENT_ID, &tuningServiceID);
	result &= xml.readStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, &tuningServiceIP);
	result &= xml.readIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, &tuningServicePort);

	result &= xml.findElement(XmlElement::APPEARANCE);

	result &= xml.readBoolAttribute(EquipmentPropNames::AUTO_APPLAY, &autoApply);
	result &= xml.readBoolAttribute(EquipmentPropNames::SHOW_SIGNALS, &showSignals);
	result &= xml.readBoolAttribute(EquipmentPropNames::SHOW_SCHEMAS, &showSchemas);
	result &= xml.readBoolAttribute(EquipmentPropNames::SHOW_SCHEMAS_LIST, &showSchemasList);
	result &= xml.readBoolAttribute(EquipmentPropNames::SHOW_SCHEMAS_TABS, &showSchemasTabs);

	bool resultStatusFlagFunction = xml.readIntAttribute(EquipmentPropNames::STATUS_FLAG_FUNCTION, &statusFlagFunction);
	if (resultStatusFlagFunction == false)
	{
		// Compatibility loading statusFlagFunction before 10.12.2020
		//
		statusFlagFunction = 0;

		bool showSOR = false;
		bool useAccessFlag = false;

		resultStatusFlagFunction = xml.readBoolAttribute(EquipmentPropNames::SHOW_SOR, &showSOR);
		resultStatusFlagFunction &= xml.readBoolAttribute(EquipmentPropNames::USE_ACCESS_FLAG, &useAccessFlag);

		if (resultStatusFlagFunction == true)
		{
			if (showSOR == true)
			{
				statusFlagFunction = 1;
			}
			else
			{
				if (useAccessFlag == true)
				{
					statusFlagFunction = 2;
				}
			}
		}
	}

	result &= resultStatusFlagFunction;

	result &= xml.readBoolAttribute(EquipmentPropNames::LOGIN_PER_OPERATION, &loginPerOperation);
	result &= xml.readStringAttribute(EquipmentPropNames::USER_ACCOUNTS, &usersAccounts);
	result &= xml.readIntAttribute(EquipmentPropNames::LOGIN_SESSION_LENGTH, &loginSessionLength);

	result &= xml.readBoolAttribute(EquipmentPropNames::FILTER_BY_EQUIPMENT, &filterByEquipment);
	result &= xml.readBoolAttribute(EquipmentPropNames::FILTER_BY_SCHEMA, &filterBySchema);

	result &= xml.readStringAttribute(EquipmentPropNames::START_SCHEMA_ID, &startSchemaID);

	result &= xml.findElement(EquipmentPropNames::SCHEMA_TAGS);

	result &= xml.readStringElement(EquipmentPropNames::SCHEMA_TAGS, &schemaTags);

	return result;
}

QStringList TuningClientSettings::getSchemaTags() const
{
	return  schemaTags.split(Separator::SEMICOLON, Qt::SkipEmptyParts);
}

QStringList TuningClientSettings::getUsersAccounts() const
{
	return  usersAccounts.split(Separator::SEMICOLON, Qt::SkipEmptyParts);
}

const TuningClientSettings& TuningClientSettings::operator = (const TuningClientSettings& src)
{
	tuningServiceID = src.tuningServiceID;
	tuningServiceIP = src.tuningServiceIP;
	tuningServicePort = src.tuningServicePort;

	autoApply = src.autoApply;

	showSignals = src.showSignals;
	showSchemas = src.showSchemas;
	showSchemasList = src.showSchemasList;
	showSchemasTabs = src.showSchemasTabs;

	statusFlagFunction = src.statusFlagFunction;

	loginPerOperation = src.loginPerOperation;
	usersAccounts = src.usersAccounts;
	loginSessionLength = src.loginSessionLength;

	filterByEquipment = src.filterByEquipment;
	filterBySchema = src.filterBySchema;

	startSchemaID = src.startSchemaID;
	schemaTags = src.schemaTags;

	return *this;
}

bool TuningClientSettings::appearanceChanged(const TuningClientSettings& src) const
{
	if (autoApply != src.autoApply ||
		filterByEquipment != src.filterByEquipment ||
		filterBySchema != src.filterBySchema ||
		showSchemasList != src.showSchemasList ||
		showSchemasTabs != src.showSchemasTabs ||
		showSchemas != src.showSchemas ||
		showSignals != src.showSignals ||
		statusFlagFunction != src.statusFlagFunction ||
		loginPerOperation != src.loginPerOperation ||
		loginSessionLength != src.loginSessionLength ||
		usersAccounts != src.usersAccounts)
	{
		return true;
	}

	return false;
}

bool TuningClientSettings::connectionChanged(const TuningClientSettings& src) const
{
	if (tuningServiceID != src.tuningServiceID ||
		tuningServiceIP != src.tuningServiceIP ||
		tuningServicePort != src.tuningServicePort ||
		autoApply != src.autoApply ||
		statusFlagFunction != src.statusFlagFunction)
	{
		return true;
	}

	return false;
}

#ifdef IS_BUILDER

	// -------------------------------------------------------------------------------------
	//
	// TuningClientSettingsGetter class implementation
	//
	// -------------------------------------------------------------------------------------


	bool TuningClientSettingsGetter::readFromDevice(const Builder::Context* context,
													const Hardware::Software* software)
	{
		TEST_PTR_RETURN_FALSE(context);

		Builder::IssueLogger* log = context->m_log;

		TEST_PTR_RETURN_FALSE(log);
		TEST_PTR_LOG_RETURN_FALSE(software, log);

		const Hardware::EquipmentSet* equipment = context->m_equipmentSet.get();

		bool result = true;

		// ConfigurationService connections checking
		//
		QString cfgServiceID1;
		QString cfgServiceID2;

		HostAddressPort cfgServiceIP1;
		HostAddressPort cfgServiceIP2;

		result = getCfgServiceConnection(	equipment,
											software,
											&cfgServiceID1, &cfgServiceIP1,
											&cfgServiceID2, &cfgServiceIP2,
											log);

		RETURN_IF_FALSE(result);

		//

		HostAddressPort tuningServiceClientIP;

		result &= getSoftwareConnection(equipment,
									   software,
									   EquipmentPropNames::TUNING_SERVICE_ID,
									   EquipmentPropNames::CLIENT_REQUEST_IP,
									   EquipmentPropNames::CLIENT_REQUEST_PORT,
									   &tuningServiceID,
									   &tuningServiceClientIP,
									   false,
									   Socket::IP_NULL,
									   PORT_TUNING_SERVICE_CLIENT_REQUEST,
									   E::SoftwareType::TuningService,
									   log);

		RETURN_IF_FALSE(result);

		tuningServiceIP = tuningServiceClientIP.addressStr();
		tuningServicePort = tuningServiceClientIP.port();

		result &= DeviceHelper::getBoolProperty(software, EquipmentPropNames::AUTO_APPLAY, &autoApply, log);
		result &= DeviceHelper::getBoolProperty(software, EquipmentPropNames::SHOW_SIGNALS, &showSignals, log);
		result &= DeviceHelper::getBoolProperty(software, EquipmentPropNames::SHOW_SCHEMAS, &showSchemas, log);

		RETURN_IF_FALSE(result);

		//
		// schemasNavigation
		//
		showSchemasList = false;
		showSchemasTabs = false;

		int schemasNavigation = 0;

		result &= DeviceHelper::getIntProperty(software, EquipmentPropNames::SCHEMAS_NAVIGATION, &schemasNavigation, log);

		RETURN_IF_FALSE(result);

		switch (schemasNavigation)
		{
		case 0:
			break;
		case 1:
			showSchemasList = true;
			break;
		case 2:
			showSchemasTabs = true;
			break;
		default:
			Q_ASSERT(false);
		}

		//
		// statusFlagFunction
		//
		result &= DeviceHelper::getIntProperty(software, EquipmentPropNames::STATUS_FLAG_FUNCTION, &statusFlagFunction, log);

		result &= DeviceHelper::getBoolProperty(software, EquipmentPropNames::LOGIN_PER_OPERATION, &loginPerOperation, log);

		result &= DeviceHelper::getStrProperty(software, EquipmentPropNames::USER_ACCOUNTS, &usersAccounts, log);

		usersAccounts.replace(' ', ';');
		usersAccounts.replace('\n', ';');
		usersAccounts.remove('\r');
		QStringList userList = usersAccounts.split(';', Qt::SkipEmptyParts);

		usersAccounts = userList.join(Separator::SEMICOLON);

		result &= DeviceHelper::getIntProperty(software, EquipmentPropNames::LOGIN_SESSION_LENGTH, &loginSessionLength, log);

		result &= DeviceHelper::getBoolProperty(software, EquipmentPropNames::FILTER_BY_EQUIPMENT, &filterByEquipment, log);
		result &= DeviceHelper::getBoolProperty(software, EquipmentPropNames::FILTER_BY_SCHEMA, &filterBySchema, log);

		result &= DeviceHelper::getStrProperty(software, EquipmentPropNames::START_SCHEMA_ID, &startSchemaID, log);

		result &= DeviceHelper::getStrProperty(software, EquipmentPropNames::SCHEMA_TAGS, &schemaTags, log);

		QStringList schemaTagList = schemaTags.split(QRegExp("\\W+"), Qt::SkipEmptyParts);

		schemaTags = schemaTagList.join(Separator::SEMICOLON);

		return result;
	}

#endif
