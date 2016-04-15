#pragma once

#include "../include/DeviceHelper.h"
#include "../include/XmlHelper.h"


const int	CHANNEL1 = 0,
			CHANNEL2 = 1;


struct DataAcquisitionServiceEthernetChannel
{
	HostAddressPort appDataReceivingIP;
	QHostAddress appDataNetmask;

	HostAddressPort diagDataReceivingIP;
	QHostAddress diagDataNetmask;

	QString archivingServiceStrID;
	HostAddressPort archivingServiceIP;
	QHostAddress archivingServiceLocalIP;

	QString cfgServiceStrID;
	HostAddressPort cfgServiceIP;
	QHostAddress cfgServiceLocalIP;

	bool readFromDevice(Hardware::DeviceObject* device, int channel, OutputLog *log);
	bool writeToXml(QXmlStreamWriter& xml, int channel);
	bool readFromXml(QXmlStreamWriter& xml, int channel);
};


struct DataAcquisitionServiceSettings
{
	HostAddressPort clientRequestIP;
	QHostAddress clientRequestNetmask;

	DataAcquisitionServiceEthernetChannel ethernetChannel[2];

	bool readFromDevice(Hardware::DeviceObject* device, OutputLog* log);
	bool writeToXml(QXmlStreamWriter& xml);
	bool readFromXml(QXmlStreamWriter& xml);
};

