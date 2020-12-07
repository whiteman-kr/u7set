#include "DiagDataServiceCfgGenerator.h"
#include "../lib/SoftwareSettings.h"


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
		XmlWriteHelper xml(xmlWriter);

		return m_settings.writeToXml(xml);
	}

	bool DiagDataServiceCfgGenerator::writeSettings()
	{
		bool result = m_settings.readFromDevice(m_context, m_software);

		RETURN_IF_FALSE(result);

		return getSettingsXml(m_cfgXml->xmlWriter());
	}
}
