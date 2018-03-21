#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include "../lib/ServiceSettings.h"
#include "../lib/WUtils.h"

// -------------------------------------------------------------------------------------
//
// ServiceSettings class implementation
//
// -------------------------------------------------------------------------------------

const char* ServiceSettings::PROP_CLIENT_REQUEST_IP = "ClientRequestIP";
const char* ServiceSettings::PROP_CLIENT_REQUEST_NETMASK = "ClientRequestNetmask";
const char* ServiceSettings::PROP_CLIENT_REQUEST_PORT = "ClientRequestPort";

const char* ServiceSettings::PROP_CFG_SERVICE_ID1 = "ConfigurationServiceID1";
const char* ServiceSettings::PROP_CFG_SERVICE_IP1 = "ConfigurationServiceIP1";
const char* ServiceSettings::PROP_CFG_SERVICE_PORT1 = "ConfigurationServicePort1";

const char* ServiceSettings::PROP_CFG_SERVICE_ID2 = "ConfigurationServiceID2";
const char* ServiceSettings::PROP_CFG_SERVICE_IP2 = "ConfigurationServiceIP2";
const char* ServiceSettings::PROP_CFG_SERVICE_PORT2 = "ConfigurationServicePort2";

bool ServiceSettings::getCfgServiceConnectionSettings(	Hardware::EquipmentSet* equipment,
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

	result &= DeviceHelper::getStrProperty(software, PROP_CFG_SERVICE_ID1, cfgServiceID1, log);
	result &= DeviceHelper::getStrProperty(software, PROP_CFG_SERVICE_ID2, cfgServiceID2, log);

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

	QString ipStr;
	int port = 0;

	if (cfgServiceID1->isEmpty() == true)
	{
		ipStr = Socket::IP_NULL;
		port = PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST;

		cfgServiceAddrPort1->setAddressPort(ipStr, port);

		// Property '%1.%2' is empty.
		//
		log->wrnCFG3016(software->equipmentIdTemplate(), PROP_CFG_SERVICE_ID1);

		result = false;
	}
	else
	{
		const Hardware::Software* cfgService1 = DeviceHelper::getSoftware(equipment, *cfgServiceID1);

		if (cfgService1 == nullptr)
		{
			// Property '%1.%2' is linked to undefined software ID '%3'.
			//
			log->errCFG3021(software->equipmentIdTemplate(), PROP_CFG_SERVICE_ID1, *cfgServiceID1);
			result = false;
		}
		else
		{
			result &= DeviceHelper::getIpPortProperty(cfgService1,
													  PROP_CLIENT_REQUEST_IP,
													  PROP_CLIENT_REQUEST_PORT,
													  cfgServiceAddrPort1, false, "", 0, log);
		}
	}

	if (cfgServiceID2->isEmpty() == true)
	{
		ipStr = Socket::IP_NULL;
		port = PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST;

		cfgServiceAddrPort2->setAddressPort(ipStr, port);

		// Property '%1.%2' is empty.
		//
		log->wrnCFG3016(software->equipmentIdTemplate(), PROP_CFG_SERVICE_ID2);

		result = false;
	}
	else
	{
		const Hardware::Software* cfgService2 = DeviceHelper::getSoftware(equipment, *cfgServiceID2);

		if (cfgService2 == nullptr)
		{
			// Property '%1.%2' is linked to undefined software ID '%3'.
			//
			log->errCFG3021(software->equipmentIdTemplate(), PROP_CFG_SERVICE_ID2, *cfgServiceID2);
			result = false;
		}
		else
		{
			result &= DeviceHelper::getIpPortProperty(cfgService2,
													  PROP_CLIENT_REQUEST_IP,
													  PROP_CLIENT_REQUEST_PORT,
													  cfgServiceAddrPort2, false, "", 0, log);
		}
	}

	return result;
}


// -------------------------------------------------------------------------------------
//
// AppDataServiceSettings class implementation
//
// -------------------------------------------------------------------------------------

const char* AppDataServiceSettings::SECTION_NAME = "Settings";

const char* AppDataServiceSettings::PROP_APP_DATA_NETMASK = "AppDataNetmask";
const char* AppDataServiceSettings::PROP_APP_DATA_RECEIVING_IP = "AppDataReceivingIP";
const char* AppDataServiceSettings::PROP_APP_DATA_RECEIVING_PORT = "AppDataReceivingPort";

const char* AppDataServiceSettings::PROP_AUTO_ARCHIVE_INTERVAL = "AutoArchiveInterval";

const char* AppDataServiceSettings::PROP_ARCH_SERVICE_ID = "ArchiveServiceID";
const char* AppDataServiceSettings::PROP_ARCH_SERVICE_IP = "ArchiveServiceIP";
const char* AppDataServiceSettings::PROP_ARCH_SERVICE_PORT = "ArchiveServicePort";

bool AppDataServiceSettings::readFromDevice(Hardware::EquipmentSet* equipment, Hardware::Software* software, Builder::IssueLogger* log)
{
	TEST_PTR_RETURN_FALSE(equipment);
	TEST_PTR_RETURN_FALSE(software);
	TEST_PTR_RETURN_FALSE(log);

	bool result = true;

	QString netmaskStr;
	QString ipStr;
	int port = 0;

	result &= DeviceHelper::getIPv4Property(software, PROP_APP_DATA_RECEIVING_IP, &ipStr, false, "", log);
	result &= DeviceHelper::getPortProperty(software, PROP_APP_DATA_RECEIVING_PORT, &port, false, 0, log);
	result &= DeviceHelper::getIPv4Property(software, PROP_APP_DATA_NETMASK, &netmaskStr, false, "", log);

	appDataReceivingIP.setAddressPort(ipStr, port);
	appDataNetmask.setAddress(netmaskStr);

	result &= DeviceHelper::getIntProperty(software, PROP_AUTO_ARCHIVE_INTERVAL, &autoArchiveInterval, log);

	result &= DeviceHelper::getIPv4Property(software, PROP_CLIENT_REQUEST_IP, &ipStr, true, Socket::IP_LOCALHOST, log);
	result &= DeviceHelper::getPortProperty(software, PROP_CLIENT_REQUEST_PORT, &port, true, PORT_APP_DATA_SERVICE_DATA, log);
	result &= DeviceHelper::getIPv4Property(software, PROP_CLIENT_REQUEST_NETMASK, &netmaskStr, true, Socket::NETMASK_DEFAULT, log);

	clientRequestIP.setAddressPort(ipStr, port);
	clientRequestNetmask.setAddress(netmaskStr);

	result &= DeviceHelper::getStrProperty(software, PROP_ARCH_SERVICE_ID, &archServiceID, log);

	if (archServiceID.isEmpty() == true)
	{
		//  Property '%1.%2' is empty.
		//
		log->wrnCFG3016(software->equipmentIdTemplate(), PROP_ARCH_SERVICE_ID);

		ipStr = Socket::IP_NULL;
		port = PORT_ARCHIVING_SERVICE_APP_DATA;

		archServiceIP.setAddressPort(ipStr, port);
	}
	else
	{
		const Hardware::Software* archService = DeviceHelper::getSoftware(equipment, archServiceID);

		if (archService == nullptr)
		{
			// Property '%1.%2' is linked to undefined software ID '%3'.
			//
			log->errCFG3021(software->equipmentIdTemplate(), PROP_ARCH_SERVICE_ID, archServiceID);
			result = false;
		}
		else
		{
			result &= DeviceHelper::getIpPortProperty(archService,
													  ArchivingServiceSettings::PROP_APP_DATA_SERVICE_REQUEST_IP,
													  ArchivingServiceSettings::PROP_APP_DATA_SERVICE_REQUEST_PORT,
													  &archServiceIP, false, "", 0, log);
		}
	}

	result &= DeviceHelper::getStrProperty(software, PROP_CFG_SERVICE_ID1, &cfgServiceID1, log);

	if (cfgServiceID1.isEmpty() == true)
	{
		ipStr = Socket::IP_NULL;
		port = PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST;

		cfgServiceIP1.setAddressPort(ipStr, port);

		// Property '%1.%2' is empty.
		//
		log->errCFG3022(software->equipmentIdTemplate(), PROP_CFG_SERVICE_ID1);

		result = false;
	}
	else
	{
		const Hardware::Software* cfgService1 = DeviceHelper::getSoftware(equipment, cfgServiceID1);

		if (cfgService1 == nullptr)
		{
			// Property '%1.%2' is linked to undefined software ID '%3'.
			//
			log->errCFG3021(software->equipmentIdTemplate(), PROP_CFG_SERVICE_ID1, cfgServiceID1);
			result = false;
		}
		else
		{
			result &= DeviceHelper::getIpPortProperty(cfgService1,
													  PROP_CLIENT_REQUEST_IP,
													  PROP_CLIENT_REQUEST_PORT,
													  &cfgServiceIP1, false, "", 0, log);
		}
	}


	result &= DeviceHelper::getStrProperty(software, PROP_CFG_SERVICE_ID2, &cfgServiceID2, log);

	return result;
}


bool AppDataServiceSettings::writeToXml(XmlWriteHelper& xml)
{
	bool result = true;

	xml.writeStartElement(SECTION_NAME);

	xml.writeHostAddressPort(PROP_APP_DATA_RECEIVING_IP, PROP_APP_DATA_RECEIVING_PORT, appDataReceivingIP);
	xml.writeHostAddress(PROP_APP_DATA_NETMASK, appDataNetmask);

	xml.writeIntElement(PROP_AUTO_ARCHIVE_INTERVAL, autoArchiveInterval);

	xml.writeHostAddressPort(PROP_CLIENT_REQUEST_IP, PROP_CLIENT_REQUEST_PORT, clientRequestIP);
	xml.writeHostAddress(PROP_CLIENT_REQUEST_NETMASK, clientRequestNetmask);

	xml.writeStringElement(PROP_ARCH_SERVICE_ID, archServiceID);
	xml.writeHostAddressPort(PROP_ARCH_SERVICE_IP, PROP_ARCH_SERVICE_PORT, archServiceIP);

	xml.writeStringElement(PROP_CFG_SERVICE_ID1, cfgServiceID1);
	xml.writeHostAddressPort(PROP_CFG_SERVICE_IP1, PROP_CFG_SERVICE_PORT1, cfgServiceIP1);

	xml.writeStringElement(PROP_CFG_SERVICE_ID2, cfgServiceID2);
	xml.writeHostAddressPort(PROP_CFG_SERVICE_IP2, PROP_CFG_SERVICE_PORT2, cfgServiceIP2);

	xml.writeEndElement();	// </Settings>

	return result;
}

bool AppDataServiceSettings::readFromXml(XmlReadHelper& xml)
{
	bool result = false;

	result = xml.findElement(SECTION_NAME);

	if (result == false)
	{
		return false;
	}

	result &= xml.readHostAddressPort(PROP_CLIENT_REQUEST_IP, PROP_CLIENT_REQUEST_PORT, &clientRequestIP);

	result &= xml.readHostAddress(PROP_CLIENT_REQUEST_NETMASK, &clientRequestNetmask);

	result &= xml.findElement(PROP_AUTO_ARCHIVE_INTERVAL);

	if (result == false)
	{
		return false;
	}

	result &= xml.readIntElement(PROP_AUTO_ARCHIVE_INTERVAL, &autoArchiveInterval);

	return result;
}


// -------------------------------------------------------------------------------------
//
// TuningServiceSettings class implementation
//
// -------------------------------------------------------------------------------------

const char* TuningServiceSettings::SECTION_NAME = "Settings";

const char* TuningServiceSettings::PROP_TUNING_DATA_IP = "TuningDataIP";
const char* TuningServiceSettings::PROP_TUNING_DATA_PORT = "TuningDataPort";
const char* TuningServiceSettings::PROP_SINGLE_LM_CONTROL = "SingleLmControl";
const char* TuningServiceSettings::PROP_DISABLE_MODULES_TYPE_CHECKING = "DisableModulesTypeChecking";

const char* TuningServiceSettings::TUNING_CLIENTS = "TuningClients";
const char* TuningServiceSettings::TUNING_CLIENT = "TuningClient";
const char* TuningServiceSettings::TUNING_SOURCES = "TuningSources";
const char* TuningServiceSettings::TUNING_SOURCE = "TuningSource";
const char* TuningServiceSettings::ATTR_EQUIIPMENT_ID = "EquipmentID";
const char* TuningServiceSettings::ATTR_COUNT = "Count";


bool TuningServiceSettings::fillTuningClientsInfo(Hardware::Software *software, Builder::IssueLogger* log)
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
		[this, &software, &result, &log](Hardware::DeviceObject* currentDevice)
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
				tuningClient->type() != E::SoftwareType::Metrology)
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

			// TuningClient is linked to this TuningService

			TuningClient tc;

			tc.equipmentID = tuningClient->equipmentIdTemplate();

			QString sourcesIDs;

			result &= DeviceHelper::getStrProperty(tuningClient, "TuningSourceEquipmentID", &sourcesIDs, log);

			sourcesIDs.replace(' ', ';');
			sourcesIDs.replace('\n', ';');
			sourcesIDs.remove('\r');

			tc.sourcesIDs = sourcesIDs.split(";", QString::SkipEmptyParts);

			for (QString& id : tc.sourcesIDs)
			{
				id = id.trimmed();
			}

			this->clients.append(tc);
		}
	);


	return result;
}

bool TuningServiceSettings::readFromDevice(Hardware::Software *software, Builder::IssueLogger* log)
{
	bool result = true;

	QString clientRequestIPStr;
	int clientRequestPort = 0;
	QString clientNetmaskStr;
	QString tuningDataIPStr;
	int tuningDataPort = 0;

	result &= DeviceHelper::getStrProperty(software, PROP_CLIENT_REQUEST_IP, &clientRequestIPStr, log);
	result &= DeviceHelper::getIntProperty(software, PROP_CLIENT_REQUEST_PORT, &clientRequestPort, log);
	result &= DeviceHelper::getStrProperty(software, PROP_CLIENT_REQUEST_NETMASK, &clientNetmaskStr, log);
	result &= DeviceHelper::getStrProperty(software, PROP_TUNING_DATA_IP, &tuningDataIPStr, log);
	result &= DeviceHelper::getIntProperty(software, PROP_TUNING_DATA_PORT, &tuningDataPort, log);

	result &= DeviceHelper::getBoolProperty(software, PROP_SINGLE_LM_CONTROL, &singleLmControl, log);
	result &= DeviceHelper::getBoolProperty(software, PROP_DISABLE_MODULES_TYPE_CHECKING, &disableModulesTypeChecking, log);

	clientRequestIP = HostAddressPort(clientRequestIPStr, clientRequestPort);
	clientRequestNetmask.setAddress(clientNetmaskStr);

	tuningDataIP = HostAddressPort(tuningDataIPStr, tuningDataPort);

	result &= fillTuningClientsInfo(software, log);

	return result;
}


bool TuningServiceSettings::writeToXml(XmlWriteHelper& xml)
{
	xml.writeStartElement(SECTION_NAME);

	xml.writeHostAddressPort(PROP_CLIENT_REQUEST_IP, PROP_CLIENT_REQUEST_PORT, clientRequestIP);
	xml.writeHostAddress(PROP_CLIENT_REQUEST_NETMASK, clientRequestNetmask);
	xml.writeHostAddressPort(PROP_TUNING_DATA_IP, PROP_TUNING_DATA_PORT, tuningDataIP);

	xml.writeBoolElement(PROP_SINGLE_LM_CONTROL, singleLmControl);
	xml.writeBoolElement(PROP_DISABLE_MODULES_TYPE_CHECKING, disableModulesTypeChecking);

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

	result = xml.findElement(SECTION_NAME);

	if (result == false)
	{
		return false;
	}

	result &= xml.readHostAddressPort(PROP_CLIENT_REQUEST_IP, PROP_CLIENT_REQUEST_PORT, &clientRequestIP);
	result &= xml.readHostAddress(PROP_CLIENT_REQUEST_NETMASK, &clientRequestNetmask);
	result &= xml.readHostAddressPort(PROP_TUNING_DATA_IP, PROP_TUNING_DATA_PORT, &tuningDataIP);

	result = xml.findElement(PROP_SINGLE_LM_CONTROL);

	if (result == false)
	{
		return false;
	}

	result &= xml.readBoolElement(PROP_SINGLE_LM_CONTROL, &singleLmControl);

	result = xml.findElement(PROP_DISABLE_MODULES_TYPE_CHECKING);

	if (result == false)
	{
		return false;
	}

	result &= xml.readBoolElement(PROP_DISABLE_MODULES_TYPE_CHECKING, &disableModulesTypeChecking);

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

const char* ArchivingServiceSettings::SECTION_NAME = "Settings";

const char* ArchivingServiceSettings::PROP_APP_DATA_SERVICE_REQUEST_IP = "AppDataServiceRequestIP";
const char* ArchivingServiceSettings::PROP_APP_DATA_SERVICE_REQUEST_PORT = "AppDataServiceRequestPort";
const char* ArchivingServiceSettings::PROP_APP_DATA_SERVICE_REQUEST_NETMASK = "AppDataServiceRequestNetmask";

const char* ArchivingServiceSettings::PROP_DIAG_DATA_SERVICE_REQUEST_IP = "DiagDataServiceRequestIP";
const char* ArchivingServiceSettings::PROP_DIAG_DATA_SERVICE_REQUEST_PORT = "DiagDataServiceRequestPort";
const char* ArchivingServiceSettings::PROP_DIAG_DATA_SERVICE_REQUEST_NETMASK = "DiagDataServiceRequestNetmask";

const char* ArchivingServiceSettings::PROP_ARCHIVE_DB_HOST_IP = "ArchiveDatabaseHostIP";
const char* ArchivingServiceSettings::PROP_ARCHIVE_DB_HOST_PORT = "ArchiveDatabaseHostPort";


bool ArchivingServiceSettings::readFromDevice(Hardware::Software* software, Builder::IssueLogger* log)
{
	bool result = true;

	QString clientRequestIPStr;
	int clientRequestPort = 0;
	QString clientNetmaskStr;

	result &= DeviceHelper::getStrProperty(software, PROP_CLIENT_REQUEST_IP, &clientRequestIPStr, log);
	result &= DeviceHelper::getIntProperty(software, PROP_CLIENT_REQUEST_PORT, &clientRequestPort, log);
	result &= DeviceHelper::getStrProperty(software, PROP_CLIENT_REQUEST_NETMASK, &clientNetmaskStr, log);

	clientRequestIP = HostAddressPort(clientRequestIPStr, clientRequestPort);
	clientRequestNetmask.setAddress(clientNetmaskStr);

	//

	QString appDataServiceRequestIPStr;
	int appDataServiceRequestPort = 0;
	QString appDataServiceNetmaskStr;

	result &= DeviceHelper::getStrProperty(software, PROP_APP_DATA_SERVICE_REQUEST_IP, &appDataServiceRequestIPStr, log);
	result &= DeviceHelper::getIntProperty(software, PROP_APP_DATA_SERVICE_REQUEST_PORT, &appDataServiceRequestPort, log);
	result &= DeviceHelper::getStrProperty(software, PROP_APP_DATA_SERVICE_REQUEST_NETMASK, &appDataServiceNetmaskStr, log);

	appDataServiceRequestIP = HostAddressPort(appDataServiceRequestIPStr, appDataServiceRequestPort);
	appDataServiceRequestNetmask.setAddress(appDataServiceNetmaskStr);

	//

	QString diagDataServiceRequestIPStr;
	int diagDataServiceRequestPort = 0;
	QString diagDataServiceNetmaskStr;

	result &= DeviceHelper::getStrProperty(software, PROP_DIAG_DATA_SERVICE_REQUEST_IP, &diagDataServiceRequestIPStr, log);
	result &= DeviceHelper::getIntProperty(software, PROP_DIAG_DATA_SERVICE_REQUEST_PORT, &diagDataServiceRequestPort, log);
	result &= DeviceHelper::getStrProperty(software, PROP_DIAG_DATA_SERVICE_REQUEST_NETMASK, &diagDataServiceNetmaskStr, log);

	diagDataServiceRequestIP = HostAddressPort(diagDataServiceRequestIPStr, diagDataServiceRequestPort);
	diagDataServiceRequestNetmask.setAddress(diagDataServiceNetmaskStr);

	//

	QString dbHostIP;
	int dbHostPort = 0;

	result &= DeviceHelper::getStrProperty(software, PROP_ARCHIVE_DB_HOST_IP, &dbHostIP, log);
	result &= DeviceHelper::getIntProperty(software, PROP_ARCHIVE_DB_HOST_PORT, &dbHostPort, log);

	dbHost = HostAddressPort(dbHostIP, dbHostPort);

	return result;
}


bool ArchivingServiceSettings::writeToXml(XmlWriteHelper& xml)
{
	bool result = true;

	xml.writeStartElement(SECTION_NAME);

	xml.writeHostAddressPort(PROP_CLIENT_REQUEST_IP, PROP_CLIENT_REQUEST_PORT, clientRequestIP);
	xml.writeHostAddress(PROP_CLIENT_REQUEST_NETMASK, clientRequestNetmask);

	xml.writeHostAddressPort(PROP_APP_DATA_SERVICE_REQUEST_IP, PROP_APP_DATA_SERVICE_REQUEST_PORT, appDataServiceRequestIP);
	xml.writeHostAddress(PROP_APP_DATA_SERVICE_REQUEST_NETMASK, appDataServiceRequestNetmask);

	xml.writeHostAddressPort(PROP_DIAG_DATA_SERVICE_REQUEST_IP, PROP_DIAG_DATA_SERVICE_REQUEST_PORT, diagDataServiceRequestIP);
	xml.writeHostAddress(PROP_DIAG_DATA_SERVICE_REQUEST_NETMASK, diagDataServiceRequestNetmask);

	xml.writeHostAddressPort(PROP_ARCHIVE_DB_HOST_IP, PROP_ARCHIVE_DB_HOST_PORT, dbHost);

	xml.writeEndElement();	// </Settings>

	return result;
}


bool ArchivingServiceSettings::readFromXml(XmlReadHelper& xml)
{
	bool result = false;

	result = xml.findElement(SECTION_NAME);

	if (result == false)
	{
		return false;
	}

	result &= xml.readHostAddressPort(PROP_CLIENT_REQUEST_IP, PROP_CLIENT_REQUEST_PORT, &clientRequestIP);
	result &= xml.readHostAddress(PROP_CLIENT_REQUEST_NETMASK, &clientRequestNetmask);

	result &= xml.readHostAddressPort(PROP_APP_DATA_SERVICE_REQUEST_IP, PROP_APP_DATA_SERVICE_REQUEST_PORT, &appDataServiceRequestIP);
	result &= xml.readHostAddress(PROP_APP_DATA_SERVICE_REQUEST_NETMASK, &appDataServiceRequestNetmask);

	result &= xml.readHostAddressPort(PROP_DIAG_DATA_SERVICE_REQUEST_IP, PROP_DIAG_DATA_SERVICE_REQUEST_PORT, &diagDataServiceRequestIP);
	result &= xml.readHostAddress(PROP_DIAG_DATA_SERVICE_REQUEST_NETMASK, &diagDataServiceRequestNetmask);

	result &= xml.readHostAddressPort(PROP_ARCHIVE_DB_HOST_IP, PROP_ARCHIVE_DB_HOST_PORT, &dbHost);

	return result;
}



