#include "DiagDataServiceCfgGenerator.h"
#include "../lib/ServiceSettings.h"


namespace Builder
{
	DiagDataServiceCfgGenerator::DiagDataServiceCfgGenerator(Context* context, Hardware::Software* software) :
		SoftwareCfgGenerator(context, software)
	{
	}


	DiagDataServiceCfgGenerator::~DiagDataServiceCfgGenerator()
	{
	}


	bool DiagDataServiceCfgGenerator::generateConfiguration()
	{
		bool result = true;

		do
		{
			if (writeSettings() == false) break;

			result = true;
		}
		while(false);

		return result;
	}

	bool DiagDataServiceCfgGenerator::getSettingsXml(QXmlStreamWriter& xmlWriter)
	{
		if (m_settings.isInitialized() == false)
		{
			bool result = m_settings.readFromDevice(m_equipment, m_software, m_log);

			RETURN_IF_FALSE(result);
		}

		XmlWriteHelper xml(xmlWriter);

		return m_settings.writeToXml(xml);
	}

	bool DiagDataServiceCfgGenerator::writeSettings()
	{
		return getSettingsXml(m_cfgXml->xmlWriter());
	}
}
