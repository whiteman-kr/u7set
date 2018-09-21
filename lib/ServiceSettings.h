#pragma once

#include "../lib/DeviceHelper.h"
#include "../lib/XmlHelper.h"
#include "../u7/Builder/IssueLogger.h"


const char* const CFG_FILE_ID_APP_DATA_SOURCES = "APP_DATA_SOURCES";
const char* const CFG_FILE_ID_APP_SIGNALS = "APP_SIGNALS";
const char* const CFG_FILE_ID_APP_SIGNAL_SET = "APP_SIGNAL_SET";
const char* const CFG_FILE_ID_UNIT_SET = "UNIT_SET";

const char* const CFG_FILE_ID_TUNING_SOURCES = "TUNING_SOURCES";
const char* const CFG_FILE_ID_TUNING_SIGNALS = "TUNING_SIGNALS";
const char* const CFG_FILE_ID_TUNING_SCHEMAS_DETAILS = "TUNING_SCHEMAS_DETAILS";
const char* const CFG_FILE_ID_TUNING_FILTERS = "TUNING_FILTERS";
const char* const CFG_FILE_ID_TUNING_GLOBALSCRIPT = "TUNING_GLOBALSCRIPT";

const char* const CFG_FILE_ID_METROLOGY_SIGNALS = "METROLOGY_SIGNALS";

class ServiceSettings
{
public:
	// common properties of services
	//
	static const char* SETTINGS_SECTION;

	static const char* PROP_APP_DATA_RECEIVING_NETMASK;
	static const char* PROP_APP_DATA_RECEIVING_IP;
	static const char* PROP_APP_DATA_RECEIVING_PORT;

	static const char* PROP_DIAG_DATA_RECEIVING_NETMASK;
	static const char* PROP_DIAG_DATA_RECEIVING_IP;
	static const char* PROP_DIAG_DATA_RECEIVING_PORT;

	static const char* PROP_TUNING_DATA_NETMASK;
	static const char* PROP_TUNING_DATA_IP;
	static const char* PROP_TUNING_DATA_PORT;

	static const char* PROP_CLIENT_REQUEST_IP;
	static const char* PROP_CLIENT_REQUEST_NETMASK;
	static const char* PROP_CLIENT_REQUEST_PORT;

	static const char* PROP_RT_TRENDS_REQUEST_IP;
	static const char* PROP_RT_TRENDS_REQUEST_PORT;

	static const char* PROP_APP_DATA_SERVICE_ID;
	static const char* PROP_APP_DATA_SERVICE_IP;
	static const char* PROP_APP_DATA_SERVICE_PORT;

	static const char* PROP_DIAG_DATA_SERVICE_ID;
	static const char* PROP_DIAG_DATA_SERVICE_IP;
	static const char* PROP_DIAG_DATA_SERVICE_PORT;

	static const char* PROP_ARCH_SERVICE_ID;
	static const char* PROP_ARCH_SERVICE_IP;
	static const char* PROP_ARCH_SERVICE_PORT;

	static const char* PROP_TUNING_SERVICE_ID;
	static const char* PROP_TUNING_SERVICE_IP;
	static const char* PROP_TUNING_SERVICE_PORT;

	static const char* PROP_CFG_SERVICE_ID1;
	static const char* PROP_CFG_SERVICE_IP1;
	static const char* PROP_CFG_SERVICE_PORT1;

	static const char* PROP_CFG_SERVICE_ID2;
	static const char* PROP_CFG_SERVICE_IP2;
	static const char* PROP_CFG_SERVICE_PORT2;

public:
	static bool getSoftwareConnection(	const Hardware::EquipmentSet* equipment,
										const Hardware::Software* thisSoftware,
										const QString& propConnectedSoftwareID,
										const QString& propConnectedSoftwareIP,
										const QString& propConnectedSoftwarePort,
										QString* connectedSoftwareID,
										HostAddressPort* connectedSoftwareIP,
										bool emptyAllowed, const QString &defaultIP, int defaultPort,
										Builder::IssueLogger* log);

	static bool getCfgServiceConnection(const Hardware::EquipmentSet* equipment,
										const Hardware::Software* software,
										QString* cfgServiceID1, HostAddressPort* cfgServiceAddrPort1,
										QString* cfgServiceID2, HostAddressPort* cfgServiceAddrPort2,
										Builder::IssueLogger* log);
};

class CfgServiceSettings : public ServiceSettings
{
public:
};

class AppDataServiceSettings : public ServiceSettings
{
private:
	static const char* PROP_AUTO_ARCHIVE_INTERVAL;

public:
	QString cfgServiceID1;
	HostAddressPort cfgServiceIP1;

	QString cfgServiceID2;
	HostAddressPort cfgServiceIP2;

	HostAddressPort appDataReceivingIP;
	QHostAddress appDataReceivingNetmask;

	int autoArchiveInterval = 5;

	QString archServiceID;
	HostAddressPort archServiceIP;

	HostAddressPort clientRequestIP;
	QHostAddress clientRequestNetmask;

	HostAddressPort rtTrendsRequestIP;

	bool readFromDevice(Hardware::EquipmentSet* equipment, Hardware::Software* software, Builder::IssueLogger* log);
	bool writeToXml(XmlWriteHelper& xml);
	bool readFromXml(XmlReadHelper& xml);
};


class TuningServiceSettings : public ServiceSettings
{
private:
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


class ArchivingServiceSettings : public ServiceSettings
{
public:
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


