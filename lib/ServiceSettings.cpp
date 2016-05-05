#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include "../include/ServiceSettings.h"


// -------------------------------------------------------------------------------------
//
// DASEthernetChannel class implementation
//
// -------------------------------------------------------------------------------------

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
// DASSettings class implementation
//
// -------------------------------------------------------------------------------------

bool AppDataServiceSettings::readFromDevice(Hardware::Software* software, Builder::IssueLogger* log)
{
	bool result = true;

	QString clientRequestIPStr;
	int clientRequestPort = 0;
	QString clientRequestNetmaskStr;

	result &= DeviceHelper::getStrProperty(software, PROP_CLIENT_REQUEST_IP, &clientRequestIPStr, log);
	result &= DeviceHelper::getIntProperty(software, PROP_CLIENT_REQUEST_PORT, &clientRequestPort, log);
	result &= DeviceHelper::getStrProperty(software, PROP_CLIENT_REQUEST_NETMASK, &clientRequestNetmaskStr, log);

	clientRequestIP = HostAddressPort(clientRequestIPStr, clientRequestPort);
	clientRequestNetmask.setAddress(clientRequestNetmaskStr);

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


