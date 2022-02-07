#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QObject>

#include "SoftwareSettings.h"
#include "../UtilsLib/WUtils.h"

// -------------------------------------------------------------------------------------
//
// SessionParams struct implementation
//
// -------------------------------------------------------------------------------------

void SessionParams::saveTo(Network::SessionParams* sp)
{
	TEST_PTR_RETURN(sp);

	sp->set_currentsettingsprofile(currentSettingsProfile.toStdString());
	sp->set_softwarerunmode(static_cast<int>(softwareRunMode));
}

void SessionParams::loadFrom(const Network::SessionParams& sp)
{
	currentSettingsProfile = QString::fromStdString(sp.currentsettingsprofile());
	softwareRunMode = static_cast<E::SoftwareRunMode>(sp.softwarerunmode());
}

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

bool SoftwareSettings::startSettingsReading(XmlReadHelper& xml)
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

SoftwareSettingsSet::SoftwareSettingsSet() :
	m_softwareType(E::SoftwareType::Unknown)
{
}

bool SoftwareSettingsSet::settingsProfileIsExists(const QString& profile)
{
	auto it = m_settingsMap.find(profile.isEmpty() == true ? SettingsProfile::DEFAULT : profile);

	return it != m_settingsMap.end();
}

bool SoftwareSettingsSet::writeToXml(XmlWriteHelper& xml)
{
	bool result = true;

	xml.writeStartElement(XmlElement::SETTINGS_SET);
	xml.writeEnumKeyAttribute(EquipmentPropNames::SOFTWARE_TYPE, m_softwareType);
	xml.writeIntAttribute(XmlAttribute::COUNT, static_cast<int>(m_settingsMap.size()));

	for(auto& p : m_settingsMap)
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

bool SoftwareSettingsSet::readFromXml(const QByteArray& xmlData)
{
	XmlReadHelper xml(xmlData);

	return readFromXml(xml);
}

bool SoftwareSettingsSet::readFromXml(XmlReadHelper& xml)
{
	m_settingsMap.clear();

	bool result = true;
	int profilesCount = 0;

	result = xml.findElement(XmlElement::SETTINGS_SET);

	E::SoftwareType swType;

	result &=xml.readEnumKeyAttribute(EquipmentPropNames::SOFTWARE_TYPE, &swType);

	if (result == false)
	{
		Q_ASSERT(false);
		return false;
	}

	if (m_softwareType == E::SoftwareType::Unknown)
	{
		m_softwareType = swType;
	}
	else
	{
		if (m_softwareType != swType)
		{
			Q_ASSERT(false);
			return false;
		}
	}

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
			result &= addSharedProfile(settings->profile, settings);
		}
		else
		{
			result = false;
		}
	}

	return result;
}

QStringList SoftwareSettingsSet::getSettingsProfiles() const
{
	QStringList profiles;

	for(auto& pp : m_settingsMap)
	{
		profiles.append(pp.first);
	}

	return profiles;
}

QString SoftwareSettingsSet::writeSettingsToXmlString(E::SoftwareType swType, const SoftwareSettings& settings)
{
	QString xmlString;

	XmlWriteHelper xml(&xmlString);

	xml.setAutoFormatting(true);

	xml.writeStartDocument();

	xml.writeStartElement(XmlElement::SETTINGS_SET);

	xml.writeEnumKeyAttribute(EquipmentPropNames::SOFTWARE_TYPE, swType);
	xml.writeIntAttribute(XmlAttribute::COUNT, 1);

	settings.writeToXml(xml);

	xml.writeEndElement();		// </SettingsSet>

	return xmlString;
}

bool SoftwareSettingsSet::readSettingsFromXmlString(const QString& xmlString, SoftwareSettings* settings)
{
	TEST_PTR_RETURN_FALSE(settings);

	XmlReadHelper xml(xmlString);

	bool result = true;

	result &= xml.findElement(XmlElement::SETTINGS_SET);

	result &= settings->readFromXml(xml);

	return result;
}

std::shared_ptr<SoftwareSettings> SoftwareSettingsSet::createAppropriateSettings(E::SoftwareType softwareType)
{
	switch(softwareType)
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

std::shared_ptr<SoftwareSettings> SoftwareSettingsSet::createAppropriateSettings()
{
	return createAppropriateSettings(m_softwareType);
}

bool SoftwareSettingsSet::addSharedProfile(const QString& profile, std::shared_ptr<SoftwareSettings> sharedSettings)
{
	if (m_settingsMap.find(profile) != m_settingsMap.end())
	{
		Q_ASSERT(false);
		return false;
	}

	m_settingsMap.insert({profile, sharedSettings});

	return true;
}

// -------------------------------------------------------------------------------------
//
// CfgServiceSettings class implementation
//
// -------------------------------------------------------------------------------------

bool CfgServiceSettings::writeToXml(XmlWriteHelper& xml) const
{
	writeStartSettings(xml);

	xml.writeHostAddressPort(EquipmentPropNames::CLIENT_REQUEST_IP,
							 EquipmentPropNames::CLIENT_REQUEST_PORT, clientRequestIP);

	xml.writeHostAddress(EquipmentPropNames::CLIENT_REQUEST_NETMASK, clientRequestNetmask);

	xml.writeStartElement(XmlElement::CLIENTS);
	xml.writeIntAttribute(XmlAttribute::COUNT, static_cast<int>(clients.count()));

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

	result = startSettingsReading(xml);

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

QStringList CfgServiceSettings::knownClients() const
{
	QStringList knownClients;

	for(const QPair<QString, E::SoftwareType>& client : clients)
	{
		knownClients.append(client.first.trimmed());
	}

	return knownClients;
}

// -------------------------------------------------------------------------------------
//
// AppDataServiceSettings class implementation
//
// -------------------------------------------------------------------------------------

bool AppDataServiceSettings::writeToXml(XmlWriteHelper& xml) const
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

	result = startSettingsReading(xml);

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

// -------------------------------------------------------------------------------------
//
// DiagDataServiceSettings class implementation
//
// -------------------------------------------------------------------------------------

bool DiagDataServiceSettings::writeToXml(XmlWriteHelper& xml) const
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

	result = startSettingsReading(xml);

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

// -------------------------------------------------------------------------------------
//
// TuningServiceSettings class implementation
//
// -------------------------------------------------------------------------------------

QStringList TuningServiceSettings::TuningClient::uniqueSourcesIDs() const
{
	QStringList ids;

	std::set<QString> existIDs;

	for(const TuningSource& ts : drivenSources)
	{
		if (existIDs.contains(ts.lmEquipmentID) == false)
		{
			ids.append(ts.lmEquipmentID);
			existIDs.insert(ts.lmEquipmentID);
		}
	}

	return ids;
}


TuningServiceSettings::TuningSource TuningServiceSettings::ChannelSettings::getTuningSource(const QString& sourceEquipmentID) const
{
	for(const TuningSource& ts : sources)
	{
		if (ts.lmEquipmentID == sourceEquipmentID)
		{
			return ts;
		}
	}

	return TuningSource();
}

bool TuningServiceSettings::isSourceExists(const QString& moduleEquipmentID) const
{
	for(int i = CHANNEL_1; i < CHANNELS_COUNT; i++)
	{
		ChannelSettings ch = channelSettings[i];

		if (ch.enable == true)
		{
			for(auto& src : ch.sources)
			{
				if (src.lmEquipmentID == moduleEquipmentID)
				{
					return true;
				}
			}
		}
		else
		{
			Q_ASSERT(ch.sources.size() == 0);
		}
	}

	return false;
}

TuningServiceSettings::TuningClient TuningServiceSettings::getTuningClient(const QString& clientEquipmentID) const
{
	for(const TuningClient& tc : clients)
	{
		if (tc.equipmentID == clientEquipmentID)
		{
			return tc;
		}
	}

	return TuningClient();
}

bool TuningServiceSettings::writeToXml(XmlWriteHelper& xml) const
{
	writeStartSettings(xml);

	xml.writeStringElement(EquipmentPropNames::EQUIPMENT_ID, equipmentID);
	xml.writeIntElement(XmlElement::CHANNEL_COUNT, channelCount);

	xml.writeHostAddressPort(EquipmentPropNames::CLIENT_REQUEST_IP,
							 EquipmentPropNames::CLIENT_REQUEST_PORT,
							 clientRequestIP);

	xml.writeHostAddress(EquipmentPropNames::CLIENT_REQUEST_NETMASK,
						 clientRequestNetmask);

	xml.writeStringElement(EquipmentPropNames::CFG_SERVICE_ID1, cfgServiceID1);
	xml.writeHostAddressPort(EquipmentPropNames::CFG_SERVICE_IP1,
							 EquipmentPropNames::CFG_SERVICE_PORT1, cfgServiceIP1);

	xml.writeStringElement(EquipmentPropNames::CFG_SERVICE_ID2, cfgServiceID2);
	xml.writeHostAddressPort(EquipmentPropNames::CFG_SERVICE_IP2,
							 EquipmentPropNames::CFG_SERVICE_PORT2, cfgServiceIP2);

	xml.writeBoolElement(EquipmentPropNames::SINGLE_LM_CONTROL, singleLmControl);
	xml.writeBoolElement(EquipmentPropNames::DISABLE_MODULES_TYPE_CHECKING, disableModulesTypeChecking);

	// write tuning clients info
	//
	xml.writeStartElement(XmlElement::TUNING_CLIENTS);
	xml.writeIntAttribute(XmlAttribute::COUNT, static_cast<int>(clients.size()));

	for(const TuningClient& tc : clients)
	{
		xml.writeStartElement(XmlElement::TUNING_CLIENT);
		xml.writeStringAttribute(EquipmentPropNames::EQUIPMENT_ID, tc.equipmentID);

		writeTuningSourcesToXml(xml, tc.drivenSources);

		xml.writeEndElement();		// TUNING_CLIENT
	}

	xml.writeEndElement();			// TUNING_CLIENTS

	for(int channel = CHANNEL_1; channel < channelCount; channel++)
	{
		const ChannelSettings& ch = channelSettings[channel];

		xml.writeStartElement(XmlElement::TUNING_CHANNEL_TEMPLATE.arg(channel + 1));		// <Channel ....>

		xml.writeBoolAttribute(EquipmentPropNames::ENABLE, ch.enable);

		if (ch.enable == true)
		{
			xml.writeStringAttribute(XmlAttribute::CONTROLLER_EQUIPMENT_ID, ch.serviceControllerEquipmentID);
			xml.writeHostAddressPortAttribute(EquipmentPropNames::TUNING_DATA_IP, ch.tuningDataIP);
			xml.writeQHostAddressAttribute(EquipmentPropNames::TUNING_DATA_NETMASK, ch.tuningDataNetmask);
			xml.writeHostAddressPortAttribute(EquipmentPropNames::TUNING_SIM_IP, ch.tuningSimIP);

			// write tuning sources info
			//
			writeTuningSourcesToXml(xml, ch.sources);
		}

		xml.writeEndElement();			// </Channel>
	}

	writeEndSettings(xml);			// </Settings>

	return true;
}

bool TuningServiceSettings::readFromXml(XmlReadHelper& xml)
{
	bool result = false;

	result = startSettingsReading(xml);

	RETURN_IF_FALSE(result);

	result &= xml.readStringElement(EquipmentPropNames::EQUIPMENT_ID, &equipmentID, true);

	result &= xml.readIntElement(XmlElement::CHANNEL_COUNT, &channelCount, true);

	result &= xml.readHostAddressPort(EquipmentPropNames::CLIENT_REQUEST_IP,
									  EquipmentPropNames::CLIENT_REQUEST_PORT,
									  &clientRequestIP);

	result &= xml.readHostAddress(EquipmentPropNames::CLIENT_REQUEST_NETMASK,
								  &clientRequestNetmask);

	result &= xml.readStringElement(EquipmentPropNames::CFG_SERVICE_ID1, &cfgServiceID1, true);
	result &= xml.readHostAddressPort(EquipmentPropNames::CFG_SERVICE_IP1,
									  EquipmentPropNames::CFG_SERVICE_PORT1, &cfgServiceIP1);

	result &= xml.readStringElement(EquipmentPropNames::CFG_SERVICE_ID2, &cfgServiceID2, true);
	result &= xml.readHostAddressPort(EquipmentPropNames::CFG_SERVICE_IP2,
									  EquipmentPropNames::CFG_SERVICE_PORT2, &cfgServiceIP2);

	result &= xml.readBoolElement(EquipmentPropNames::SINGLE_LM_CONTROL, &singleLmControl, true);
	result &= xml.readBoolElement(EquipmentPropNames::DISABLE_MODULES_TYPE_CHECKING, &disableModulesTypeChecking, true);

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

		result &= readTuningSourcesFromXml(xml, &tc.drivenSources);

		clients.push_back(tc);
	}

	for(int channel = CHANNEL_1; channel < channelCount; channel++)
	{
		result &= xml.findElement(XmlElement::TUNING_CHANNEL_TEMPLATE.arg(channel + 1));

		CONTINUE_IF_FALSE(result);

		ChannelSettings& ch = channelSettings[channel];

		result &= xml.readBoolAttribute(EquipmentPropNames::ENABLE, &ch.enable);

		if (ch.enable == true)
		{
			result &= xml.readStringAttribute(XmlAttribute::CONTROLLER_EQUIPMENT_ID, &ch.serviceControllerEquipmentID);
			result &= xml.readHostAddressPortAttribute(EquipmentPropNames::TUNING_DATA_IP, &ch.tuningDataIP);
			result &= xml.readQHostAddressAttribute(EquipmentPropNames::TUNING_DATA_NETMASK, &ch.tuningDataNetmask);
			result &= xml.readHostAddressPortAttribute(EquipmentPropNames::TUNING_SIM_IP, &ch.tuningSimIP);

			result &= readTuningSourcesFromXml(xml, &ch.sources);

			RETURN_IF_FALSE(result);
		}
		else
		{
			ch.serviceControllerEquipmentID.clear();
			ch.tuningDataIP = HostAddressPort();
			ch.tuningDataNetmask = QHostAddress();
			ch.tuningSimIP = HostAddressPort();

			ch.sources.clear();
		}
	}

	return result;
}

bool TuningServiceSettings::writeTuningSourcesToXml(XmlWriteHelper& xml,
												  const std::vector<TuningSource>& sources)
{
	xml.writeStartElement(XmlElement::TUNING_SOURCES);
	xml.writeIntAttribute(XmlAttribute::COUNT, static_cast<int>(sources.size()));

	for(uint i = 0; i < sources.size(); i++)
	{
		const TuningSource& ts = sources[i];

		xml.writeStartElement(XmlElement::TUNING_SOURCE);

		xml.writeStringAttribute(EquipmentPropNames::LM_EQUIPMENT_ID, ts.lmEquipmentID);
		xml.writeStringAttribute(EquipmentPropNames::PORT_EQUIPMENT_ID, ts.portEquipmentID);
		xml.writeStringAttribute(EquipmentPropNames::TUNING_DATA_IP, ts.tuningDataIP.addressPortStr());

		xml.writeEndElement();		// TUNING_SOURCE
	}

	xml.writeEndElement();			// TUNING_SOURCES

	return true;
}

bool TuningServiceSettings::readTuningSourcesFromXml(XmlReadHelper& xml,
													 std::vector<TuningSource>* sources)
{
	TEST_PTR_RETURN_FALSE(sources);

	sources->clear();

	bool result = true;

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

		sources->push_back(ts);
	}

	return result;
}

// -------------------------------------------------------------------------------------
//
// ArchivingServiceSettings class implementation
//
// -------------------------------------------------------------------------------------

bool ArchivingServiceSettings::writeToXml(XmlWriteHelper& xml) const
{
	writeStartSettings(xml);

	xml.writeStringElement(EquipmentPropNames::CFG_SERVICE_ID1, cfgServiceID1);
	xml.writeHostAddressPort(EquipmentPropNames::CFG_SERVICE_IP1,
							 EquipmentPropNames::CFG_SERVICE_PORT1, cfgServiceIP1);

	xml.writeStringElement(EquipmentPropNames::CFG_SERVICE_ID2, cfgServiceID2);
	xml.writeHostAddressPort(EquipmentPropNames::CFG_SERVICE_IP2,
							 EquipmentPropNames::CFG_SERVICE_PORT2, cfgServiceIP2);

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

	result = startSettingsReading(xml);

	RETURN_IF_FALSE(result);

	result &= xml.readStringElement(EquipmentPropNames::CFG_SERVICE_ID1, &cfgServiceID1, true);
	result &= xml.readHostAddressPort(EquipmentPropNames::CFG_SERVICE_IP1,
									  EquipmentPropNames::CFG_SERVICE_PORT1, &cfgServiceIP1);

	result &= xml.readStringElement(EquipmentPropNames::CFG_SERVICE_ID2, &cfgServiceID2, true);
	result &= xml.readHostAddressPort(EquipmentPropNames::CFG_SERVICE_IP2,
									  EquipmentPropNames::CFG_SERVICE_PORT2, &cfgServiceIP2);

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

// -------------------------------------------------------------------------------------
//
// TestClientSettings class implementation
//
// -------------------------------------------------------------------------------------

bool TestClientSettings::writeToXml(XmlWriteHelper& xml) const
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

	result = startSettingsReading(xml);

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

// -------------------------------------------------------------------------------------
//
// MetrologySettings class implementation
//
// -------------------------------------------------------------------------------------

bool MetrologySettings::writeToXml(XmlWriteHelper& xml) const
{
	writeStartSettings(xml);

	xml.writeStringElement(EquipmentPropNames::CFG_SERVICE_ID1, cfgServiceID1);
	xml.writeHostAddressPort(EquipmentPropNames::CFG_SERVICE_IP1,
							 EquipmentPropNames::CFG_SERVICE_PORT1, cfgServiceIP1);

	xml.writeStringElement(EquipmentPropNames::CFG_SERVICE_ID2, cfgServiceID2);
	xml.writeHostAddressPort(EquipmentPropNames::CFG_SERVICE_IP2,
							 EquipmentPropNames::CFG_SERVICE_PORT2, cfgServiceIP2);

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

	result = startSettingsReading(xml);

	RETURN_IF_FALSE(result);

	result &= xml.readStringElement(EquipmentPropNames::CFG_SERVICE_ID1, &cfgServiceID1, true);
	result &= xml.readHostAddressPort(EquipmentPropNames::CFG_SERVICE_IP1,
									  EquipmentPropNames::CFG_SERVICE_PORT1, &cfgServiceIP1);

	result &= xml.readStringElement(EquipmentPropNames::CFG_SERVICE_ID2, &cfgServiceID2, true);
	result &= xml.readHostAddressPort(EquipmentPropNames::CFG_SERVICE_IP2,
									  EquipmentPropNames::CFG_SERVICE_PORT2, &cfgServiceIP2);

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

// -------------------------------------------------------------------------------------
//
// MonitorSettings class implementation
//
// -------------------------------------------------------------------------------------

bool MonitorSettings::writeToXml(XmlWriteHelper& xml) const
{
	writeStartSettings(xml);

	//

	xml.writeStringElement(EquipmentPropNames::CFG_SERVICE_ID1, cfgServiceID1);
	xml.writeHostAddressPort(EquipmentPropNames::CFG_SERVICE_IP1,
							 EquipmentPropNames::CFG_SERVICE_PORT1, cfgServiceIP1);

	xml.writeStringElement(EquipmentPropNames::CFG_SERVICE_ID2, cfgServiceID2);
	xml.writeHostAddressPort(EquipmentPropNames::CFG_SERVICE_IP2,
							 EquipmentPropNames::CFG_SERVICE_PORT2, cfgServiceIP2);

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

	xml.writeStartElement(XmlElement::TUNING_SERVICES);
	xml.writeBoolAttribute(EquipmentPropNames::TUNING_ENABLE, tuningEnabled);
	xml.writeIntAttribute(XmlAttribute::COUNT, static_cast<int>(tuningServices.size()));

	for(const auto& tsc : tuningServices)
	{
		xml.writeStartElement(XmlElement::TUNING_SERVICE);

		xml.writeStringAttribute(EquipmentPropNames::EQUIPMENT_ID, tsc.tuningServiceID);
		xml.writeStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, tsc.clientRequestIP);
		xml.writeIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, tsc.clientRequestPort);
		xml.writeStringListAttribute(XmlAttribute::DRIVEN_SOURCES, tsc.drivenSources);

		xml.writeEndElement();		// </TuningService>
	}

	xml.writeEndElement();		// </TuningServices>

	//

	xml.writeStartElement(XmlElement::SECURITY);

	xml.writeBoolAttribute(EquipmentPropNames::TUNING_LOGIN, tuningLogin);
	xml.writeStringAttribute(EquipmentPropNames::TUNING_USER_ACCOUNTS, tuningUserAccounts);
	xml.writeIntAttribute(EquipmentPropNames::TUNING_SESSION_TIMEOUT, tuningSessionTimeout);

	xml.writeEndElement();			// </TuningSecurity>

	//

	writeEndSettings(xml);;			// </Settings>

	return true;
}

bool MonitorSettings::readFromXml(XmlReadHelper& xml)
{
	clear();

	bool result = true;

	result = startSettingsReading(xml);

	RETURN_IF_FALSE(result);

	//

	result &= xml.readStringElement(EquipmentPropNames::CFG_SERVICE_ID1, &cfgServiceID1, true);
	result &= xml.readHostAddressPort(EquipmentPropNames::CFG_SERVICE_IP1,
									  EquipmentPropNames::CFG_SERVICE_PORT1, &cfgServiceIP1);

	result &= xml.readStringElement(EquipmentPropNames::CFG_SERVICE_ID2, &cfgServiceID2, true);
	result &= xml.readHostAddressPort(EquipmentPropNames::CFG_SERVICE_IP2,
									  EquipmentPropNames::CFG_SERVICE_PORT2, &cfgServiceIP2);
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

	result &= xml.findElement(XmlElement::TUNING_SERVICES);

	RETURN_IF_FALSE(result);

	result &= xml.readBoolAttribute(EquipmentPropNames::TUNING_ENABLE, &tuningEnabled);

	int count = 0;

	result &= xml.readIntAttribute(XmlAttribute::COUNT, &count);

	tuningServices.clear();

	for(int i = 0; i < count; i++)
	{
		TuningService tsc;

		result &= xml.findElement(XmlElement::TUNING_SERVICE);

		result &= xml.readStringAttribute(EquipmentPropNames::EQUIPMENT_ID, &tsc.tuningServiceID);
		result &= xml.readStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, &tsc.clientRequestIP);
		result &= xml.readIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, &tsc.clientRequestPort);
		result &= xml.readStringListAttribute(XmlAttribute::DRIVEN_SOURCES, &tsc.drivenSources);

		tuningServices.push_back(tsc);
	}

	result &= xml.findElement(XmlElement::SECURITY);
	result &= xml.readBoolAttribute(EquipmentPropNames::TUNING_LOGIN, &tuningLogin);
	result &= xml.readStringAttribute(EquipmentPropNames::TUNING_USER_ACCOUNTS, &tuningUserAccounts);
	result &= xml.readIntAttribute(EquipmentPropNames::TUNING_SESSION_TIMEOUT, &tuningSessionTimeout);

	return result;
}

QStringList MonitorSettings::getSchemaTags() const
{
	return  schemaTags.split(Separator::SEMICOLON, Qt::SkipEmptyParts);
}

QStringList MonitorSettings::getUsersAccounts() const
{
	return  tuningUserAccounts.split(Separator::SEMICOLON, Qt::SkipEmptyParts);
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
	tuningServices.clear();
}

// -------------------------------------------------------------------------------------
//
// TuningClientSettings class implementation
//
// -------------------------------------------------------------------------------------

bool TuningClientSettings::writeToXml(XmlWriteHelper& xml) const
{
	writeStartSettings(xml);

	//

	xml.writeStringElement(EquipmentPropNames::CFG_SERVICE_ID1, cfgServiceID1);
	xml.writeHostAddressPort(EquipmentPropNames::CFG_SERVICE_IP1,
							 EquipmentPropNames::CFG_SERVICE_PORT1, cfgServiceIP1);

	xml.writeStringElement(EquipmentPropNames::CFG_SERVICE_ID2, cfgServiceID2);
	xml.writeHostAddressPort(EquipmentPropNames::CFG_SERVICE_IP2,
							 EquipmentPropNames::CFG_SERVICE_PORT2, cfgServiceIP2);

	//

	xml.writeStartElement(XmlElement::TUNING_SERVICES);
	xml.writeIntAttribute(XmlAttribute::COUNT, static_cast<int>(tuningServices.size()));

	for(const auto& tsc : tuningServices)
	{
		xml.writeStartElement(XmlElement::TUNING_SERVICE);

		xml.writeStringAttribute(EquipmentPropNames::EQUIPMENT_ID, tsc.tuningServiceID);
		xml.writeStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, tsc.clientRequestIP);
		xml.writeIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, tsc.clientRequestPort);
		xml.writeStringListAttribute(XmlAttribute::DRIVEN_SOURCES, tsc.drivenSources);
		xml.writeBoolAttribute(EquipmentPropNames::SINGLE_LM_CONTROL, tsc.singleLmControl);

		xml.writeEndElement();		// </TuningService>
	}

	xml.writeEndElement();		// </TuningServices>

	xml.writeStartElement(XmlElement::APPEARANCE);

	xml.writeBoolAttribute(EquipmentPropNames::AUTO_APPLAY, autoApply);
	xml.writeBoolAttribute(EquipmentPropNames::SHOW_SIGNALS, showSignals);
	xml.writeBoolAttribute(EquipmentPropNames::SHOW_SCHEMAS, showSchemas);
	xml.writeBoolAttribute(EquipmentPropNames::SHOW_SCHEMAS_LIST, showSchemasList);
	xml.writeBoolAttribute(EquipmentPropNames::SHOW_SCHEMAS_TABS, showSchemasTabs);
	xml.writeIntAttribute(EquipmentPropNames::STATUS_FLAG_FUNCTION, statusFlagFunction);

	xml.writeBoolAttribute(EquipmentPropNames::TUNING_LOGIN, tuningLogin);
	xml.writeStringAttribute(EquipmentPropNames::TUNING_USER_ACCOUNTS, tuningUserAccounts);
	xml.writeIntAttribute(EquipmentPropNames::TUNING_SESSION_TIMEOUT, tuningSessionTimeout);
	xml.writeBoolAttribute(EquipmentPropNames::LOGIN_PER_OPERATION, loginPerOperation);

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

	result = startSettingsReading(xml);

	RETURN_IF_FALSE(result);

	result &= xml.readStringElement(EquipmentPropNames::CFG_SERVICE_ID1, &cfgServiceID1, true);
	result &= xml.readHostAddressPort(EquipmentPropNames::CFG_SERVICE_IP1,
									  EquipmentPropNames::CFG_SERVICE_PORT1, &cfgServiceIP1);

	result &= xml.readStringElement(EquipmentPropNames::CFG_SERVICE_ID2, &cfgServiceID2, true);
	result &= xml.readHostAddressPort(EquipmentPropNames::CFG_SERVICE_IP2,
									  EquipmentPropNames::CFG_SERVICE_PORT2, &cfgServiceIP2);

	result &= xml.findElement(XmlElement::TUNING_SERVICES);

	RETURN_IF_FALSE(result);

	int count = 0;

	result &= xml.readIntAttribute(XmlAttribute::COUNT, &count);

	tuningServices.clear();

	for(int i = 0; i < count; i++)
	{
		TuningService tsc;

		result &= xml.findElement(XmlElement::TUNING_SERVICE);

		result &= xml.readStringAttribute(EquipmentPropNames::EQUIPMENT_ID, &tsc.tuningServiceID);
		result &= xml.readStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, &tsc.clientRequestIP);
		result &= xml.readIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, &tsc.clientRequestPort);
		result &= xml.readStringListAttribute(XmlAttribute::DRIVEN_SOURCES, &tsc.drivenSources);
		result &= xml.readBoolAttribute(EquipmentPropNames::SINGLE_LM_CONTROL, &tsc.singleLmControl);

		tuningServices.push_back(tsc);
	}

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

	result &= xml.readBoolAttribute(EquipmentPropNames::TUNING_LOGIN, &tuningLogin);
	result &= xml.readStringAttribute(EquipmentPropNames::TUNING_USER_ACCOUNTS, &tuningUserAccounts);
	result &= xml.readIntAttribute(EquipmentPropNames::TUNING_SESSION_TIMEOUT, &tuningSessionTimeout);
	result &= xml.readBoolAttribute(EquipmentPropNames::LOGIN_PER_OPERATION, &loginPerOperation);

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
	return  tuningUserAccounts.split(Separator::SEMICOLON, Qt::SkipEmptyParts);
}
/*
const TuningClientSettings& TuningClientSettings::operator = (const TuningClientSettings& src)
{
	tuningServices = src.tuningServices;

	autoApply = src.autoApply;

	showSignals = src.showSignals;
	showSchemas = src.showSchemas;
	showSchemasList = src.showSchemasList;
	showSchemasTabs = src.showSchemasTabs;

	statusFlagFunction = src.statusFlagFunction;

	tuningLogin = src.tuningLogin;
	tuningUserAccounts = src.tuningUserAccounts;
	tuningSessionTimeout = src.tuningSessionTimeout;
	loginPerOperation = src.loginPerOperation;

	filterByEquipment = src.filterByEquipment;
	filterBySchema = src.filterBySchema;

	startSchemaID = src.startSchemaID;
	schemaTags = src.schemaTags;

	return *this;
}*/

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
		tuningLogin != src.tuningLogin ||
		tuningSessionTimeout != src.tuningSessionTimeout ||
		tuningUserAccounts != src.tuningUserAccounts ||
		loginPerOperation != src.loginPerOperation)
	{
		return true;
	}

	return false;
}

bool TuningClientSettings::connectionChanged(const TuningClientSettings& src) const
{
	if (tuningServices.size() != src.tuningServices.size() ||
		autoApply != src.autoApply ||
		statusFlagFunction != src.statusFlagFunction)
	{
		return true;
	}

	// tuningServices.size() and src.tuningServices.size() are equal!
	//
	for(const auto& tsc : tuningServices)
	{
		if (std::find(src.tuningServices.begin(), src.tuningServices.end(), tsc) == src.tuningServices.end())
		{
			return true;
		}
	}

	return false;
}
