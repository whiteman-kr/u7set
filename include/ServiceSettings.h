#pragma once

#include "../include/DeviceHelper.h"
#include "../include/XmlHelper.h"
#include "../u7/Builder/IssueLogger.h"


class DASEthernetChannel
{
private:
	const char* const PROP_APP_DATA_NETMASK = "AppDataNetmask";
	const char* const PROP_APP_DATA_RECEIVING_IP = "AppDataReceivingIP";
	const char* const PROP_APP_DATA_RECEIVING_PORT = "AppDataReceivingPort";

	const char* const PROP_DIAG_DATA_NETMASK = "DiagDataNetmask";
	const char* const PROP_DIAG_DATA_RECEIVING_IP = "DiagDataReceivingIP";
	const char* const PROP_DIAG_DATA_RECEIVING_PORT = "DiagDataReceivingPort";

	const char* const PROP_ARCH_SERVICE_ID = "ArchiveServiceID";

	const char* const PROP_CFG_SERVICE_ID = "ConfigurationServiceID";

	const char* const SECTION_FORMAT_STR = "EthernetChannel%1";

	QString sectionName(int channel);			// channel from 0

public:
	HostAddressPort appDataReceivingIP;
	QHostAddress appDataNetmask;

	HostAddressPort diagDataReceivingIP;
	QHostAddress diagDataNetmask;

	QString archServiceStrID;
	HostAddressPort archServiceIP;

	QString cfgServiceStrID;
	HostAddressPort cfgServiceIP;

	bool readFromDevice(Hardware::DeviceController *controller, OutputLog *log);
	bool writeToXml(XmlWriteHelper& xml, int channel);
	bool readFromXml(XmlReadHelper& xml, int channel);
};


class DASSettings
{
private:
	const int DATA_CHANNEL_COUNT = 2;

	const char* const SECTION_NAME = "Settings";
	const char* const PROP_CLIENT_REQUEST_IP = "ClientRequestIP";
	const char* const PROP_CLIENT_REQUEST_PORT = "ClientRequestPort";
	const char* const PROP_CLIENT_REQUEST_NETMASK = "ClientRequestNetmask";

public:
	HostAddressPort clientRequestIP;
	QHostAddress clientRequestNetmask;

	DASEthernetChannel ethernetChannel[2];

	bool readFromDevice(Hardware::Software *software, OutputLog* log);
	bool writeToXml(XmlWriteHelper& xml);
	bool readFromXml(XmlReadHelper& xml);
};

