#pragma once

#include "../CommonLib/Types.h"
#include "SoftwareSettings.h"

class SoftwareXmlInfo
{
public:
	QString caption;
	QString equipmentID;

	std::shared_ptr<const CfgServiceSettings> cfgServiceSettings(const QString& profile) const;
	std::shared_ptr<const AppDataServiceSettings> appDataServiceSettings(const QString& profile) const;
	std::shared_ptr<const DiagDataServiceSettings> diagDataServiceSettings(const QString& profile) const;
	std::shared_ptr<const ArchivingServiceSettings> archivingServiceSettings(const QString& profile) const;
	std::shared_ptr<const TuningServiceSettings> tuningServiceSettings(const QString& profile) const;
	std::shared_ptr<const TestClientSettings> testClientSettings(const QString& profile) const;
	std::shared_ptr<const MetrologySettings> metrologySettings(const QString& profile) const;
	std::shared_ptr<const MonitorSettings> monitorSettings(const QString& profile) const;
	std::shared_ptr<const TuningClientSettings> tuningClientSettings(const QString& profile) const;

	template<typename T>
	std::shared_ptr<const T> getSettingsProfile(const QString& profile) const;

	E::SoftwareType softwareType() const;

private:
	bool readFromXml(XmlReadHelper& xmlReader);

	friend class SoftwareXmlReader;

private:
	SoftwareSettingsSet m_settingsSet;
};

template<typename T>
std::shared_ptr<const T> SoftwareXmlInfo::getSettingsProfile(const QString& profile) const
{
	return 	m_settingsSet.getSettingsProfile<T>(profile);
}

class SoftwareXmlReader
{
public:
	bool readSoftwareXml(const QString& fileName);
	bool readSoftwareXml(const QByteArray& fileData);

	const SoftwareXmlInfo* getSoftwareXmlInfo(const QString& equipmentID);	// returns nullptr if software with equipmentID isn't found

	const std::map<QString, SoftwareXmlInfo>& softwareXmlInfo() { return m_softwareXmlInfo; }

private:
	std::map<QString, SoftwareXmlInfo> m_softwareXmlInfo;
};

