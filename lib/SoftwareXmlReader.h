#pragma once

#include "Types.h"
#include "ServiceSettings.h"

class SoftwareInfoXml
{
public:
	QString caption;
	QString equipmentID;
	E::SoftwareType softwareType;

	const AppDataServiceSettings& appDataServiceSettings() const;
	const ArchivingServiceSettings& archivingServiceSettings() const;

private:
	AppDataServiceSettings m_appDataServiceSettings;
	ArchivingServiceSettings m_archivingServiceSettings;

};

class SoftwareXmlReader
{
public:
	SoftwareXmlReader();


};

