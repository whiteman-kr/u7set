#pragma once

#include "../include/DeviceHelper.h"
#include "../include/XmlHelper.h"
#include "../u7/Builder/IssueLogger.h"


const int	ETHERNET01 = 1,
			ETHERNET02 = 2;


struct DASEthernetChannel
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

	bool readFromDevice(Hardware::DeviceController *controller, OutputLog *log);
	bool writeToXml(QXmlStreamWriter& xmlWriter, int channel);
	bool readFromXml(QXmlStreamReader& xml, int channel);
};


struct DASSettings
{
	HostAddressPort clientRequestIP;
	QHostAddress clientRequestNetmask;

	DASEthernetChannel ethernetChannel[2];

	bool readFromDevice(Hardware::Software *software, OutputLog* log);
	bool writeToXml(QXmlStreamWriter& xmlWriter);
	bool readFromXml(QXmlStreamReader& xmlReader);
};

