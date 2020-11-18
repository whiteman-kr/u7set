#pragma once

#include "ConstStrings.h"
#include "DeviceHelper.h"
#include "XmlHelper.h"
#include "SocketIO.h"
#include "WUtils.h"

#include "../Builder/IssueLogger.h"

class SoftwareSettings : public QObject
{
public:
	virtual ~SoftwareSettings();

	static bool getSoftwareConnection(const Hardware::EquipmentSet* equipment,
										const Hardware::Software* thisSoftware,
										const QString& propConnectedSoBftwareID,
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

	virtual bool readFromDevice(const Hardware::EquipmentSet* equipment,
								const Hardware::Software* software,
								Builder::IssueLogger* log) = 0;

	virtual bool writeToXml(XmlWriteHelper& xml) = 0;
	virtual bool readFromXml(XmlReadHelper& xml) = 0;
};

class CfgServiceSettings : public SoftwareSettings
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

	//

	bool readFromDevice(const Hardware::EquipmentSet* equipment,
						const Hardware::Software* software,
						Builder::IssueLogger* log) override;

	bool writeToXml(XmlWriteHelper& xml) override;
	bool readFromXml(XmlReadHelper& xml) override;

	QStringList knownClients();
};

class AppDataServiceSettings : public SoftwareSettings
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

	//

	bool readFromDevice(const Hardware::EquipmentSet* equipment,
						const Hardware::Software* software,
						Builder::IssueLogger* log) override;
	bool writeToXml(XmlWriteHelper& xml) override;
	bool readFromXml(XmlReadHelper& xml) override;
};

class DiagDataServiceSettings : public SoftwareSettings
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

	//

	bool readFromDevice(const Hardware::EquipmentSet* equipment,
						const Hardware::Software* software,
						Builder::IssueLogger* log) override;
	bool writeToXml(XmlWriteHelper& xml) override;
	bool readFromXml(XmlReadHelper& xml) override;
};

class TuningServiceSettings : public SoftwareSettings
{
private:
	static const char* TUNING_CLIENTS;
	static const char* TUNING_CLIENT;
	static const char* TUNING_SOURCES;
	static const char* TUNING_SOURCE;

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

	//

	bool readFromDevice(const Hardware::EquipmentSet* equipment,
						const Hardware::Software* software,
						Builder::IssueLogger* log) override;

	bool writeToXml(XmlWriteHelper& xml) override;
	bool readFromXml(XmlReadHelper& xml) override;

private:
	bool fillTuningClientsInfo(const Hardware::Software* software, bool singleLmControlEnabled, Builder::IssueLogger* log);
};

class ArchivingServiceSettings : public SoftwareSettings
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

	//

	bool readFromDevice(const Hardware::EquipmentSet* equipment,
						const Hardware::Software* software,
						Builder::IssueLogger* log) override;

	bool writeToXml(XmlWriteHelper& xml) override;
	bool readFromXml(XmlReadHelper& xml) override;

	bool checkSettings(const Hardware::Software* software, Builder::IssueLogger* log);

	const ArchivingServiceSettings& operator = (const ArchivingServiceSettings& src);
};

class TestClientSettings : public SoftwareSettings
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

	//

	bool readFromDevice(const Hardware::EquipmentSet* equipment,
						const Hardware::Software* software,
						Builder::IssueLogger* log) override;

	bool writeToXml(XmlWriteHelper& xml) override;
	bool readFromXml(XmlReadHelper& xml) override;
};

class MetrologySettings : public SoftwareSettings
{
public:
	bool appDataServicePropertyIsValid1 = false;
	QString appDataServiceID1;
	QString appDataServiceIP1;
	int appDataServicePort1 = 0;

	bool appDataServicePropertyIsValid2 = false;
	QString appDataServiceID2;
	QString appDataServiceIP2;
	int appDataServicePort2 = 0;

	bool tuningServicePropertyIsValid = false;
	QString tuningServiceID;
	QString softwareMetrologyID;
	QString tuningServiceIP;
	int tuningServicePort = 0;

	//

	bool readFromDevice(const Hardware::EquipmentSet* equipment,
						const Hardware::Software* software,
						Builder::IssueLogger* log) override;

	bool writeToXml(XmlWriteHelper& xmlWriter) override;
	bool readFromXml(XmlReadHelper& xmlReader) override;
};

class MonitorSettings : public SoftwareSettings
{
public:
	QString startSchemaId;
	QString schemaTags;

	QString appDataServiceID1;
	QString appDataServiceIP1;
	int appDataServicePort1 = 0;
	QString realtimeDataIP1;
	int realtimeDataPort1 = 0;

	QString appDataServiceID2;
	QString appDataServiceIP2;
	int appDataServicePort2 = 0;
	QString realtimeDataIP2;
	int realtimeDataPort2 = 0;

	QString archiveServiceID1;
	QString archiveServiceIP1;
	int archiveServicePort1 = 0;

	QString archiveServiceID2;
	QString archiveServiceIP2;
	int archiveServicePort2 = 0;

	bool tuningEnabled = false;
	QString tuningServiceID;
	QString tuningServiceIP;
	int tuningServicePort = 0;
	QString tuningSources;

	//

	bool readFromDevice(const Hardware::EquipmentSet* equipment,
						const Hardware::Software* software,
						Builder::IssueLogger* log) override;

	bool writeToXml(XmlWriteHelper& xmlWriter) override;
	bool readFromXml(XmlReadHelper& xmlReader) override;

	QStringList getSchemaTags() const;
	QStringList getTuningSources() const;

private:
	void clear();

	bool readAppDataServiceAndArchiveSettings(const Hardware::EquipmentSet* equipment,
								   const Hardware::Software* software,
								   Builder::IssueLogger* log);

	bool readTuningSettings(const Hardware::EquipmentSet* equipment,
								   const Hardware::Software* software,
								   Builder::IssueLogger* log);

};

class TuningClientSettings : public SoftwareSettings
{
public:
	QString tuningServiceID;
	QString tuningServiceIP;
	int tuningServicePort = 0;

	bool autoApply = false;
	bool showSignals = false;
	bool showSchemas = false;
	bool showSchemasList = false;
	bool showSchemasTabs = false;
	bool showSOR = false;
	bool useAccessFlag = false;
	bool loginPerOperation = false;
	QString usersAccounts;
	int loginSessionLength = 0;

	bool filterByEquipment = false;
	bool filterBySchema = false;

	QString startSchemaID;

	QString schemaTags;

	//

	bool readFromDevice(const Hardware::EquipmentSet* equipment,
						const Hardware::Software* software,
						Builder::IssueLogger* log) override;

	bool writeToXml(XmlWriteHelper& xmlWriter) override;
	bool readFromXml(XmlReadHelper& xmlReader) override;

	QStringList getSchemaTags() const;
	QStringList getUsersAccounts() const;
};
