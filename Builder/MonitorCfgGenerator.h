#pragma once

#include "SoftwareCfgGenerator.h"
#include "../lib/SoftwareSettings.h"

namespace Builder
{
	class Context;

	class MonitorCfgGenerator : public SoftwareCfgGenerator
	{
	public:
		MonitorCfgGenerator(Context* context, Hardware::Software* software);
		~MonitorCfgGenerator();

		virtual bool createSettingsProfile(const QString& profile) override;
		virtual bool generateConfigurationStep1() override;

	protected:
		bool initSchemaTags();
		bool initTuningSources();

		bool saveScriptProperties(QString scriptProperty, QString fileName);

		bool writeSchemasByTags();

		// Generate tuning signals file
		//
		bool writeTuningSignals();
		bool writeMonitorBehavior();
		bool writeMonitorLogo();

	private:
		QStringList m_tuningSources;

		QStringList m_schemaTagList;		// Generated in writeMonitorSettings
	};
}

