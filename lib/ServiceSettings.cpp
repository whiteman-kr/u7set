#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include "../lib/ServiceSettings.h"


// -------------------------------------------------------------------------------------
//
// AppDataServiceChannel class implementation
//
// -------------------------------------------------------------------------------------

const char* AppDataServiceChannel::PROP_APP_DATA_NETMASK = "AppDataNetmask";
const char* AppDataServiceChannel::PROP_APP_DATA_RECEIVING_IP = "AppDataReceivingIP";
const char* AppDataServiceChannel::PROP_APP_DATA_RECEIVING_PORT = "AppDataReceivingPort";

const char* AppDataServiceChannel::PROP_ARCH_SERVICE_ID = "ArchiveServiceID";

const char* AppDataServiceChannel::PROP_CFG_SERVICE_ID = "ConfigurationServiceID";

const char* AppDataServiceChannel::SECTION_FORMAT_STR = "DataChannel%1";


bool AppDataServiceChannel::readFromDevice(Hardware::DeviceController* controller, Builder::IssueLogger* log)
{
	bool result = true;

	QString appDataNetmaskStr;
	QString appDataReceivingIPStr;
	int appDataReceivingPort = 0;

	result &= DeviceHelper::getStrProperty(controller, PROP_APP_DATA_NETMASK, &appDataNetmaskStr, log);
	result &= DeviceHelper::getStrProperty(controller, PROP_APP_DATA_RECEIVING_IP, &appDataReceivingIPStr, log);
	result &= DeviceHelper::getIntProperty(controller, PROP_APP_DATA_RECEIVING_PORT, &appDataReceivingPort, log);

	result &= DeviceHelper::getStrProperty(controller, PROP_ARCH_SERVICE_ID, &archServiceStrID, log);

	result &= DeviceHelper::getStrProperty(controller, PROP_CFG_SERVICE_ID, &cfgServiceStrID, log);

	if (result == true)
	{
		appDataNetmask.setAddress(appDataNetmaskStr);
		appDataReceivingIP = HostAddressPort(appDataReceivingIPStr, appDataReceivingPort);
	}

	return result;
}


QString AppDataServiceChannel::sectionName(int channel)
{
	return QString(SECTION_FORMAT_STR).arg(channel + 1);
}


bool AppDataServiceChannel::writeToXml(XmlWriteHelper& xml, int channel)
{
	xml.writeStartElement(sectionName(channel));

	xml.writeHostAddressPort(PROP_APP_DATA_RECEIVING_IP, PROP_APP_DATA_RECEIVING_PORT, appDataReceivingIP);
	xml.writeHostAddress(PROP_APP_DATA_NETMASK, appDataNetmask);

	xml.writeStringElement(PROP_ARCH_SERVICE_ID, archServiceStrID);

	xml.writeStringElement(PROP_CFG_SERVICE_ID, cfgServiceStrID);

	xml.writeEndElement();		//	</DataChannelN>

	return true;
}


bool AppDataServiceChannel::readFromXml(XmlReadHelper& xml, int channel)
{
	if (xml.findElement(sectionName(channel)) == false)
	{
		return false;
	}

	bool result = true;

	result &= xml.readHostAddressPort(PROP_APP_DATA_RECEIVING_IP, PROP_APP_DATA_RECEIVING_PORT, &appDataReceivingIP);
	result &= xml.readHostAddress(PROP_APP_DATA_NETMASK, &appDataNetmask);

	if (xml.findElement(PROP_ARCH_SERVICE_ID) == false)
	{
		return false;
	}

	result &= xml.readStringElement(PROP_ARCH_SERVICE_ID, &archServiceStrID);

	if (xml.findElement(PROP_CFG_SERVICE_ID) == false)
	{
		return false;
	}

	result &= xml.readStringElement(PROP_CFG_SERVICE_ID, &cfgServiceStrID);

	return result;
}


// -------------------------------------------------------------------------------------
//
// AppDataServiceSettings class implementation
//
// -------------------------------------------------------------------------------------

const char* AppDataServiceSettings::DATA_CHANNEL_CONTROLLER_ID_FORMAT_STR = "_DATACH0%1";
const char* AppDataServiceSettings::SECTION_NAME = "Settings";
const char* AppDataServiceSettings::PROP_CLIENT_REQUEST_IP = "ClientRequestIP";
const char* AppDataServiceSettings::PROP_CLIENT_REQUEST_PORT = "ClientRequestPort";
const char* AppDataServiceSettings::PROP_CLIENT_REQUEST_NETMASK = "ClientRequestNetmask";


bool AppDataServiceSettings::readFromDevice(Hardware::Software* software, Builder::IssueLogger* log)
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

	for(int channel = 0; channel < DATA_CHANNEL_COUNT; channel++)
	{
		Hardware::DeviceController* ethernet =
				DeviceHelper::getChildControllerBySuffix(software,
					QString(DATA_CHANNEL_CONTROLLER_ID_FORMAT_STR).arg(channel + 1), log);

		if (ethernet != nullptr)
		{
			result &= appDataServiceChannel[channel].readFromDevice(ethernet, log);
		}
	}

	return result;
}


bool AppDataServiceSettings::writeToXml(XmlWriteHelper& xml)
{
	bool result = true;

	xml.writeStartElement(SECTION_NAME);

	xml.writeHostAddressPort(PROP_CLIENT_REQUEST_IP, PROP_CLIENT_REQUEST_PORT, clientRequestIP);
	xml.writeHostAddress(PROP_CLIENT_REQUEST_NETMASK, clientRequestNetmask);

	for(int channel = 0; channel < DATA_CHANNEL_COUNT; channel++)
	{
		appDataServiceChannel[channel].writeToXml(xml, channel);
	}

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

	for(int channel = 0; channel < DATA_CHANNEL_COUNT; channel++)
	{
		result &= appDataServiceChannel[channel].readFromXml(xml, channel);
	}

	return result;
}


// -------------------------------------------------------------------------------------
//
// TuningServiceSettings class implementation
//
// -------------------------------------------------------------------------------------

const char* TuningServiceSettings::SECTION_NAME = "Settings";
const char* TuningServiceSettings::PROP_CLIENT_REQUEST_IP = "ClientRequestIP";
const char* TuningServiceSettings::PROP_CLIENT_REQUEST_PORT = "ClientRequestPort";
const char* TuningServiceSettings::PROP_CLIENT_REQUEST_NETMASK = "ClientRequestNetmask";
const char* TuningServiceSettings::PROP_TUNING_DATA_IP = "TuningDataIP";
const char* TuningServiceSettings::PROP_TUNING_DATA_PORT = "TuningDataPort";

const char* TuningServiceSettings::TUNING_MEMORY_SETTINGS_ELEMENT = "TuningMemorySettings";
const char* TuningServiceSettings::PROP_TUNING_DATA_OFFSET = "TuningDataOffset";
const char* TuningServiceSettings::PROP_TUNING_DATA_SIZE = "TuningDataSize";
const char* TuningServiceSettings::PROP_TUNING_ROM_FRAME_COUNT = "TuningROMFrameCount";
const char* TuningServiceSettings::PROP_TUNING_ROM_FRAME_SIZE = "TuningROMFrameSize";
const char* TuningServiceSettings::PROP_TUNING_ROM_SIZE = "TuningROMSize";

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
			QString tuningServiceID1;
			QString tuningServiceID2;

			result &= DeviceHelper::getStrProperty(tuningClient, "TuningServiceID1", &tuningServiceID1, log);
			result &= DeviceHelper::getStrProperty(tuningClient, "TuningServiceID2", &tuningServiceID2, log);

			if (result == false)
			{
				return;
			}

			if (tuningServiceID1 != software->equipmentIdTemplate() &&
				tuningServiceID2 != software->equipmentIdTemplate())
			{
				return;
			}

			// TuningClient is linked to this TuningService

			TuningClient tc;

			tc.equipmentID = tuningClient->equipmentIdTemplate();

			QString sourcesIDs;

			result &= DeviceHelper::getStrProperty(tuningClient, "TuningSourceEquipmentID", &sourcesIDs, log);

			tc.sourcesIDs = sourcesIDs.split("\n", QString::SkipEmptyParts);

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

	result &= DeviceHelper::getIntProperty(software, PROP_TUNING_DATA_OFFSET, &tuningDataOffsetW, log);
	result &= DeviceHelper::getIntProperty(software, PROP_TUNING_DATA_SIZE, &tuningDataSizeW, log);
	result &= DeviceHelper::getIntProperty(software, PROP_TUNING_ROM_FRAME_COUNT, &tuningRomFrameCount, log);
	result &= DeviceHelper::getIntProperty(software, PROP_TUNING_ROM_FRAME_SIZE, &tuningRomFrameSizeW, log);
	result &= DeviceHelper::getIntProperty(software, PROP_TUNING_ROM_SIZE, &tuningRomSizeW, log);

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

	xml.writeStartElement(TUNING_MEMORY_SETTINGS_ELEMENT);

	xml.writeIntAttribute(PROP_TUNING_DATA_OFFSET, tuningDataOffsetW);
	xml.writeIntAttribute(PROP_TUNING_DATA_SIZE, tuningDataSizeW);
	xml.writeIntAttribute(PROP_TUNING_ROM_FRAME_COUNT, tuningRomFrameCount);
	xml.writeIntAttribute(PROP_TUNING_ROM_FRAME_SIZE, tuningRomFrameSizeW);
	xml.writeIntAttribute(PROP_TUNING_ROM_SIZE, tuningRomSizeW);

	xml.writeEndElement();	// </TuningMemorySettings>

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

	if (result == false)
	{
		return false;
	}

	result = xml.findElement(TUNING_MEMORY_SETTINGS_ELEMENT);

	if (result == false)
	{
		return false;
	}

	result &= xml.readIntAttribute(PROP_TUNING_DATA_OFFSET, &tuningDataOffsetW);
	result &= xml.readIntAttribute(PROP_TUNING_DATA_SIZE, &tuningDataSizeW);
	result &= xml.readIntAttribute(PROP_TUNING_ROM_FRAME_COUNT, &tuningRomFrameCount);
	result &= xml.readIntAttribute(PROP_TUNING_ROM_FRAME_SIZE, &tuningRomFrameSizeW);
	result &= xml.readIntAttribute(PROP_TUNING_ROM_SIZE, &tuningRomSizeW);

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

const char* ArchivingServiceSettings::PROP_CLIENT_REQUEST_IP = "ClientRequestIP";
const char* ArchivingServiceSettings::PROP_CLIENT_REQUEST_PORT = "ClientRequestPort";
const char* ArchivingServiceSettings::PROP_CLIENT_REQUEST_NETMASK = "ClientRequestNetmask";

const char* ArchivingServiceSettings::PROP_APP_DATA_SERVICE_REQUEST_IP = "AppDataServiceRequestIP";
const char* ArchivingServiceSettings::PROP_APP_DATA_SERVICE_REQUEST_PORT = "AppDataServiceRequestPort";
const char* ArchivingServiceSettings::PROP_APP_DATA_SERVICE_REQUEST_NETMASK = "AppDataServiceRequestNetmask";

const char* ArchivingServiceSettings::PROP_DIAG_DATA_SERVICE_REQUEST_IP = "DiagDataServiceRequestIP";
const char* ArchivingServiceSettings::PROP_DIAG_DATA_SERVICE_REQUEST_PORT = "DiagDataServiceRequestPort";
const char* ArchivingServiceSettings::PROP_DIAG_DATA_SERVICE_REQUEST_NETMASK = "DiagDataServiceRequestNetmask";


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

	return result;
}



