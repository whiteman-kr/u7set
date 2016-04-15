#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include "../include/ServicesSettings.h"


bool DataAcquisitionServiceEthernetChannel::readFromDevice(Hardware::DeviceObject* device, int channel, OutputLog *log)
{
	bool result = true;

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

	return result;
}


bool DataAcquisitionServiceSettings::readFromXml(QXmlStreamWriter& xml)
{
	bool result = true;

	return result;

}


