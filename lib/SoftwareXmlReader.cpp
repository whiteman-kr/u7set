#include "SoftwareXmlReader.h"

const AppDataServiceSettings& SoftwareInfoXml::appDataServiceSettings() const
{
	Q_ASSERT(softwareType == E::SoftwareType::AppDataService);

	return m_appDataServiceSettings;
}

const ArchivingServiceSettings& SoftwareInfoXml::archivingServiceSettings() const
{
	Q_ASSERT(softwareType == E::SoftwareType::ArchiveService);

	return m_archivingServiceSettings;
}

SoftwareXmlReader::SoftwareXmlReader()
{

}
