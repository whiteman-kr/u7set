#ifndef ONLINE_LIB_DOMAIN
#error Do not include this file in the project! Link OnlineLib instead.
#endif

#include "../UtilsLib/XmlHelper.h"
#include "../UtilsLib/WUtils.h"
#include "SocketIO.h"
#include "SoftwareSettings.h"

// -------------------------------------------------------------------------------------
//
// SessionParams struct implementation
//
// -------------------------------------------------------------------------------------

void SessionParams::saveTo(Network::SessionParams* sp) const
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

void SessionParams::clear()
{
	currentSettingsProfile.clear();
	softwareRunMode = E::SoftwareRunMode::Normal;
}

// -------------------------------------------------------------------------------------
//
// SoftwareSettings class implementation
//
// -------------------------------------------------------------------------------------

SoftwareSettings::SoftwareSettings(const QString& profile) :
	profile(profile)
{
}

SoftwareSettings::~SoftwareSettings()
{
}

void SoftwareSettings::writeStartSettings(XmlWriteHelper& xml) const
{
	xml.writeStartElement(XmlElement::SETTINGS);	//	<Settings>

	xml.writeStringAttribute(XmlAttribute::PROFILE, profile);
	xml.writeStringAttribute(EquipmentPropNames::HOSTNAME, hostname);
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
	result = xml.readStringAttribute(EquipmentPropNames::HOSTNAME, &hostname);

	return result;
}

void SoftwareSettings::writeRqControllersToXml(XmlWriteHelper& xml, const std::vector<RqCtrlSettings>& rcSettings) const
{
	xml.writeStartElement(XmlElement::REQUEST_CONTROLLERS);
	xml.writeIntAttribute(XmlAttribute::COUNT, TO_INT(rcSettings.size()));

	for(const RqCtrlSettings& rcs : rcSettings)
	{
		rcs.writeToXml(xml);
	}

	xml.writeEndElement();	// </RequestControllers>
}

bool SoftwareSettings::readRqControllersFromXml(XmlReadHelper& xml, std::vector<RqCtrlSettings>* rcSettings)
{
	TEST_PTR_RETURN_FALSE(rcSettings);

	bool result = true;

	rcSettings->clear();

	result &= xml.findElement(XmlElement::REQUEST_CONTROLLERS);

	int rqCtrlsCount = 0;

	result &= xml.readIntAttribute(XmlAttribute::COUNT, &rqCtrlsCount);

	rcSettings->reserve(rqCtrlsCount);

	for(int i = 0; i < rqCtrlsCount; i++)
	{
		RqCtrlSettings rcs;

		result &= rcs.readFromXml(xml);

		BREAK_IF_FALSE(result);

		rcSettings->emplace_back(rcs);
	}

	return result;
}

template<typename SERVICETYPE>		// SERVICETYPE is one of TuningService, AppDataService, ArchiveService
void SoftwareSettings::setShortId(std::vector<SERVICETYPE>* services)
{
	if (services == nullptr)
	{
		Q_ASSERT(services);
		return;
	}

	if (services->empty() == true)
	{
		return;
	}

	struct ServiceRecord
	{
		QString serviceId;		// Full ArchiveServiceID
		QString shortId;		// Shorted ArchiveServiceID
		QString currentId;		// Current work version (used for creating shortId)
	};

	std::vector<ServiceRecord> ss;
	ss.reserve(services->size());

	for (const SERVICETYPE& as : *services)
	{
		ss.push_back({as.equipmentId, as.equipmentId, as.equipmentId});
	}

	if (ss.size() == 1)
	{
		// If there is just one service then make it a bit shorter (remove system)
		//
		QString shortId = ss[0].shortId;

		if (qsizetype underscoreIndex = shortId.indexOf('_');
				underscoreIndex != -1)
		{
			ss[0].shortId = shortId.right(shortId.size() - (underscoreIndex + 1));
		}
		else
		{
			// ss[0].shortId = shortId;
		}
	}
	else
	{
		for (qsizetype i = 0; i < ss[0].serviceId.size(); i++)
		{
			QChar firstLetter = ss[0].currentId[0];

			if (firstLetter == QChar('_'))
			{
				std::ranges::for_each(ss, [](ServiceRecord& sr){	sr.shortId = sr.currentId; sr.shortId.remove(0, 1);});
			}

			bool firstLetterIsSame = std::all_of(ss.begin(), ss.end(),
												 [firstLetter](const ServiceRecord& sr)
			{
				return sr.currentId[0] == firstLetter;
			});

			if (firstLetterIsSame == true)
			{
				std::ranges::for_each(ss, [](ServiceRecord& sr){	sr.currentId.remove(0, 1);});
			}
			else
			{
				break;
			}
		}
	}

	// Set result
	//
	Q_ASSERT(services->size() == ss.size());

	for (size_t i = 0; i < services->size(); i++)
	{
		services->at(i).shortenId = ss.at(i).shortId;
	}

	return;
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

void SoftwareSettingsSet::clear()
{
	m_softwareType = E::SoftwareType::Unknown;
	m_settingsMap.clear();
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

	case E::SoftwareType::TestSuite:
		return std::make_shared<TestSuiteSettings>();

	case E::SoftwareType::GatewayService:
		return std::make_shared<GatewayServiceSettings>();

	case E::SoftwareType::Diagnostics:
		return std::make_shared<DiagnosticsSettings>();

	case E::SoftwareType::AdsBridge:
		return std::make_shared<AdsBridgeSettings>();

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
// RqCtrlSettings class implementation
//
// -------------------------------------------------------------------------------------

int RqCtrlSettings::ID() const
{
	Q_ASSERT(m_propsMask & PROP_ID);
	return m_ID;
}

void RqCtrlSettings::setID(int id)
{
	m_ID = id;
	m_propsMask |= PROP_ID;
}

QString RqCtrlSettings::equipmentID() const
{
	Q_ASSERT(m_propsMask & PROP_EQUIPMENT_ID);
	return m_equipmentID;
}

void RqCtrlSettings::setEquipmentID(const QString& equipmentID)
{
	m_equipmentID = equipmentID;
	m_propsMask |= PROP_EQUIPMENT_ID;
}

bool RqCtrlSettings::enable() const
{
	Q_ASSERT(m_propsMask & PROP_ENABLE);
	return m_enable;
}

void RqCtrlSettings::setEnable(bool enable)
{
	m_enable = enable;
	m_propsMask |= PROP_ENABLE;
}

E::SecurityLevel RqCtrlSettings::securityLevel() const
{
	Q_ASSERT(m_propsMask & PROP_SECURITY_LEVEL);
	return m_securityLevel;
}

void RqCtrlSettings::setSecurityLevel(E::SecurityLevel level)
{
	m_securityLevel = level;
	m_propsMask |= PROP_SECURITY_LEVEL;
}

HostAddressPort RqCtrlSettings::clientRequestIP() const
{
	Q_ASSERT(m_propsMask & PROP_CLIENT_REQUEST_IP);
	return m_clientRequestIP;
}

void RqCtrlSettings::setClientRequestIP(const HostAddressPort& addrPort)
{
	m_clientRequestIP = addrPort;
	m_propsMask |= PROP_CLIENT_REQUEST_IP;
}

bool RqCtrlSettings::hasClientRequestIP() const
{
	return (m_propsMask & PROP_CLIENT_REQUEST_IP) != 0;
}

QHostAddress RqCtrlSettings::clientRequestNetmask() const
{
	Q_ASSERT(m_propsMask & PROP_CLIENT_REQUEST_NETMASK);
	return m_clientRequestNetmask;
}

void RqCtrlSettings::setClientRequestNetmask(const QHostAddress& netmask)
{
	m_clientRequestNetmask = netmask;
	m_propsMask |= PROP_CLIENT_REQUEST_NETMASK;
}

HostAddressPort RqCtrlSettings::rtTrendsRequestIP() const
{
	Q_ASSERT(m_propsMask & PROP_RT_TRENDS_REQUEST_IP);
	return m_rtTrendsRequestIP;
}

void RqCtrlSettings::setRtTrendsRequestIP(const HostAddressPort& addrPort)
{
	m_rtTrendsRequestIP = addrPort;
	m_propsMask |= PROP_RT_TRENDS_REQUEST_IP;
}

bool RqCtrlSettings::hasRtTrendsRequestIP() const
{
	return (m_propsMask & PROP_RT_TRENDS_REQUEST_IP) != 0;
}

quint32 RqCtrlSettings::propsMask() const
{
	return m_propsMask;
}

void RqCtrlSettings::clear()
{
	m_propsMask = 0;

	m_ID = -1;
	m_equipmentID.clear();
	m_enable = false;
	m_securityLevel = E::SecurityLevel::Basic;
	m_clientRequestIP.clear();
	m_clientRequestNetmask.clear();
	m_rtTrendsRequestIP.clear();
}

bool RqCtrlSettings::isValid() const
{
	return m_ID != -1;
}

bool RqCtrlSettings::operator < (const RqCtrlSettings& rcs) const
{
	return m_ID < rcs.m_ID;
}

bool RqCtrlSettings::writeToXml(XmlWriteHelper& xml) const
{
	bool result = true;

	xml.writeStartElement(XmlElement::REQUEST_CONTROLLER);

	Q_ASSERT(m_propsMask != 0);
	xml.writeUInt32Attribute(XmlAttribute::PROPS_MASK, m_propsMask, true);

	Q_ASSERT(m_propsMask & PROP_ID);
	Q_ASSERT(m_propsMask & PROP_EQUIPMENT_ID);

	for(const quint32 propFlag : m_knownPropsFlags)
	{
		if ((m_propsMask & propFlag) == 0)
		{
			continue;
		}

		switch(propFlag)
		{
		case PROP_ID:
			xml.writeIntAttribute(XmlAttribute::ID, m_ID);
			break;

		case PROP_EQUIPMENT_ID:
			xml.writeStringAttribute(XmlAttribute::EQUIPMENT_ID, m_equipmentID);
			break;

		case PROP_ENABLE:
			xml.writeBoolAttribute(XmlAttribute::ENABLE, m_enable);
			break;

		case PROP_SECURITY_LEVEL:
			xml.writeEnumKeyAttribute(XmlAttribute::SECURITY_LEVEL, m_securityLevel);
			break;

		case PROP_CLIENT_REQUEST_IP:
			xml.writeIPv4PortAttribute(XmlAttribute::CLIENT_REQUEST_IP, m_clientRequestIP);
			break;

		case PROP_CLIENT_REQUEST_NETMASK:
			xml.writeIPv4Attribute(XmlAttribute::CLIENT_REQUEST_NETMASK, m_clientRequestNetmask);
			break;

		case PROP_RT_TRENDS_REQUEST_IP:
			xml.writeIPv4PortAttribute(XmlAttribute::RT_TRENDS_REQUEST_IP, m_rtTrendsRequestIP);
			break;

		default:
			Q_ASSERT(false);
			result = false;
		}
	}

	xml.writeEndElement();	// </RequestController>

	return result;
}

bool RqCtrlSettings::readFromXml(XmlReadHelper& xml)
{
	bool result = true;

	result &= xml.findElement(XmlElement::REQUEST_CONTROLLER);

	result &= xml.readUInt32Attribute(XmlAttribute::PROPS_MASK, &m_propsMask);

	Q_ASSERT(m_propsMask & PROP_ID);
	Q_ASSERT(m_propsMask & PROP_EQUIPMENT_ID);

	for(const quint32 propFlag : m_knownPropsFlags)
	{
		if ((m_propsMask & propFlag) == 0)
		{
			continue;
		}

		switch(propFlag)
		{
		case PROP_ID:
			result &= xml.readIntAttribute(XmlAttribute::ID, &m_ID);
			break;

		case PROP_EQUIPMENT_ID:
			result &= xml.readStringAttribute(XmlAttribute::EQUIPMENT_ID, &m_equipmentID);
			break;

		case PROP_ENABLE:
			result &= xml.readBoolAttribute(XmlAttribute::ENABLE, &m_enable);
			break;

		case PROP_SECURITY_LEVEL:
			result &= xml.readEnumKeyAttribute(XmlAttribute::SECURITY_LEVEL, &m_securityLevel);
			break;

		case PROP_CLIENT_REQUEST_IP:
			result &= xml.readIPv4PortAttribute(XmlAttribute::CLIENT_REQUEST_IP, &m_clientRequestIP);
			break;

		case PROP_CLIENT_REQUEST_NETMASK:
			result &= xml.readIPv4Attribute(XmlAttribute::CLIENT_REQUEST_NETMASK, &m_clientRequestNetmask);
			break;

		case PROP_RT_TRENDS_REQUEST_IP:
			result &= xml.readIPv4PortAttribute(XmlAttribute::RT_TRENDS_REQUEST_IP, &m_rtTrendsRequestIP);
			break;

		default:
			Q_ASSERT(false);
			result = false;
		}
	}

	return result;
}

bool RqCtrlSettings::isKnownPropsFlag(quint32 propFlag)
{
	return m_knownPropsFlags.contains(propFlag);
}

// -------------------------------------------------------------------------------------
//
// CfgServiceSettings class implementation
//
// -------------------------------------------------------------------------------------

bool CfgServiceSettings::writeToXml(XmlWriteHelper& xml) const
{
	writeStartSettings(xml);

	xml.writeBoolElement(EquipmentPropNames::CHECK_HOSTNAME, checkHostname);

	writeRqControllersToXml(xml, rcSettings);

	xml.writeStartElement(XmlElement::CLIENTS);
	xml.writeIntAttribute(XmlAttribute::COUNT, TO_INT(clients.size()));

	for(const ClientInfo& ci : clients)
	{
		xml.writeStartElement(XmlElement::CLIENT);

		xml.writeStringAttribute(EquipmentPropNames::EQUIPMENT_ID, ci.equipmentID);
		xml.writeStringAttribute(EquipmentPropNames::SOFTWARE_TYPE, E::valueToString(ci.softwareType));
		xml.writeStringAttribute(EquipmentPropNames::HOSTNAME, ci.hostname);

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

	result &= xml.readBoolElement(EquipmentPropNames::CHECK_HOSTNAME, &checkHostname, true);

	result &= readRqControllersFromXml(xml, &rcSettings);

	RETURN_IF_FALSE(result);

	result &= xml.findElement(XmlElement::CLIENTS);

	int clientsCount = 0;

	result &= xml.readIntAttribute(XmlAttribute::COUNT, &clientsCount);

	clients.reserve(clientsCount);

	for(int i = 0; i < clientsCount; i++)
	{
		result &= xml.findElement(XmlElement::CLIENT);

		ClientInfo ci;

		result &= xml.readStringAttribute(EquipmentPropNames::EQUIPMENT_ID, &ci.equipmentID);

		QString softwareTypeStr;

		result &= xml.readStringAttribute(EquipmentPropNames::SOFTWARE_TYPE, &softwareTypeStr);

		bool ok = false;

		ci.softwareType = E::stringToValue<E::SoftwareType>(softwareTypeStr, &ok);

		result &= ok;

		result &= xml.readStringAttribute(EquipmentPropNames::HOSTNAME, &ci.hostname);

		BREAK_IF_FALSE(result);

		clients.emplace_back(ci);
	}

	return result;
}

QStringList CfgServiceSettings::knownClients() const
{
	QStringList knownClients;

	for(const ClientInfo& client : clients)
	{
		knownClients.append(client.equipmentID.trimmed());
	}

	return knownClients;
}

// -------------------------------------------------------------------------------------
//
// AppDataServiceSettings class implementation
//
// -------------------------------------------------------------------------------------

RqCtrlSettings AppDataServiceSettings::getRequestControllerSettings(const QString& rcEquipmentID)
{
	for(const RqCtrlSettings& rcs : rcSettings)
	{
		if (rcs.equipmentID() == rcEquipmentID)
		{
			return rcs;
		}
	};

	return RqCtrlSettings();
}

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
	xml.writeIntElement(EquipmentPropNames::DISCRETES_LOG_HOURS, discretesLogHours);

	xml.writeStringElement(EquipmentPropNames::ARCH_SERVICE_ID, archServiceID);
	xml.writeHostAddressPort(EquipmentPropNames::ARCH_SERVICE_IP,
							 EquipmentPropNames::ARCH_SERVICE_PORT, archServiceIP);

	writeRqControllersToXml(xml, rcSettings);

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
	result &= xml.readIntElement(EquipmentPropNames::DISCRETES_LOG_HOURS, &discretesLogHours, true);

	result &= xml.readStringElement(EquipmentPropNames::ARCH_SERVICE_ID, &archServiceID, true);
	result &= xml.readHostAddressPort(EquipmentPropNames::ARCH_SERVICE_IP,
									  EquipmentPropNames::ARCH_SERVICE_PORT, &archServiceIP);

	result &= readRqControllersFromXml(xml, &rcSettings);

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

	xml.writeHostAddressPort(EquipmentPropNames::RT_TRENDS_REQUEST_IP,
							 EquipmentPropNames::RT_TRENDS_REQUEST_PORT, rtTrendsRequestIP);

	xml.writeEnumKeyElement<E::SecurityLevel>(EquipmentPropNames::SECURITY_LEVEL, securityLevel);

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

	result &= xml.readHostAddressPort(EquipmentPropNames::RT_TRENDS_REQUEST_IP,
									  EquipmentPropNames::RT_TRENDS_REQUEST_PORT, &rtTrendsRequestIP);

	result &= xml.readEnumKeyElement<E::SecurityLevel>(EquipmentPropNames::SECURITY_LEVEL, &securityLevel, true);

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

	xml.writeEnumKeyElement<E::SecurityLevel>(EquipmentPropNames::SECURITY_LEVEL, securityLevel);

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
		xml.writeStringAttribute(EquipmentPropNames::SOFTWARE_TYPE, E::valueToString(tc.softwareType));
		xml.writeBoolAttribute(EquipmentPropNames::TUNING_LOGIN, tc.tuningLogin);
		xml.writeStringAttribute(XmlAttribute::MATS_USERS, tc.matsUsers);

		writeTuningSourcesToXml(xml, tc.drivenSources);

		xml.writeEndElement();		// TUNING_CLIENT
	}

	xml.writeEndElement();			// TUNING_CLIENTS

	// write MATS users info
	//
	xml.writeStartElement(XmlElement::MATS_USERS);
	xml.writeIntAttribute(XmlAttribute::COUNT, static_cast<int>(matsUsers.size()));

	for(const auto& matsUser : matsUsers)
	{
		matsUser.save(*xml.xmlStreamWriter());
	}

	xml.writeEndElement();			// MATS_USERS

	// write channels info
	//
	for(int channel = CHANNEL_1; channel < channelCount; channel++)
	{
		const ChannelSettings& ch = channelSettings[channel];

		xml.writeStartElement(XmlElement::TUNING_CHANNEL_TEMPLATE.arg(channel + 1));		// <Channel ....>

		xml.writeBoolAttribute(EquipmentPropNames::ENABLE, ch.enable);

		if (ch.enable == true)
		{
			xml.writeStringAttribute(XmlAttribute::CONTROLLER_EQUIPMENT_ID, ch.serviceControllerEquipmentID);
			xml.writeIPv4PortAttribute(EquipmentPropNames::TUNING_DATA_IP, ch.tuningDataIP);
			xml.writeIPv4Attribute(EquipmentPropNames::TUNING_DATA_NETMASK, ch.tuningDataNetmask);
			xml.writeIPv4PortAttribute(EquipmentPropNames::TUNING_SIM_IP, ch.tuningSimIP);

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

	result &= xml.readEnumKeyElement<E::SecurityLevel>(EquipmentPropNames::SECURITY_LEVEL, &securityLevel, true);

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

		QString swTypeStr;
		result &= xml.readStringAttribute(EquipmentPropNames::SOFTWARE_TYPE, &swTypeStr);

		bool ok = false;
		tc.softwareType = E::stringToValue<E::SoftwareType>(swTypeStr, &ok);

		result &= ok;

		result &= xml.readBoolAttribute(EquipmentPropNames::TUNING_LOGIN, &tc.tuningLogin);
		result &= xml.readStringAttribute(XmlAttribute::MATS_USERS, &tc.matsUsers);

		result &= readTuningSourcesFromXml(xml, &tc.drivenSources);

		clients.push_back(tc);
	}

	// read MATS users info
	//
	matsUsers.clear();

	result = xml.findElement(XmlElement::MATS_USERS);

	RETURN_IF_FALSE(result);

	int matsUsersCount = 0;

	result = xml.readIntAttribute(XmlAttribute::COUNT, &matsUsersCount);

	RETURN_IF_FALSE(result);

	for(int i = 0; i < matsUsersCount; i++)
	{
		OnlineLib::MatsUser mu;

		bool res = xml.findElement(XmlElement::MATS_USER);

		if (res == false)
		{
			result = false;
			break;
		}

		result &= mu.load(*xml.xmlStreamReader());

		matsUsers.emplace_back(mu);
	}

	RETURN_IF_FALSE(result);

	// read channels info
	//
	for(int channel = CHANNEL_1; channel < channelCount; channel++)
	{
		result &= xml.findElement(XmlElement::TUNING_CHANNEL_TEMPLATE.arg(channel + 1));

		CONTINUE_IF_FALSE(result);

		ChannelSettings& ch = channelSettings[channel];

		result &= xml.readBoolAttribute(EquipmentPropNames::ENABLE, &ch.enable);

		if (ch.enable == true)
		{
			result &= xml.readStringAttribute(XmlAttribute::CONTROLLER_EQUIPMENT_ID, &ch.serviceControllerEquipmentID);
			result &= xml.readIPv4PortAttribute(EquipmentPropNames::TUNING_DATA_IP, &ch.tuningDataIP);
			result &= xml.readIPv4Attribute(EquipmentPropNames::TUNING_DATA_NETMASK, &ch.tuningDataNetmask);
			result &= xml.readIPv4PortAttribute(EquipmentPropNames::TUNING_SIM_IP, &ch.tuningSimIP);

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

	xml.writeEnumKeyElement<E::SecurityLevel>(EquipmentPropNames::SECURITY_LEVEL, securityLevel);

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

	result &= xml.readEnumKeyElement<E::SecurityLevel>(EquipmentPropNames::SECURITY_LEVEL, &securityLevel, true);

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

	// ConfigServices (1/2)
	//
	{
		xml.writeStartElement(XmlElement::CFG_SERVICE1);

		xml.writeStringAttribute(EquipmentPropNames::EQUIPMENT_ID, configService1.equipmentId);

		xml.writeStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, configService1.address.addressStr());
		xml.writeIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, configService1.address.port());

		xml.writeEndElement();			// </CfgService1>
	}

	{
		xml.writeStartElement(XmlElement::CFG_SERVICE2);

		xml.writeStringAttribute(EquipmentPropNames::EQUIPMENT_ID, configService2.equipmentId);

		xml.writeStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, configService2.address.addressStr());
		xml.writeIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, configService2.address.port());

		xml.writeEndElement();			// </CfgService2>
	}

	// Some Schema staff
	//
	xml.writeStringElement(EquipmentPropNames::START_SCHEMA_ID, startSchemaId);
	xml.writeStringElement(EquipmentPropNames::SCHEMA_TAGS, schemaTags);
		
	xml.writeStringElement(EquipmentPropNames::APP_SIGNAL_LIST_IDS, appSignalListIDs.join(Separator::SEMICOLON));
	xml.writeStringElement(EquipmentPropNames::APP_SIGNAL_LIST_MASKS, appSignalListMasks.join(Separator::SEMICOLON));
	xml.writeStringElement(EquipmentPropNames::APP_SIGNAL_LIST_TAGS, appSignalListTags.join(Separator::SEMICOLON));

	// AppDataServices
	//
	for (const SoftwareEndpoint::AppDataService& ads : appDataServices)
	{
		xml.writeStartElement(XmlElement::APP_DATA_SERVICE);

		xml.writeStringAttribute(EquipmentPropNames::EQUIPMENT_ID, ads.equipmentId);

		xml.writeStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, ads.address.addressStr());
		xml.writeIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, ads.address.port());

		xml.writeStringAttribute(EquipmentPropNames::RT_TRENDS_REQUEST_IP, ads.realtimeAddress.addressStr());
		xml.writeIntAttribute(EquipmentPropNames::RT_TRENDS_REQUEST_PORT, ads.realtimeAddress.port());

		xml.writeEndElement();			// </AppDataService>
	}

	// ArchiveServices
	//
	for (const SoftwareEndpoint::ArchiveService& as : archiveServices)
	{
		xml.writeStartElement(XmlElement::ARCHIVE_SERVICE);

		xml.writeStringAttribute(EquipmentPropNames::EQUIPMENT_ID, as.equipmentId);
		xml.writeStringAttribute(EquipmentPropNames::APP_DATA_SERVICE_ID, as.appDataServiceId);

		xml.writeStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, as.address.addressStr());
		xml.writeIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, as.address.port());

		xml.writeEndElement();			// </ArchiveService>
	}

	// Tunings
	//
	xml.writeStartElement(XmlElement::TUNING_SERVICES);
	xml.writeBoolAttribute(EquipmentPropNames::TUNING_ENABLE, tuningEnabled);
	xml.writeIntAttribute(XmlAttribute::COUNT, static_cast<int>(tuningServices.size()));

	for(const auto& tsc : tuningServices)
	{
		xml.writeStartElement(XmlElement::TUNING_SERVICE);

		xml.writeStringAttribute(EquipmentPropNames::EQUIPMENT_ID, tsc.equipmentId);
		xml.writeStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, tsc.address.addressStr());
		xml.writeIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, tsc.address.port());
		xml.writeStringListAttribute(XmlAttribute::DRIVEN_SOURCES, tsc.drivenSources);

		xml.writeEndElement();		// </TuningService>
	}

	xml.writeEndElement();		// </TuningServices>

	// --
	//
	xml.writeStartElement(XmlElement::TUNING_SECURITY);

	xml.writeBoolAttribute(EquipmentPropNames::TUNING_LOGIN, tuningLogin);
	xml.writeStringAttribute(EquipmentPropNames::TUNING_USER_ACCOUNTS, tuningUserAccounts);
	xml.writeIntAttribute(EquipmentPropNames::TUNING_SESSION_TIMEOUT, tuningSessionTimeout);

	xml.writeEndElement();			// </TuningSecurity>

	// <SignalLog>
	//
	xml.writeStartElement(XmlElement::SIGNAL_LOG);

	xml.writeBoolAttribute(EquipmentPropNames::ENABLE, signalLogEnable);
	xml.writeStringAttribute(XmlElement::SIGNAL_LOG_ATTRIBUTE_TAG_CRITICAL, signalLogTagCritical.trimmed());
	xml.writeStringAttribute(XmlElement::SIGNAL_LOG_ATTRIBUTE_TAG_WARNING, signalLogTagWarning.trimmed());

	xml.writeEndElement(); // </SignalLog>

	// <Appearance>
	//
	xml.writeStartElement(XmlElement::APPEARANCE);
	xml.writeIntAttribute(EquipmentPropNames::STATUS_FLAG_FUNCTION, static_cast<int>(statusFlagFunction));
	xml.writeEndElement(); // </Appearance>

	// --
	//
	writeEndSettings(xml);			// </Settings>

	return true;
}

bool MonitorSettings::readFromXml(XmlReadHelper& xml)
{
	clear();

	bool result = true;

	result = startSettingsReading(xml);

	RETURN_IF_FALSE(result);

	while (xml.readNextStartElement() == true)
	{
		if (xml.name() == XmlElement::CFG_SERVICE1)
		{
			SoftwareEndpoint::ConfigService cs;
			QString clientIp;
			int clientPort = 0;

			result &= xml.readStringAttribute(EquipmentPropNames::EQUIPMENT_ID, &cs.equipmentId);
			result &= xml.readStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, &clientIp);
			result &= xml.readIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, &clientPort);

			cs.address.setAddressPort(clientIp, clientPort);
			configService1 = cs;

			xml.skipCurrentElement();
			continue;
		}

		if (xml.name() == XmlElement::CFG_SERVICE2)
		{
			SoftwareEndpoint::ConfigService cs;
			QString clientIp;
			int clientPort = 0;

			result &= xml.readStringAttribute(EquipmentPropNames::EQUIPMENT_ID, &cs.equipmentId);
			result &= xml.readStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, &clientIp);
			result &= xml.readIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, &clientPort);

			cs.address.setAddressPort(clientIp, clientPort);
			configService2 = cs;

			xml.skipCurrentElement();
			continue;
		}

		if (xml.name() == EquipmentPropNames::START_SCHEMA_ID)
		{
			startSchemaId = xml.elementText();
			continue;
		}

		if (xml.name() == EquipmentPropNames::SCHEMA_TAGS)
		{
			schemaTags = xml.elementText();
			continue;
		}

		if (xml.name() == EquipmentPropNames::APP_SIGNAL_LIST_IDS)
		{
			appSignalListIDs = xml.elementText().split(Separator::SEMICOLON);
			continue;
		}
		if (xml.name() == EquipmentPropNames::APP_SIGNAL_LIST_MASKS)
		{
			appSignalListMasks = xml.elementText().split(Separator::SEMICOLON);
			continue;
		}
		if (xml.name() == EquipmentPropNames::APP_SIGNAL_LIST_TAGS)
		{
			appSignalListTags = xml.elementText().split(Separator::SEMICOLON);
			continue;
		}

		if (xml.name() == XmlElement::APP_DATA_SERVICE)
		{
			SoftwareEndpoint::AppDataService ads;
			QString clientIp;
			int clientPort = 0;
			QString rtIp;
			int rtPort = 0;

			result &= xml.readStringAttribute(EquipmentPropNames::EQUIPMENT_ID, &ads.equipmentId);
			result &= xml.readStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, &clientIp);
			result &= xml.readIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, &clientPort);
			result &= xml.readStringAttribute(EquipmentPropNames::RT_TRENDS_REQUEST_IP, &rtIp);
			result &= xml.readIntAttribute(EquipmentPropNames::RT_TRENDS_REQUEST_PORT, &rtPort);

			ads.address.setAddressPort(clientIp, clientPort);
			ads.realtimeAddress.setAddressPort(rtIp, rtPort);

			appDataServices.push_back(ads);

			xml.skipCurrentElement();
			continue;
		}

		if (xml.name() == XmlElement::ARCHIVE_SERVICE)
		{
			SoftwareEndpoint::ArchiveService archiveService;

			QString clientRequestIp;
			int clientRequestPort = 0;


			result &= xml.readStringAttribute(EquipmentPropNames::EQUIPMENT_ID, &archiveService.equipmentId);
			result &= xml.readStringAttribute(EquipmentPropNames::APP_DATA_SERVICE_ID, &archiveService.appDataServiceId);

			result &= xml.readStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, &clientRequestIp);
			result &= xml.readIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, &clientRequestPort);

			archiveService.address.setAddressPort(clientRequestIp, clientRequestPort);

			archiveServices.push_back(archiveService);

			xml.skipCurrentElement();
			continue;
		}

		if (xml.name() == XmlElement::TUNING_SERVICES)
		{
			result &= xml.readBoolAttribute(EquipmentPropNames::TUNING_ENABLE, &tuningEnabled);

			int count = 0;
			result &= xml.readIntAttribute(XmlAttribute::COUNT, &count);

			for(int i = 0; i < count; i++)
			{
				SoftwareEndpoint::TuningService tsc;

				result &= xml.findElement(XmlElement::TUNING_SERVICE);

				QString clientRequestAddress;
				int clientRequestPort = 0;

				result &= xml.readStringAttribute(EquipmentPropNames::EQUIPMENT_ID, &tsc.equipmentId);
				result &= xml.readStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, &clientRequestAddress);
				result &= xml.readIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, &clientRequestPort);
				result &= xml.readStringListAttribute(XmlAttribute::DRIVEN_SOURCES, &tsc.drivenSources);

				tsc.address = {clientRequestAddress, clientRequestPort};

				tuningServices.push_back(tsc);

				xml.skipCurrentElement();
			}

			xml.skipCurrentElement();
			continue;
		}

		if (xml.name() == XmlElement::TUNING_SECURITY)
		{
			result &= xml.readBoolAttribute(EquipmentPropNames::TUNING_LOGIN, &tuningLogin);
			result &= xml.readStringAttribute(EquipmentPropNames::TUNING_USER_ACCOUNTS, &tuningUserAccounts);
			result &= xml.readIntAttribute(EquipmentPropNames::TUNING_SESSION_TIMEOUT, &tuningSessionTimeout);

			xml.skipCurrentElement();
			continue;
		}

		if (xml.name() == XmlElement::SIGNAL_LOG)
		{
			result &= xml.readBoolAttribute(EquipmentPropNames::ENABLE, &signalLogEnable);
			result &= xml.readStringAttribute(XmlElement::SIGNAL_LOG_ATTRIBUTE_TAG_CRITICAL, &signalLogTagCritical);
			result &= xml.readStringAttribute(XmlElement::SIGNAL_LOG_ATTRIBUTE_TAG_WARNING, &signalLogTagWarning);

			xml.skipCurrentElement();
			continue;
		}

		if (xml.name() == XmlElement::APPEARANCE)
		{
			int value = 0;
			bool resultStatusFlagFunction = xml.readIntAttribute(EquipmentPropNames::STATUS_FLAG_FUNCTION, &value);
			if (resultStatusFlagFunction == true)
			{
				statusFlagFunction = static_cast<LmStatusFlagMode>(value);
			}
			result &= resultStatusFlagFunction;

			xml.skipCurrentElement();
			continue;
		}

		// Unknown element
		//
		qDebug() << "MonitorSettings::readFromXml UnknownElement " << xml.name();
		xml.skipCurrentElement();
	}

	SoftwareSettings::setShortId<SoftwareEndpoint::AppDataService>(&appDataServices);
	SoftwareSettings::setShortId<SoftwareEndpoint::ArchiveService>(&archiveServices);
	SoftwareSettings::setShortId<SoftwareEndpoint::TuningService>(&tuningServices);

	result &= (appDataServices.empty() == false);

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
	*this = MonitorSettings{};
}

// -------------------------------------------------------------------------------------
//
// AdsBridgeSettings class implementation
//
// -------------------------------------------------------------------------------------

bool AdsBridgeSettings::writeToXml(XmlWriteHelper& xml) const
{
	writeStartSettings(xml);

	// AppDataServices
	//
	for (const SoftwareEndpoint::AppDataService& ads : appDataServices)
	{
		xml.writeStartElement(XmlElement::APP_DATA_SERVICE);

		xml.writeStringAttribute(EquipmentPropNames::EQUIPMENT_ID, ads.equipmentId);

		xml.writeStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, ads.address.addressStr());
		xml.writeIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, ads.address.port());

		xml.writeStringAttribute(EquipmentPropNames::RT_TRENDS_REQUEST_IP, ads.realtimeAddress.addressStr());
		xml.writeIntAttribute(EquipmentPropNames::RT_TRENDS_REQUEST_PORT, ads.realtimeAddress.port());

		xml.writeEndElement(); // </AppDataService>
	}

	// --
	//
	writeEndSettings(xml); // </Settings>

	return true;
}

bool AdsBridgeSettings::readFromXml(XmlReadHelper& xml)
{
	clear();

	bool result = true;

	result = startSettingsReading(xml);

	RETURN_IF_FALSE(result);

	while (xml.readNextStartElement() == true)
	{
		if (xml.name() == XmlElement::APP_DATA_SERVICE)
		{
			SoftwareEndpoint::AppDataService ads;
			QString clientIp;
			int clientPort = 0;
			QString rtIp;
			int rtPort = 0;

			result &= xml.readStringAttribute(EquipmentPropNames::EQUIPMENT_ID, &ads.equipmentId);
			result &= xml.readStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, &clientIp);
			result &= xml.readIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, &clientPort);
			result &= xml.readStringAttribute(EquipmentPropNames::RT_TRENDS_REQUEST_IP, &rtIp);
			result &= xml.readIntAttribute(EquipmentPropNames::RT_TRENDS_REQUEST_PORT, &rtPort);

			ads.address.setAddressPort(clientIp, clientPort);
			ads.realtimeAddress.setAddressPort(rtIp, rtPort);

			appDataServices.push_back(ads);

			xml.skipCurrentElement();
			continue;
		}

		// Unknown element
		//
		qDebug() << "AdsBridgeSettings::readFromXml UnknownElement " << xml.name();
		xml.skipCurrentElement();
	}

	SoftwareSettings::setShortId<SoftwareEndpoint::AppDataService>(&appDataServices);

	result &= (appDataServices.empty() == false);

	return result;
}

bool AdsBridgeSettings::readFromXml(const QByteArray& xml)
{
	XmlReadHelper helper{xml};
	return readFromXml(helper);
}

void AdsBridgeSettings::clear()
{
	*this = AdsBridgeSettings{};
}

// -------------------------------------------------------------------------------------
//
// DiagnosticsSettings class implementation
//
// -------------------------------------------------------------------------------------

bool DiagnosticsSettings::writeToXml(XmlWriteHelper& xml) const
{
	writeStartSettings(xml);

	// ConfigServices (1/2)
	//
	{
		xml.writeStartElement(XmlElement::CFG_SERVICE1);

		xml.writeStringAttribute(EquipmentPropNames::EQUIPMENT_ID, configService1.equipmentId);

		xml.writeStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, configService1.address.addressStr());
		xml.writeIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, configService1.address.port());

		xml.writeEndElement();			// </CfgService1>
	}

	{
		xml.writeStartElement(XmlElement::CFG_SERVICE2);

		xml.writeStringAttribute(EquipmentPropNames::EQUIPMENT_ID, configService2.equipmentId);

		xml.writeStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, configService2.address.addressStr());
		xml.writeIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, configService2.address.port());

		xml.writeEndElement();			// </CfgService2>
	}

	// Some Schema staff
	//
	xml.writeStringElement(EquipmentPropNames::START_SCHEMA_ID, startSchemaId);
	xml.writeStringElement(EquipmentPropNames::SCHEMA_TAGS, schemaTags);

	// DiagDataServices
	//
	for (const SoftwareEndpoint::DiagDataService& dds : diagDataServices)
	{
		xml.writeStartElement(XmlElement::DIAG_DATA_SERVICE);

		xml.writeStringAttribute(EquipmentPropNames::EQUIPMENT_ID, dds.equipmentId);

		xml.writeStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, dds.address.addressStr());
		xml.writeIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, dds.address.port());

		xml.writeStringAttribute(EquipmentPropNames::RT_TRENDS_REQUEST_IP, dds.realtimeAddress.addressStr());
		xml.writeIntAttribute(EquipmentPropNames::RT_TRENDS_REQUEST_PORT, dds.realtimeAddress.port());

		xml.writeEndElement();			// </DiagDataServices>
	}

	// ArchiveServices
	//
	for (const SoftwareEndpoint::ArchiveService& as : archiveServices)
	{
		xml.writeStartElement(XmlElement::ARCHIVE_SERVICE);

		xml.writeStringAttribute(EquipmentPropNames::EQUIPMENT_ID, as.equipmentId);
		xml.writeStringAttribute(EquipmentPropNames::APP_DATA_SERVICE_ID, as.appDataServiceId);

		xml.writeStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, as.address.addressStr());
		xml.writeIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, as.address.port());

		xml.writeEndElement();			// </ArchiveService>
	}

	// --
	//
	writeEndSettings(xml);			// </Settings>

	return true;
}

bool DiagnosticsSettings::readFromXml(XmlReadHelper& xml)
{
	clear();

	bool result = true;

	result = startSettingsReading(xml);

	RETURN_IF_FALSE(result);

	while (xml.readNextStartElement() == true)
	{
		if (xml.name() == XmlElement::CFG_SERVICE1)
		{
			SoftwareEndpoint::ConfigService cs;
			QString clientIp;
			int clientPort = 0;

			result &= xml.readStringAttribute(EquipmentPropNames::EQUIPMENT_ID, &cs.equipmentId);
			result &= xml.readStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, &clientIp);
			result &= xml.readIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, &clientPort);

			cs.address.setAddressPort(clientIp, clientPort);
			configService1 = cs;

			xml.skipCurrentElement();
			continue;
		}

		if (xml.name() == XmlElement::CFG_SERVICE2)
		{
			SoftwareEndpoint::ConfigService cs;
			QString clientIp;
			int clientPort = 0;

			result &= xml.readStringAttribute(EquipmentPropNames::EQUIPMENT_ID, &cs.equipmentId);
			result &= xml.readStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, &clientIp);
			result &= xml.readIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, &clientPort);

			cs.address.setAddressPort(clientIp, clientPort);
			configService2 = cs;

			xml.skipCurrentElement();
			continue;
		}

		if (xml.name() == EquipmentPropNames::START_SCHEMA_ID)
		{
			startSchemaId = xml.elementText();
			continue;
		}

		if (xml.name() == EquipmentPropNames::SCHEMA_TAGS)
		{
			schemaTags = xml.elementText();
			continue;
		}

		if (xml.name() == XmlElement::DIAG_DATA_SERVICE)
		{
			SoftwareEndpoint::DiagDataService dds;
			QString clientIp;
			int clientPort = 0;
			QString rtIp;
			int rtPort = 0;

			result &= xml.readStringAttribute(EquipmentPropNames::EQUIPMENT_ID, &dds.equipmentId);
			result &= xml.readStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, &clientIp);
			result &= xml.readIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, &clientPort);
			result &= xml.readStringAttribute(EquipmentPropNames::RT_TRENDS_REQUEST_IP, &rtIp);
			result &= xml.readIntAttribute(EquipmentPropNames::RT_TRENDS_REQUEST_PORT, &rtPort);

			dds.address.setAddressPort(clientIp, clientPort);
			dds.realtimeAddress.setAddressPort(rtIp, rtPort);

			diagDataServices.push_back(dds);

			xml.skipCurrentElement();
			continue;
		}

		if (xml.name() == XmlElement::ARCHIVE_SERVICE)
		{
			SoftwareEndpoint::ArchiveService archiveService;

			QString clientRequestIp;
			int clientRequestPort = 0;


			result &= xml.readStringAttribute(EquipmentPropNames::EQUIPMENT_ID, &archiveService.equipmentId);
			result &= xml.readStringAttribute(EquipmentPropNames::APP_DATA_SERVICE_ID, &archiveService.appDataServiceId);

			result &= xml.readStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, &clientRequestIp);
			result &= xml.readIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, &clientRequestPort);

			archiveService.address.setAddressPort(clientRequestIp, clientRequestPort);

			archiveServices.push_back(archiveService);

			xml.skipCurrentElement();
			continue;
		}

		// Unknown element
		//
		qDebug() << "DiagnosticsSettings::readFromXml UnknownElement " << xml.name();
		xml.skipCurrentElement();
	}

	SoftwareSettings::setShortId<SoftwareEndpoint::DiagDataService>(&diagDataServices);
	SoftwareSettings::setShortId<SoftwareEndpoint::ArchiveService>(&archiveServices);

	result &= (diagDataServices.empty() == false);

	return result;
}

QStringList DiagnosticsSettings::getSchemaTags() const
{
	return  schemaTags.split(Separator::SEMICOLON, Qt::SkipEmptyParts);
}

//QStringList DiagnosticsSettings::getUsersAccounts() const
//{
//	return  tuningUserAccounts.split(Separator::SEMICOLON, Qt::SkipEmptyParts);
//}

void DiagnosticsSettings::clear()
{
	*this = DiagnosticsSettings{};
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

		xml.writeStringAttribute(EquipmentPropNames::EQUIPMENT_ID, tsc.equipmentId);
		xml.writeStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, tsc.address.addressStr());
		xml.writeIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, tsc.address.port());
		xml.writeStringListAttribute(XmlAttribute::DRIVEN_SOURCES, tsc.drivenSources);
		xml.writeBoolAttribute(EquipmentPropNames::SINGLE_LM_CONTROL, tsc.singleLmControl);


		xml.writeEndElement();		// </TuningService>
	}

	xml.writeEndElement();		// </TuningServices>

	xml.writeStartElement(XmlElement::APPEARANCE);

	xml.writeBoolAttribute(EquipmentPropNames::SHOW_SIGNALS, showSignals);
	xml.writeBoolAttribute(EquipmentPropNames::SHOW_SCHEMAS, showSchemas);
	xml.writeBoolAttribute(EquipmentPropNames::SHOW_SCHEMAS_LIST, showSchemasList);
	xml.writeBoolAttribute(EquipmentPropNames::SHOW_SCHEMAS_TABS, showSchemasTabs);
	xml.writeIntAttribute(EquipmentPropNames::STATUS_FLAG_FUNCTION, static_cast<int>(statusFlagFunction));
	xml.writeIntAttribute(EquipmentPropNames::APPLY_MODE, static_cast<int>(applyMode));

	xml.writeBoolAttribute(EquipmentPropNames::TUNING_LOGIN, tuningLogin);
	xml.writeStringAttribute(EquipmentPropNames::TUNING_USER_ACCOUNTS, tuningUserAccounts);
	xml.writeIntAttribute(EquipmentPropNames::TUNING_SESSION_TIMEOUT, tuningSessionTimeout);
	xml.writeBoolAttribute(EquipmentPropNames::LOGIN_PER_OPERATION, loginPerOperation);

	xml.writeStringAttribute(EquipmentPropNames::APP_SIGNAL_LIST_IDS, appSignalListIDs.join(Separator::SEMICOLON));
	xml.writeStringAttribute(EquipmentPropNames::APP_SIGNAL_LIST_MASKS, appSignalListMasks.join(Separator::SEMICOLON));
	xml.writeStringAttribute(EquipmentPropNames::APP_SIGNAL_LIST_TAGS, appSignalListTags.join(Separator::SEMICOLON));

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
		SoftwareEndpoint::TuningService tsc;

		result &= xml.findElement(XmlElement::TUNING_SERVICE);

		QString clientRequestAddress;
		int clientRequestPort = 0;

		result &= xml.readStringAttribute(EquipmentPropNames::EQUIPMENT_ID, &tsc.equipmentId);
		result &= xml.readStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, &clientRequestAddress);
		result &= xml.readIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, &clientRequestPort);
		result &= xml.readStringListAttribute(XmlAttribute::DRIVEN_SOURCES, &tsc.drivenSources);
		result &= xml.readBoolAttribute(EquipmentPropNames::SINGLE_LM_CONTROL, &tsc.singleLmControl);

		tsc.address = {clientRequestAddress, clientRequestPort};

		tuningServices.push_back(tsc);
	}

	result &= xml.findElement(XmlElement::APPEARANCE);

	result &= xml.readBoolAttribute(EquipmentPropNames::SHOW_SIGNALS, &showSignals);
	result &= xml.readBoolAttribute(EquipmentPropNames::SHOW_SCHEMAS, &showSchemas);
	result &= xml.readBoolAttribute(EquipmentPropNames::SHOW_SCHEMAS_LIST, &showSchemasList);
	result &= xml.readBoolAttribute(EquipmentPropNames::SHOW_SCHEMAS_TABS, &showSchemasTabs);

	//
	// statusFlagFunction
	//

	int value = 0;
	bool resultStatusFlagFunction = xml.readIntAttribute(EquipmentPropNames::STATUS_FLAG_FUNCTION, &value);
	if (resultStatusFlagFunction == true)
	{
		statusFlagFunction = static_cast<LmStatusFlagMode>(value);
	}
	else
	{
		// Compatibility loading statusFlagFunction before 10.12.2020
		//
		statusFlagFunction = LmStatusFlagMode::None;

		bool showSOR = false;
		bool useAccessFlag = false;

		resultStatusFlagFunction = xml.readBoolAttribute(EquipmentPropNames::SHOW_SOR, &showSOR);
		resultStatusFlagFunction &= xml.readBoolAttribute(EquipmentPropNames::USE_ACCESS_FLAG, &useAccessFlag);

		if (resultStatusFlagFunction == true)
		{
			if (showSOR == true)
			{
				statusFlagFunction = LmStatusFlagMode::SOR;
			}
			else
			{
				if (useAccessFlag == true)
				{
					statusFlagFunction = LmStatusFlagMode::AccessKey;
				}
			}
		}
	}

	result &= resultStatusFlagFunction;

	//
	// applyMode
	//
	value = 0;
	bool resultApplyMode = xml.readIntAttribute(EquipmentPropNames::APPLY_MODE, &value);
	if (resultApplyMode == true)
	{
		applyMode = static_cast<ApplyMode>(value);
	}

	//

	result &= xml.readBoolAttribute(EquipmentPropNames::TUNING_LOGIN, &tuningLogin);
	result &= xml.readStringAttribute(EquipmentPropNames::TUNING_USER_ACCOUNTS, &tuningUserAccounts);
	result &= xml.readIntAttribute(EquipmentPropNames::TUNING_SESSION_TIMEOUT, &tuningSessionTimeout);
	result &= xml.readBoolAttribute(EquipmentPropNames::LOGIN_PER_OPERATION, &loginPerOperation);

	result &= xml.readStringListAttribute(EquipmentPropNames::APP_SIGNAL_LIST_IDS, &appSignalListIDs);
	result &= xml.readStringListAttribute(EquipmentPropNames::APP_SIGNAL_LIST_MASKS, &appSignalListMasks);
	result &= xml.readStringListAttribute(EquipmentPropNames::APP_SIGNAL_LIST_TAGS, &appSignalListTags);

	result &= xml.readBoolAttribute(EquipmentPropNames::FILTER_BY_EQUIPMENT, &filterByEquipment);
	result &= xml.readBoolAttribute(EquipmentPropNames::FILTER_BY_SCHEMA, &filterBySchema);

	result &= xml.readStringAttribute(EquipmentPropNames::START_SCHEMA_ID, &startSchemaID);

	result &= xml.findElement(EquipmentPropNames::SCHEMA_TAGS);

	result &= xml.readStringElement(EquipmentPropNames::SCHEMA_TAGS, &schemaTags);

	SoftwareSettings::setShortId<SoftwareEndpoint::TuningService>(&tuningServices);

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
	if (applyMode != src.applyMode ||
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
			applyMode != src.applyMode ||
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

// -------------------------------------------------------------------------------------
//
// TestSuiteSettings class implementation
//
// -------------------------------------------------------------------------------------

bool TestSuiteSettings::writeToXml(XmlWriteHelper& xml) const
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

	xml.writeStartElement(XmlElement::APP_DATA_SERVICES);
	xml.writeIntAttribute(XmlAttribute::COUNT, static_cast<int>(appDataServices.size()));

	for(const SoftwareEndpoint::AppDataService& ads : appDataServices)
	{
		xml.writeStartElement(XmlElement::APP_DATA_SERVICE);

		xml.writeStringAttribute(EquipmentPropNames::EQUIPMENT_ID, ads.equipmentId);
		xml.writeHostAddressPort(EquipmentPropNames::CLIENT_REQUEST_IP, EquipmentPropNames::CLIENT_REQUEST_PORT, ads.address);
		xml.writeHostAddressPort(EquipmentPropNames::RT_TRENDS_REQUEST_IP, EquipmentPropNames::RT_TRENDS_REQUEST_PORT, ads.realtimeAddress);

		xml.writeEndElement();		// </AppDataService>
	}

	xml.writeEndElement();		// </AppDataServices>

	//

	xml.writeStartElement(XmlElement::TUNING_SERVICES);
	xml.writeBoolAttribute(EquipmentPropNames::TUNING_ENABLE, tuningEnabled);
	xml.writeIntAttribute(XmlAttribute::COUNT, static_cast<int>(tuningServices.size()));

	for(const auto& tsc : tuningServices)
	{
		xml.writeStartElement(XmlElement::TUNING_SERVICE);

		xml.writeStringAttribute(EquipmentPropNames::EQUIPMENT_ID, tsc.equipmentId);
		xml.writeStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, tsc.address.addressStr());
		xml.writeIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, tsc.address.port());
		xml.writeStringListAttribute(XmlAttribute::DRIVEN_SOURCES, tsc.drivenSources);

		xml.writeEndElement();		// </TuningService>
	}

	xml.writeEndElement();		// </TuningServices>

	//

	// --
	//
	xml.writeStartElement(XmlElement::TESTING_SECURITY);

	xml.writeBoolAttribute(EquipmentPropNames::TESTING_LOGIN, login);
	xml.writeStringAttribute(EquipmentPropNames::TESTING_USER_ACCOUNTS, userAccounts);

	xml.writeEndElement();			// </TestSecurity>

	// --
	//
	xml.writeStartElement(XmlElement::TESTING_REPORTS);

	xml.writeStringAttribute(EquipmentPropNames::TESTING_PLANT, plant);
	xml.writeStringAttribute(EquipmentPropNames::TESTING_UNIT, unit);
	xml.writeStringAttribute(EquipmentPropNames::TESTING_SYSTEM, system);

	xml.writeEndElement();			// </TestingReports>

	// --
	//
	xml.writeStartElement(XmlElement::TESTING_SETTINGS);
	xml.writeStringAttribute(EquipmentPropNames::TESTING_SCRIPTTAGS, scriptTags);
	xml.writeEndElement();          // </TestingSettings>



	writeEndSettings(xml);;			// </Settings>

	return true;
}

bool TestSuiteSettings::readFromXml(XmlReadHelper& xml)
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

	result &= xml.findElement(XmlElement::APP_DATA_SERVICES);

	RETURN_IF_FALSE(result);

	int count = 0;

	result &= xml.readIntAttribute(XmlAttribute::COUNT, &count);

	appDataServices.clear();

	for(int i = 0; i < count; i++)
	{
		SoftwareEndpoint::AppDataService ads;

		result &= xml.findElement(XmlElement::APP_DATA_SERVICE);

		result &= xml.readStringAttribute(EquipmentPropNames::EQUIPMENT_ID, &ads.equipmentId);
		result &= xml.readHostAddressPort(EquipmentPropNames::CLIENT_REQUEST_IP,
										  EquipmentPropNames::CLIENT_REQUEST_PORT, &ads.address);
		result &= xml.readHostAddressPort(EquipmentPropNames::RT_TRENDS_REQUEST_IP,
										  EquipmentPropNames::RT_TRENDS_REQUEST_PORT, &ads.realtimeAddress);


		appDataServices.push_back(ads);
	}

	//

	result &= xml.findElement(XmlElement::TUNING_SERVICES);

	RETURN_IF_FALSE(result);

	result &= xml.readBoolAttribute(EquipmentPropNames::TUNING_ENABLE, &tuningEnabled);

	count = 0;

	result &= xml.readIntAttribute(XmlAttribute::COUNT, &count);

	tuningServices.clear();

	for(int i = 0; i < count; i++)
	{
		SoftwareEndpoint::TuningService tsc;

		result &= xml.findElement(XmlElement::TUNING_SERVICE);

		QString clientRequestAddress;
		int clientRequestPort = 0;

		result &= xml.readStringAttribute(EquipmentPropNames::EQUIPMENT_ID, &tsc.equipmentId);
		result &= xml.readStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, &clientRequestAddress);
		result &= xml.readIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, &clientRequestPort);
		result &= xml.readStringListAttribute(XmlAttribute::DRIVEN_SOURCES, &tsc.drivenSources);

		tsc.address = {clientRequestAddress, clientRequestPort};

		tuningServices.push_back(tsc);
	}

	result &= xml.findElement(XmlElement::TESTING_SECURITY);
	result &= xml.readBoolAttribute(EquipmentPropNames::TESTING_LOGIN, &login);
	result &= xml.readStringAttribute(EquipmentPropNames::TESTING_USER_ACCOUNTS, &userAccounts);

	result &= xml.findElement(XmlElement::TESTING_REPORTS);
	result &= xml.readStringAttribute(EquipmentPropNames::TESTING_PLANT, &plant);
	result &= xml.readStringAttribute(EquipmentPropNames::TESTING_UNIT, &unit);
	result &= xml.readStringAttribute(EquipmentPropNames::TESTING_SYSTEM, &system);

	result &= xml.findElement(XmlElement::TESTING_SETTINGS);
	result &= xml.readStringAttribute(EquipmentPropNames::TESTING_SCRIPTTAGS, &scriptTags);

	return result;
}

void TestSuiteSettings::clear()
{
	*this = TestSuiteSettings{};
}

QStringList TestSuiteSettings::getUsersAccounts() const
{
	return userAccounts.split(Separator::SEMICOLON, Qt::SkipEmptyParts);
}


// -------------------------------------------------------------------------------------
//
// GatewayServiceSettings class implementation
//
// -------------------------------------------------------------------------------------

bool GatewayServiceSettings::writeToXml(XmlWriteHelper& xml) const
{
	writeStartSettings(xml);

	//

	xml.writeStringElement(EquipmentPropNames::CFG_SERVICE_ID1,
						   cfgService1.equipmentId);
	xml.writeHostAddressPort(EquipmentPropNames::CFG_SERVICE_IP1,
							 EquipmentPropNames::CFG_SERVICE_PORT1,
							 cfgService1.address);

	xml.writeStringElement(EquipmentPropNames::CFG_SERVICE_ID2,
						   cfgService2.equipmentId);
	xml.writeHostAddressPort(EquipmentPropNames::CFG_SERVICE_IP2,
							 EquipmentPropNames::CFG_SERVICE_PORT2,
							 cfgService2.address);
	//

	xml.writeStringElement(EquipmentPropNames::APP_DATA_SERVICE_ID1,
						   appDataService1.equipmentId);
	xml.writeHostAddressPort(EquipmentPropNames::APP_DATA_SERVICE_IP1,
							 EquipmentPropNames::APP_DATA_SERVICE_PORT1,
							 appDataService1.address);

	xml.writeStringElement(EquipmentPropNames::APP_DATA_SERVICE_ID2,
						   appDataService2.equipmentId);
	xml.writeHostAddressPort(EquipmentPropNames::APP_DATA_SERVICE_IP2,
							 EquipmentPropNames::APP_DATA_SERVICE_PORT2,
							 appDataService2.address);
	//

	xml.writeStringElement(EquipmentPropNames::TUNING_SERVICE_ID1,
						   tuningService1.equipmentId);
	xml.writeHostAddressPort(EquipmentPropNames::TUNING_SERVICE_IP1,
							 EquipmentPropNames::TUNING_SERVICE_PORT1,
							 tuningService1.address);

	xml.writeStringElement(EquipmentPropNames::TUNING_SERVICE_ID2,
						   tuningService2.equipmentId);
	xml.writeHostAddressPort(EquipmentPropNames::TUNING_SERVICE_IP2,
							 EquipmentPropNames::TUNING_SERVICE_PORT2,
							 tuningService2.address);

	//

	writeEndSettings(xml);

	return true;
}

bool GatewayServiceSettings::readFromXml(XmlReadHelper& xml)
{
	bool result = true;

	result = startSettingsReading(xml);

	RETURN_IF_FALSE(result);

	//

	bool okCfg1 = true;

	okCfg1 &= xml.readStringElement(EquipmentPropNames::CFG_SERVICE_ID1, &cfgService1.equipmentId, true);
	okCfg1 &= xml.readHostAddressPort(EquipmentPropNames::CFG_SERVICE_IP1,
									  EquipmentPropNames::CFG_SERVICE_PORT1,
									  &cfgService1.address);

	bool okCfg2 = true;

	okCfg2 &= xml.readStringElement(EquipmentPropNames::CFG_SERVICE_ID2, &cfgService2.equipmentId, true);
	okCfg2 &= xml.readHostAddressPort(EquipmentPropNames::CFG_SERVICE_IP2,
									  EquipmentPropNames::CFG_SERVICE_PORT2,
									  &cfgService2.address);

	result &= okCfg1 || okCfg2;

	//

	bool res = true;

	bool okApp1 = xml.readStringElement(EquipmentPropNames::APP_DATA_SERVICE_ID1, &appDataService1.equipmentId, true);

	res = xml.readHostAddressPort(EquipmentPropNames::APP_DATA_SERVICE_IP1,
								  EquipmentPropNames::APP_DATA_SERVICE_PORT1,
								  &appDataService1.address);

	okApp1 = okApp1 && (appDataService1.equipmentId.isEmpty() || res);

	bool okApp2 = xml.readStringElement(EquipmentPropNames::APP_DATA_SERVICE_ID2, &appDataService2.equipmentId, true);

	res = xml.readHostAddressPort(EquipmentPropNames::APP_DATA_SERVICE_IP2,
								  EquipmentPropNames::APP_DATA_SERVICE_PORT2,
								  &appDataService2.address);

	okApp2 = okApp2 && (appDataService2.equipmentId.isEmpty() || res);

	result &= okApp1 || okApp2;

	//

	bool okTun1 = xml.readStringElement(EquipmentPropNames::TUNING_SERVICE_ID1, &tuningService1.equipmentId, true);

	res = xml.readHostAddressPort(EquipmentPropNames::TUNING_SERVICE_IP1,
								  EquipmentPropNames::TUNING_SERVICE_PORT1,
								  &tuningService1.address);

	okTun1 = okTun1 && (tuningService1.equipmentId.isEmpty() || res);

	bool okTun2 = xml.readStringElement(EquipmentPropNames::TUNING_SERVICE_ID2, &tuningService2.equipmentId, true);

	res = xml.readHostAddressPort(EquipmentPropNames::TUNING_SERVICE_IP2,
										  EquipmentPropNames::TUNING_SERVICE_PORT2,
										  &tuningService2.address);

	okTun2 = okTun2 && (tuningService2.equipmentId.isEmpty() || res);

	result &= okTun1 || okTun2;

	return result;
}

