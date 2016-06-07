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

	return result;
}



