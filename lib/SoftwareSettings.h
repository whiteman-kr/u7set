#pragma once

#include "ConstStrings.h"
#include "../UtilsLib/XmlHelper.h"
#include "../OnlineLib/SocketIO.h"
#include "../UtilsLib/WUtils.h"
#include "../Proto/network.pb.h"

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

	bool settingsProfileIsExists(const QString& profile);

	bool writeToXml(XmlWriteHelper& xml);

	bool readFromXml(const QByteArray& xmlData);
	bool readFromXml(XmlReadHelper& xml);

	void setSoftwareType(E::SoftwareType softwareType) { m_softwareType = softwareType; }
	E::SoftwareType softwareType() const { return m_softwareType; }

	QStringList getSettingsProfiles() const;

	static QString writeSettingsToXmlString(E::SoftwareType swType, const SoftwareSettings& settings);
	static bool readSettingsFromXmlString(const QString& xmlString, SoftwareSettings* settings);
	static std::shared_ptr<SoftwareSettings> createAppropriateSettings(E::SoftwareType softwareType);

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

		bool isValid() { return lmEquipmentID.isEmpty() == false; }
	};

	struct ChannelSettings
	{
		bool enable = false;

		QString serviceControllerEquipmentID;

		HostAddressPort clientRequestIP;
		QHostAddress clientRequestNetmask;

		HostAddressPort tuningDataIP;
		QHostAddress tuningDataNetmask;

		HostAddressPort tuningSimIP;

		std::vector<TuningSource> sources;
		std::vector<TuningClient> clients;

		TuningSource getTuningSource(const QString& sourceEquipmentID) const;
	};

	static const int CHANNELS_COUNT = 2;

	QString equipmentID;

	int channelCount = 0;

	QString cfgServiceID1;
	HostAddressPort cfgServiceIP1;

	QString cfgServiceID2;
	HostAddressPort cfgServiceIP2;

	bool singleLmControl = true;
	bool disableModulesTypeChecking = false;

	ChannelSettings channelSettings[CHANNELS_COUNT];

	std::vector<TuningClient> getAllUniqueClients() const;

private:
	// this methods should be call by SoftwareSettingsSet only
	//
	bool writeToXml(XmlWriteHelper& xml) const override;
	bool readFromXml(XmlReadHelper& xml) override;

	friend class SoftwareSettingsSet;
};

class ArchivingServiceSettings : public SoftwareSettings
{
public:
	QString cfgServiceID1;
	HostAddressPort cfgServiceIP1;

	QString cfgServiceID2;
	HostAddressPort cfgServiceIP2;

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

class MetrologySettings : public SoftwareSettings
{
public:
	QString cfgServiceID1;
	HostAddressPort cfgServiceIP1;

	QString cfgServiceID2;
	HostAddressPort cfgServiceIP2;

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


class MonitorSettings : public SoftwareSettings
{
public:
	QString cfgServiceID1;
	HostAddressPort cfgServiceIP1;

	QString cfgServiceID2;
	HostAddressPort cfgServiceIP2;

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

	bool tuningLogin = false;
	QString tuningUserAccounts;
	int tuningSessionTimeout = 0;

private:
	// this methods should be call by SoftwareSettingsSet only
	//
	bool writeToXml(XmlWriteHelper& xml) const override;
	bool readFromXml(XmlReadHelper& xml) override;

	friend class SoftwareSettingsSet;

public:
	QStringList getSchemaTags() const;
	QStringList getTuningSources() const;
	QStringList getUsersAccounts() const;

	void clear();
};

class TuningClientSettings : public SoftwareSettings
{
public:
	QString cfgServiceID1;
	HostAddressPort cfgServiceIP1;

	QString cfgServiceID2;
	HostAddressPort cfgServiceIP2;

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
	bool tuningLogin = false;
	QString tuningUserAccounts;
	int tuningSessionTimeout = 120;

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

