#pragma once

#include "../lib/DeviceHelper.h"
#include "../lib/XmlHelper.h"
#include "../u7/Builder/IssueLogger.h"


const char* const CFG_FILE_ID_DATA_SOURCES = "APP_DATA_SOURCES";
const char* const CFG_FILE_ID_APP_SIGNALS = "APP_SIGNALS";
const char* const CFG_FILE_ID_TUNING_SIGNALS = "TUNING_SIGNALS";
const char* const CFG_FILE_ID_SCHEMAS_DETAILS = "TUNING_SCHEMAS";
const char* const CFG_FILE_ID_TUNING_FILTERS = "TUNING_FILTERS";
const char* const CFG_FILE_ID_METROLOGY_SIGNALS = "METROLOGY_SIGNALS";


class AppDataServiceChannel
{
private:
	static const char* PROP_APP_DATA_NETMASK;
	static const char* PROP_APP_DATA_RECEIVING_IP;
	static const char* PROP_APP_DATA_RECEIVING_PORT;

	static const char* PROP_ARCH_SERVICE_ID;

	static const char* PROP_CFG_SERVICE_ID;

	static const char* SECTION_FORMAT_STR;

	QString sectionName(int channel);			// channel from 0

public:
	HostAddressPort appDataReceivingIP;
	QHostAddress appDataNetmask;

	QString archServiceStrID;
	HostAddressPort archServiceIP;

	QString cfgServiceStrID;
	HostAddressPort cfgServiceIP;

	bool readFromDevice(Hardware::DeviceController *controller, Builder::IssueLogger* log);
	bool writeToXml(XmlWriteHelper& xml, int channel);
	bool readFromXml(XmlReadHelper& xml, int channel);
};


class AppDataServiceSettings
{
public:
	static const int DATA_CHANNEL_COUNT = 2;

private:
	static const char* DATA_CHANNEL_CONTROLLER_ID_FORMAT_STR;
	static const char* SECTION_NAME;
	static const char* PROP_CLIENT_REQUEST_IP;
	static const char* PROP_CLIENT_REQUEST_PORT;
	static const char* PROP_CLIENT_REQUEST_NETMASK;

public:
	HostAddressPort clientRequestIP;
	QHostAddress clientRequestNetmask;

	AppDataServiceChannel appDataServiceChannel[DATA_CHANNEL_COUNT];

	bool readFromDevice(Hardware::Software *software, Builder::IssueLogger* log);
	bool writeToXml(XmlWriteHelper& xml);
	bool readFromXml(XmlReadHelper& xml);
};


class TuningServiceSettings
{
private:
	static const char* SECTION_NAME;
	static const char* PROP_CLIENT_REQUEST_IP;
	static const char* PROP_CLIENT_REQUEST_PORT;
	static const char* PROP_CLIENT_REQUEST_NETMASK;
	static const char* PROP_TUNING_DATA_IP;
	static const char* PROP_TUNING_DATA_PORT;

	static const char* TUNING_MEMORY_SETTINGS_ELEMENT;
	static const char* PROP_TUNING_DATA_OFFSET;
	static const char* PROP_TUNING_DATA_SIZE;
	static const char* PROP_TUNING_ROM_FRAME_COUNT;
	static const char* PROP_TUNING_ROM_FRAME_SIZE;
	static const char* PROP_TUNING_ROM_SIZE;

	static const char* TUNING_CLIENTS;
	static const char* TUNING_CLIENT;
	static const char* TUNING_SOURCES;
	static const char* TUNING_SOURCE;
	static const char* ATTR_EQUIIPMENT_ID;
	static const char* ATTR_COUNT;

	bool fillTuningClientsInfo(Hardware::Software *software, Builder::IssueLogger* log);

public:
	struct TuningClient
	{
		QString equipmentID;
		QStringList sourcesIDs;
	};

	HostAddressPort clientRequestIP;
	QHostAddress clientRequestNetmask;

	HostAddressPort tuningDataIP;

	QVector<TuningClient> clients;

	int tuningDataOffsetW = 0;
	int tuningDataSizeW = 0;
	int tuningRomFrameCount = 0;
	int tuningRomFrameSizeW = 0;
	int tuningRomSizeW = 0;

	bool readFromDevice(Hardware::Software *software, Builder::IssueLogger* log);
	bool writeToXml(XmlWriteHelper& xml);
	bool readFromXml(XmlReadHelper& xml);
};


class ArchivingServiceSettings
{
public:

private:
	static const char* SECTION_NAME;

	static const char* PROP_CLIENT_REQUEST_IP;
	static const char* PROP_CLIENT_REQUEST_PORT;
	static const char* PROP_CLIENT_REQUEST_NETMASK;

	static const char* PROP_APP_DATA_SERVICE_REQUEST_IP;
	static const char* PROP_APP_DATA_SERVICE_REQUEST_PORT;
	static const char* PROP_APP_DATA_SERVICE_REQUEST_NETMASK;

	static const char* PROP_DIAG_DATA_SERVICE_REQUEST_IP;
	static const char* PROP_DIAG_DATA_SERVICE_REQUEST_PORT;
	static const char* PROP_DIAG_DATA_SERVICE_REQUEST_NETMASK;

public:
	HostAddressPort clientRequestIP;
	QHostAddress clientRequestNetmask;

	HostAddressPort appDataServiceRequestIP;
	QHostAddress appDataServiceRequestNetmask;

	HostAddressPort diagDataServiceRequestIP;
	QHostAddress diagDataServiceRequestNetmask;

	bool readFromDevice(Hardware::Software *software, Builder::IssueLogger* log);
	bool writeToXml(XmlWriteHelper& xml);
	bool readFromXml(XmlReadHelper& xml);
};


