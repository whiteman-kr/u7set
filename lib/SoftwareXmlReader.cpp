#include "SoftwareXmlReader.h"
#include "../UtilsLib/WUtils.h"

// -----------------------------------------------------------------------------------------------
//
// Class SoftwareXmlInfo implementation
//
// -----------------------------------------------------------------------------------------------

std::shared_ptr<const CfgServiceSettings> SoftwareXmlInfo::cfgServiceSettings(const QString& profile) const
{
	return m_settingsSet.getSettingsProfile<CfgServiceSettings>(profile);
}

std::shared_ptr<const AppDataServiceSettings> SoftwareXmlInfo::appDataServiceSettings(const QString& profile) const
{
	return m_settingsSet.getSettingsProfile<AppDataServiceSettings>(profile);
}

std::shared_ptr<const DiagDataServiceSettings> SoftwareXmlInfo::diagDataServiceSettings(const QString& profile) const
{
	return m_settingsSet.getSettingsProfile<DiagDataServiceSettings>(profile);
}

std::shared_ptr<const ArchivingServiceSettings> SoftwareXmlInfo::archivingServiceSettings(const QString& profile) const
{
	return m_settingsSet.getSettingsProfile<ArchivingServiceSettings>(profile);
}

std::shared_ptr<const TuningServiceSettings> SoftwareXmlInfo::tuningServiceSettings(const QString& profile) const
{
	return m_settingsSet.getSettingsProfile<TuningServiceSettings>(profile);
}

std::shared_ptr<const TestClientSettings> SoftwareXmlInfo::testClientSettings(const QString& profile) const
{
	return m_settingsSet.getSettingsProfile<TestClientSettings>(profile);
}

std::shared_ptr<const MetrologySettings> SoftwareXmlInfo::metrologySettings(const QString& profile) const
{
	return m_settingsSet.getSettingsProfile<MetrologySettings>(profile);
}

std::shared_ptr<const MonitorSettings> SoftwareXmlInfo::monitorSettings(const QString& profile) const
{
	return m_settingsSet.getSettingsProfile<MonitorSettings>(profile);
}

std::shared_ptr<const TuningClientSettings> SoftwareXmlInfo::tuningClientSettings(const QString& profile) const
{
	return m_settingsSet.getSettingsProfile<TuningClientSettings>(profile);
}

E::SoftwareType SoftwareXmlInfo::softwareType() const
{
	return m_settingsSet.softwareType();
}

bool SoftwareXmlInfo::readFromXml(XmlReadHelper& xmlReader)
{
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

	m_settingsSet.setSoftwareType(static_cast<E::SoftwareType>(typeInt));

	result = m_settingsSet.readFromXml(xmlReader);

	Q_ASSERT(result == true);

	return result;
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

