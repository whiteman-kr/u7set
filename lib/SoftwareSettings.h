#pragma once

#include "ConstStrings.h"
#include "XmlHelper.h"
#include "SocketIO.h"
#include "WUtils.h"

#ifdef IS_BUILDER

#include "DeviceHelper.h"
#include "../Builder/Context.h"
#include "../Builder/IssueLogger.h"
#include "../TuningService/TuningSource.h"

#endif

class SoftwareSettings : public QObject
{
public:
	SoftwareSettings() = default;
	SoftwareSettings(const SoftwareSettings&);
	virtual ~SoftwareSettings();

	virtual bool writeToXml(XmlWriteHelper& xml) = 0;
	virtual bool readFromXml(XmlReadHelper& xml) = 0;
};

#ifdef IS_BUILDER

	class SoftwareSettingsGetter
	{
	public:
		virtual ~SoftwareSettingsGetter();

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

		static bool getLmPropertiesFromDevice(	const Hardware::DeviceModule* lm,
												DataSource::DataType dataType,
												int adapterNo,
												E::LanControllerType lanControllerType,
												const Builder::Context* context,
												DataSource* ds);

		virtual bool readFromDevice(const Builder::Context* context,
									const Hardware::Software* software) = 0;
	};

#endif

class CfgServiceSettings : public SoftwareSettings
{
public:
	HostAddressPort clientRequestIP;
	QHostAddress clientRequestNetmask;

	QList<QPair<QString, E::SoftwareType>> clients;

	//

	bool writeToXml(XmlWriteHelper& xml) override;
	bool readFromXml(XmlReadHelper& xml) override;

	QStringList knownClients();
};

#ifdef IS_BUILDER

	class CfgServiceSettingsGetter : public CfgServiceSettings, public SoftwareSettingsGetter
	{
	public:
		bool readFromDevice(const Builder::Context* context,
							const Hardware::Software* software) override;
	};

#endif

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

	bool writeToXml(XmlWriteHelper& xml) override;
	bool readFromXml(XmlReadHelper& xml) override;
};

#ifdef IS_BUILDER

	class AppDataServiceSettingsGetter : public AppDataServiceSettings, public SoftwareSettingsGetter
	{
	public:
		bool readFromDevice(const Builder::Context* context,
							const Hardware::Software* software) override;
	};

#endif


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

	bool writeToXml(XmlWriteHelper& xml) override;
	bool readFromXml(XmlReadHelper& xml) override;
};

#ifdef IS_BUILDER

	class DiagDataServiceSettingsGetter : public DiagDataServiceSettings, public SoftwareSettingsGetter
	{
	public:
		bool readFromDevice(const Builder::Context* context,
							const Hardware::Software* software) override;
	};

#endif

class TuningServiceSettings : public SoftwareSettings
{
public:
	TuningServiceSettings() = default;
	TuningServiceSettings(const TuningServiceSettings&) = default;

	struct TuningClient
	{
		QString equipmentID;
		QStringList sourcesIDs;
	};

	struct TuningSource
	{
		QString lmEquipmentID;
		QString portEquipmentID;
		HostAddressPort tuningDataIP;
	};

	QString equipmentID;

	HostAddressPort clientRequestIP;
	QHostAddress clientRequestNetmask;

	HostAddressPort tuningDataIP;
	QHostAddress tuningDataNetmask;

	bool singleLmControl = true;
	bool disableModulesTypeChecking = false;

	HostAddressPort tuningSimIP;			// for now this option isn't read from equipment
	                                        // it can be initialized from TuningService cmd line option -sim
	                                        // or inside Simulator for run TuningServiceCommunicator
	std::vector<TuningSource> sources;
	std::vector<TuningClient> clients;

	//

	bool writeToXml(XmlWriteHelper& xml) override;
	bool readFromXml(XmlReadHelper& xml) override;
};

#ifdef IS_BUILDER

	class TuningServiceSettingsGetter : public TuningServiceSettings, public SoftwareSettingsGetter
	{
	public:
		bool readFromDevice(const Builder::Context* context,
							const Hardware::Software* software) override;
	private:
		bool fillTuningSourcesInfo(const Builder::Context* context,
								   const Hardware::Software* software);

		bool fillTuningClientsInfo(const Builder::Context* context,
								   const Hardware::Software* software,
								   bool singleLmControlEnabled);

	public:
		QVector<Tuning::TuningSource> tuningSources;
	};

#endif

class ArchivingServiceSettings : public SoftwareSettings
{
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

	bool writeToXml(XmlWriteHelper& xml) override;
	bool readFromXml(XmlReadHelper& xml) override;

	const ArchivingServiceSettings& operator = (const ArchivingServiceSettings& src);
};

#ifdef IS_BUILDER

	class ArchivingServiceSettingsGetter : public ArchivingServiceSettings, public SoftwareSettingsGetter
	{
	public:
		bool readFromDevice(const Builder::Context* context,
							const Hardware::Software* software) override;

		bool checkSettings(const Hardware::Software* software, Builder::IssueLogger* log);
	};

#endif

class TestClientSettings : public SoftwareSettings
{
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

	bool writeToXml(XmlWriteHelper& xml) override;
	bool readFromXml(XmlReadHelper& xml) override;
};

#ifdef IS_BUILDER

	class TestClientSettingsGetter : public TestClientSettings, public SoftwareSettingsGetter
	{
	public:

		bool readFromDevice(const Builder::Context* context,
							const Hardware::Software* software) override;
	};

#endif

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

	bool writeToXml(XmlWriteHelper& xmlWriter) override;
	bool readFromXml(XmlReadHelper& xmlReader) override;
};

#ifdef IS_BUILDER

	class MetrologySettingsGetter : public MetrologySettings, public SoftwareSettingsGetter
	{
	public:
		bool readFromDevice(const Builder::Context* context,
							const Hardware::Software* software) override;
	};

#endif

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

	bool writeToXml(XmlWriteHelper& xmlWriter) override;
	bool readFromXml(XmlReadHelper& xmlReader) override;

	QStringList getSchemaTags() const;
	QStringList getTuningSources() const;

	void clear();
};

#ifdef IS_BUILDER

	class MonitorSettingsGetter : public MonitorSettings, public SoftwareSettingsGetter
	{
	public:
		bool readFromDevice(const Builder::Context* context,
							const Hardware::Software* software) override;


	private:
		bool readAppDataServiceAndArchiveSettings(const Builder::Context* context,
												  const Hardware::Software* software);

		bool readTuningSettings(const Builder::Context* context,
								const Hardware::Software* software);
	};

#endif

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

	bool writeToXml(XmlWriteHelper& xmlWriter) override;
	bool readFromXml(XmlReadHelper& xmlReader) override;

	QStringList getSchemaTags() const;
	QStringList getUsersAccounts() const;
};

#ifdef IS_BUILDER

	class TuningClientSettingsGetter : public TuningClientSettings, public SoftwareSettingsGetter
	{
	public:
		bool readFromDevice(const Builder::Context* context,
							const Hardware::Software* software) override;
	};

#endif

