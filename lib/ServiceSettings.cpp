#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include "../include/ServiceSettings.h"


// -------------------------------------------------------------------------------------
//
// DASEthernetChannel class implementation
//
// -------------------------------------------------------------------------------------

bool DASEthernetChannel::readFromDevice(Hardware::DeviceController* controller, OutputLog *log)
{
	bool result = true;

	QString appDataNetmaskStr;
	QString appDataReceivingIPStr;
	int appDataReceivingPort = 0;

	result &= DeviceHelper::getStrProperty(controller, PROP_APP_DATA_NETMASK, &appDataNetmaskStr, log);
	result &= DeviceHelper::getStrProperty(controller, PROP_APP_DATA_RECEIVING_IP, &appDataReceivingIPStr, log);
	result &= DeviceHelper::getIntProperty(controller, PROP_APP_DATA_RECEIVING_PORT, &appDataReceivingPort, log);

	QString diagDataNetmaskStr;
	QString diagDataReceivingIPStr;
	int diagDataReceivingPort = 0;

	result &= DeviceHelper::getStrProperty(controller, PROP_DIAG_DATA_NETMASK, &diagDataNetmaskStr, log);
	result &= DeviceHelper::getStrProperty(controller, PROP_DIAG_DATA_RECEIVING_IP, &diagDataReceivingIPStr, log);
	result &= DeviceHelper::getIntProperty(controller, PROP_DIAG_DATA_RECEIVING_PORT, &diagDataReceivingPort, log);

	result &= DeviceHelper::getStrProperty(controller, PROP_ARCH_SERVICE_ID, &archServiceStrID, log);

	result &= DeviceHelper::getStrProperty(controller, PROP_CFG_SERVICE_ID, &cfgServiceStrID, log);

	if (result == true)
	{
		appDataNetmask.setAddress(appDataNetmaskStr);
		appDataReceivingIP = HostAddressPort(appDataReceivingIPStr, appDataReceivingPort);

		diagDataNetmask.setAddress(diagDataNetmaskStr);
		diagDataReceivingIP = HostAddressPort(diagDataReceivingIPStr, diagDataReceivingPort);
	}

	return result;
}


QString DASEthernetChannel::sectionName(int channel)
{
	return QString(SECTION_FORMAT_STR).arg(channel + 1);
}


bool DASEthernetChannel::writeToXml(XmlWriteHelper& xml, int channel)
{
	xml.writeStartElement(sectionName(channel));

	xml.writeHostAddressPort(PROP_APP_DATA_RECEIVING_IP, PROP_APP_DATA_RECEIVING_PORT, appDataReceivingIP);
	xml.writeHostAddress(PROP_APP_DATA_NETMASK, appDataNetmask);

	xml.writeHostAddressPort(PROP_DIAG_DATA_RECEIVING_IP, PROP_DIAG_DATA_RECEIVING_PORT, diagDataReceivingIP);
	xml.writeHostAddress(PROP_DIAG_DATA_NETMASK, diagDataNetmask);

	xml.writeStringElement(PROP_ARCH_SERVICE_ID, archServiceStrID);

	xml.writeStringElement(PROP_CFG_SERVICE_ID, cfgServiceStrID);

	xml.writeEndElement();		//	</EthernetChannelN>

	return true;
}


bool DASEthernetChannel::readFromXml(XmlReadHelper& xml, int channel)
{
	if (xml.name() != sectionName(channel))
	{
		return false;
	}

	bool result = true;

	int ch = 0;

	result &= xml.readIntAttribute("Channel", &ch);

	if (ch != channel)
	{
		return false;
	}

	result &= xml.readHostAddressPort(PROP_APP_DATA_RECEIVING_IP, PROP_APP_DATA_RECEIVING_PORT, &appDataReceivingIP);
	result &= xml.readHostAddress(PROP_APP_DATA_NETMASK, &appDataNetmask);

	result &= xml.readHostAddressPort(PROP_DIAG_DATA_RECEIVING_IP, PROP_DIAG_DATA_RECEIVING_PORT, &diagDataReceivingIP);
	result &= xml.readHostAddress(PROP_DIAG_DATA_NETMASK, &diagDataNetmask);

	result &= xml.readStringElement(PROP_ARCH_SERVICE_ID, &archServiceStrID);
	result &= xml.readStringElement(PROP_CFG_SERVICE_ID, &cfgServiceStrID);

	return result;
}


// -------------------------------------------------------------------------------------
//
// DASSettings class implementation
//
// -------------------------------------------------------------------------------------

bool DASSettings::readFromDevice(Hardware::Software* software, OutputLog *log)
{
	bool result = true;

	QString clientRequestIPStr;
	int clientRequestPort = 0;
	QString clientRequestNetmaskStr;

	result &= DeviceHelper::getStrProperty(software, "ClientRequestIP", &clientRequestIPStr, log);
	result &= DeviceHelper::getIntProperty(software, "ClientRequestPort", &clientRequestPort, log);
	result &= DeviceHelper::getStrProperty(software, "ClientRequestNetmask", &clientRequestNetmaskStr, log);

	clientRequestIP = HostAddressPort(clientRequestIPStr, clientRequestPort);
	clientRequestNetmask.setAddress(clientRequestNetmaskStr);

	for(int channel = 0; channel < DATA_CHANNEL_COUNT; channel++)
	{
		Hardware::DeviceController* ethernet =
				DeviceHelper::getChildControllerBySuffix(software, QString("_ETHERNET0%1").arg(channel + 1));

		if (ethernet != nullptr)
		{
			result &= ethernetChannel[channel].readFromDevice(ethernet, log);
		}
	}

	return result;
}


bool DASSettings::writeToXml(XmlWriteHelper& xml)
{
	bool result = true;

	xml.writeStartElement(SECTION_NAME);

	xml.writeHostAddressPort(PROP_CLIENT_REQUEST_IP, PROP_CLIENT_REQUEST_PORT, clientRequestIP);
	xml.writeHostAddress(PROP_CLIENT_REQUEST_NETMASK, clientRequestNetmask);

	for(int channel = 0; channel < DATA_CHANNEL_COUNT; channel++)
	{
		ethernetChannel[channel].writeToXml(xml, channel);
	}

	xml.writeEndElement();	// </Settings>

	return result;
}


bool DASSettings::readFromXml(XmlReadHelper& xml)
{
	bool result = false;

	while(xml.atEnd() == false)
	{
		if (xml.readNextStartElement() == false)
		{
			continue;
		}

		qDebug() << xml.name();

		if (xml.name() == "Settings")
		{
			result = true;

			result &= xml.readHostAddressPort(PROP_CLIENT_REQUEST_IP, PROP_CLIENT_REQUEST_PORT, &clientRequestIP);

			result &= xml.readHostAddress(PROP_CLIENT_REQUEST_NETMASK, &clientRequestNetmask);

			for(int channel = 0; channel < DATA_CHANNEL_COUNT; channel++)
			{
				ethernetChannel[channel].readFromXml(xml, channel);
			}

			break;
		}
	}

	return result;
}


