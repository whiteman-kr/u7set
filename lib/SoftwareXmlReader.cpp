#include "SoftwareXmlReader.h"
#include "../lib/WUtils.h"

// -----------------------------------------------------------------------------------------------
//
// Class SoftwareXmlInfo implementation
//
// -----------------------------------------------------------------------------------------------

bool SoftwareXmlInfo::readFromXml(XmlReadHelper& xmlReader)
{
	if (m_settings != nullptr)
	{
		delete m_settings;
		m_settings = nullptr;
	}

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
		// no settings defined for this software for now
		break;

	case E::SoftwareType::ConfigurationService:
		m_settings = new CfgServiceSettings;
		break;

	case E::SoftwareType::AppDataService:
		m_settings = new AppDataServiceSettings;
		break;

	case E::SoftwareType::DiagDataService:
		m_settings = new DiagDataServiceSettings;
		break;

	case E::SoftwareType::ArchiveService:
		m_settings = new ArchivingServiceSettings;
		break;

	case E::SoftwareType::TuningService:
		m_settings = new TuningServiceSettings;
		break;

	case E::SoftwareType::TestClient:
		m_settings = new TestClientSettings;
		break;

	case E::SoftwareType::Metrology:
		m_settings = new MetrologySettings;
		break;

	case E::SoftwareType::Monitor:
		m_settings = new MonitorSettings;
		break;

	case E::SoftwareType::TuningClient:
		m_settings = new TuningClientSettings;
		break;

	default:
		Q_ASSERT(false);
		result = false;
	}

	if (m_settings == nullptr)
	{
		return false;
	}

	result = m_settings->readFromXml(xmlReader);

	Q_ASSERT(result == true);

	return result;
}

const CfgServiceSettings* SoftwareXmlInfo::cfgServiceSettings() const
{
	Q_ASSERT(softwareType == E::SoftwareType::ConfigurationService);
	Q_ASSERT(m_settings != nullptr);

	return dynamic_cast<const CfgServiceSettings*>(m_settings);
}

const AppDataServiceSettings* SoftwareXmlInfo::appDataServiceSettings() const
{
	Q_ASSERT(softwareType == E::SoftwareType::AppDataService);
	Q_ASSERT(m_settings != nullptr);

	return dynamic_cast<const AppDataServiceSettings*>(m_settings);
}

const DiagDataServiceSettings* SoftwareXmlInfo::diagDataServiceSettings() const
{
	Q_ASSERT(softwareType == E::SoftwareType::DiagDataService);
	Q_ASSERT(m_settings != nullptr);

	return dynamic_cast<const DiagDataServiceSettings*>(m_settings);
}

const ArchivingServiceSettings* SoftwareXmlInfo::archivingServiceSettings() const
{
	Q_ASSERT(softwareType == E::SoftwareType::ArchiveService);
	Q_ASSERT(m_settings != nullptr);

	return dynamic_cast<const ArchivingServiceSettings*>(m_settings);
}

const TuningServiceSettings* SoftwareXmlInfo::tuningServiceSettings() const
{
	Q_ASSERT(softwareType == E::SoftwareType::TuningService);
	Q_ASSERT(m_settings != nullptr);

	return dynamic_cast<const TuningServiceSettings*>(m_settings);
}

const TestClientSettings* SoftwareXmlInfo::testClientSettings() const
{
	Q_ASSERT(softwareType == E::SoftwareType::TestClient);
	Q_ASSERT(m_settings != nullptr);

	return dynamic_cast<const TestClientSettings*>(m_settings);
}

const MetrologySettings* SoftwareXmlInfo::metrologySettings() const
{
	Q_ASSERT(softwareType == E::SoftwareType::Metrology);
	Q_ASSERT(m_settings != nullptr);

	return dynamic_cast<const MetrologySettings*>(m_settings);
}

const MonitorSettings* SoftwareXmlInfo::monitorSettings() const
{
	Q_ASSERT(softwareType == E::SoftwareType::Monitor);
	Q_ASSERT(m_settings != nullptr);

	return dynamic_cast<const MonitorSettings*>(m_settings);
}

const TuningClientSettings* SoftwareXmlInfo::tuningClientSettings() const
{
	Q_ASSERT(softwareType == E::SoftwareType::TuningClient);
	Q_ASSERT(m_settings != nullptr);

	return dynamic_cast<const TuningClientSettings*>(m_settings);
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

	XmlReadHelper xmlReader(fileData);

	if (xmlReader.findElement(XmlElement::SOFTWARE_ITEMS) == false)
	{
		return false;
	}

	bool result = true;

	while(xmlReader.findElement(XmlElement::SOFTWARE) == true)
	{
		SoftwareXmlInfo swXmlInfo;

		bool res = swXmlInfo.readFromXml(xmlReader);

		if (res == true)
		{
			m_softwareXmlInfo.insert(std::pair<QString, SoftwareXmlInfo>(swXmlInfo.equipmentID, swXmlInfo));
		}
		else
		{
			result = false;
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

