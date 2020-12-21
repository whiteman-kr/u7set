#pragma once

#include "SoftwareCfgGenerator.h"
#include "../lib/SoftwareSettings.h"
#include "../lib/WUtils.h"
#include "../lib/DeviceHelper.h"
#include "../lib/XmlHelper.h"
#include "../lib/DataSource.h"

namespace Builder
{
	class AppDataServiceCfgGenerator : public SoftwareCfgGenerator
	{
	public:
		AppDataServiceCfgGenerator(Context* context,
								   Hardware::Software* software);
		~AppDataServiceCfgGenerator();

		virtual bool createSettingsProfile(const QString& profile) override;
		virtual bool generateConfiguration() override;
		virtual bool getSettingsXml(QXmlStreamWriter& xmlWriter) override;

	private:
		bool getAssociatedLMs();

		bool writeSettings();
		bool writeAppDataSourcesXml();
		bool writeAppSignalsXml();
		bool addLinkToAppSignalsFile();

		bool writeBatFile();
		bool writeShFile();

		bool findAppDataSourceAssociatedSignals(DataSource& appDataSource);

	private:
		AppDataServiceSettingsGetter m_settingsGetter;

		//

		QStringList m_associatedLMs;
		QHash<QString, bool> m_associatedAppSignals;
	};
}
