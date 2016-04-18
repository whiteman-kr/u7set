#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include "../include/ServiceSettings.h"


bool DASEthernetChannel::readFromDevice(Hardware::DeviceController* controller, OutputLog *log)
{
	bool result = true;

	QString appDataNetmaskStr;
	QString appDataReceivingIPStr;
	int appDataReceivingPort = 0;

	result &= DeviceHelper::getStrProperty(controller, QString("AppDataNetmask"), &appDataNetmaskStr, log);
	result &= DeviceHelper::getStrProperty(controller, QString("AppDataReceivingIP"), &appDataReceivingIPStr, log);
	result &= DeviceHelper::getIntProperty(controller, QString("AppDataReceivingPort"), &appDataReceivingPort, log);

	QString diagDataNetmaskStr;
	QString diagDataReceivingIPStr;
	int diagDataReceivingPort = 0;

	result &= DeviceHelper::getStrProperty(controller, QString("DiagDataNetmask"), &diagDataNetmaskStr, log);
	result &= DeviceHelper::getStrProperty(controller, QString("DiagDataReceivingIP"), &diagDataReceivingIPStr, log);
	result &= DeviceHelper::getIntProperty(controller, QString("DiagDataReceivingPort"), &diagDataReceivingPort, log);

	QString archivingServiceLocalIPStr;

	result &= DeviceHelper::getStrProperty(controller, QString("ArchivingServiceLocalIP"), &archivingServiceLocalIPStr, log);
	result &= DeviceHelper::getStrProperty(controller, QString("ArchivingServiceID"), &archivingServiceStrID, log);

	QString cfgServiceLocalIPStr;

	result &= DeviceHelper::getStrProperty(controller, QString("CfgServiceLocalIP"), &cfgServiceLocalIPStr, log);
	result &= DeviceHelper::getStrProperty(controller, QString("CfgServiceID"), &cfgServiceStrID, log);

	if (result == true)
	{
		appDataNetmask.setAddress(appDataNetmaskStr);
		appDataReceivingIP = HostAddressPort(appDataReceivingIPStr, appDataReceivingPort);

		diagDataNetmask.setAddress(diagDataNetmaskStr);
		diagDataReceivingIP = HostAddressPort(diagDataReceivingIPStr, diagDataReceivingPort);

		archivingServiceLocalIP.setAddress(archivingServiceLocalIPStr);
		cfgServiceLocalIP.setAddress(cfgServiceLocalIPStr);
	}

	return result;
}


bool DASEthernetChannel::writeToXml(QXmlStreamWriter& xmlWriter, int channel)
{
	XmlHelper xml(xmlWriter);

	xml.writeStartElement("Ethernet");
	xml.writeAttribute("Channel", QString::number(channel));

	xml.writeHostAddressPortElement("AppDataReceivingIP", appDataReceivingIP);
	xml.writeHostAddressElement("AppDataNetmask", appDataNetmask);

	xml.writeHostAddressPortElement("DiagDataReceivingIP", diagDataReceivingIP);
	xml.writeHostAddressElement("DiagDataNetmask", diagDataNetmask);

	xml.writeTextElement("ArchivingServiceID", archivingServiceStrID);
	xml.writeHostAddressElement("ArchivingServiceLocalIP", archivingServiceLocalIP);

	xml.writeTextElement("CfgServiceID", cfgServiceStrID);
	xml.writeHostAddressElement("CfgServiceLocalIP", cfgServiceLocalIP);

	xml.writeEndElement();		//	</Ethernet>

	return true;
}


bool DASEthernetChannel::readFromXml(QXmlStreamReader& xmlReader, int channel)
{
	if (xmlReader.name() != "Ethernet")
	{
		return false;
	}

	bool result = true;

	XmlHelper xml(xmlReader);

	int ch = 0;

	result &= xml.readIntAttribute("Channel", &ch);

	if (ch != channel)
	{
		return false;
	}

	xmlReader.readNextStartElement();
	result &= xml.readHostAddressPortElement("AppDataReceivingIP", &appDataReceivingIP);

	xmlReader.readNextStartElement();
	result &= xml.readHostAddressElement("AppDataNetmask", &appDataNetmask);

	xmlReader.readNextStartElement();
	result &= xml.readHostAddressPortElement("DiagDataReceivingIP", &diagDataReceivingIP);

	///////////////////////////////////////////////////////////////////////////


	return result;
}




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

	for(int channel = 0; channel <= 1; channel++)
	{
		Hardware::DeviceController* ethernet =
				DeviceHelper::getChildControllerBySuffix(software, QString("_ETHERNET0%1").arg(ETHERNET01 + channel));

		if (ethernet != nullptr)
		{
			result &= ethernetChannel[channel].readFromDevice(ethernet, log);
		}
	}

	return result;
}


bool DASSettings::writeToXml(QXmlStreamWriter& xmlWriter)
{
	bool result = true;

	XmlHelper xml(xmlWriter);

	xml.writeStartElement("Settings");

	xml.writeHostAddressPortElement("ClientRequestIP", clientRequestIP);
	xml.writeHostAddressElement("ClientRequestNetmask", clientRequestNetmask);

	for(int channel = 0; channel <= 1; channel++)
	{
		ethernetChannel[channel].writeToXml(xmlWriter, ETHERNET01 + channel);
	}

	xml.writeEndElement();	// </Settings>

	return result;
}


bool DASSettings::readFromXml(QXmlStreamReader &xmlReader)
{
	XmlHelper xml(xmlReader);

	bool result = false;

	while(xmlReader.atEnd() == false)
	{
		if (xmlReader.readNextStartElement() == false)
		{
			continue;
		}

		//QString name = xmlReader.name().toString();

		qDebug() << xmlReader.name().toString();

		if (xmlReader.name() == "Settings")
		{
			result = true;

			xmlReader.readNextStartElement();
			result &= xml.readHostAddressPortElement("ClientRequestIP", &clientRequestIP);

			xmlReader.readNextStartElement();
			result &= xml.readHostAddressElement("ClientRequestNetmask", &clientRequestNetmask);

			xmlReader.readNextStartElement();
			ethernetChannel[0].readFromXml(xmlReader, ETHERNET01);

			xmlReader.readNextStartElement();
			ethernetChannel[1].readFromXml(xmlReader, ETHERNET02);

			break;
		}
	}

	return result;
}


