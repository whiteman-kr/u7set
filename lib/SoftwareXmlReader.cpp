#include "SoftwareXmlReader.h"
#include "../lib/WUtils.h"

// -----------------------------------------------------------------------------------------------
//
// Class SoftwareXmlInfo implementation
//
// -----------------------------------------------------------------------------------------------

bool SoftwareXmlInfo::readFromXml(XmlReadHelper& xmlReader)
{
	m_cfgServiceSettings.resetInitialized();
	m_appDataServiceSettings.resetInitialized();
	m_diagDataServiceSettings.resetInitialized();
	m_archivingServiceSettings.resetInitialized();
	m_tuningServiceSettings.resetInitialized();
	m_testClientSettings.resetInitialized();

	if (xmlReader.checkElement(XmlElement::SOFTWARE) == false)
	{
		Q_ASSERT(false);
		return false;
	}

	bool result = true;

	result &= xmlReader.readStringAttribute(XmlAttribute::CAPTION, &caption);
	result &= xmlReader.readStringAttribute(XmlAttribute::ID, &equipmentID);

	int typeInt = 0;

	result &= xmlReader.readIntAttribute(XmlAttribute::TYPE, &typeInt);

	RETURN_IF_FALSE(result);

	if (E::contains<E::SoftwareType>(typeInt) == false)
	{
		Q_ASSERT(false);		// unknown software type
		return false;
	}

	softwareType = static_cast<E::SoftwareType>(typeInt);

	switch(softwareType)
	{
	case E::SoftwareType::Unknown:
	case E::SoftwareType::BaseService:
	case E::SoftwareType::ServiceControlManager:
		break;											// no settings defined for this software

	case E::SoftwareType::ConfigurationService:
		result = m_cfgServiceSettings.readFromXml(xmlReader);
		break;

	case E::SoftwareType::AppDataService:
		result = m_appDataServiceSettings.readFromXml(xmlReader);
		break;

	case E::SoftwareType::DiagDataService:
		result = m_diagDataServiceSettings.readFromXml(xmlReader);
		break;

	case E::SoftwareType::ArchiveService:
		result = m_archivingServiceSettings.readFromXml(xmlReader);
		break;

	case E::SoftwareType::TuningService:
		result = m_tuningServiceSettings.readFromXml(xmlReader);
		break;

	case E::SoftwareType::TestClient:
		result = m_testClientSettings.readFromXml(xmlReader);
		break;

	case E::SoftwareType::Monitor:
	case E::SoftwareType::TuningClient:
	case E::SoftwareType::Metrology:
		Q_ASSERT(false);					// reading should be implemented
		result = false;
		break;

	default:
		Q_ASSERT(false);
		result = false;
	}

	return result;
}

const CfgServiceSettings& SoftwareXmlInfo::cfgServiceSettings() const
{
	Q_ASSERT(softwareType == E::SoftwareType::ConfigurationService);
	Q_ASSERT(m_cfgServiceSettings.isInitialized() == true);

	return m_cfgServiceSettings;
}

const AppDataServiceSettings& SoftwareXmlInfo::appDataServiceSettings() const
{
	Q_ASSERT(softwareType == E::SoftwareType::AppDataService);
	Q_ASSERT(m_appDataServiceSettings.isInitialized() == true);

	return m_appDataServiceSettings;
}

const DiagDataServiceSettings& SoftwareXmlInfo::diagDataServiceSettings() const
{
	Q_ASSERT(softwareType == E::SoftwareType::DiagDataService);
	Q_ASSERT(m_diagDataServiceSettings.isInitialized() == true);

	return m_diagDataServiceSettings;
}

const ArchivingServiceSettings& SoftwareXmlInfo::archivingServiceSettings() const
{
	Q_ASSERT(softwareType == E::SoftwareType::ArchiveService);
	Q_ASSERT(m_archivingServiceSettings.isInitialized() == true);

	return m_archivingServiceSettings;
}

const TuningServiceSettings& SoftwareXmlInfo::tuningServiceSettings() const
{
	Q_ASSERT(softwareType == E::SoftwareType::TuningService);
	Q_ASSERT(m_tuningServiceSettings.isInitialized() == true);

	return m_tuningServiceSettings;
}

const TestClientSettings& SoftwareXmlInfo::testClientSettings() const
{
	Q_ASSERT(softwareType == E::SoftwareType::TestClient);
	Q_ASSERT(m_testClientSettings.isInitialized() == true);

	return m_testClientSettings;
}

// -----------------------------------------------------------------------------------------------
//
// Class SoftwareXmlInfo implementation
//
// -----------------------------------------------------------------------------------------------

bool SoftwareXmlReader::readSoftwareXml(const QString& fileName)
{
	QFile file(fileName);

	if (file.open(QIODevice::ReadOnly) == false)
	{
		return false;
	}

	QByteArray fileData = file.readAll();

	file.close();

	if (fileData.size() == 0)
	{
		return false;
	}

	return readSoftwareXml(fileData);
}

bool SoftwareXmlReader::readSoftwareXml(const QByteArray& fileData)
{
	m_softwareXmlInfo.clear();

	bool result = true;

	XmlReadHelper xmlReader(fileData);

	if (xmlReader.findElement(XmlElement::SOFTWARE_ITEMS) == false)
	{
		return false;
	}

	while(xmlReader.findElement(XmlElement::SOFTWARE) == true)
	{
		SoftwareXmlInfo swXmlInfo;

		bool result = swXmlInfo.readFromXml(xmlReader);

		if (result == true)
		{
			m_softwareXmlInfo.insert(std::pair<QString, SoftwareXmlInfo>(swXmlInfo.equipmentID, swXmlInfo));
		}
	}

	return result;

}

// returns nullptr if software with equipmentID isn't found
//
const SoftwareXmlInfo* SoftwareXmlReader::getSoftwareXmlInfo(const QString& equipmentID)
{
	auto result = m_softwareXmlInfo.find(equipmentID);

	if (result == m_softwareXmlInfo.end())
	{
		return nullptr;
	}

	return &result->second;
}

