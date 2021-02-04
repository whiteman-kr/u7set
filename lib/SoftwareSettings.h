#pragma once

#include "ConstStrings.h"
#include "XmlHelper.h"
#include "SocketIO.h"
#include "WUtils.h"
#include "../Proto/network.pb.h"

#ifdef IS_BUILDER

#include "DeviceHelper.h"
#include "../Builder/Context.h"
#include "../Builder/IssueLogger.h"
#include "../TuningService/TuningSource.h"

#endif

struct SessionParams
{
	QString currentSettingsProfile;
	E::SoftwareRunMode softwareRunMode = E::SoftwareRunMode::Normal;

	void saveTo(Network::SessionParams* sp);
	void loadFrom(const Network::SessionParams& sp);
};

class SoftwareSettings : public QObject
{
public:
	SoftwareSettings() = default;
	SoftwareSettings(const SoftwareSettings& copy);
	SoftwareSettings(const QString& profile);
	virtual ~SoftwareSettings();

	const SoftwareSettings& operator = (const SoftwareSettings& copy);

protected:
	void writeStartSettings(XmlWriteHelper& xml) const;
	void writeEndSettings(XmlWriteHelper& xml) const;

	bool startSettingsReading(XmlReadHelper& xml);

private:
	// this methods should be call by SoftwareSettingsSet only
	//
	virtual bool writeToXml(XmlWriteHelper& xml) const = 0;
	virtual bool readFromXml(XmlReadHelper& xml) = 0;

	friend class SoftwareSettingsSet;

public:
	QString profile;
};

class SoftwareSettingsSet
{
public:
	SoftwareSettingsSet(E::SoftwareType softwareType);
	SoftwareSettingsSet();

	template<typename T>
	bool addProfile(const QString& profile, const SoftwareSettings& settings);

	template<typename T>
	std::shared_ptr<const T> getSettingsProfile(const QString& profile) const;

	template<typename T>
	std::shared_ptr<const T> getSettingsDefaultProfile() const;

	bool writeToXml(XmlWriteHelper& xml);

	bool readFromXml(const QByteArray& xmlData);
	bool readFromXml(XmlReadHelper& xml);

	void setSoftwareType(E::SoftwareType softwareType) { m_softwareType = softwareType; }
	E::SoftwareType softwareType() const { return m_softwareType; }

	QStringList getSettingsProfiles() const;

	static QString writeSettingsToXmlString(E::SoftwareType swType, const SoftwareSettings& settings);
	static bool readSettingsFromXmlString(const QString& xmlString, SoftwareSettings* settings);

private:
	std::shared_ptr<SoftwareSettings> createAppropriateSettings();
	bool addSharedProfile(const QString& profile, std::shared_ptr<SoftwareSettings> sharedSettings);

private:
	E::SoftwareType m_softwareType = E::SoftwareType::Unknown;
	std::map<QString, std::shared_ptr<SoftwareSettings>> m_settingsMap;	// profileName -> SoftwareSettings*
};

template<typename T>
bool SoftwareSettingsSet::addProfile(const QString& profile, const SoftwareSettings& settings)
{
	QString profileName = profile.isEmpty() == true ? SettingsProfile::DEFAULT : profile;

	const T* typedPtr = dynamic_cast<const T*>(&settings);

	if (typedPtr == nullptr)
	{
		Q_ASSERT(false);
		return false;
	}

	std::shared_ptr<T> sharedSettings = std::make_shared<T>(*typedPtr);

	sharedSettings->profile = profileName;

	return addSharedProfile(profileName, sharedSettings);
}

template<typename T>
std::shared_ptr<const T> SoftwareSettingsSet::getSettingsProfile(const QString& profile) const
{
	auto it = m_settingsMap.find(profile.isEmpty() == true ? SettingsProfile::DEFAULT : profile);

	if (it != m_settingsMap.end())
	{
		return std::dynamic_pointer_cast<const T>(it->second);
	}

	return nullptr;
}

template<typename T>
std::shared_ptr<const T> SoftwareSettingsSet::getSettingsDefaultProfile() const
{
	return getSettingsProfile<T>(SettingsProfile::DEFAULT);
}


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

private:
	// this methods should be call by SoftwareSettingsSet only
	//
	bool writeToXml(XmlWriteHelper& xml) const override;
	bool readFromXml(XmlReadHelper& xml) override;

	friend class SoftwareSettingsSet;

public:
	QStringList knownClients() const;
};

#ifdef IS_BUILDER

	class CfgServiceSettingsGetter : public CfgServiceSettings, public SoftwareSettingsGetter
	{
	public:
		bool readFromDevice(const Builder::Context* context,
							const Hardware::Software* software) override;

	private:
		bool buildClientsList(const Builder::Context* context, const Hardware::Software* software);
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

private:
	// this methods should be call by SoftwareSettingsSet only
	//
	bool writeToXml(XmlWriteHelper& xml) const override;
	bool readFromXml(XmlReadHelper& xml) override;

	friend class SoftwareSettingsSet;
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

private:
	// this methods should be call by SoftwareSettingsSet only
	//
	bool writeToXml(XmlWriteHelper& xml) const override;
	bool readFromXml(XmlReadHelper& xml) override;

	friend class SoftwareSettingsSet;
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

	HostAddressPort tuningSimIP;

	std::vector<TuningSource> sources;
	std::vector<TuningClient> clients;

private:
	// this methods should be call by SoftwareSettingsSet only
	//
	bool writeToXml(XmlWriteHelper& xml) const override;
	bool readFromXml(XmlReadHelper& xml) override;

	friend class SoftwareSettingsSet;
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

private:
	// this methods should be call by SoftwareSettingsSet only
	//
	bool writeToXml(XmlWriteHelper& xml) const override;
	bool readFromXml(XmlReadHelper& xml) override;

	friend class SoftwareSettingsSet;
};

#ifdef IS_BUILDER

	class ArchivingServiceSettingsGetter : public ArchivingServiceSettings, public SoftwareSettingsGetter
	{
	public:
		bool readFromDevice(const Builder::Context* context,
							const Hardware::Software* software) override;

	private:
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

private:
	// this methods should be call by SoftwareSettingsSet only
	//
	bool writeToXml(XmlWriteHelper& xml) const override;
	bool readFromXml(XmlReadHelper& xml) override;

	friend class SoftwareSettingsSet;
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

private:
	// this methods should be call by SoftwareSettingsSet only
	//
	bool writeToXml(XmlWriteHelper& xml) const override;
	bool readFromXml(XmlReadHelper& xml) override;

	friend class SoftwareSettingsSet;
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

private:
	// this methods should be call by SoftwareSettingsSet only
	//
	bool writeToXml(XmlWriteHelper& xml) const override;
	bool readFromXml(XmlReadHelper& xml) override;

	friend class SoftwareSettingsSet;

public:
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

	bool autoApply = true;

	bool showSignals = true;
	bool showSchemas = true;
	bool showSchemasList = true;
	bool showSchemasTabs = true;

	int statusFlagFunction = 0;	// LmStatusFlagMode::None

	bool loginPerOperation = false;
	QString usersAccounts;
	int loginSessionLength = 120;

	bool filterByEquipment = true;
	bool filterBySchema = true;

	QString startSchemaID;
	QString schemaTags;

private:
	// this methods should be call by SoftwareSettingsSet only
	//
	bool writeToXml(XmlWriteHelper& xml) const override;
	bool readFromXml(XmlReadHelper& xml) override;

	friend class SoftwareSettingsSet;

public:
	QStringList getSchemaTags() const;
	QStringList getUsersAccounts() const;

	const TuningClientSettings& operator = (const TuningClientSettings& src);

	bool appearanceChanged(const TuningClientSettings& src) const;
	bool connectionChanged(const TuningClientSettings& src) const;
};

#ifdef IS_BUILDER

	class TuningClientSettingsGetter : public TuningClientSettings, public SoftwareSettingsGetter
	{
	public:
		bool readFromDevice(const Builder::Context* context,
							const Hardware::Software* software) override;
	};

#endif

