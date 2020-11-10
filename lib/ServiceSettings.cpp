#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include "../lib/ServiceSettings.h"
#include "../lib/WUtils.h"

// -------------------------------------------------------------------------------------
//
// ServiceSettings class implementation
//
// -------------------------------------------------------------------------------------

const char* ServiceSettings::SETTINGS_SECTION = "Settings";

const char* ServiceSettings::ATTR_COUNT = "Count";
const char* ServiceSettings::ATTR_EQUIIPMENT_ID = "EquipmentID";
const char* ServiceSettings::ATTR_SOFTWARE_TYPE = "SoftwareType";

bool ServiceSettings::getSoftwareConnection(const Hardware::EquipmentSet* equipment,
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

bool ServiceSettings::getCfgServiceConnection(	const Hardware::EquipmentSet *equipment,
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

// -------------------------------------------------------------------------------------
//
// CfgServiceSettings class implementation
//
// -------------------------------------------------------------------------------------

const char* CfgServiceSettings::CLIENTS_SECTION = "Clients";
const char* CfgServiceSettings::CLIENT = "Client";
const char* CfgServiceSettings::CLIENT_EQUIPMENT_ID = "EquipmentID";
const char* CfgServiceSettings::CLIENT_SOFTWARE_TYPE = "SoftwareType";

bool CfgServiceSettings::readFromDevice(Hardware::Software* software, Builder::IssueLogger* log)
{
	bool result = true;

	result &= DeviceHelper::getIpPortProperty(software, EquipmentPropNames::CLIENT_REQUEST_IP,
											  EquipmentPropNames::CLIENT_REQUEST_PORT, &clientRequestIP, false, "", 0, log);
	result &= DeviceHelper::getIPv4Property(software, EquipmentPropNames::CLIENT_REQUEST_NETMASK, &clientRequestNetmask, false, "", log);

	return result;
}

bool CfgServiceSettings::writeToXml(XmlWriteHelper& xml)
{
	xml.writeStartElement(SETTINGS_SECTION);

	xml.writeHostAddressPort(EquipmentPropNames::CLIENT_REQUEST_IP,
							 EquipmentPropNames::CLIENT_REQUEST_PORT, clientRequestIP);

	xml.writeHostAddress(EquipmentPropNames::CLIENT_REQUEST_NETMASK, clientRequestNetmask);

	xml.writeEndElement();	// </Settings>

	xml.writeStartElement(CLIENTS_SECTION);
	xml.writeIntAttribute(ATTR_COUNT, clients.count());

	for(const QPair<QString, E::SoftwareType>& pair : clients)
	{
		xml.writeStartElement(CLIENT);

		xml.writeStringAttribute(ATTR_EQUIIPMENT_ID, pair.first);
		xml.writeStringAttribute(ATTR_SOFTWARE_TYPE, E::valueToString(pair.second));

		xml.writeEndElement();	// </Client>
	}

	xml.writeEndElement();	// </Clients>

	return true;
}

bool CfgServiceSettings::readFromXml(XmlReadHelper& xml)
{
	clients.clear();

	bool result = false;

	result = xml.findElement(SETTINGS_SECTION);

	if (result == false)
	{
		return false;
	}

	result &= xml.readHostAddressPort(EquipmentPropNames::CLIENT_REQUEST_IP,
									  EquipmentPropNames::CLIENT_REQUEST_PORT, &clientRequestIP);

	result &= xml.readHostAddress(EquipmentPropNames::CLIENT_REQUEST_NETMASK, &clientRequestNetmask);

	result = xml.findElement(CLIENTS_SECTION);

	if (result == false)
	{
		return false;
	}

	int clientsCount = 0;

	result &= xml.readIntAttribute(ATTR_COUNT, &clientsCount);

	for(int i = 0; i < clientsCount; i++)
	{
		result &= xml.findElement(CLIENT);

		QString equipmentID;
		QString softwareTypeStr;

		result &= xml.readStringAttribute(ATTR_EQUIIPMENT_ID, &equipmentID);
		result &= xml.readStringAttribute(ATTR_SOFTWARE_TYPE, &softwareTypeStr);

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

// -------------------------------------------------------------------------------------
//
// AppDataServiceSettings class implementation
//
// -------------------------------------------------------------------------------------

bool AppDataServiceSettings::readFromDevice(Hardware::EquipmentSet* equipment, Hardware::Software* software, Builder::IssueLogger* log)
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

bool AppDataServiceSettings::writeToXml(XmlWriteHelper& xml)
{
	bool result = true;

	xml.writeStartElement(SETTINGS_SECTION);

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

	return result;
}

bool AppDataServiceSettings::readFromXml(XmlReadHelper& xml)
{
	bool result = false;

	result = xml.findElement(SETTINGS_SECTION);

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

// -------------------------------------------------------------------------------------
//
// DiagDataServiceSettings class implementation
//
// -------------------------------------------------------------------------------------

bool DiagDataServiceSettings::readFromDevice(Hardware::EquipmentSet* equipment,
											 Hardware::Software* software, Builder::IssueLogger* log)
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

bool DiagDataServiceSettings::writeToXml(XmlWriteHelper& xml)
{
	bool result = true;

	xml.writeStartElement(SETTINGS_SECTION);

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

	return result;
}

bool DiagDataServiceSettings::readFromXml(XmlReadHelper& xml)
{
	bool result = false;

	result = xml.findElement(SETTINGS_SECTION);

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

// -------------------------------------------------------------------------------------
//
// TuningServiceSettings class implementation
//
// -------------------------------------------------------------------------------------

const char* TuningServiceSettings::TUNING_CLIENTS = "TuningClients";
const char* TuningServiceSettings::TUNING_CLIENT = "TuningClient";
const char* TuningServiceSettings::TUNING_SOURCES = "TuningSources";
const char* TuningServiceSettings::TUNING_SOURCE = "TuningSource";

bool TuningServiceSettings::fillTuningClientsInfo(Hardware::Software *software, bool singleLmControlEnabled, Builder::IssueLogger* log)
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

bool TuningServiceSettings::readFromDevice(Hardware::Software *software, Builder::IssueLogger* log)
{
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

	result &= fillTuningClientsInfo(software, singleLmControl, log);

	return result;
}

bool TuningServiceSettings::writeToXml(XmlWriteHelper& xml)
{
	xml.writeStartElement(SETTINGS_SECTION);

	xml.writeHostAddressPort(EquipmentPropNames::CLIENT_REQUEST_IP,
							 EquipmentPropNames::CLIENT_REQUEST_PORT, clientRequestIP);
	xml.writeHostAddress(EquipmentPropNames::CLIENT_REQUEST_NETMASK, clientRequestNetmask);
	xml.writeHostAddressPort(EquipmentPropNames::TUNING_DATA_IP,
							 EquipmentPropNames::TUNING_DATA_PORT, tuningDataIP);
	xml.writeHostAddress(EquipmentPropNames::TUNING_DATA_NETMASK, tuningDataNetmask);

	xml.writeBoolElement(EquipmentPropNames::SINGLE_LM_CONTROL, singleLmControl);
	xml.writeBoolElement(EquipmentPropNames::DISABLE_MODULES_TYPE_CHECKING, disableModulesTypeChecking);

	xml.writeEndElement();	// </Settings>

	// write tuning clients info
	//
	xml.writeStartElement(TUNING_CLIENTS);
	xml.writeIntAttribute(ATTR_COUNT, clients.count());

	for(int i = 0; i < clients.count(); i++)
	{
		TuningClient& tc = clients[i];

		xml.writeStartElement(TUNING_CLIENT);
		xml.writeStringAttribute(ATTR_EQUIIPMENT_ID, tc.equipmentID);

		xml.writeStartElement(TUNING_SOURCES);
		xml.writeIntAttribute(ATTR_COUNT, tc.sourcesIDs.count());

		for(QString& sourceID : tc.sourcesIDs)
		{
			xml.writeStartElement(TUNING_SOURCE);
			xml.writeStringAttribute(ATTR_EQUIIPMENT_ID, sourceID);

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

	result = xml.findElement(SETTINGS_SECTION);

	if (result == false)
	{
		return false;
	}

	result &= xml.readHostAddressPort(EquipmentPropNames::CLIENT_REQUEST_IP,
									  EquipmentPropNames::CLIENT_REQUEST_PORT, &clientRequestIP);
	result &= xml.readHostAddress(EquipmentPropNames::CLIENT_REQUEST_NETMASK, &clientRequestNetmask);
	result &= xml.readHostAddressPort(EquipmentPropNames::TUNING_DATA_IP,
									  EquipmentPropNames::TUNING_DATA_PORT, &tuningDataIP);
	result &= xml.readHostAddress(EquipmentPropNames::TUNING_DATA_NETMASK, &tuningDataNetmask);

	result = xml.findElement(EquipmentPropNames::SINGLE_LM_CONTROL);

	if (result == false)
	{
		return false;
	}

	result &= xml.readBoolElement(EquipmentPropNames::SINGLE_LM_CONTROL, &singleLmControl);

	result = xml.findElement(EquipmentPropNames::DISABLE_MODULES_TYPE_CHECKING);

	if (result == false)
	{
		return false;
	}

	result &= xml.readBoolElement(EquipmentPropNames::DISABLE_MODULES_TYPE_CHECKING, &disableModulesTypeChecking);

	if (result == false)
	{
		return false;
	}

	// read tuning clients info
	//
	clients.clear();

	result = xml.findElement(TUNING_CLIENTS);

	if (result == false)
	{
		return false;
	}

	int clientsCount = 0;

	result = xml.readIntAttribute(ATTR_COUNT, &clientsCount);

	if (result == false)
	{
		return false;
	}

	for(int i = 0; i < clientsCount; i++)
	{
		TuningClient tc;

		result = xml.findElement(TUNING_CLIENT);

		if (result == false)
		{
			break;
		}

		result = xml.readStringAttribute(ATTR_EQUIIPMENT_ID, &tc.equipmentID);

		if (result == false)
		{
			break;
		}

		result = xml.findElement(TUNING_SOURCES);

		if (result == false)
		{
			break;
		}

		int sourcesCount = 0;

		result = xml.readIntAttribute(ATTR_COUNT, &sourcesCount);

		if (result == false)
		{
			break;
		}

		for(int s = 0; s < sourcesCount; s++)
		{
			result = xml.findElement(TUNING_SOURCE);

			if (result == false)
			{
				break;
			}

			QString sourceID;

			result = xml.readStringAttribute(ATTR_EQUIIPMENT_ID, &sourceID);

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

// -------------------------------------------------------------------------------------
//
// ArchivingServiceSettings class implementation
//
// -------------------------------------------------------------------------------------

bool ArchivingServiceSettings::readFromDevice(Hardware::Software* software, Builder::IssueLogger* log)
{
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

bool ArchivingServiceSettings::checkSettings(Hardware::Software *software, Builder::IssueLogger* log)
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

bool ArchivingServiceSettings::writeToXml(XmlWriteHelper& xml)
{
	bool result = true;

	xml.writeStartElement(SETTINGS_SECTION);

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

	return result;
}

bool ArchivingServiceSettings::readFromXml(XmlReadHelper& xml)
{
	bool result = false;

	result = xml.findElement(SETTINGS_SECTION);

	if (result == false)
	{
		return false;
	}

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

const char* TestClientSettings::CFG_SERVICE1_SECTION = "CfgService1";
const char* TestClientSettings::CFG_SERVICE2_SECTION = "CfgService2";
const char* TestClientSettings::APP_DATA_SERVICE_SECTION = "AppDataService";
const char* TestClientSettings::DIAG_DATA_SERVICE_SECTION = "DiagDataService";
const char* TestClientSettings::ARCH_SERVICE_SECTION = "ArchService";
const char* TestClientSettings::TUNING_SERVICE_SECTION = "TuningService";

bool TestClientSettings::readFromDevice(Hardware::EquipmentSet* equipment, Hardware::Software* software, Builder::IssueLogger* log)
{
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

bool TestClientSettings::writeToXml(XmlWriteHelper& xml)
{
	xml.writeStartElement(SETTINGS_SECTION);

	//

	xml.writeStartElement(CFG_SERVICE1_SECTION);
	xml.writeStringAttribute(ATTR_EQUIIPMENT_ID, cfgService1_equipmentID);
	xml.writeHostAddressPort(EquipmentPropNames::CLIENT_REQUEST_IP,
							 EquipmentPropNames::CLIENT_REQUEST_PORT, cfgService1_clientRequestIP);
	xml.writeEndElement();	// </CgService1>

	//

	xml.writeStartElement(CFG_SERVICE2_SECTION);
	xml.writeStringAttribute(ATTR_EQUIIPMENT_ID, cfgService2_equipmentID);
	xml.writeHostAddressPort(EquipmentPropNames::CLIENT_REQUEST_IP,
							 EquipmentPropNames::CLIENT_REQUEST_PORT, cfgService2_clientRequestIP);
	xml.writeEndElement();	// </CgService2>

	//

	xml.writeStartElement(APP_DATA_SERVICE_SECTION);
	xml.writeStringAttribute(ATTR_EQUIIPMENT_ID, appDataService_equipmentID);
	xml.writeHostAddressPort(EquipmentPropNames::APP_DATA_RECEIVING_IP,
							 EquipmentPropNames::APP_DATA_RECEIVING_PORT, appDataService_appDataReceivingIP);
	xml.writeHostAddressPort(EquipmentPropNames::CLIENT_REQUEST_IP,
							 EquipmentPropNames::CLIENT_REQUEST_PORT, appDataService_clientRequestIP);
	xml.writeEndElement();	// </AppDataService>

	//

	xml.writeStartElement(DIAG_DATA_SERVICE_SECTION);
	xml.writeStringAttribute(ATTR_EQUIIPMENT_ID, diagDataService_equipmentID);
	xml.writeHostAddressPort(EquipmentPropNames::DIAG_DATA_RECEIVING_IP,
							 EquipmentPropNames::DIAG_DATA_RECEIVING_PORT, diagDataService_diagDataReceivingIP);
	xml.writeHostAddressPort(EquipmentPropNames::CLIENT_REQUEST_IP,
							 EquipmentPropNames::CLIENT_REQUEST_PORT, diagDataService_clientRequestIP);
	xml.writeEndElement();	// </DiagDataService>

	//

	xml.writeStartElement(ARCH_SERVICE_SECTION);
	xml.writeStringAttribute(ATTR_EQUIIPMENT_ID, archService_equipmentID);
	xml.writeHostAddressPort(EquipmentPropNames::APP_DATA_RECEIVING_IP,
							 EquipmentPropNames::APP_DATA_RECEIVING_PORT, archService_appDataReceivingIP);
	xml.writeHostAddressPort(EquipmentPropNames::DIAG_DATA_RECEIVING_IP,
							 EquipmentPropNames::DIAG_DATA_RECEIVING_PORT, archService_diagDataReceivingIP);
	xml.writeHostAddressPort(EquipmentPropNames::CLIENT_REQUEST_IP,
							 EquipmentPropNames::CLIENT_REQUEST_PORT, archService_clientRequestIP);
	xml.writeEndElement();	// </ArchService>

	//

	xml.writeStartElement(TUNING_SERVICE_SECTION);
	xml.writeStringAttribute(ATTR_EQUIIPMENT_ID, tuningService_equipmentID);
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

	result &= xml.findElement(SETTINGS_SECTION);

	result &= xml.findElement(CFG_SERVICE1_SECTION);
	result &= xml.readStringAttribute(ATTR_EQUIIPMENT_ID, &cfgService1_equipmentID);
	result &= xml.readHostAddressPort(EquipmentPropNames::CLIENT_REQUEST_IP,
									  EquipmentPropNames::CLIENT_REQUEST_PORT, &cfgService1_clientRequestIP);

	result &= xml.findElement(CFG_SERVICE2_SECTION);
	result &= xml.readStringAttribute(ATTR_EQUIIPMENT_ID, &cfgService2_equipmentID);
	result &= xml.readHostAddressPort(EquipmentPropNames::CLIENT_REQUEST_IP,
									  EquipmentPropNames::CLIENT_REQUEST_PORT, &cfgService2_clientRequestIP);

	result &= xml.findElement(APP_DATA_SERVICE_SECTION);
	result &= xml.readStringAttribute(ATTR_EQUIIPMENT_ID, &appDataService_equipmentID);
	result &= xml.readHostAddressPort(EquipmentPropNames::APP_DATA_RECEIVING_IP,
									  EquipmentPropNames::APP_DATA_RECEIVING_PORT, &appDataService_appDataReceivingIP);
	result &= xml.readHostAddressPort(EquipmentPropNames::CLIENT_REQUEST_IP,
									  EquipmentPropNames::CLIENT_REQUEST_PORT, &appDataService_clientRequestIP);

	result &= xml.findElement(DIAG_DATA_SERVICE_SECTION);
	result &= xml.readStringAttribute(ATTR_EQUIIPMENT_ID, &diagDataService_equipmentID);
	result &= xml.readHostAddressPort(EquipmentPropNames::DIAG_DATA_RECEIVING_IP,
									  EquipmentPropNames::DIAG_DATA_RECEIVING_PORT, &diagDataService_diagDataReceivingIP);
	result &= xml.readHostAddressPort(EquipmentPropNames::CLIENT_REQUEST_IP,
									  EquipmentPropNames::CLIENT_REQUEST_PORT, &diagDataService_clientRequestIP);

	result &= xml.findElement(ARCH_SERVICE_SECTION);
	result &= xml.readStringAttribute(ATTR_EQUIIPMENT_ID, &archService_equipmentID);
	result &= xml.readHostAddressPort(EquipmentPropNames::APP_DATA_RECEIVING_IP,
									  EquipmentPropNames::APP_DATA_RECEIVING_PORT, &archService_appDataReceivingIP);
	result &= xml.readHostAddressPort(EquipmentPropNames::DIAG_DATA_RECEIVING_IP,
									  EquipmentPropNames::DIAG_DATA_RECEIVING_PORT, &archService_diagDataReceivingIP);
	result &= xml.readHostAddressPort(EquipmentPropNames::CLIENT_REQUEST_IP,
									  EquipmentPropNames::CLIENT_REQUEST_PORT, &archService_clientRequestIP);

	result &= xml.findElement(TUNING_SERVICE_SECTION);
	result &= xml.readStringAttribute(ATTR_EQUIIPMENT_ID, &tuningService_equipmentID);
	result &= xml.readHostAddressPort(EquipmentPropNames::TUNING_DATA_IP,
									  EquipmentPropNames::TUNING_DATA_PORT, &tuningService_tuningDataIP);
	result &= xml.readHostAddressPort(EquipmentPropNames::CLIENT_REQUEST_IP,
									  EquipmentPropNames::CLIENT_REQUEST_PORT, &tuningService_clientRequestIP);

	return result;
}



