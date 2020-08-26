#pragma once

#include "ConstStrings.h"
#include "DeviceHelper.h"
#include "XmlHelper.h"
#include "SocketIO.h"
#include "../Builder/IssueLogger.h"


const char* const CFG_FILE_ID_APP_DATA_SOURCES = "APP_DATA_SOURCES";
const char* const CFG_FILE_ID_APP_SIGNALS = "APP_SIGNALS";
const char* const CFG_FILE_ID_APP_SIGNAL_SET = "APP_SIGNAL_SET";
const char* const CFG_FILE_ID_COMPARATOR_SET = "COMPARATOR_SET";
const char* const CFG_FILE_ID_UNIT_SET = "UNIT_SET";

const char* const CFG_FILE_ID_TUNING_SOURCES = "TUNING_SOURCES";
const char* const CFG_FILE_ID_TUNING_SIGNALS = "TUNING_SIGNALS";
const char* const CFG_FILE_ID_TUNING_SCHEMAS_DETAILS = "TUNING_SCHEMAS_DETAILS";
const char* const CFG_FILE_ID_TUNING_FILTERS = "TUNING_FILTERS";
const char* const CFG_FILE_ID_TUNING_GLOBALSCRIPT = "TUNING_GLOBALSCRIPT";
const char* const CFG_FILE_ID_TUNING_CONFIGARRIVEDSCRIPT = "TUNING_CONFIGARRIVEDSCRIPT";

const char* const CFG_FILE_ID_BEHAVIOR = "CLIENT_BEHAVIOR";
const char* const CFG_FILE_ID_LOGO = "LOGO";

const char* const CFG_FILE_ID_METROLOGY_ITEMS = "METROLOGY_ITEMS";
const char* const CFG_FILE_ID_METROLOGY_SIGNAL_SET = "METROLOGY_SIGNAL_SET";

class ServiceSettings
{
public:
	// common properties of services
	//
	static const char* SETTINGS_SECTION;

	static const char* ATTR_COUNT;
	static const char* ATTR_EQUIIPMENT_ID;
	static const char* ATTR_SOFTWARE_TYPE;

public:
	static bool getSoftwareConnection(const Hardware::EquipmentSet* equipment,
										const Hardware::Software* thisSoftware,
										const QString& propConnectedSoftwareID,
										const QString& propConnectedSoftwareIP,
										const QString& propConnectedSoftwarePort,
										QString* connectedSoftwareID,
										HostAddressPort* connectedSoftwareIP,
										bool emptyAllowed,
										const QString &defaultIP,
										int defaultPort,
										E::SoftwareType requiredSoftwareType,
										Builder::IssueLogger* log);

	static bool getCfgServiceConnection(const Hardware::EquipmentSet* equipment,
										const Hardware::Software* software,
										QString* cfgServiceID1, HostAddressPort* cfgServiceAddrPort1,
										QString* cfgServiceID2, HostAddressPort* cfgServiceAddrPort2,
										Builder::IssueLogger* log);
};

class CfgServiceSettings : public ServiceSettings
{
private:
	static const char* CLIENTS_SECTION;
	static const char* CLIENT;
	static const char* CLIENT_EQUIPMENT_ID;
	static const char* CLIENT_SOFTWARE_TYPE;

public:
	HostAddressPort clientRequestIP;
	QHostAddress clientRequestNetmask;

	QList<QPair<QString, E::SoftwareType>> clients;

	bool writeToXml(XmlWriteHelper& xml);
	bool readFromXml(XmlReadHelper& xml);

	QStringList knownClients();
};

class AppDataServiceSettings : public ServiceSettings
{
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

class DiagDataServiceSettings : public ServiceSettings
{
public:
	QString cfgServiceID1;
	HostAddressPort cfgServiceIP1;

	QString cfgServiceID2;
	HostAddressPort cfgServiceIP2;

	HostAddressPort diagDataReceivingIP;
	QHostAddress diagDataReceivingNetmask;

	QString archServiceID;
	HostAddressPort archServiceIP;

	HostAddressPort clientRequestIP;
	QHostAddress clientRequestNetmask;

	bool readFromDevice(Hardware::EquipmentSet* equipment, Hardware::Software* software, Builder::IssueLogger* log);
	bool writeToXml(XmlWriteHelper& xml);
	bool readFromXml(XmlReadHelper& xml);
};

class TuningServiceSettings : public ServiceSettings
{
private:
	static const char* TUNING_CLIENTS;
	static const char* TUNING_CLIENT;
	static const char* TUNING_SOURCES;
	static const char* TUNING_SOURCE;

	bool fillTuningClientsInfo(Hardware::Software *software, bool singleLmControlEnabled, Builder::IssueLogger* log);

public:
	struct TuningClient
	{
		QString equipmentID;
		QStringList sourcesIDs;
	};

	HostAddressPort clientRequestIP;
	QHostAddress clientRequestNetmask;

	HostAddressPort tuningDataIP;
	QHostAddress tuningDataNetmask;

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

	static const char* PROP_ARCHIVE_SHORT_TERM_PERIOD;
	static const char* PROP_ARCHIVE_LONG_TERM_PERIOD;
	static const char* PROP_ARCHIVE_LOCATION;

public:
	HostAddressPort clientRequestIP;
	QHostAddress clientRequestNetmask;

	HostAddressPort appDataReceivingIP;
	QHostAddress appDataReceivingNetmask;

	HostAddressPort diagDataReceivingIP;
	QHostAddress diagDataReceivingNetmask;

	int shortTermArchivePeriod = 10;
	int longTermArchivePeriod = 365;

	QString archiveLocation;

	bool readFromDevice(Hardware::Software *software, Builder::IssueLogger* log);
	bool checkSettings(Hardware::Software *software, Builder::IssueLogger* log);

	bool writeToXml(XmlWriteHelper& xml);
	bool readFromXml(XmlReadHelper& xml);
};

class TestClientSettings : public ServiceSettings
{
public:
	static const char* CFG_SERVICE1_SECTION;
	static const char* CFG_SERVICE2_SECTION;
	static const char* APP_DATA_SERVICE_SECTION;
	static const char* DIAG_DATA_SERVICE_SECTION;
	static const char* ARCH_SERVICE_SECTION;
	static const char* TUNING_SERVICE_SECTION;
	static const char* PROP_TUNING_SERVICE_SECTION;

public:
	QString			cfgService1_equipmentID;
	HostAddressPort cfgService1_clientRequestIP;

	QString			cfgService2_equipmentID;
	HostAddressPort cfgService2_clientRequestIP;

	QString			appDataService_equipmentID;
	HostAddressPort appDataService_appDataReceivingIP;
	HostAddressPort appDataService_clientRequestIP;

	QString			diagDataService_equipmentID;
	HostAddressPort diagDataService_diagDataReceivingIP;
	HostAddressPort diagDataService_clientRequestIP;

	QString			archService_equipmentID;
	HostAddressPort archService_appDataReceivingIP;
	HostAddressPort archService_diagDataReceivingIP;
	HostAddressPort archService_clientRequestIP;

	QString			tuningService_equipmentID;
	HostAddressPort tuningService_tuningDataIP;
	HostAddressPort tuningService_clientRequestIP;
	QStringList		tuningService_tuningSources;

	bool readFromDevice(Hardware::EquipmentSet* equipment, Hardware::Software* software, Builder::IssueLogger* log);
	bool writeToXml(XmlWriteHelper& xml);
	bool readFromXml(XmlReadHelper& xml);
};


