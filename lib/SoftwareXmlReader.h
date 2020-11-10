#pragma once

#include "Types.h"
#include "ServiceSettings.h"

class SoftwareXmlInfo
{
public:
	QString caption;
	QString equipmentID;
	E::SoftwareType softwareType;

	bool readFromXml(XmlReadHelper& xmlReader);

	const CfgServiceSettings& cfgServiceSettings() const;
	const AppDataServiceSettings& appDataServiceSettings() const;
	const DiagDataServiceSettings& diagDataServiceSettings() const;
	const ArchivingServiceSettings& archivingServiceSettings() const;
	const TuningServiceSettings& tuningServiceSettings() const;
	const TestClientSettings& testClientSettings() const;

private:
	CfgServiceSettings m_cfgServiceSettings;
	AppDataServiceSettings m_appDataServiceSettings;
	DiagDataServiceSettings m_diagDataServiceSettings;
	ArchivingServiceSettings m_archivingServiceSettings;
	TuningServiceSettings m_tuningServiceSettings;
	TestClientSettings m_testClientSettings;

	// Settings structures isn't defined for:
	//
	// Monitor
	// Metrology
	// TuningClient

};

class SoftwareXmlReader
{
public:
	bool readSoftwareXml(const QString& fileName);
	bool readSoftwareXml(const QByteArray& fileData);

	const SoftwareXmlInfo* getSoftwareXmlInfo(const QString& equipmentID);	// returns nullptr if software with equipmentID isn't found

private:
	std::map<QString, SoftwareXmlInfo> m_softwareXmlInfo;
};

