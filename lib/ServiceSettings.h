#pragma once

#include "../lib/DeviceHelper.h"
#include "../lib/XmlHelper.h"
#include "../u7/Builder/IssueLogger.h"


const char* const CFG_FILE_ID_DATA_SOURCES = "APP_DATA_SOURCES";
const char* const CFG_FILE_ID_APP_SIGNALS = "APP_SIGNALS";
const char* const CFG_FILE_ID_APP_SIGNAL_SET = "APP_SIGNAL_SET";
const char* const CFG_FILE_ID_UNIT_SET = "UNIT_SET";

const char* const CFG_FILE_ID_TUNING_SIGNALS = "TUNING_SIGNALS";
const char* const CFG_FILE_ID_TUNING_SCHEMAS_DETAILS = "TUNING_SCHEMAS_DETAILS";
const char* const CFG_FILE_ID_TUNING_FILTERS = "TUNING_FILTERS";
const char* const CFG_FILE_ID_TUNING_GLOBALSCRIPT = "TUNING_GLOBALSCRIPT";

const char* const CFG_FILE_ID_METROLOGY_SIGNALS = "METROLOGY_SIGNALS";

class CfgServiceSettings
{
public:
	static const char* PROP_CLIENT_REQUEST_IP;
	static const char* PROP_CLIENT_REQUEST_NETMASK;
	static const char* PROP_CLIENT_REQUEST_PORT;
};

class AppDataServiceChannel
{
private:
	static const char* PROP_APP_DATA_NETMASK;
	static const char* PROP_APP_DATA_RECEIVING_IP;
	static const char* PROP_APP_DATA_RECEIVING_PORT;

	static const char* PROP_ARCH_SERVICE_ID;
	static const char* PROP_ARCH_SERVICE_IP;
	static const char* PROP_ARCH_SERVICE_PORT;

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

	bool readFromDevice(Hardware::EquipmentSet* equipment, Hardware::DeviceController* controller, Builder::IssueLogger* log);
	bool writeToXml(XmlWriteHelper& xml, int channel);
	bool readFromXml(XmlReadHelper& xml, int channel);
};


class AppDataServiceSettings
{
public:
	static const int DATA_CHANNEL_1 = 0;
	static const int DATA_CHANNEL_2 = 1;

	static const int DATA_CHANNEL_COUNT = 2;

private:
	static const char* DATA_CHANNEL_CONTROLLER_ID_FORMAT_STR;
	static const char* SECTION_NAME;
	static const char* PROP_CLIENT_REQUEST_IP;
	static const char* PROP_CLIENT_REQUEST_PORT;
	static const char* PROP_CLIENT_REQUEST_NETMASK;

	static const char* PROP_AUTO_ARCHIVE_INTERVAL;

public:
	HostAddressPort clientRequestIP;
	QHostAddress clientRequestNetmask;

	AppDataServiceChannel appDataServiceChannel[DATA_CHANNEL_COUNT];

	int autoArchiveInterval = 5;

	bool readFromDevice(Hardware::EquipmentSet* equipment, Hardware::Software* software, Builder::IssueLogger* log);
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
	static const char* PROP_SINGLE_LM_CONTROL;
	static const char* PROP_DISABLE_MODULES_TYPE_CHECKING;

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

	bool singleLmControl = true;
	bool disableModulesTypeChecking = false;

	QVector<TuningClient> clients;

	bool readFromDevice(Hardware::Software *software, Builder::IssueLogger* log);
	bool writeToXml(XmlWriteHelper& xml);
	bool readFromXml(XmlReadHelper& xml);
};


class ArchivingServiceSettings
{
public:
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

	static const char* PROP_ARCHIVE_DB_HOST_IP;
	static const char* PROP_ARCHIVE_DB_HOST_PORT;

public:
	HostAddressPort clientRequestIP;
	QHostAddress clientRequestNetmask;

	HostAddressPort appDataServiceRequestIP;
	QHostAddress appDataServiceRequestNetmask;

	HostAddressPort diagDataServiceRequestIP;
	QHostAddress diagDataServiceRequestNetmask;

	HostAddressPort dbHost = HostAddressPort("127.0.0.1", 5432);

	bool readFromDevice(Hardware::Software *software, Builder::IssueLogger* log);
	bool writeToXml(XmlWriteHelper& xml);
	bool readFromXml(XmlReadHelper& xml);
};


