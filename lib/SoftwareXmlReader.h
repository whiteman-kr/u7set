#pragma once

#include "Types.h"
#include "SoftwareSettings.h"

class SoftwareXmlInfo
{
public:
	QString caption;
	QString equipmentID;
	E::SoftwareType softwareType;

	const CfgServiceSettings* cfgServiceSettings() const;
	const AppDataServiceSettings* appDataServiceSettings() const;
	const DiagDataServiceSettings* diagDataServiceSettings() const;
	const ArchivingServiceSettings* archivingServiceSettings() const;
	const TuningServiceSettings* tuningServiceSettings() const;
	const TestClientSettings* testClientSettings() const;
	const MetrologySettings* metrologySettings() const;
	const MonitorSettings* monitorSettings() const;
	const TuningClientSettings* tuningClientSettings() const;

private:
	bool readFromXml(XmlReadHelper& xmlReader);

	friend class SoftwareXmlReader;

private:
	SoftwareSettings* m_settings = nullptr;
};

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

