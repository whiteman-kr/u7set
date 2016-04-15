#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include "../include/ServicesSettings.h"


bool DataAcquisitionServiceEthernetChannel::readFromDevice(Hardware::DeviceObject* device, int channel, OutputLog *log)
{
	if (channel < CHANNEL1 || channel > CHANNEL2)
	{
		assert(false);
		return false;
	}

	bool result = true;

	QString appDataNetmaskStr;
	QString appDataReceivingIPStr;
	int appDataReceivingPort = 0;

	int no = channel + 1;

	result &= DeviceHelper::getStrProperty(device, QString("AppDataNetmask%1").arg(no), &appDataNetmaskStr, log);
	result &= DeviceHelper::getStrProperty(device, QString("AppDataReceivingIP%1").arg(no), &appDataReceivingIPStr, log);
	result &= DeviceHelper::getIntProperty(device, QString("AppDataReceivingPort%1").arg(no), &appDataReceivingPort, log);

	QString diagDataNetmaskStr;
	QString diagDataReceivingIPStr;
	int diagDataReceivingPort = 0;

	result &= DeviceHelper::getStrProperty(device, QString("DiagDataNetmask%1").arg(no), &diagDataNetmaskStr, log);
	result &= DeviceHelper::getStrProperty(device, QString("DiagDataReceivingIP%1").arg(no), &diagDataReceivingIPStr, log);
	result &= DeviceHelper::getIntProperty(device, QString("DiagDataReceivingPort%1").arg(no), &diagDataReceivingPort, log);


	QString archivingServiceLocalIPStr;

	result &= DeviceHelper::getStrProperty(device, QString("ArchivingServiceLocalIP%1").arg(no), &archivingServiceLocalIPStr, log);
	result &= DeviceHelper::getStrProperty(device, QString("ArchivingServiceID%1").arg(no), &archivingServiceStrID, log);

	QString cfgServiceLocalIPStr;

	result &= DeviceHelper::getStrProperty(device, QString("CfgServiceLocalIP%1").arg(no), &cfgServiceLocalIPStr, log);
	result &= DeviceHelper::getStrProperty(device, QString("CfgServiceID%1").arg(no), &cfgServiceStrID, log);

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


bool DataAcquisitionServiceSettings::readFromDevice(Hardware::DeviceObject* device, OutputLog *log)
{
	bool result = true;

	QString clientRequestIPStr;
	int clientRequestPort = 0;
	QString clientRequestNetmaskStr;

	result &= DeviceHelper::getStrProperty(device, "ClientRequestIP", &clientRequestIPStr, log);
	result &= DeviceHelper::getIntProperty(device, "ClientRequestPort", &clientRequestPort, log);
	result &= DeviceHelper::getStrProperty(device, "ClientRequestNetmask", &clientRequestNetmaskStr, log);

	clientRequestIP = HostAddressPort(clientRequestIPStr, clientRequestPort);
	clientRequestNetmask.setAddress(clientRequestNetmaskStr);

	result &= ethernetChannel[CHANNEL1].readFromDevice(device, CHANNEL1, log);
	result &= ethernetChannel[CHANNEL2].readFromDevice(device, CHANNEL2, log);

	return result;
}


bool DataAcquisitionServiceSettings::writeToXml(QXmlStreamWriter& xml)
{
	bool result = true;

	xml.writeStartElement("DataAcquisitionServiceSettings");

	XmlHelper::writeHostAddressPortElement(xml, "ClientRequestIP", clientRequestIP);

	xml.writeEndElement();

	xml.writeEndElement();

	return result;
}


bool DataAcquisitionServiceSettings::readFromXml(QXmlStreamWriter& xml)
{
	bool result = true;

	return result;

}


