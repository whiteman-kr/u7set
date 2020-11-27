#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QObject>

#include "SoftwareSettings.h"
#include "WUtils.h"

// -------------------------------------------------------------------------------------
//
// ServiceSettings class implementation
//
// -------------------------------------------------------------------------------------

SoftwareSettings::SoftwareSettings(const SoftwareSettings&)
{
}

SoftwareSettings::~SoftwareSettings()
{
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

#endif


// -------------------------------------------------------------------------------------
//
// CfgServiceSettings class implementation
//
// -------------------------------------------------------------------------------------

bool CfgServiceSettings::writeToXml(XmlWriteHelper& xml)
{
	xml.writeStartElement(XmlElement::SETTINGS);

	xml.writeHostAddressPort(EquipmentPropNames::CLIENT_REQUEST_IP,
							 EquipmentPropNames::CLIENT_REQUEST_PORT, clientRequestIP);

	xml.writeHostAddress(EquipmentPropNames::CLIENT_REQUEST_NETMASK, clientRequestNetmask);

	xml.writeEndElement();	// </Settings>

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

	return true;
}

bool CfgServiceSettings::readFromXml(XmlReadHelper& xml)
{
	clients.clear();

	bool result = false;

	result = xml.findElement(XmlElement::SETTINGS);

	if (result == false)
	{
		return false;
	}

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

	bool CfgServiceSettingsGetter::readFromDevice(const Hardware::EquipmentSet* equipment,
											const Hardware::Software* software,
											Builder::IssueLogger* log)
	{
		TEST_PTR_RETURN_FALSE(log);
		TEST_PTR_LOG_RETURN_FALSE(equipment, log);
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
	xml.writeStartElement(XmlElement::SETTINGS);

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

	xml.writeEndElement();	// </Settings>

	return true;
}

bool AppDataServiceSettings::readFromXml(XmlReadHelper& xml)
{
	bool result = false;

	result = xml.findElement(XmlElement::SETTINGS);

	if (result == false)
	{
		return false;
	}

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

	bool AppDataServiceSettingsGetter::readFromDevice(const Hardware::EquipmentSet* equipment,
												const Hardware::Software* software,
												Builder::IssueLogger* log)
	{
		TEST_PTR_RETURN_FALSE(log);
		TEST_PTR_LOG_RETURN_FALSE(equipment, log);
		TEST_PTR_LOG_RETURN_FALSE(software, log);

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
	xml.writeStartElement(XmlElement::SETTINGS);

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

	xml.writeEndElement();	// </Settings>

	return true;
}

bool DiagDataServiceSettings::readFromXml(XmlReadHelper& xml)
{
	bool result = false;

	result = xml.findElement(XmlElement::SETTINGS);

	if (result == false)
	{
		return false;
	}

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

	bool DiagDataServiceSettingsGetter::readFromDevice(const Hardware::EquipmentSet* equipment,
												 const Hardware::Software* software,
												 Builder::IssueLogger* log)
	{
		TEST_PTR_RETURN_FALSE(log);
		TEST_PTR_LOG_RETURN_FALSE(equipment, log);
		TEST_PTR_LOG_RETURN_FALSE(software, log);

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
	xml.writeStartElement(XmlElement::SETTINGS);

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

	xml.writeEndElement();	// </Settings>

	// write tuning clients info
	//
	xml.writeStartElement(XmlElement::TUNING_CLIENTS);
	xml.writeIntAttribute(XmlAttribute::COUNT, clients.count());

	for(int i = 0; i < clients.count(); i++)
	{
		TuningClient& tc = clients[i];

		xml.writeStartElement(XmlElement::TUNING_CLIENT);
		xml.writeStringAttribute(EquipmentPropNames::EQUIPMENT_ID, tc.equipmentID);

		xml.writeStartElement(XmlElement::TUNING_SOURCES);
		xml.writeIntAttribute(XmlAttribute::COUNT, tc.sourcesIDs.count());

		for(QString& sourceID : tc.sourcesIDs)
		{
			xml.writeStartElement(XmlElement::TUNING_SOURCE);
			xml.writeStringAttribute(EquipmentPropNames::EQUIPMENT_ID, sourceID);

			xml.writeEndElement();	// TUNING_SOURCE
		}

		xml.writeEndElement();		// TUNING_SOURCES

		xml.writeEndElement();		// TUNING_CLIENT
	}

	xml.writeEndElement();			// TUNING_CLIENTS

	return true;
}

bool TuningServiceSettings::readFromXml(XmlReadHelper& xml)
{
	bool result = false;

	result = xml.findElement(XmlElement::SETTINGS);

	RETURN_IF_FALSE(result);

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

		result = xml.findElement(XmlElement::TUNING_CLIENT);

		if (result == false)
		{
			break;
		}

		result = xml.readStringAttribute(EquipmentPropNames::EQUIPMENT_ID, &tc.equipmentID);

		if (result == false)
		{
			break;
		}

		result = xml.findElement(XmlElement::TUNING_SOURCES);

		if (result == false)
		{
			break;
		}

		int sourcesCount = 0;

		result = xml.readIntAttribute(XmlAttribute::COUNT, &sourcesCount);

		if (result == false)
		{
			break;
		}

		for(int s = 0; s < sourcesCount; s++)
		{
			result = xml.findElement(XmlElement::TUNING_SOURCE);

			if (result == false)
			{
				break;
			}

			QString sourceID;

			result = xml.readStringAttribute(EquipmentPropNames::EQUIPMENT_ID, &sourceID);

			if (result == false)
			{
				break;
			}

			sourceID = sourceID.trimmed();

			tc.sourcesIDs.append(sourceID);
		}

		if (result == false)
		{
			break;
		}

		clients.append(tc);
	}

	return result;
}

#ifdef IS_BUILDER

	// -------------------------------------------------------------------------------------
	//
	// TuningServiceSettingsGetter class implementation
	//
	// -------------------------------------------------------------------------------------

	bool TuningServiceSettingsGetter::readFromDevice(const Hardware::EquipmentSet* equipment,
											   const Hardware::Software* software,
											   Builder::IssueLogger* log)
	{
		TEST_PTR_RETURN_FALSE(log);
		TEST_PTR_LOG_RETURN_FALSE(equipment, log);
		TEST_PTR_LOG_RETURN_FALSE(software, log);

		bool result = true;

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

		result &= fillTuningClientsInfo(software, singleLmControl, log);

		return result;
	}

	bool TuningServiceSettingsGetter::fillTuningClientsInfo(const Hardware::Software* software, bool singleLmControlEnabled, Builder::IssueLogger* log)
	{
		clients.clear();

		if (software == nullptr)
		{
			assert(false);
			return false;
		}

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

				result &= DeviceHelper::getStrProperty(tuningClient, "TuningServiceID", &tuningServiceID, log);

				if (result == false)
				{
					return;
				}

				if (tuningServiceID != software->equipmentIdTemplate())
				{
					return;
				}

				bool tuningEnable = true;			// by default tuning is enabled for known clients without property "TuningEnable"

				if (DeviceHelper::isPropertyExists(tuningClient, "TuningEnable") == true)
				{
					result &= DeviceHelper::getBoolProperty(tuningClient, "TuningEnable", &tuningEnable, log);

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

				this->clients.append(tc);
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
	xml.writeStartElement(XmlElement::SETTINGS);

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

	xml.writeEndElement();	// </Settings>

	return true;
}

bool ArchivingServiceSettings::readFromXml(XmlReadHelper& xml)
{
	bool result = false;

	result = xml.findElement(XmlElement::SETTINGS);

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

const ArchivingServiceSettings& ArchivingServiceSettings::operator = (const ArchivingServiceSettings& src)
{
	clientRequestIP = src.clientRequestIP;
	clientRequestNetmask = src.clientRequestNetmask;

	appDataReceivingIP = src.appDataReceivingIP;
	appDataReceivingNetmask = src.appDataReceivingNetmask;

	diagDataReceivingIP = src.diagDataReceivingIP;
	diagDataReceivingNetmask = src.diagDataReceivingNetmask;

	shortTermArchivePeriod = src.shortTermArchivePeriod;
	longTermArchivePeriod = src.longTermArchivePeriod;

	archiveLocation = src.archiveLocation;

	return *this;
}

#ifdef IS_BUILDER

	// -------------------------------------------------------------------------------------
	//
	// ArchivingServiceSettingsGetter class implementation
	//
	// -------------------------------------------------------------------------------------

	bool ArchivingServiceSettingsGetter::readFromDevice(const Hardware::EquipmentSet* equipment,
												  const Hardware::Software* software,
												  Builder::IssueLogger* log)
	{
		TEST_PTR_RETURN_FALSE(log);
		TEST_PTR_LOG_RETURN_FALSE(equipment, log);
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

/*const char* TestClientSettings::CFG_SERVICE1_SECTION = "CfgService1";
const char* TestClientSettings::CFG_SERVICE2_SECTION = "CfgService2";
const char* TestClientSettings::APP_DATA_SERVICE_SECTION = "AppDataService";
const char* TestClientSettings::DIAG_DATA_SERVICE_SECTION = "DiagDataService";
const char* TestClientSettings::ARCH_SERVICE_SECTION = "ArchService";
const char* TestClientSettings::TUNING_SERVICE_SECTION = "TuningService";*/

bool TestClientSettings::writeToXml(XmlWriteHelper& xml)
{
	xml.writeStartElement(XmlElement::SETTINGS);

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

	xml.writeEndElement();	// </Settings>

	return true;
}

bool TestClientSettings::readFromXml(XmlReadHelper& xml)
{
	bool result = true;

	result &= xml.findElement(XmlElement::SETTINGS);

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

	bool TestClientSettingsGetter::readFromDevice(const Hardware::EquipmentSet* equipment,
											const Hardware::Software* software,
											Builder::IssueLogger* log)
	{
		TEST_PTR_RETURN_FALSE(log);
		TEST_PTR_LOG_RETURN_FALSE(equipment, log);
		TEST_PTR_LOG_RETURN_FALSE(software, log);

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

bool MetrologySettings::writeToXml(XmlWriteHelper& xmlWriter)
{
	xmlWriter.writeStartElement(XmlElement::SETTINGS);

	xmlWriter.writeStartElement(XmlElement::APP_DATA_SERVICE);

		xmlWriter.writeBoolAttribute(XmlAttribute::APP_DATA_SERVICE_PROPERTY_IS_VALID1, appDataServicePropertyIsValid1);
		xmlWriter.writeStringAttribute(EquipmentPropNames::APP_DATA_SERVICE_ID1, appDataServiceID1);
		xmlWriter.writeStringAttribute(XmlAttribute::APP_DATA_SERVICE_IP1, appDataServiceIP1);
		xmlWriter.writeIntAttribute(XmlAttribute::APP_DATA_SERVICE_PORT1, appDataServicePort1);

		xmlWriter.writeBoolAttribute(XmlAttribute::APP_DATA_SERVICE_PROPERTY_IS_VALID2, appDataServicePropertyIsValid2);
		xmlWriter.writeStringAttribute(EquipmentPropNames::APP_DATA_SERVICE_ID2, appDataServiceID2);
		xmlWriter.writeStringAttribute(XmlAttribute::APP_DATA_SERVICE_IP2, appDataServiceIP2);
		xmlWriter.writeIntAttribute(XmlAttribute::APP_DATA_SERVICE_PORT2, appDataServicePort2);

	xmlWriter.writeEndElement();		// </AppDataService>

	xmlWriter.writeStartElement(XmlElement::TUNING_SERVICE);

		xmlWriter.writeBoolAttribute(XmlAttribute::TUNING_SERVICE_PROPERTY_IS_VALID, tuningServicePropertyIsValid);
		xmlWriter.writeStringAttribute(XmlAttribute::SOFTWARE_METROLOGY_ID, softwareMetrologyID);
		xmlWriter.writeStringAttribute(XmlAttribute::TUNING_SERVICE_IP, tuningServiceIP);
		xmlWriter.writeIntAttribute(XmlAttribute::TUNING_SERVICE_PORT, tuningServicePort);

	xmlWriter.writeEndElement();		// </TuningService>

	xmlWriter.writeEndElement();		// </Settings>

	return true;
}

bool MetrologySettings::readFromXml(XmlReadHelper& xmlReader)
{
	bool result = true;

	result &= xmlReader.findElement(XmlElement::SETTINGS);

	// for compatibility current settings and old settings
	// read current settings but if attributeRead == false, then try read old settings
	//
	bool attributeRead = true;

	// AppDataService
	//
	result &= xmlReader.findElement(XmlElement::APP_DATA_SERVICE);

		// primary
		//
	attributeRead = xmlReader.readBoolAttribute(XmlAttribute::APP_DATA_SERVICE_PROPERTY_IS_VALID1, &appDataServicePropertyIsValid1);
	if (attributeRead == false)
	{
		result &= xmlReader.readBoolAttribute("PropertyIsValid1", &appDataServicePropertyIsValid1);
	}

	result &= xmlReader.readStringAttribute(EquipmentPropNames::APP_DATA_SERVICE_ID1, &appDataServiceID1);

	attributeRead = xmlReader.readStringAttribute(XmlAttribute::APP_DATA_SERVICE_IP1, &appDataServiceIP1);
	if (attributeRead == false)
	{
		result &= xmlReader.readStringAttribute("ip1", &appDataServiceIP1);
	}

	attributeRead = xmlReader.readIntAttribute(XmlAttribute::APP_DATA_SERVICE_PORT1, &appDataServicePort1);
	if (attributeRead == false)
	{
		result &= xmlReader.readIntAttribute("port1", &appDataServicePort1);
	}

		// reserve
		//
	attributeRead = xmlReader.readBoolAttribute(XmlAttribute::APP_DATA_SERVICE_PROPERTY_IS_VALID2, &appDataServicePropertyIsValid2);
	if (attributeRead == false)
	{
		result &= xmlReader.readBoolAttribute("PropertyIsValid2", &appDataServicePropertyIsValid2);
	}

	result &= xmlReader.readStringAttribute(EquipmentPropNames::APP_DATA_SERVICE_ID2, &appDataServiceID2);

	attributeRead = xmlReader.readStringAttribute(XmlAttribute::APP_DATA_SERVICE_IP2, &appDataServiceIP2);
	if (attributeRead == false)
	{
		result &= xmlReader.readStringAttribute("ip2", &appDataServiceIP2);
	}

	attributeRead = xmlReader.readIntAttribute(XmlAttribute::APP_DATA_SERVICE_PORT2, &appDataServicePort2);
	if (attributeRead == false)
	{
		result &= xmlReader.readIntAttribute("port2", &appDataServicePort2);
	}

	// TuningService
	//
	result &= xmlReader.findElement(XmlElement::TUNING_SERVICE);

	attributeRead = xmlReader.readBoolAttribute(XmlAttribute::TUNING_SERVICE_PROPERTY_IS_VALID, &tuningServicePropertyIsValid);
	if (attributeRead == false)
	{
		result &= xmlReader.readBoolAttribute("PropertyIsValid", &tuningServicePropertyIsValid);
	}

	result &= xmlReader.readStringAttribute(XmlAttribute::SOFTWARE_METROLOGY_ID, &softwareMetrologyID);

	attributeRead = xmlReader.readStringAttribute(XmlAttribute::TUNING_SERVICE_IP, &tuningServiceIP);
	if (attributeRead == false)
	{
		result &= xmlReader.readStringAttribute("ip", &tuningServiceIP);
	}

	attributeRead = xmlReader.readIntAttribute(XmlAttribute::TUNING_SERVICE_PORT, &tuningServicePort);
	if (attributeRead == false)
	{
		result &= xmlReader.readIntAttribute("port", &tuningServicePort);
	}

	return result;
}

#ifdef IS_BUILDER

	// -------------------------------------------------------------------------------------
	//
	// MetrologySettings class implementation
	//
	// -------------------------------------------------------------------------------------


	bool MetrologySettingsGetter::readFromDevice(const Hardware::EquipmentSet* equipment,
											const Hardware::Software* software,
											Builder::IssueLogger* log)
	{
		TEST_PTR_RETURN_FALSE(log);
		TEST_PTR_LOG_RETURN_FALSE(equipment, log);
		TEST_PTR_LOG_RETURN_FALSE(software, log);

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

bool MonitorSettings::writeToXml(XmlWriteHelper& xmlWriter)
{
	xmlWriter.writeStartElement(XmlElement::SETTINGS);

	//

	xmlWriter.writeStringElement(EquipmentPropNames::START_SCHEMA_ID, startSchemaId);
	xmlWriter.writeStringElement(EquipmentPropNames::SCHEMA_TAGS, schemaTags);

	//

	xmlWriter.writeStartElement(XmlElement::APP_DATA_SERVICE1);

	xmlWriter.writeStringAttribute(EquipmentPropNames::EQUIPMENT_ID, appDataServiceID1);
	xmlWriter.writeStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, appDataServiceIP1);
	xmlWriter.writeIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, appDataServicePort1);
	xmlWriter.writeStringAttribute(EquipmentPropNames::RT_TRENDS_REQUEST_IP, realtimeDataIP1);
	xmlWriter.writeIntAttribute(EquipmentPropNames::RT_TRENDS_REQUEST_PORT, realtimeDataPort1);

	xmlWriter.writeEndElement();			// </AppDataService1>

	//

	xmlWriter.writeStartElement(XmlElement::APP_DATA_SERVICE2);

	xmlWriter.writeStringAttribute(EquipmentPropNames::EQUIPMENT_ID, appDataServiceID2);
	xmlWriter.writeStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, appDataServiceIP2);
	xmlWriter.writeIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, appDataServicePort2);
	xmlWriter.writeStringAttribute(EquipmentPropNames::RT_TRENDS_REQUEST_IP, realtimeDataIP2);
	xmlWriter.writeIntAttribute(EquipmentPropNames::RT_TRENDS_REQUEST_PORT, realtimeDataPort2);

	xmlWriter.writeEndElement();			// </AppDataService2>

	//

	xmlWriter.writeStartElement(XmlElement::ARCHIVE_SERVICE1);

	xmlWriter.writeStringAttribute(EquipmentPropNames::EQUIPMENT_ID, archiveServiceID1);
	xmlWriter.writeStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, archiveServiceIP1);
	xmlWriter.writeIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, archiveServicePort1);

	xmlWriter.writeEndElement();			// </ArchiveService1>

	//

	xmlWriter.writeStartElement(XmlElement::ARCHIVE_SERVICE2);

	xmlWriter.writeStringAttribute(EquipmentPropNames::EQUIPMENT_ID, archiveServiceID2);
	xmlWriter.writeStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, archiveServiceIP2);
	xmlWriter.writeIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, archiveServicePort2);

	xmlWriter.writeEndElement();			// </ArchiveService2>

	//

	xmlWriter.writeStartElement(XmlElement::TUNING_SERVICE);

	xmlWriter.writeBoolAttribute(EquipmentPropNames::TUNING_ENABLE, tuningEnabled);
	xmlWriter.writeStringAttribute(EquipmentPropNames::EQUIPMENT_ID, tuningServiceID);
	xmlWriter.writeStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, tuningServiceIP);
	xmlWriter.writeIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, tuningServicePort);

	xmlWriter.writeStringElement(EquipmentPropNames::TUNING_SOURCE_EQUIPMENT_ID, tuningSources);

	xmlWriter.writeEndElement();			// </TuningService>

	//

	xmlWriter.writeEndElement();			// </Settings>

	return true;
}

bool MonitorSettings::readFromXml(XmlReadHelper& xmlReader)
{
	clear();

	bool result = true;

	result &= xmlReader.findElement(XmlElement::SETTINGS);

	//

	result &= xmlReader.findElement(EquipmentPropNames::START_SCHEMA_ID);
	result &= xmlReader.readStringElement(EquipmentPropNames::START_SCHEMA_ID, &startSchemaId);

	result &= xmlReader.findElement(EquipmentPropNames::SCHEMA_TAGS);
	result &= xmlReader.readStringElement(EquipmentPropNames::SCHEMA_TAGS, &schemaTags);

	//

	result &= xmlReader.findElement(XmlElement::APP_DATA_SERVICE1);
	result &= xmlReader.readStringAttribute(EquipmentPropNames::EQUIPMENT_ID, &appDataServiceID1);
	result &= xmlReader.readStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, &appDataServiceIP1);
	result &= xmlReader.readIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, &appDataServicePort1);
	result &= xmlReader.readStringAttribute(EquipmentPropNames::RT_TRENDS_REQUEST_IP, &realtimeDataIP1);
	result &= xmlReader.readIntAttribute(EquipmentPropNames::RT_TRENDS_REQUEST_PORT, &realtimeDataPort1);

	//

	result &= xmlReader.findElement(XmlElement::APP_DATA_SERVICE2);
	result &= xmlReader.readStringAttribute(EquipmentPropNames::EQUIPMENT_ID, &appDataServiceID2);
	result &= xmlReader.readStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, &appDataServiceIP2);
	result &= xmlReader.readIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, &appDataServicePort2);
	result &= xmlReader.readStringAttribute(EquipmentPropNames::RT_TRENDS_REQUEST_IP, &realtimeDataIP2);
	result &= xmlReader.readIntAttribute(EquipmentPropNames::RT_TRENDS_REQUEST_PORT, &realtimeDataPort2);

	//

	result &= xmlReader.findElement(XmlElement::ARCHIVE_SERVICE1);
	result &= xmlReader.readStringAttribute(EquipmentPropNames::EQUIPMENT_ID, &archiveServiceID1);
	result &= xmlReader.readStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, &archiveServiceIP1);
	result &= xmlReader.readIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, &archiveServicePort1);

	//

	result &=  xmlReader.findElement(XmlElement::ARCHIVE_SERVICE2);
	result &= xmlReader.readStringAttribute(EquipmentPropNames::EQUIPMENT_ID, &archiveServiceID2);
	result &= xmlReader.readStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, &archiveServiceIP2);
	result &= xmlReader.readIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, &archiveServicePort2);

	//

	result &= xmlReader.findElement(XmlElement::TUNING_SERVICE);
	result &= xmlReader.readBoolAttribute(EquipmentPropNames::TUNING_ENABLE, &tuningEnabled);
	result &= xmlReader.readStringAttribute(EquipmentPropNames::EQUIPMENT_ID, &tuningServiceID);
	result &= xmlReader.readStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, &tuningServiceIP);
	result &= xmlReader.readIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, &tuningServicePort);

	result &= xmlReader.findElement(EquipmentPropNames::TUNING_SOURCE_EQUIPMENT_ID);
	result &= xmlReader.readStringElement(EquipmentPropNames::TUNING_SOURCE_EQUIPMENT_ID, &tuningSources);

	return result;
}

QStringList MonitorSettings::getSchemaTags() const
{
	return  schemaTags.split(Separator::SEMICOLON);
}

QStringList MonitorSettings::getTuningSources() const
{
	return  tuningSources.split(Separator::SEMICOLON);
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

	bool MonitorSettingsGetter::readFromDevice(const Hardware::EquipmentSet* equipment,
										const Hardware::Software* software,
										Builder::IssueLogger* log)
	{
		clear();

		TEST_PTR_RETURN_FALSE(log);
		TEST_PTR_LOG_RETURN_FALSE(equipment, log);
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

		result = readAppDataServiceAndArchiveSettings(equipment, software, log);

		RETURN_IF_FALSE(result);

		result = readTuningSettings(equipment, software, log);

		return result;
	}

	bool MonitorSettingsGetter::readAppDataServiceAndArchiveSettings(const Hardware::EquipmentSet* equipment,
												   const Hardware::Software* software,
												   Builder::IssueLogger* log)
	{
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

			result &= adsSettings1.readFromDevice(equipment, appDataService1, log);

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

			result &= adsSettings2.readFromDevice(equipment, appDataService2, log);

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

	bool MonitorSettingsGetter::readTuningSettings(	const Hardware::EquipmentSet* equipment,
												const Hardware::Software* software,
												Builder::IssueLogger* log)
	{
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

bool TuningClientSettings::writeToXml(XmlWriteHelper& xmlWriter)
{
	xmlWriter.writeStartElement(XmlElement::SETTINGS);

	xmlWriter.writeStartElement(XmlElement::TUNING_SERVICE);

	xmlWriter.writeStringAttribute(EquipmentPropNames::EQUIPMENT_ID, tuningServiceID);
	xmlWriter.writeStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, tuningServiceIP);
	xmlWriter.writeIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, tuningServicePort);

	xmlWriter.writeEndElement();		// </TuningService>

	xmlWriter.writeStartElement(XmlElement::APPEARANCE);

	xmlWriter.writeBoolAttribute(EquipmentPropNames::AUTO_APPLAY, autoApply);
	xmlWriter.writeBoolAttribute(EquipmentPropNames::SHOW_SIGNALS, showSignals);
	xmlWriter.writeBoolAttribute(EquipmentPropNames::SHOW_SCHEMAS, showSchemas);
	xmlWriter.writeBoolAttribute(EquipmentPropNames::SHOW_SCHEMAS_LIST, showSchemasList);
	xmlWriter.writeBoolAttribute(EquipmentPropNames::SHOW_SCHEMAS_TABS, showSchemasTabs);
	xmlWriter.writeBoolAttribute(EquipmentPropNames::SHOW_SOR, showSOR);
	xmlWriter.writeBoolAttribute(EquipmentPropNames::USE_ACCESS_FLAG, useAccessFlag);
	xmlWriter.writeBoolAttribute(EquipmentPropNames::LOGIN_PER_OPERATION, loginPerOperation);
	xmlWriter.writeStringAttribute(EquipmentPropNames::USER_ACCOUNTS, usersAccounts);
	xmlWriter.writeIntAttribute(EquipmentPropNames::LOGIN_SESSION_LENGTH, loginSessionLength);

	xmlWriter.writeBoolAttribute(EquipmentPropNames::FILTER_BY_EQUIPMENT, filterByEquipment);
	xmlWriter.writeBoolAttribute(EquipmentPropNames::FILTER_BY_SCHEMA, filterBySchema);

	xmlWriter.writeStringAttribute(EquipmentPropNames::START_SCHEMA_ID, startSchemaID);

	xmlWriter.writeEndElement();		// </Appearance>

	xmlWriter.writeStringElement(EquipmentPropNames::SCHEMA_TAGS, schemaTags);

	xmlWriter.writeEndElement();		// </Settings>

	return true;
}

bool TuningClientSettings::readFromXml(XmlReadHelper& xmlReader)
{
	bool result = true;

	result &= xmlReader.findElement(XmlElement::SETTINGS);

	result &= xmlReader.findElement(XmlElement::TUNING_SERVICE);

	result &= xmlReader.readStringAttribute(EquipmentPropNames::EQUIPMENT_ID, &tuningServiceID);
	result &= xmlReader.readStringAttribute(EquipmentPropNames::CLIENT_REQUEST_IP, &tuningServiceIP);
	result &= xmlReader.readIntAttribute(EquipmentPropNames::CLIENT_REQUEST_PORT, &tuningServicePort);

	result &= xmlReader.findElement(XmlElement::APPEARANCE);

	result &= xmlReader.readBoolAttribute(EquipmentPropNames::AUTO_APPLAY, &autoApply);
	result &= xmlReader.readBoolAttribute(EquipmentPropNames::SHOW_SIGNALS, &showSignals);
	result &= xmlReader.readBoolAttribute(EquipmentPropNames::SHOW_SCHEMAS, &showSchemas);
	result &= xmlReader.readBoolAttribute(EquipmentPropNames::SHOW_SCHEMAS_LIST, &showSchemasList);
	result &= xmlReader.readBoolAttribute(EquipmentPropNames::SHOW_SCHEMAS_TABS, &showSchemasTabs);
	result &= xmlReader.readBoolAttribute(EquipmentPropNames::SHOW_SOR, &showSOR);
	result &= xmlReader.readBoolAttribute(EquipmentPropNames::USE_ACCESS_FLAG, &useAccessFlag);
	result &= xmlReader.readBoolAttribute(EquipmentPropNames::LOGIN_PER_OPERATION, &loginPerOperation);
	result &= xmlReader.readStringAttribute(EquipmentPropNames::USER_ACCOUNTS, &usersAccounts);
	result &= xmlReader.readIntAttribute(EquipmentPropNames::LOGIN_SESSION_LENGTH, &loginSessionLength);

	result &= xmlReader.readBoolAttribute(EquipmentPropNames::FILTER_BY_EQUIPMENT, &filterByEquipment);
	result &= xmlReader.readBoolAttribute(EquipmentPropNames::FILTER_BY_SCHEMA, &filterBySchema);

	result &= xmlReader.readStringAttribute(EquipmentPropNames::START_SCHEMA_ID, &startSchemaID);

	result &= xmlReader.findElement(EquipmentPropNames::SCHEMA_TAGS);

	result &= xmlReader.readStringElement(EquipmentPropNames::SCHEMA_TAGS, &schemaTags);

	return result;
}

QStringList TuningClientSettings::getSchemaTags() const
{
	return  schemaTags.split(Separator::SEMICOLON);
}

QStringList TuningClientSettings::getUsersAccounts() const
{
	return  usersAccounts.split(Separator::SEMICOLON);
}

#ifdef IS_BUILDER

	// -------------------------------------------------------------------------------------
	//
	// TuningClientSettingsGetter class implementation
	//
	// -------------------------------------------------------------------------------------


	bool TuningClientSettingsGetter::readFromDevice(	const Hardware::EquipmentSet* equipment,
												const Hardware::Software* software,
												Builder::IssueLogger* log)
	{
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
		showSOR = false;
		useAccessFlag = false;

		int statusFlagFunction = 0;

		result &= DeviceHelper::getIntProperty(software, EquipmentPropNames::STATUS_FLAG_FUNCTION, &statusFlagFunction, log);

		RETURN_IF_FALSE(result);

		switch (statusFlagFunction)
		{
		case 0:
			break;
		case 1:
			showSOR = true;
			break;
		case 2:
			useAccessFlag = true;
			break;
		default:
			Q_ASSERT(false);
		}

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
